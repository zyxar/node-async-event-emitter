#include "Event.h"
#include <CrossCallback.h>

using namespace v8;

void InitAll(Local<Object> exports)
{
    CrossCallbackWrap::Init(exports);
    Event::Init(exports);
}

NODE_MODULE(addon, InitAll)
