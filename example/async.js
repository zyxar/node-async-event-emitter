#!/usr/bin/env node

'use strict';

var addon = require('./build/Release/addon.node');

var evt = new addon.Event();

evt.run(function (a, b, c) {
  console.log('[', typeof a, ']:', a);
  console.log('[', typeof b, ']:', b);
  console.log('[', typeof c, ']:', c);
  setTimeout(function () {
    evt.close();
  }, 1000);
});
