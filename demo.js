"use strict";

var util = require("util");
var udev = require("./udev");
var fs = require("fs");

udev.list().forEach(function(udevice){

        //Finds CD / DVD
        //if(udevice.SUBSYSTEM === "block" && udevice.DEVTYPE === "disk" && udevice.ID_CDROM){
        //}
});

var events = [];
function onEvent(device){
   //console.log(JSON.stringify(device, null, 4));
   events.push(device); 
   console.log("Media-Type: " + device.MEDIA_TYPE + " / Action: " + device.ACTION);
}

var listMode = false;
if(listMode){
    fs.writeFileSync("output.json", JSON.stringify(udev.list(), null, 4));
}
else{
    var monitor = udev.monitor();

    monitor.on("add", onEvent);
    monitor.on("remove", onEvent);
    //monitor.on("device_event", onEvent);

    process.on("SIGINT", function(){
        fs.writeFileSync("output.json", JSON.stringify(events, null, 4));
        process.exit(0);
    });
}
