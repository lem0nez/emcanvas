// Copyright © 2021 Nikita Dudko. All rights reserved.
// Licensed under the Apache License, Version 2.0

// Replacement for the Console. It makes easier
// to debug the application in mobile browsers.
var log = '';

function prependZerosIfNeed(str, fixedSize) {
  return '0'.repeat(Math.abs(str.toString().length - fixedSize)) + str;
}

function writeLog(level, text) {
  var date = new Date();

  var hour = prependZerosIfNeed(date.getHours(), 2);
  var min = prependZerosIfNeed(date.getMinutes(), 2);
  var sec = prependZerosIfNeed(date.getSeconds(), 2);
  var ms = prependZerosIfNeed(date.getMilliseconds(), 3);

  log += '[' + hour + ':' + min + ':' + sec + '.' + ms +
         '] <' + level + '> ' + text + '\n';
}

var Module = {
  'print': function(text) {
    writeLog('I', text);
  },
  'printErr': function(text) {
    writeLog('E', text);
  },
  'canvas': (function() {
    var canvas = document.getElementById('canvas');
    canvas.addEventListener('webglcontextlost', function(element) {
      // May occur when a browser will decide to
      // free up some of shared GPU resources.
      alert('WebGL context lost. You will need to reload the page');
    });
    return canvas;
  })(),

  // Set to false as the atexit functions should be
  // called to clean up all initialized SDL subsystems.
  'noExitRuntime': false,
  /*
   * Main function will be called manually, because the onRuntimeInitialized
   * callback executes JS code IN PARALLEL with (not BEFORE) the main function
   * execution.
   */
  'noInitialRun': true,
  'onRuntimeInitialized': function() {
    var loader = document.getElementById('loader');
    loader.classList.add('hidden');
    loader.addEventListener('transitionend', function() {
      // Not required anymore after loading the application.
      this.remove();
    });

    var px_ratio = window.devicePixelRatio || 1.0;
    // Scrollbars aren't counted as they are hidden using CSS.
    var width = document.documentElement.clientWidth;
    var scale = width / (width * px_ratio);

    var viewport = document.querySelector('meta[name=viewport]');
    // Adapt viewport scale to match the physical screen size.
    viewport.setAttribute('content',
        'width=device-width, user-scalable=no, initial-scale=' + scale);

    // Occupy all viewport space by canvas. Don't use the previously
    // stored width as value was changed after changing the viewport scale.
    canvas.width = document.documentElement.clientWidth;
    canvas.height = document.documentElement.clientHeight;
    canvas.classList.add('shown');

    callMain(arguments);
  }
};

window.onerror = function() {
  alert('Error occurred while executing the application');
};
