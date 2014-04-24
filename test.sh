#!/bin/sh

node-gyp rebuild
node demo.js > test.json
