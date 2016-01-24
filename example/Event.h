#ifndef Event_h
#define Event_h

#include <CrossCallback.h>
class Runner;
class Event : public CrossCallbackWrap {
public:
    Event()
        : r{ nullptr }
    {
    }
    ~Event() {}
    static void Init(v8::Local<v8::Object>);
    Runner* r;

private:
    static v8::Persistent<v8::Function> constructor;
    static void New(const v8::FunctionCallbackInfo<v8::Value>&);
    static void Close(const v8::FunctionCallbackInfo<v8::Value>&);
    static void Run(const v8::FunctionCallbackInfo<v8::Value>&);
    static void Emit(const v8::FunctionCallbackInfo<v8::Value>&);
};

#endif
