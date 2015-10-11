#include <CrossCallback.h>

using namespace v8;

void InitAll(Local<Object> exports)
{
    CrossCallbackWrap::Init(exports);
}

NODE_MODULE(addon, InitAll)
