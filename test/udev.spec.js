/* jshint -W024 */
/* jshint expr:true */

"use strict";
var chai = require("chai");
var sinonChai = require("sinon-chai");
var sinon = require("sinon");
var expect = chai.expect;
var udev = require("../udev");
var spawn = require("child_process").spawn;
var path = require("path");

chai.use(sinonChai);

var UDEV_SIGNAL_SCRIPT = path.join(__dirname, "udev_signal.sh");

/**
 * NOTE:
 * For best testing experience, it is recommended to plug in / insert 
 * each a DVD, SDCARD, HDD and USB storage media.
 * Mainly, it will be tested if DVDs change event will be converted
 * correctly to add or remove
 */
describe("udev", function(){
    describe("#list", function(){

        it("should only list valid storage media", function(){
            var list = udev.list();

            list.forEach(function(device){
                if(device.ID_BUS === "ata"){
                    expect(device.MEDIA_TYPE).to.equal("dvd");
                }
                expect(["dvd", "sdcard", "usb", "unknown"]).to.include(device.MEDIA_TYPE);
                expect(device.ID_FS_TYPE, "No ID_FS_TYPE found").to.be.ok;
            });
        });
    });

    describe("#monitor", function(){
        var hasDVD = false;

        before(function(){
            udev.list().forEach(function(device){
                if(device.MEDIA_TYPE === "dvd"){
                    hasDVD = true;
                }
            });
        });

        it("should instantiate the monitor", function(done){
            this.timeout(10000);
            var monitor = udev.monitor();

            var onadd = sinon.spy(function(device){
                expect(device.ACTION).to.equal("add");
                expect(device.MEDIA_TYPE).to.equal("dvd");
                done();
            });

            var onremove = sinon.spy();

            monitor.on("add", onadd);
            monitor.on("remove", onremove);

            var proc = spawn(UDEV_SIGNAL_SCRIPT, ["/dev/sr0", "change"]); 

            proc.on("close", function(code){
                expect(code).to.equal(0);

                if(!hasDVD){
                    expect(onadd).to.not.have.been.called;
                    expect(onremove).to.not.have.been.called;
                    done();
                }
            });
        });
    });
});
