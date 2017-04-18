# async-event-emitter

Asynchronous Event and Callback Handling for Node.js with Native addon.

Node.js is efficient for I/O, however, sometimes your program still needs multithreading and native addons to accomplish complex, comprehensive tasks. `async-event-emitter` comes to fill the gap for this need, playing with `libuv` shipped with Node.js, such that you can utilize Node.js as a coordinator or scheduler while maintain your C/C++ worker concurrent and asynchronous, using the general event driven style.

## Requirements

- Node.js v4+
- node-gyp
- C++11 compiler
- macOS or Linux

## Install

- `npm install async-event-emitter`

The above line will install this module into your local `node_modules/`:

```
node_modules/
└── async-event-emitter
    ├── CHANGES.md
    ├── LICENSE
    ├── async
    │   ├── Argument.cc
    │   ├── Argument.h
    │   ├── EventEmitter.h
    │   ├── EventEmitterObjectWrap.cc
    │   ├── EventEmitterObjectWrap.h
    │   ├── NodeEventEmitter.cc
    │   ├── NodeEventEmitter.h
    │   └── internal
    │       ├── uv_deque.h
    │       └── uv_deque_test.cc
    ├── index.js
    └── package.json
```


## Getting Started

Check [example](example) for details.
