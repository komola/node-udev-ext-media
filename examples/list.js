"use strict";

var udev = require("../udev-ext-media");
var fs = require("fs");

fs.writeFileSync("output.json", JSON.stringify(udev.list(), null, 4));

console.log(JSON.stringify(udev.list(), null, 4));
