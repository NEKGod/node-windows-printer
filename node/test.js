// hello.js
const addon = require('../build/Release/printer');
console.log(addon.addToStartup("H:\\pojo\\printWinService\\release\\win-unpacked\\win10自助打印软件.exe"))
// let printerList = addon.getWPrinterList()
// console.log(printerList)

// console.log(addon.getPrinterInfo(printerList[0].pPrinterName))
// console.log(addon.getPrinterJobList('Canon G5080 series中文'))
