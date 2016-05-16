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

#ifndef AsyncCallback_h
#define AsyncCallback_h

#include "Argument.h"
#include <atomic>
#include <string>

namespace cross {

class AsyncCallback {
public:
    AsyncCallback(){};
    virtual ~AsyncCallback(){};

    virtual bool notify(const std::string& event, const Argument&) = 0; // event
    virtual bool call(const Argument& argument) { return notify("", argument); } // callback

    template <typename... ArgType>
    bool emit(const std::string& event, const ArgType&... args)
    {
        const unsigned size = sizeof...(ArgType);
        Argument m[size] = { args... };
        unsigned i = 1;
        Argument* ptr = &m[0];
        while (i < size) {
            auto p = new Argument{ m[i] };
            ptr->next(p);
            ptr = p;
            ++i;
        }
        return notify(event, m[0]);
    }

    template <typename... ArgType>
    bool operator()(const ArgType&... args)
    {
        const unsigned size = sizeof...(ArgType);
        Argument m[size] = { args... };
        unsigned i = 1;
        Argument* ptr = &m[0];
        while (i < size) {
            auto p = new Argument{ m[i] };
            ptr->next(p);
            ptr = p;
            ++i;
        }
        return this->call(m[0]);
    }
};

} // namespace cross

#endif // AsyncCallback_h
