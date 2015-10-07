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
        virtual ~Argument();
        Argument();
        Argument(const std::string& rhs, DataType t = STRING);
        Argument(double rhs);
        Argument(int rhs);
        Argument(const Argument& rhs);
        Argument& operator=(const Argument& rhs);

    private:
        std::atomic<uint32_t>* refcnt;
    };
    virtual bool notify(const std::string& event, const Argument&) = 0; // event
    virtual bool operator()(const Argument&) = 0; // callback
};

} // namespace cross

#endif // AsyncCallback_h
