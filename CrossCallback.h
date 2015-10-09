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

#ifndef CrossCallback_h
#define CrossCallback_h

#include "AsyncCallback.h"

#include <memory>
#include <mutex>
#include <node.h>
#include <node_object_wrap.h>
#include <queue>
#include <uv.h>

class UvAsyncCallback : public cross::AsyncCallback {
public:
    explicit UvAsyncCallback();
    explicit UvAsyncCallback(uv_loop_t*);
    virtual ~UvAsyncCallback();
    virtual bool notify(const std::string& event, const Message& message);
    virtual bool operator()(const Message& message) { return notify("", message); }
    virtual size_t size();

protected:
    struct Data {
        std::string event;
        Message message;
    };
    virtual void operator()(const Data& data) = 0;

private:
    uv_async_t* mUvHandle;
    std::mutex mLock;
    std::queue<Data> mBuffer;

    void process();
    static void closeCallback(uv_handle_t*);
    static void callback(uv_async_t*);
};

class NodeAsyncCallback : public UvAsyncCallback {
public:
    static std::shared_ptr<NodeAsyncCallback> New(v8::Isolate*, const v8::Local<v8::Function>&);
    static std::shared_ptr<NodeAsyncCallback> New(const v8::Local<v8::Function>&);
    virtual ~NodeAsyncCallback();

protected:
    explicit NodeAsyncCallback();
    explicit NodeAsyncCallback(v8::Isolate*, const v8::Local<v8::Function>&);
    void operator()(const Data& data);
    v8::Persistent<v8::Object> mStore;
};

class CrossCallbackWrap : public node::ObjectWrap, public NodeAsyncCallback {
public:
    static void Init(v8::Local<v8::Object> exports);
    inline static void SETUP_CROSSCALLBACK_PROTOTYPE_METHODS(v8::Local<v8::FunctionTemplate> tmpl)
    {
        NODE_SET_PROTOTYPE_METHOD(tmpl, "on", On);
        NODE_SET_PROTOTYPE_METHOD(tmpl, "addEventListener", On);
        NODE_SET_PROTOTYPE_METHOD(tmpl, "off", Off);
        NODE_SET_PROTOTYPE_METHOD(tmpl, "removeEventListener", Off);
        NODE_SET_PROTOTYPE_METHOD(tmpl, "removeAllEventListeners", Clear);
        NODE_SET_PROTOTYPE_METHOD(tmpl, "clearEventListener", Clear);
    }

protected:
    explicit CrossCallbackWrap();
    virtual ~CrossCallbackWrap();

private:
    static void New(const v8::FunctionCallbackInfo<v8::Value>& args);
    static void Self(const v8::FunctionCallbackInfo<v8::Value>& args);
    static void Emit(const v8::FunctionCallbackInfo<v8::Value>& args);
    static void On(const v8::FunctionCallbackInfo<v8::Value>& args);
    static void Off(const v8::FunctionCallbackInfo<v8::Value>& args);
    static void Clear(const v8::FunctionCallbackInfo<v8::Value>& args);

    static v8::Persistent<v8::Function> constructor;
};

#endif // CrossCallback_h
