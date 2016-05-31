#include "Event.h"
#include <chrono>
#include <memory>
#include <thread>
#include <unistd.h>

using namespace v8;

class Runner {
public:
    Runner(async::EventEmitter* cb = nullptr)
        : mCallback{ cb }
    {
    }
    ~Runner()
    {
        if (mCallback)
            mCallback.reset();
        mThread.join();
    }
    void run()
    {
        mThread = std::thread(&Runner::loop, this);
    }

private:
    void loop()
    {
        auto start = std::chrono::steady_clock::now();
        while (1) {
            auto now = std::chrono::steady_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(now - start).count();
            if (duration > 1000000)
                break;
            if (mCallback)
                (*mCallback)("WOW!", 1, 0.01f, true, false, nullptr);
            usleep(100);
        }
    }
    std::unique_ptr<async::EventEmitter> mCallback;
    std::thread mThread;
};

Persistent<Function> Event::constructor;

void Event::Init(Local<Object> exports)
{
    Isolate* isolate = exports->GetIsolate();
    Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);
    tpl->SetClassName(String::NewFromUtf8(isolate, "Event"));
    tpl->InstanceTemplate()->SetInternalFieldCount(1);
    SETUP_CROSSCALLBACK_PROTOTYPE_METHODS(tpl);
    NODE_SET_PROTOTYPE_METHOD(tpl, "run", Run);
    NODE_SET_PROTOTYPE_METHOD(tpl, "close", Close);
    NODE_SET_PROTOTYPE_METHOD(tpl, "emit", Emit);
    NODE_SET_PROTOTYPE_METHOD(tpl, "urge", Urge);
    constructor.Reset(isolate, tpl->GetFunction());
    exports->Set(String::NewFromUtf8(isolate, "Event"), tpl->GetFunction());
}

void Event::New(const v8::FunctionCallbackInfo<v8::Value>& arguments)
{
    Isolate* isolate = arguments.GetIsolate();
    if (arguments.IsConstructCall()) {
        Event* n = new Event();
        n->Wrap(arguments.This());
        arguments.GetReturnValue().Set(arguments.This());
    } else {
        const int argc = 1;
        Local<Value> argv[argc] = { arguments[0] };
        Local<Function> cons = Local<Function>::New(isolate, constructor);
        arguments.GetReturnValue().Set(cons->NewInstance(argc, argv));
    }
}

void Event::Run(const v8::FunctionCallbackInfo<v8::Value>& arguments)
{
    Isolate* isolate = arguments.GetIsolate();
    if (arguments.Length() == 0 || !arguments[0]->IsFunction()) {
        isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "Wrong arguments")));
        return;
    }
    Event* n = ObjectWrap::Unwrap<Event>(arguments.Holder());
    if (n->r)
        delete n->r;
    n->r = new Runner(NodeEventEmitter::New(Local<Function>::Cast(arguments[0])));
    n->r->run();
}

void Event::Close(const v8::FunctionCallbackInfo<v8::Value>& arguments)
{
    Event* n = ObjectWrap::Unwrap<Event>(arguments.Holder());
    if (n->r) {
        delete n->r;
        n->r = nullptr;
    }
}

void Event::Emit(const FunctionCallbackInfo<Value>& arguments)
{
    if (arguments.Length() < 2 || !arguments[0]->IsString())
        return;
    auto n = ObjectWrap::Unwrap<Event>(arguments.Holder());
    auto event = std::string(*String::Utf8Value(arguments[0]->ToString()));
    auto data = async::Argument{ nullptr };
    auto ptr = &data;
    async::Argument* p = nullptr;
    for (int i = 1; i < arguments.Length(); ++i) {
        if (arguments[i]->IsInt32() || arguments[i]->IsUint32())
            p = new async::Argument{ int(arguments[i]->IntegerValue()) };
        else if (arguments[i]->IsNumber())
            p = new async::Argument{ arguments[i]->NumberValue() };
        else if (arguments[i]->IsBoolean())
            p = new async::Argument{ *arguments[i]->ToBoolean() };
        else if (arguments[i]->IsString())
            p = new async::Argument{ std::string(*String::Utf8Value(arguments[i]->ToString())) };
        else
            continue;
        ptr->next(p);
        ptr = p;
    }
    n->notify(event, *data.next());
}

void Event::Urge(const FunctionCallbackInfo<Value>& arguments)
{
    if (arguments.Length() < 2 || !arguments[0]->IsString())
        return;
    Event* n = ObjectWrap::Unwrap<Event>(arguments.Holder());
    std::string event = std::string(*String::Utf8Value(arguments[0]->ToString()));
    auto data = async::Argument{ nullptr };
    auto ptr = &data;
    async::Argument* p = nullptr;
    for (int i = 1; i < arguments.Length(); ++i) {
        if (arguments[i]->IsInt32() || arguments[i]->IsUint32())
            p = new async::Argument{ int(arguments[i]->IntegerValue()) };
        else if (arguments[i]->IsNumber())
            p = new async::Argument{ arguments[i]->NumberValue() };
        else if (arguments[i]->IsBoolean())
            p = new async::Argument{ *arguments[i]->ToBoolean() };
        else if (arguments[i]->IsString())
            p = new async::Argument{ std::string(*String::Utf8Value(arguments[i]->ToString())) };
        else
            continue;
        ptr->next(p);
        ptr = p;
    }
    n->prompt(event, *data.next());
}
