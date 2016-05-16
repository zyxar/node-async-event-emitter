#include "Event.h"
#include <NodeAsyncCallback.h>

using namespace v8;

void InitAll(Local<Object> exports)
{
    cross::AsyncCallbackObjectWrap::Init(exports);
    Event::Init(exports);
}

NODE_MODULE(addon, InitAll)
