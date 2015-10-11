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

#include <atomic>
#include <string>

namespace cross {

class AsyncCallback {
public:
    AsyncCallback();
    virtual ~AsyncCallback();
    struct Message {
        enum Type {
            UNDEFINED,
            STRING,
            JSON,
            NUMBER,
            INTEGER,
        };
        Type type;
        uintptr_t payload;
        virtual ~Message();
        Message();
        Message(const char* rhs, Type t = STRING);
        Message(const std::string& rhs, Type t = STRING);
        Message(double rhs);
        Message(int rhs);
        Message(const Message& rhs);
        Message& operator=(const Message& rhs);
        const Message* next() const;
        void next(Message* p) { nextptr = p; }
        size_t size() const;

    private:
        std::atomic<uint32_t>* refcnt;
        Message* nextptr;
    };

    virtual bool notify(const std::string& event, const Message&) = 0; // event
    virtual bool call(const Message& message) { return notify("", message); } // callback

    template <typename... Arguments>
    bool emit(const std::string& event, const Arguments&... args)
    {
        const unsigned size = sizeof...(Arguments);
        Message m[size] = { args... };
        unsigned i = 1;
        Message* ptr = &m[0];
        while (i < size) {
            auto p = new Message{ m[i] };
            ptr->next(p);
            ptr = p;
            ++i;
        }
        return notify(event, m[0]);
    }

    template <typename... Arguments>
    bool operator()(const Arguments&... args)
    {
        const unsigned size = sizeof...(Arguments);
        Message m[size] = { args... };
        unsigned i = 1;
        Message* ptr = &m[0];
        while (i < size) {
            auto p = new Message{ m[i] };
            ptr->next(p);
            ptr = p;
            ++i;
        }
        return this->call(m[0]);
    }
};

} // namespace cross

#endif // AsyncCallback_h
