#!/usr/bin/env node

'use strict';

var addon = require('./build/Release/addon.node');

var evt = new addon.CrossCallback();
var abcHandler = function(v) {
  console.log('1:', v);
};
evt.on('abc', abcHandler);

evt.on('abc', function(v) {
  console.log('2:', v);
});

console.log(evt.self());

setInterval(function() {
  evt.emit('abc', Math.random());
}, 100);

setTimeout(function() {
  evt.removeEventListener('abc', abcHandler);
  console.log(evt.self());
  evt.on('abc', abcHandler);
  console.log(evt.self());
  evt.removeEventListener('abc', abcHandler);
  console.log(evt.self());
  evt.on('bcd', abcHandler);
}, 1000);

setTimeout(function() {
  evt.clearEventListener('abc');
  console.log(evt.self());
  evt.clearEventListener();
  console.log(evt.self());
}, 1500);

setTimeout(function() {
  process.exit();
}, 2000);
