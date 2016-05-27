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

#include "../Argument.h"
#include <chrono>
#include <iostream>
#include <thread>

std::ostream& operator<<(std::ostream& os, const async::Argument& argv)
{
    switch (argv.type) {
    case async::Argument::NUMBER:
        os << argv.value<double>();
        break;
    case async::Argument::INTEGER:
        os << argv.value<int>();
        break;
    case async::Argument::BOOLEAN:
        os << argv.value<bool>();
        break;
    case async::Argument::STRING:
        os << argv.value<const char*>();
        break;
    default:
        os << std::endl;
        break;
    }
    return os;
}

class Q : public async::internal::uv_deque<async::Argument> {
public:
    Q() {}
    ~Q() { mThread.join(); }

    virtual void process(const async::internal::uv_deque<async::Argument>::Data<async::Argument>& data)
    {
        auto ptr = &data.argument;
        std::cout << data.event << ": ";
        while (ptr) {
            std::cout << *ptr << ", ";
            ptr = ptr->next();
        }
        std::cout << std::endl;
    }

    void run()
    {
        mThread = std::thread(&Q::loop, this);
    }

protected:
    template <class... Args>
    bool emit(const std::string& event, const Args&... args)
    {
        const unsigned size = sizeof...(Args);
        async::Argument m[size] = { args... };
        unsigned i = 1;
        async::Argument* ptr = &m[0];
        while (i < size) {
            auto p = new async::Argument{ m[i] };
            ptr->next(p);
            ptr = p;
            ++i;
        }
        return push_back(event, m[0]);
    }

private:
    void loop()
    {
        std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
        while (1) {
            std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(now - start).count();
            if (duration > 1000000)
                break;
            emit("duration", 1, 2.01f, "WOW", true, false, duration);
            usleep(100000);
        }
    }
    std::thread mThread;
};

int main(int argc, char const* argv[])
{
    Q q{};
    q.run();

    uv_run(uv_default_loop(), UV_RUN_DEFAULT);
    return 0;
}
