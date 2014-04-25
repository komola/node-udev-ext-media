"use strict";

var udev = require("../udev-ext-media");
var fs = require("fs");

var events = [];

function onEvent(device){
   //console.log(JSON.stringify(device, null, 4));
   events.push(device); 
   console.log("Media-Type: " + device.MEDIA_TYPE + " / Action: " + device.ACTION);
}

var monitor = udev.monitor();

monitor.on("add", onEvent);
monitor.on("remove", onEvent);
//monitor.on("device_event", onEvent);

process.on("SIGINT", function(){
    fs.writeFileSync("output.json", JSON.stringify(events, null, 4));
    process.exit(0);
});
