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

#ifndef uv_deque_h
#define uv_deque_h

#include "Argument.h"
#include <deque>
#include <mutex>
#include <uv.h>

namespace async {
namespace internal {
    class uv_deque {
    public:
        explicit uv_deque(uv_loop_t* = uv_default_loop());
        virtual ~uv_deque();
        virtual bool push_back(const std::string& event, const Argument& argument);
        virtual bool push_front(const std::string& event, const Argument& argument);
        virtual size_t size();

    protected:
        struct Data {
            std::string event;
            Argument argument;
        };
        virtual void process(const Data& data) = 0;

    private:
        uv_async_t* handle_;
        std::mutex mutex_;
        std::deque<Data> queue_;

        void process();
        static void closeCallback(uv_handle_t*);
        static void callback(uv_async_t*);
    };
} // namespace internal
} // namespace async

#endif // uv_deque_h
