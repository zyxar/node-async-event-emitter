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

#include "uv_deque.h"

namespace async {
namespace internal {

    uv_deque::uv_deque(uv_loop_t* loop)
    {
        handle_ = reinterpret_cast<uv_async_t*>(malloc(sizeof(uv_async_t)));
        handle_->data = this;
        uv_async_init(loop, handle_, uv_deque::callback);
    }

    uv_deque::~uv_deque()
    {
        uv_close(reinterpret_cast<uv_handle_t*>(handle_), uv_deque::closeCallback);
    }

    size_t uv_deque::size()
    {
        return queue_.size();
    }

    // main or other thread

    bool uv_deque::push_back(const std::string& event, const Argument& argument)
    {
        if (uv_is_active(reinterpret_cast<uv_handle_t*>(handle_))) {
            {
                std::lock_guard<std::mutex> lock(mutex_);
                queue_.push_back(Data{ event, argument });
            }
            uv_async_send(handle_);
            return true;
        }
        return false;
    }

    // main or other thread

    bool uv_deque::push_front(const std::string& event, const Argument& argument)
    {
        if (uv_is_active(reinterpret_cast<uv_handle_t*>(handle_))) {
            {
                std::lock_guard<std::mutex> lock(mutex_);
                queue_.push_front(Data{ event, argument });
            }
            uv_async_send(handle_);
            return true;
        }
        return false;
    }

    void uv_deque::closeCallback(uv_handle_t* handle)
    {
        free(handle);
    }

    void uv_deque::callback(uv_async_t* handle)
    {
        reinterpret_cast<uv_deque*>(handle->data)->process();
    }

    // main thread

    void uv_deque::process()
    {
        while (!queue_.empty()) {
            {
                std::unique_lock<std::mutex> lock(mutex_);
                Data data = queue_.front();
                queue_.pop_front();
                lock.unlock();
                process(data);
            }
        }
    }

} // namespace internal
} // namespace async
