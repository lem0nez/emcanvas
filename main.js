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
  print: function(text) {
    if (arguments.length > 1) {
      text = Array.prototype.slice.call(arguments).join(' ');
    }
    writeLog('I', text);
  },
  printErr: function(text) {
    if (arguments.length > 1) {
      text = Array.prototype.slice.call(arguments).join(' ');
    }
    writeLog('E', text);
  },

  canvas: (function() {
    var canvas = document.getElementById('canvas');
    canvas.addEventListener('webglcontextlost', function(element) {
      // May occur when a browser will decide to
      // free up some of shared GPU resources.
      alert('WebGL context lost. You will need to reload the page.');
    });
    return canvas;
  })(),
  monitorRunDependencies: function(count) {
    if (count != 0) {
      return;
    }

    var loader = document.getElementById('loader');
    loader.classList.add('hidden');
    loader.addEventListener('transitionend', function() {
      // Not required anymore after loading the application.
      this.remove();
    });
    canvas.classList.add('shown');
  }
};

window.onerror = function() {
  alert('Error occurred while executing the application.');
};
