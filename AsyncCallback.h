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
    AsyncCallback(){};
    virtual ~AsyncCallback(){};
    enum DataType {
        UNDEFINED,
        STRING,
        JSON,
        NUMBER,
        INTEGER,
    };
    struct Argument {
        DataType type;
        uintptr_t payload;
        virtual ~Argument()
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
                    delete reinterpret_cast<std::string*>(payload);
                    break;
                case UNDEFINED:
                default:
                    return;
                }
            }
        }

        Argument()
            : type{ UNDEFINED }
            , payload{ 0 }
            , refcnt{ new std::atomic<uint32_t>(1) }
        {
        }
        Argument(const std::string& rhs, DataType t = STRING)
            : type{ t == JSON ? JSON : STRING }
            , payload{ reinterpret_cast<uintptr_t>(new std::string{ rhs }) }
            , refcnt{ new std::atomic<uint32_t>(1) }
        {
        }
        Argument(double rhs)
            : type{ NUMBER }
            , payload{ reinterpret_cast<uintptr_t>(new double{ rhs }) }
            , refcnt{ new std::atomic<uint32_t>(1) }
        {
        }
        Argument(int rhs)
            : type{ INTEGER }
            , payload{ reinterpret_cast<uintptr_t>(new int{ rhs }) }
            , refcnt{ new std::atomic<uint32_t>(1) }
        {
        }
        Argument(const Argument& rhs)
            : type{ rhs.type }
            , payload{ rhs.payload }
            , refcnt{ rhs.refcnt }
        {
            if (this == &rhs)
                return;
            if (!refcnt)
                refcnt = new std::atomic<uint32_t>(1);
            else
                ++(*refcnt);
        }
        Argument& operator=(const Argument& rhs)
        {
            if (this == &rhs)
                return *this;
            --(*refcnt);
            type = rhs.type;
            payload = rhs.payload;
            refcnt = rhs.refcnt;
            if (!refcnt)
                refcnt = new std::atomic<uint32_t>(1);
            else
                ++(*refcnt);
            return *this;
        }

    private:
        std::atomic<uint32_t>* refcnt;
    };
    virtual bool notify(const std::string& event, const Argument&) = 0; // event
    virtual bool operator()(const Argument&) = 0; // callback
};

} // namespace cross

#endif // AsyncCallback_h
