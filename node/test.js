// hello.js
const addon = require('../build/Release/printer');
let printerList = addon.getWPrinterList()
console.log(printerList)

// console.log(addon.getPrinterInfo(printerList[0].pPrinterName))
// console.log(addon.getPrinterJobList('Canon G5080 series中文'))
