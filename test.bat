chcp 65001
rd /s /q "./build"
node-gyp configure && node-gyp rebuild
node "./node/test.js"