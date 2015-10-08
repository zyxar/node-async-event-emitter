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

#include "AsyncCallback.h"

namespace cross {

AsyncCallback::AsyncCallback() {}
AsyncCallback::~AsyncCallback() {}

AsyncCallback::Argument::~Argument()
{
    --(*refcnt);
    if ((*refcnt) == 0) {
        delete refcnt;
        refcnt = nullptr;
        switch (type) {
        case INTEGER:
            delete reinterpret_cast<int*>(payload);
            break;
        case NUMBER:
            delete reinterpret_cast<double*>(payload);
            break;
        case STRING:
        case JSON:
            delete[] reinterpret_cast<const char*>(payload);
            break;
        case UNDEFINED:
        default:
            return;
        }
    }
    if (nextptr)
        delete nextptr;
}

AsyncCallback::Argument::Argument()
    : type{ UNDEFINED }
    , payload{ 0 }
    , refcnt{ new std::atomic<uint32_t>(1) }
    , nextptr{ nullptr }
{
}

static const char* createString(const std::string& s)
{
    size_t len = s.size() + 1;
    char* r = new char[len];
    memcpy(r, s.c_str(), len - 1);
    r[len - 1] = 0;
    return r;
}

AsyncCallback::Argument::Argument(const std::string& rhs, DataType t)
    : type{ t == JSON ? JSON : STRING }
    , payload{ reinterpret_cast<uintptr_t>(createString(rhs)) }
    , refcnt{ new std::atomic<uint32_t>(1) }
    , nextptr{ nullptr }
{
}

AsyncCallback::Argument::Argument(double rhs)
    : type{ NUMBER }
    , payload{ reinterpret_cast<uintptr_t>(new double{ rhs }) }
    , refcnt{ new std::atomic<uint32_t>(1) }
    , nextptr{ nullptr }
{
}

AsyncCallback::Argument::Argument(int rhs)
    : type{ INTEGER }
    , payload{ reinterpret_cast<uintptr_t>(new int{ rhs }) }
    , refcnt{ new std::atomic<uint32_t>(1) }
    , nextptr{ nullptr }
{
}

AsyncCallback::Argument::Argument(const Argument& rhs)
    : type{ rhs.type }
    , payload{ rhs.payload }
    , refcnt{ rhs.refcnt }
    , nextptr{ rhs.nextptr }
{
    if (this == &rhs)
        return;
    if (!refcnt)
        refcnt = new std::atomic<uint32_t>(1);
    else
        ++(*refcnt);
}

AsyncCallback::Argument& AsyncCallback::Argument::operator=(const Argument& rhs)
{
    if (this == &rhs)
        return *this;
    --(*refcnt);
    type = rhs.type;
    payload = rhs.payload;
    refcnt = rhs.refcnt;
    nextptr = rhs.nextptr;
    if (!refcnt)
        refcnt = new std::atomic<uint32_t>(1);
    else
        ++(*refcnt);
    return *this;
}

const AsyncCallback::Argument* AsyncCallback::Argument::next() const
{
    return nextptr;
}

size_t AsyncCallback::Argument::size() const
{
    if (!nextptr)
        return 1;
    return 1 + nextptr->size();
}

} // namespace cross
