var udev = require("./build/Release/udev-ext-media");
var util = require("util");

var EventEmitter = require("events").EventEmitter;

//This method overwrites the close-method, this is not ideal...
//going back to the "old" inheritance method
//util.inherits(udev.Monitor, EventEmitter);
udev.Monitor.prototype.__proto__ = EventEmitter.prototype;

module.exports = {
    monitor: function() { return new udev.Monitor(); },
    list: udev.list
}
