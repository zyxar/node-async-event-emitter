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

#ifndef NodeEventEmitter_h
#define NodeEventEmitter_h

#include "EventEmitter.h"
#include "internal/uv_deque.h"
#include <node.h>
#include <node_object_wrap.h>

namespace async {

class NodeEventEmitter : public ::async::EventEmitter, public ::async::internal::uv_deque<::async::Argument> {
public:
    static NodeEventEmitter* New(v8::Isolate*, const v8::Local<v8::Function>&);
    static NodeEventEmitter* New(const v8::Local<v8::Function>&);
    virtual ~NodeEventEmitter();

protected:
    explicit NodeEventEmitter();
    explicit NodeEventEmitter(v8::Isolate*, const v8::Local<v8::Function>&);

    // ::async::internal::uv_deque<Argument>
    void process(const Data<::async::Argument>& data);
    // ::async::EventEmitter
    virtual bool notify(const std::string& event, const Argument&);
    virtual bool prompt(const std::string& event, const Argument&);
    // storage
    v8::Persistent<v8::Object> mStore;
};

} // namespace async

#endif // NodeEventEmitter_h
