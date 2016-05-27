// Copyright 2016 Markus Tzoe

// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at

// http://www.apache.org/licenses/LICENSE-2.0

// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "NodeEventEmitter.h"

using namespace v8;

namespace async {

Persistent<Function> EventEmitterObjectWrap::constructor;
EventEmitterObjectWrap::EventEmitterObjectWrap()
    : NodeEventEmitter{}
{
}

EventEmitterObjectWrap::~EventEmitterObjectWrap() {}

void EventEmitterObjectWrap::Init(Local<Object> exports)
{
    auto isolate = exports->GetIsolate();

    // Prepare constructor template
    Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);
    tpl->SetClassName(String::NewFromUtf8(isolate, "EventEmitter"));
    tpl->InstanceTemplate()->SetInternalFieldCount(1);

    // Prototype
    SETUP_CROSSCALLBACK_PROTOTYPE_METHODS(tpl);
    NODE_SET_PROTOTYPE_METHOD(tpl, "emit", Emit);
    NODE_SET_PROTOTYPE_METHOD(tpl, "self", Self);

    constructor.Reset(isolate, tpl->GetFunction());
    exports->Set(String::NewFromUtf8(isolate, "EventEmitter"), tpl->GetFunction());
}

void EventEmitterObjectWrap::Init(Local<Object> exports, Local<Object> module)
{
    auto isolate = exports->GetIsolate();

    // Prepare constructor template
    Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);
    tpl->SetClassName(String::NewFromUtf8(isolate, "EventEmitter"));
    tpl->InstanceTemplate()->SetInternalFieldCount(1);

    // Prototype
    SETUP_CROSSCALLBACK_PROTOTYPE_METHODS(tpl);
    NODE_SET_PROTOTYPE_METHOD(tpl, "emit", Emit);
    NODE_SET_PROTOTYPE_METHOD(tpl, "self", Self);

    constructor.Reset(isolate, tpl->GetFunction());
    module->Set(String::NewFromUtf8(isolate, "exports"), tpl->GetFunction());
}

void EventEmitterObjectWrap::New(const FunctionCallbackInfo<Value>& arguments)
{
    auto isolate = arguments.GetIsolate();
    if (arguments.IsConstructCall()) {
        auto n = new EventEmitterObjectWrap();
        n->Wrap(arguments.This());
        arguments.GetReturnValue().Set(arguments.This());
    } else {
        Local<Value> argv[] = { arguments[0] };
        Local<Function> cons = Local<Function>::New(isolate, constructor);
        arguments.GetReturnValue().Set(cons->NewInstance(1, argv));
    }
}

void EventEmitterObjectWrap::Self(const FunctionCallbackInfo<Value>& arguments)
{
    auto n = ObjectWrap::Unwrap<EventEmitterObjectWrap>(arguments.Holder());
    arguments.GetReturnValue().Set(n->mStore);
}

void EventEmitterObjectWrap::Emit(const FunctionCallbackInfo<Value>& arguments)
{
    if (arguments.Length() < 2 || !arguments[0]->IsString())
        return;
    auto n = ObjectWrap::Unwrap<EventEmitterObjectWrap>(arguments.Holder());
    auto event = std::string(*String::Utf8Value(arguments[0]->ToString()));
    auto data = async::Argument{ 0 };
    auto ptr = &data;
    async::Argument* p = nullptr;
    for (int i = 1; i < arguments.Length(); ++i) {
        if (arguments[i]->IsInt32() || arguments[i]->IsUint32())
            p = new async::Argument{ int(arguments[i]->IntegerValue()) };
        else if (arguments[i]->IsNumber())
            p = new async::Argument{ arguments[i]->NumberValue() };
        else if (arguments[i]->IsBoolean())
            p = new async::Argument{ *arguments[i]->ToBoolean() };
        else if (arguments[i]->IsString())
            p = new async::Argument{ std::string(*String::Utf8Value(arguments[i]->ToString())) };
        else
            continue;
        ptr->next(p);
        ptr = p;
    }
    n->notify(event, *data.next());
}

void EventEmitterObjectWrap::On(const FunctionCallbackInfo<Value>& arguments)
{
    auto isolate = arguments.GetIsolate();
    if (arguments.Length() < 2 || !arguments[0]->IsString() || !arguments[1]->IsFunction())
        return;
    auto n = ObjectWrap::Unwrap<EventEmitterObjectWrap>(arguments.Holder());
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

void EventEmitterObjectWrap::Off(const FunctionCallbackInfo<Value>& arguments)
{
    auto isolate = arguments.GetIsolate();
    if (arguments.Length() < 2 || !arguments[0]->IsString() || !arguments[1]->IsFunction())
        return;
    auto n = ObjectWrap::Unwrap<EventEmitterObjectWrap>(arguments.Holder());
    auto store = Local<Object>::New(isolate, n->mStore);
    auto val = store->Get(arguments[0]);
    if (!val->IsArray())
        return;
    auto array = Local<Array>::Cast(val);
    auto new_length = array->Length();
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

void EventEmitterObjectWrap::Clear(const FunctionCallbackInfo<Value>& arguments)
{
    auto isolate = arguments.GetIsolate();
    auto n = ObjectWrap::Unwrap<EventEmitterObjectWrap>(arguments.Holder());
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

// ------------------------NodeEventEmitter-----------------------------------

NodeEventEmitter* NodeEventEmitter::New(Isolate* isolate, const Local<Function>& f)
{
    return (new NodeEventEmitter(isolate, f));
}

NodeEventEmitter* NodeEventEmitter::New(const Local<Function>& f)
{
    return New(Isolate::GetCurrent(), f);
}

bool NodeEventEmitter::notify(const std::string& event, const Argument& argument)
{
    return push_back(event, argument);
}

bool NodeEventEmitter::prompt(const std::string& event, const Argument& argument)
{
    return push_front(event, argument);
}

void NodeEventEmitter::process(const Data& data)
{
    auto isolate = Isolate::GetCurrent();
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
        case async::Argument::NUMBER:
            argv[i] = Number::New(isolate, ptr->value<double>());
            break;
        case async::Argument::INTEGER:
            argv[i] = Integer::New(isolate, ptr->value<int>());
            break;
        case async::Argument::BOOLEAN:
            argv[i] = Boolean::New(isolate, ptr->value<bool>());
            break;
        case async::Argument::STRING:
            argv[i] = String::NewFromUtf8(isolate, ptr->value<const char*>());
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
    auto array = Local<Array>::Cast(val);
    for (uint32_t i = 0; i < array->Length(); ++i) {
        auto f = array->Get(i);
        if (f->IsFunction()) {
            Local<Function>::Cast(f)->Call(isolate->GetCurrentContext()->Global(), argc, argv);
            if (try_catch.HasCaught()) {
                node::FatalException(isolate, try_catch);
            }
        }
    }
    delete[] argv;
}

NodeEventEmitter::NodeEventEmitter()
    : uv_deque{}
    , mStore{ Isolate::GetCurrent(), Object::New(Isolate::GetCurrent()) }
{
}

NodeEventEmitter::NodeEventEmitter(Isolate* isolate, const Local<Function>& f)
    : uv_deque{}
    , mStore{ isolate, f }
{
}

NodeEventEmitter::~NodeEventEmitter() {}

} // namespace async
