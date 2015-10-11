#!/usr/bin/env node

'use strict';

var addon = require('./build/Release/addon.node');

var evt = new addon.Event();

evt.run(function (a, b, c) {
  console.log(a, b, c);
  setTimeout(function () {
    evt.close();
  }, 2000);
});
