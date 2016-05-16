// Copyright 2015 Markus Tzoe

// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at

// http://www.apache.org/licenses/LICENSE-2.0

// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "NodeAsyncCallback.h"

using namespace v8;

namespace cross {

Persistent<Function> AsyncCallbackObjectWrap::constructor;
AsyncCallbackObjectWrap::AsyncCallbackObjectWrap()
    : NodeAsyncCallback{}
{
}

AsyncCallbackObjectWrap::~AsyncCallbackObjectWrap()
{
}

void AsyncCallbackObjectWrap::Init(Local<Object> exports)
{
    Isolate* isolate = exports->GetIsolate();

    // Prepare constructor template
    Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);
    tpl->SetClassName(String::NewFromUtf8(isolate, "CrossCallback"));
    tpl->InstanceTemplate()->SetInternalFieldCount(1);

    // Prototype
    SETUP_CROSSCALLBACK_PROTOTYPE_METHODS(tpl);
    NODE_SET_PROTOTYPE_METHOD(tpl, "emit", Emit);
    NODE_SET_PROTOTYPE_METHOD(tpl, "self", Self);

    constructor.Reset(isolate, tpl->GetFunction());
    exports->Set(String::NewFromUtf8(isolate, "CrossCallback"), tpl->GetFunction());
}

void AsyncCallbackObjectWrap::New(const FunctionCallbackInfo<Value>& arguments)
{
    Isolate* isolate = arguments.GetIsolate();
    if (arguments.IsConstructCall()) {
        AsyncCallbackObjectWrap* n = new AsyncCallbackObjectWrap();
        n->Wrap(arguments.This());
        arguments.GetReturnValue().Set(arguments.This());
    } else {
        Local<Value> argv[] = { arguments[0] };
        Local<Function> cons = Local<Function>::New(isolate, constructor);
        arguments.GetReturnValue().Set(cons->NewInstance(1, argv));
    }
}

void AsyncCallbackObjectWrap::Self(const FunctionCallbackInfo<Value>& arguments)
{
    AsyncCallbackObjectWrap* n = ObjectWrap::Unwrap<AsyncCallbackObjectWrap>(arguments.Holder());
    arguments.GetReturnValue().Set(n->mStore);
}

void AsyncCallbackObjectWrap::Emit(const FunctionCallbackInfo<Value>& arguments)
{
    if (arguments.Length() < 2 || !arguments[0]->IsString())
        return;
    AsyncCallbackObjectWrap* n = ObjectWrap::Unwrap<AsyncCallbackObjectWrap>(arguments.Holder());
    std::string event = std::string(*String::Utf8Value(arguments[0]->ToString()));
    std::string data = std::string(*String::Utf8Value(arguments[1]->ToString()));
    n->emit<std::string>(event, data);
}

void AsyncCallbackObjectWrap::On(const FunctionCallbackInfo<Value>& arguments)
{
    Isolate* isolate = arguments.GetIsolate();
    if (arguments.Length() < 2 || !arguments[0]->IsString() || !arguments[1]->IsFunction())
        return;
    AsyncCallbackObjectWrap* n = ObjectWrap::Unwrap<AsyncCallbackObjectWrap>(arguments.Holder());
    auto store = Local<Object>::New(isolate, n->mStore);
    auto val = store->Get(arguments[0]);
    if (val->IsArray()) {
        Local<Array> array = Local<Array>::Cast(val);
        array->Set(array->Length(), arguments[1]);
    } else {
        Local<Array> array = Array::New(isolate);
        array->Set(0, arguments[1]);
        Local<Object>::New(isolate, n->mStore)->Set(arguments[0], array);
    }
}

void AsyncCallbackObjectWrap::Off(const FunctionCallbackInfo<Value>& arguments)
{
    Isolate* isolate = arguments.GetIsolate();
    if (arguments.Length() < 2 || !arguments[0]->IsString() || !arguments[1]->IsFunction())
        return;
    AsyncCallbackObjectWrap* n = ObjectWrap::Unwrap<AsyncCallbackObjectWrap>(arguments.Holder());
    auto store = Local<Object>::New(isolate, n->mStore);
    auto val = store->Get(arguments[0]);
    if (!val->IsArray())
        return;
    Local<Array> array = Local<Array>::Cast(val);
    uint32_t new_length = array->Length();
    for (uint32_t i = 0; i < new_length; ++i) {
        if (array->Get(i)->Equals(arguments[1])) {
            for (uint32_t j = i; j < new_length; ++j) {
                array->Set(j, array->Get(j + 1));
            }
            --new_length;
            array->Delete(new_length);
            array->Set(String::NewFromUtf8(isolate, "length"), Integer::New(isolate, new_length));
        }
    }
}

void AsyncCallbackObjectWrap::Clear(const FunctionCallbackInfo<Value>& arguments)
{
    Isolate* isolate = arguments.GetIsolate();
    AsyncCallbackObjectWrap* n = ObjectWrap::Unwrap<AsyncCallbackObjectWrap>(arguments.Holder());
    auto store = Local<Object>::New(isolate, n->mStore);
    if (arguments.Length() == 0) {
        n->mStore.Reset(isolate, Object::New(isolate));
        return;
    } else if (arguments[0]->IsString()) {
        auto val = store->Get(arguments[0]);
        if (!val->IsArray())
            return;
        store->Delete(arguments[0]);
    }
}

// ------------------------NodeAsyncCallback-----------------------------------

NodeAsyncCallback* NodeAsyncCallback::New(Isolate* isolate, const Local<Function>& f)
{
    return (new NodeAsyncCallback(isolate, f));
}

NodeAsyncCallback* NodeAsyncCallback::New(const Local<Function>& f)
{
    return New(Isolate::GetCurrent(), f);
}

void NodeAsyncCallback::operator()(const Data& data)
{
    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope(isolate);
    auto store = Local<Object>::New(isolate, mStore);
    if (store.IsEmpty())
        return;

    const unsigned argc = data.argument.size();
    auto argv = new Local<Value>[argc];
    auto ptr = &data.argument;
    unsigned i = 0;
    while (ptr) {
        switch (ptr->type) {
        case cross::Argument::NUMBER:
            argv[i] = Number::New(isolate, *reinterpret_cast<double*>(ptr->payload));
            break;
        case cross::Argument::INTEGER:
            argv[i] = Integer::New(isolate, *reinterpret_cast<int*>(ptr->payload));
            break;
        case cross::Argument::JSON:
            argv[i] = JSON::Parse(String::NewFromUtf8(isolate, reinterpret_cast<const char*>(ptr->payload)));
            break;
        case cross::Argument::STRING:
            argv[i] = String::NewFromUtf8(isolate, reinterpret_cast<const char*>(ptr->payload));
            break;
        default:
            argv[i] = Undefined(isolate);
            break;
        }
        ++i;
        ptr = ptr->next();
    }

    TryCatch try_catch;
    if (store->IsFunction()) {
        Local<Function>::Cast(store)->Call(isolate->GetCurrentContext()->Global(), argc, argv);
        if (try_catch.HasCaught()) {
            node::FatalException(isolate, try_catch);
        }
        mStore.Reset();
        delete[] argv;
        return;
    }

    auto val = store->Get(String::NewFromUtf8(isolate, data.event.c_str()));
    if (!val->IsArray()) {
        delete[] argv;
        return;
    }
    Local<Array> array = Local<Array>::Cast(val);
    for (uint32_t i = 0; i < array->Length(); ++i) {
        Local<Value> f = array->Get(i);
        if (f->IsFunction()) {
            Local<Function>::Cast(f)->Call(isolate->GetCurrentContext()->Global(), argc, argv);
            if (try_catch.HasCaught()) {
                node::FatalException(isolate, try_catch);
            }
        }
    }
    delete[] argv;
}

NodeAsyncCallback::NodeAsyncCallback()
    : UvAsyncCallback{ uv_default_loop() }
    , mStore{ Isolate::GetCurrent(), Object::New(Isolate::GetCurrent()) }
{
}

NodeAsyncCallback::NodeAsyncCallback(Isolate* isolate, const Local<Function>& f)
    : UvAsyncCallback{ uv_default_loop() }
    , mStore{ isolate, f }
{
}

NodeAsyncCallback::~NodeAsyncCallback()
{
    mStore.Reset();
};
}
