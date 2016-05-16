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

#ifndef UvAsyncCallback_h
#define UvAsyncCallback_h

#include "Argument.h"
#include "AsyncCallback.h"

#include <mutex>
#include <queue>
#include <uv.h>

namespace cross {

class UvAsyncCallback : public ::cross::AsyncCallback {
public:
    explicit UvAsyncCallback();
    explicit UvAsyncCallback(uv_loop_t*);
    virtual ~UvAsyncCallback();
    virtual bool notify(const std::string& event, const Argument& argument);
    virtual size_t size();

protected:
    struct Data {
        std::string event;
        Argument argument;
    };
    virtual void operator()(const Data& data) = 0;

private:
    uv_async_t* mUvHandle;
    std::mutex mLock;
    std::queue<Data> mBuffer;

    void process();
    static void closeCallback(uv_handle_t*);
    static void callback(uv_async_t*);
};
}

#endif // UvAsyncCallback_h
