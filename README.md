# Modified version of node-udev

Original: https://github.com/cheery/node-udev

##What does this modification do differently to the original one?
- This module is optimized to receive add / remove events of DVDs, USB-Storage- and SDCARD-Devices
- Filters SATA-HDDs to prevent add / remove for installed harddrives (which contain OS data etc.)
- Filters all devices which are not intended for external storage
- List-Function lists only available DVDs, USB- and SDCARD-Devices 

##For which reason did I make this?
I use this for a node application, which is intended to mount storage devices as
soon as they get plugged in. On startup, there are no events so I also wanted to list all
available devices. Since I don't consider my builtin HDD as an external storage,
I filter all SATA-featured HDDs like /dev/sda.

I put a lot of effort into the device recognition, so no virtual device events slip through.
UDEV raises a lot of events and I don't want to manage those in my node-application (i.e. 
DVD-Drives raise 2 events, although 1 event is important for the add-event)

Feel free to use this module on your own demand.

##Credit
All credit for the essential base goes to Github-user cheery!
https://github.com/cheery


