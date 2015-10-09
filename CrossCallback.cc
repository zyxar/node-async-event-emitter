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

#include "CrossCallback.h"

using namespace v8;

Persistent<Function> CrossCallbackWrap::constructor;
CrossCallbackWrap::CrossCallbackWrap()
    : NodeAsyncCallback{}
{
}

CrossCallbackWrap::~CrossCallbackWrap()
{
}

void CrossCallbackWrap::Init(Local<Object> exports)
{
    Isolate* isolate = Isolate::GetCurrent();

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

void CrossCallbackWrap::New(const FunctionCallbackInfo<Value>& args)
{
    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope(isolate);

    if (args.IsConstructCall()) {
        CrossCallbackWrap* n = new CrossCallbackWrap();
        n->Wrap(args.This());
        args.GetReturnValue().Set(args.This());
    }
    else {
        const int argc = 1;
        Local<Value> argv[argc] = { args[0] };
        Local<Function> cons = Local<Function>::New(isolate, constructor);
        args.GetReturnValue().Set(cons->NewInstance(argc, argv));
    }
}

void CrossCallbackWrap::Self(const FunctionCallbackInfo<Value>& args)
{
    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope(isolate);
    CrossCallbackWrap* n = ObjectWrap::Unwrap<CrossCallbackWrap>(args.Holder());
    args.GetReturnValue().Set(n->mStore);
}

void CrossCallbackWrap::Emit(const FunctionCallbackInfo<Value>& args)
{
    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope(isolate);
    if (args.Length() < 2 || !args[0]->IsString())
        return;
    CrossCallbackWrap* n = ObjectWrap::Unwrap<CrossCallbackWrap>(args.Holder());
    std::string event = std::string(*String::Utf8Value(args[0]->ToString()));
    std::string data = std::string(*String::Utf8Value(args[1]->ToString()));
    n->emit<std::string>(event, data);
}

void CrossCallbackWrap::On(const FunctionCallbackInfo<Value>& args)
{
    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope(isolate);
    if (args.Length() < 2 || !args[0]->IsString() || !args[1]->IsFunction())
        return;
    CrossCallbackWrap* n = ObjectWrap::Unwrap<CrossCallbackWrap>(args.Holder());
    auto store = Local<Object>::New(isolate, n->mStore);
    auto val = store->Get(args[0]);
    if (val->IsArray()) {
        Local<Array> array = Local<Array>::Cast(val);
        array->Set(array->Length(), args[1]);
    }
    else {
        Local<Array> array = Array::New(isolate);
        array->Set(0, args[1]);
        Local<Object>::New(isolate, n->mStore)->Set(args[0], array);
    }
}

void CrossCallbackWrap::Off(const FunctionCallbackInfo<Value>& args)
{
    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope(isolate);
    if (args.Length() < 2 || !args[0]->IsString() || !args[1]->IsFunction())
        return;
    CrossCallbackWrap* n = ObjectWrap::Unwrap<CrossCallbackWrap>(args.Holder());
    auto store = Local<Object>::New(isolate, n->mStore);
    auto val = store->Get(args[0]);
    if (!val->IsArray())
        return;
    Local<Array> array = Local<Array>::Cast(val);
    uint32_t new_length = array->Length();
    for (uint32_t i = 0; i < new_length; ++i) {
        if (array->Get(i)->Equals(args[1])) {
            for (uint32_t j = i; j < new_length; ++j) {
                array->Set(j, array->Get(j + 1));
            }
            --new_length;
            array->Delete(new_length);
            array->Set(String::NewFromUtf8(isolate, "length"), Integer::New(isolate, new_length));
        }
    }
}

void CrossCallbackWrap::Clear(const FunctionCallbackInfo<Value>& args)
{
    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope(isolate);
    CrossCallbackWrap* n = ObjectWrap::Unwrap<CrossCallbackWrap>(args.Holder());
    auto store = Local<Object>::New(isolate, n->mStore);
    if (args.Length() == 0) {
        n->mStore.Reset(isolate, Object::New(isolate));
        return;
    }
    else if (args[0]->IsString()) {
        auto val = store->Get(args[0]);
        if (!val->IsArray())
            return;
        store->Delete(args[0]);
    }
}

// ------------------------NodeAsyncCallback-----------------------------------

std::shared_ptr<NodeAsyncCallback> NodeAsyncCallback::New(Isolate* isolate, const Local<Function>& f)
{
    return std::shared_ptr<NodeAsyncCallback>(new NodeAsyncCallback(isolate, f));
}

std::shared_ptr<NodeAsyncCallback> NodeAsyncCallback::New(const Local<Function>& f)
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

    const unsigned argc = data.message.size();
    auto argv = new Local<Value>[argc];
    auto ptr = &data.message;
    unsigned i = 0;
    while (ptr) {
        switch (ptr->type) {
        case AsyncCallback::Message::NUMBER:
            argv[i] = Number::New(isolate, *reinterpret_cast<double*>(ptr->payload));
            break;
        case AsyncCallback::Message::INTEGER:
            argv[i] = Integer::New(isolate, *reinterpret_cast<int*>(ptr->payload));
            break;
        case AsyncCallback::Message::JSON:
            argv[i] = JSON::Parse(String::NewFromUtf8(isolate, reinterpret_cast<const char*>(ptr->payload)));
            break;
        case AsyncCallback::Message::STRING:
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

// ------------------------UvAsyncCallback-------------------------------------

UvAsyncCallback::UvAsyncCallback()
{
    mUvHandle = reinterpret_cast<uv_async_t*>(malloc(sizeof(uv_async_t)));
    mUvHandle->data = this;
    uv_async_init(uv_default_loop(), mUvHandle, UvAsyncCallback::callback);
}

UvAsyncCallback::UvAsyncCallback(uv_loop_t* loop)
{
    mUvHandle = reinterpret_cast<uv_async_t*>(malloc(sizeof(uv_async_t)));
    mUvHandle->data = this;
    uv_async_init(loop, mUvHandle, UvAsyncCallback::callback);
}

UvAsyncCallback::~UvAsyncCallback()
{
    uv_close(reinterpret_cast<uv_handle_t*>(mUvHandle), UvAsyncCallback::closeCallback);
}

// main thread
void UvAsyncCallback::process()
{
    Data data;
    while (!mBuffer.empty()) {
        {
            std::lock_guard<std::mutex> lock(mLock);
            data = mBuffer.front();
            mBuffer.pop();
        }
        (*this)(data);
    }
}

size_t UvAsyncCallback::size()
{
    return mBuffer.size();
}

// other thread
bool UvAsyncCallback::notify(const std::string& event, const Message& message)
{
    if (uv_is_active(reinterpret_cast<uv_handle_t*>(mUvHandle))) {
        {
            std::lock_guard<std::mutex> lock(mLock);
            mBuffer.push(Data{ event, message });
        }
        uv_async_send(mUvHandle);
        return true;
    }
    return false;
}

void UvAsyncCallback::closeCallback(uv_handle_t* handle)
{
    free(handle);
}

void UvAsyncCallback::callback(uv_async_t* handle)
{
    reinterpret_cast<UvAsyncCallback*>(handle->data)->process();
}
