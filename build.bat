rd /s /q "./build"
node-gyp configure && node-gyp rebuild --target=26.2.0 --dist-url=https://electronjs.org/headers --proxy=http://127.0.0.1:7890
