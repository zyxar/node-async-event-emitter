#ifndef Event_h
#define Event_h

#include <node.h>
#include <node_object_wrap.h>
class Runner;
class Event : public node::ObjectWrap {
public:
    Event()
        : r{ nullptr }
    {
    }
    ~Event() {}
    static void Init(v8::Local<v8::Object> exports);
    Runner* r;

private:
    static v8::Persistent<v8::Function> constructor;
    static void New(const v8::FunctionCallbackInfo<v8::Value>& args);
    static void Close(const v8::FunctionCallbackInfo<v8::Value>& args);
    static void Run(const v8::FunctionCallbackInfo<v8::Value>& args);
};

#endif
