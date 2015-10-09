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
        Message(const std::string& rhs, Type t = STRING);
        Message(double rhs);
        Message(int rhs);
        Message(const Message& rhs);
        Message& operator=(const Message& rhs);
        const Message* next() const;
        size_t size() const;

    private:
        std::atomic<uint32_t>* refcnt;
        Message* nextptr;
    };
    virtual bool notify(const std::string& event, const Message&) = 0; // event
    virtual bool operator()(const Message&) = 0; // callback
    template <typename... Arguments>
    bool call(const std::string& event, Arguments... args)
    {
        Message arg{ args... };
        return notify(event, arg);
    };

private:
};

} // namespace cross

#endif // AsyncCallback_h
