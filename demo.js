"use strict";

var util = require("util");
var udev = require("./udev");

udev.list().forEach(function(udevice){

        //Finds CD / DVD
        //if(udevice.SUBSYSTEM === "block" && udevice.DEVTYPE === "disk" && udevice.ID_CDROM){
        //}
});


console.log(JSON.stringify(udev.list(), null, 4));
