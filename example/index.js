#!/usr/bin/env node
'use strict';

var evt = new (require('./build/Release/addon.node').Event)();
var abcHandler = function() {
  var args = arguments;
  console.log('1:', Object.keys(args).map(function (k) {
    return args[k];
  }).join(', '));
};

evt.on('alert', function (message) {
  console.log('alert: ' + message);
});

evt.run(function () {
  var args = arguments;
  Object.keys(args).map(function (k) {
    console.log('[', typeof args[k], ']:', args[k]);
  });
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
}, 2000);

var arg = [ 'abc', Math.random(), 1 ];
setInterval(function() {
  evt.emit.apply(evt, arg);
  arg.push(Math.random());
}, 100);

setTimeout(function() {
  evt.emit('alert', 'Notification');
  evt.emit('alert', 'Notification');
  evt.emit('alert', 'Notification');
  evt.emit('alert', 'Notification');
  evt.emit('alert', 'Notification');
  evt.emit('alert', 'Notification');
  evt.emit('alert', 'Notification');
  evt.emit('alert', 'Notification');
  evt.emit('alert', 'Notification');
  evt.emit('alert', 'Notification');
  evt.urge('alert', 'Prompt');
  evt.urge('alert', 'Prompt');
  evt.urge('alert', 'Prompt');
  evt.urge('alert', 'Prompt');
  evt.urge('alert', 'Prompt');
  evt.urge('alert', 'Prompt');
  evt.urge('alert', 'Prompt');
  evt.urge('alert', 'Prompt');
  evt.urge('alert', 'Prompt');
  evt.urge('alert', 'Prompt');
}, 1200);
