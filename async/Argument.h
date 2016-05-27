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

#ifndef Argument_h
#define Argument_h

#include <atomic>
#include <cstddef>
#include <string>

namespace async {

struct Argument {
    enum Type {
        UNDEFINED,
        BOOLEAN,
        NUMBER,
        INTEGER,
        STRING,
    };
    Type type;
    uintptr_t payload;
    virtual ~Argument();
    Argument() = delete;
    Argument(std::nullptr_t);
    Argument(const char* rhs);
    Argument(const std::string& rhs);
    Argument(bool rhs);
    Argument(double rhs);
    Argument(int64_t rhs);
    Argument(uint64_t rhs);
    Argument(int32_t rhs);
    Argument(uint32_t rhs);

    Argument(const Argument& rhs);
    Argument& operator=(const Argument& rhs);
    const Argument* next() const;
    void next(Argument* p) { nextptr = p; }
    size_t size() const;

    template <typename T>
    T value() const;

private:
    std::atomic<uint32_t>* refcnt;
    Argument* nextptr;
};

} // namespace async

#endif // Argument_h
