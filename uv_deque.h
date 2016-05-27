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

#include <deque>
#include <mutex>
#include <string>
#include <uv.h>

namespace async {
namespace internal {
    template <class T>
    class uv_deque {
    public:
        explicit uv_deque(uv_loop_t* loop = uv_default_loop())
        {
            handle_ = reinterpret_cast<uv_async_t*>(malloc(sizeof(uv_async_t)));
            handle_->data = this;
            uv_async_init(loop, handle_, uv_deque<T>::callback);
        }

        virtual ~uv_deque()
        {
            uv_close(reinterpret_cast<uv_handle_t*>(handle_), uv_deque<T>::closeCallback);
        }

        virtual bool push_back(const std::string& event, const T& argument)
        {
            if (uv_is_active(reinterpret_cast<uv_handle_t*>(handle_))) {
                {
                    std::lock_guard<std::mutex> lock(mutex_);
                    queue_.push_back(Data<T>{ event, argument });
                }
                uv_async_send(handle_);
                return true;
            }
            return false;
        }

        virtual bool push_front(const std::string& event, const T& argument)
        {
            if (uv_is_active(reinterpret_cast<uv_handle_t*>(handle_))) {
                {
                    std::lock_guard<std::mutex> lock(mutex_);
                    queue_.push_front(Data<T>{ event, argument });
                }
                uv_async_send(handle_);
                return true;
            }
            return false;
        }

        virtual size_t size()
        {
            return queue_.size();
        }

    protected:
        template <class D>
        struct Data {
            std::string event;
            D argument;
        };
        virtual void process(const Data<T>& data) = 0;

    private:
        uv_async_t* handle_;
        std::mutex mutex_;
        std::deque<Data<T>> queue_;

        void process()
        {
            while (!queue_.empty()) {
                {
                    std::unique_lock<std::mutex> lock(mutex_);
                    Data<T> data = queue_.front();
                    queue_.pop_front();
                    lock.unlock();
                    process(data);
                }
            }
        }

        static void closeCallback(uv_handle_t* handle)
        {
            free(handle);
        }

        static void callback(uv_async_t* handle)
        {
            reinterpret_cast<uv_deque*>(handle->data)->process();
        }
    };
} // namespace internal
} // namespace async

#endif // uv_deque_h
