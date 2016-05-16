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

#include "Argument.h"
#include <string.h>

namespace cross {

Argument::~Argument()
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
        if (nextptr)
            delete nextptr;
    }
}

Argument::Argument(const char* rhs, Type t)
    : type{ t == JSON ? JSON : STRING }
    , payload{ reinterpret_cast<uintptr_t>(strdup(rhs)) }
    , refcnt{ new std::atomic<uint32_t>(1) }
    , nextptr{ nullptr }
{
}

Argument::Argument(const std::string& rhs, Type t)
    : type{ t == JSON ? JSON : STRING }
    , payload{ reinterpret_cast<uintptr_t>(strdup(rhs.c_str())) }
    , refcnt{ new std::atomic<uint32_t>(1) }
    , nextptr{ nullptr }
{
}

Argument::Argument(double rhs)
    : type{ NUMBER }
    , payload{ reinterpret_cast<uintptr_t>(new double{ rhs }) }
    , refcnt{ new std::atomic<uint32_t>(1) }
    , nextptr{ nullptr }
{
}

Argument::Argument(int rhs)
    : type{ INTEGER }
    , payload{ reinterpret_cast<uintptr_t>(new int{ rhs }) }
    , refcnt{ new std::atomic<uint32_t>(1) }
    , nextptr{ nullptr }
{
}

Argument::Argument(const Argument& rhs)
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

Argument& Argument::operator=(const Argument& rhs)
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

const Argument* Argument::next() const
{
    return nextptr;
}

size_t Argument::size() const
{
    if (!nextptr)
        return 1;
    return 1 + nextptr->size();
}

} // namespace cross
