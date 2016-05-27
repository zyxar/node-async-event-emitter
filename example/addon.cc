#include "Event.h"
#include <async/EventEmitterObjectWrap.h>

using namespace v8;

void InitAll(Local<Object> exports)
{
    async::EventEmitterObjectWrap::Init(exports);
    Event::Init(exports);
}

NODE_MODULE(addon, InitAll)
