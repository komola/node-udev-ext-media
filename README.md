# Node Module udev-ext-media 

- This module is optimized to receive add / remove events of DVDs, USB-Storage- and SDCARD-Devices
- Filters SATA-HDDs to prevent add / remove for installed harddrives (which contain OS data etc.)
- Filters all devices which are not intended for external storage
- List-Function lists only available DVDs, USB- and SDCARD-Devices 

**Derived Codebase (node-udev):** 
https://github.com/cheery/node-udev
##For what Use-Case?
I use this for a node application, which is intended to mount storage devices as
soon as they get plugged in. On startup, there are no events so I also wanted to list all
available devices. Since I don't consider my builtin HDD as an external storage,
I filter all SATA-featured HDDs like /dev/sda.

I put a lot of effort into the device recognition, so no virtual device events slip through.
UDEV raises a lot of events and I don't want to manage those in my node-application (i.e. 
DVD-Drives raise 2 events, although 1 event is important for the add-event)

Feel free to use this module on your own demand.

##Installation
This package is not featured in NPM, you have to add this file in your package.json
file:

**Part of package.json:**
```
{
    "name" : "myproject",
    "version" : "1.0",
    ...
    ...
    "dependencies" : {
        "udev-ext-media" : "git+ssh://git@github.com:magicpat/node-udev.git"
    }
}
```

After adding this dependency, just download by entering `npm install`

##Usage
After installation, access the udev module
```javascript
var udev = require("udev-ext-media");

var monitor = udev.monitor();

//Device has member attributes as given by UDEV
monitor.on("add", function(device){
    console.log(device.MEDIA_TYPE); //Will output "usb", "dvd", "sdcard", else "unknown"  
});
```

Check out the `examples` and `test` directory for more code examples.

##Development
For development, use GRUNT to run the mocha tests.
Since these tests are targeted to connected hardware, insert various DVD,
USB, SDCARD, HDD devices before starting the tests.

**Run one-time tests**
```
grunt test
```

**Filewatcher tests**
```
grunt watch:mocha
```
Grunt will launch tests as soon as udev.cc, udev.js or any spec-files change

##Credit
All credit for the essential base goes to Github-user cheery!
https://github.com/cheery
