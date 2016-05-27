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

#ifndef EventEmitterObjectWrap_h
#define EventEmitterObjectWrap_h

#include "NodeEventEmitter.h"

namespace async {

class EventEmitterObjectWrap : public node::ObjectWrap, public NodeEventEmitter {
public:
    static void Init(v8::Local<v8::Object> exports);
    static void Init(v8::Local<v8::Object> exports, v8::Local<v8::Object> module);
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
    explicit EventEmitterObjectWrap();
    virtual ~EventEmitterObjectWrap();

private:
    static void New(const v8::FunctionCallbackInfo<v8::Value>&);
    static void Self(const v8::FunctionCallbackInfo<v8::Value>&);
    static void Emit(const v8::FunctionCallbackInfo<v8::Value>&);
    static void On(const v8::FunctionCallbackInfo<v8::Value>&);
    static void Off(const v8::FunctionCallbackInfo<v8::Value>&);
    static void Clear(const v8::FunctionCallbackInfo<v8::Value>&);

    static v8::Persistent<v8::Function> constructor;
};
}

#endif // EventEmitterObjectWrap_h
