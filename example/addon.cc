#include <CrossCallback.h>
#include "Event.h"

using namespace v8;

void InitAll(Local<Object> exports)
{
    CrossCallbackWrap::Init(exports);
    Event::Init(exports);
}

NODE_MODULE(addon, InitAll)
