// hello.js
const addon = require('../build/Release/printer');

// let printerList = addon.getPrinterList()
// console.log(printerList);
console.log(addon.getPrinterInfo('Canon G5080 series中文'))
console.log(addon.getPrinterJobList('Canon G5080 series中文'))
