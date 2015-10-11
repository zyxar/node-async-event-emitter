#include "Event.h"
#include <chrono>
#include <CrossCallback.h>
#include <boost/thread.hpp>
#include <memory>

using namespace v8;

class Runner {
public:
    Runner(cross::AsyncCallback* cb = nullptr)
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
        mThread = boost::thread(&Runner::loop, this);
    }

private:
    void loop()
    {
        std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
        while (1) {
            std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
            if (std::chrono::duration_cast<std::chrono::microseconds>(now - start).count() > 1000000)
                break;
            if (mCallback)
                (*mCallback.get())("WOW!", 1, 0.01f);
            usleep(10000);
        }
    }
    std::unique_ptr<cross::AsyncCallback> mCallback;
    boost::thread mThread;
};

Persistent<Function> Event::constructor;

void Event::Init(Local<Object> exports)
{
    Isolate* isolate = Isolate::GetCurrent();
    Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);
    tpl->SetClassName(String::NewFromUtf8(isolate, "Event"));
    tpl->InstanceTemplate()->SetInternalFieldCount(1);
    NODE_SET_PROTOTYPE_METHOD(tpl, "run", Run);
    NODE_SET_PROTOTYPE_METHOD(tpl, "close", Close);
    constructor.Reset(isolate, tpl->GetFunction());
    exports->Set(String::NewFromUtf8(isolate, "Event"), tpl->GetFunction());
}

void Event::New(const v8::FunctionCallbackInfo<v8::Value>& args)
{
    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope(isolate);
    if (args.IsConstructCall()) {
        Event* n = new Event();
        n->Wrap(args.This());
        args.GetReturnValue().Set(args.This());
    }
    else {
        const int argc = 1;
        Local<Value> argv[argc] = { args[0] };
        Local<Function> cons = Local<Function>::New(isolate, constructor);
        args.GetReturnValue().Set(cons->NewInstance(argc, argv));
    }
}

void Event::Run(const v8::FunctionCallbackInfo<v8::Value>& args)
{
    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope(isolate);
    if (args.Length() == 0 || !args[0]->IsFunction()) {
        isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "Wrong arguments")));
        return;
    }
    Event* n = ObjectWrap::Unwrap<Event>(args.Holder());
    if (n->r)
        delete n->r;
    n->r = new Runner(NodeAsyncCallback::New(Local<Function>::Cast(args[0])));
    n->r->run();
}

void Event::Close(const v8::FunctionCallbackInfo<v8::Value>& args)
{
    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope(isolate);
    Event* n = ObjectWrap::Unwrap<Event>(args.Holder());
    if (n->r) {
        delete n->r;
        n->r = nullptr;
    }
}
