rd /s /q "./build"
node-gyp configure && node-gyp rebuild --target=25.5.0 --arch=x64 --dist-url=https://electronjs.org/headers
