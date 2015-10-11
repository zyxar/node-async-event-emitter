#!/usr/bin/env node

'use strict';

var addon = require('./build/Release/addon.node');

var evt = new addon.Event();
var abcHandler = function(v) {
  console.log('1:', v);
};

evt.run(function (a, b, c) {
  console.log('[', typeof a, ']:', a);
  console.log('[', typeof b, ']:', b);
  console.log('[', typeof c, ']:', c);
});

evt.on('abc', abcHandler);
evt.on('abc', function(v) {
  console.log('2:', v);
});

setTimeout(function () {
  evt.removeEventListener('abc', abcHandler);
  evt.on('abc', abcHandler);
  evt.removeEventListener('abc', abcHandler);
  evt.on('bcd', abcHandler);
  evt.close();
}, 1000);

setTimeout(function() {
  evt.clearEventListener('abc');
  evt.clearEventListener();
  process.exit();
}, 1500);

setInterval(function() {
  evt.emit('abc', Math.random());
}, 100);
