var path = require("path");

/*jshint strict:false*/
module.exports = function(grunt){

    grunt.initConfig({
        pkg : grunt.file.readJSON("package.json"),
        watch : {
            mocha : {
                files : ["udev.cc", "udev.js", "test/**/*.js"],
                tasks : ["gyp", "mochaTest:test"]
            }
        },
        mochaTest: {
            test: {
                options: {
                    reporter: "spec"
                },
                src: ["test/*.spec.js"]
            } 
        },
        gyp: {
            udev: {
                command : "rebuild"
            }
        }
    });

    grunt.loadNpmTasks("grunt-contrib-watch");
    grunt.loadNpmTasks("grunt-mocha-test");
    grunt.loadNpmTasks("grunt-node-gyp");

    grunt.registerTask("default", function(){
        grunt.log.subhead(">>> Use 'grunt test' to run all Mocha tests");
        grunt.log.subhead(">>> Use 'grunt watch:mocha' to run tests while watching files");
    });

    grunt.registerTask("test", [ "mochaTest:test" ]);
};
