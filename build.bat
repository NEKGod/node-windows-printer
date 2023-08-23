rd /s /q "./build"
node-gyp configure && node-gyp rebuild --target=21.4.4 --dist-url=https://electronjs.org/headers --proxy=http://127.0.0.1:7890
