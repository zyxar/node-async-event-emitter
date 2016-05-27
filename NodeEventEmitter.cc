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
