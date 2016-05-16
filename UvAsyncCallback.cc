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

#include "UvAsyncCallback.h"

namespace cross {

UvAsyncCallback::UvAsyncCallback()
{
    mUvHandle = reinterpret_cast<uv_async_t*>(malloc(sizeof(uv_async_t)));
    mUvHandle->data = this;
    uv_async_init(uv_default_loop(), mUvHandle, UvAsyncCallback::callback);
}

UvAsyncCallback::UvAsyncCallback(uv_loop_t* loop)
{
    mUvHandle = reinterpret_cast<uv_async_t*>(malloc(sizeof(uv_async_t)));
    mUvHandle->data = this;
    uv_async_init(loop, mUvHandle, UvAsyncCallback::callback);
}

UvAsyncCallback::~UvAsyncCallback()
{
    uv_close(reinterpret_cast<uv_handle_t*>(mUvHandle), UvAsyncCallback::closeCallback);
}

// main thread
void UvAsyncCallback::process()
{
    while (!mBuffer.empty()) {
        {
            std::unique_lock<std::mutex> lock(mLock);
            Data data = mBuffer.front();
            mBuffer.pop_front();
            lock.unlock();
            (*this)(data);
        }
    }
}

size_t UvAsyncCallback::size()
{
    return mBuffer.size();
}

// other thread
bool UvAsyncCallback::notify(const std::string& event, const Argument& argument)
{
    if (uv_is_active(reinterpret_cast<uv_handle_t*>(mUvHandle))) {
        {
            std::lock_guard<std::mutex> lock(mLock);
            mBuffer.push_back(Data{ event, argument });
        }
        uv_async_send(mUvHandle);
        return true;
    }
    return false;
}

// other thread
bool UvAsyncCallback::prompt(const std::string& event, const Argument& argument)
{
    if (uv_is_active(reinterpret_cast<uv_handle_t*>(mUvHandle))) {
        {
            std::lock_guard<std::mutex> lock(mLock);
            mBuffer.push_front(Data{ event, argument });
        }
        uv_async_send(mUvHandle);
        return true;
    }
    return false;
}

void UvAsyncCallback::closeCallback(uv_handle_t* handle)
{
    free(handle);
}

void UvAsyncCallback::callback(uv_async_t* handle)
{
    reinterpret_cast<UvAsyncCallback*>(handle->data)->process();
}
}
