#define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING
#define _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS
#include "printer/printer.h"
#include <node.h>
#include <Windows.h>
#include <iostream>
#include <vector>
#include <node_api.h>
#include <codecvt>

namespace printer {
    using v8::Context;
    using v8::FunctionCallbackInfo;
    using v8::Isolate;
    using v8::Local;
    using v8::Object;
    using v8::String;
    using v8::Array;
    using v8::Value;

    void Method(const FunctionCallbackInfo<Value> &args) {
        Isolate *isolate = args.GetIsolate();
        args.GetReturnValue().Set(String::NewFromUtf8(
                isolate, "world").ToLocalChecked());
    }

    void getPrinterList(const FunctionCallbackInfo<Value> &args) {
        // 设置控制台以支持 UTF-8 编码
        SetConsoleOutputCP(CP_UTF8);

        Isolate *isolate = args.GetIsolate();
        // 创建一个新的V8对象
        Local<Context> context = isolate->GetCurrentContext();
        // 创建一个新的V8数组
        Local<Array> printList = Array::New(isolate);
        std::vector<wchar_t> printerBuffer;
        DWORD bufferSize = 0;
        DWORD printerCount = 0;
        // 获取需要的缓冲区大小和打印机数量
        EnumPrintersW(PRINTER_ENUM_LOCAL, nullptr, 2, nullptr, 0, &bufferSize, &printerCount);
        if (bufferSize < 0) {
            FUNCTION_RETURN(args, printList)
        }
        printerBuffer.resize(bufferSize);
        // 获取打印机列表
        auto *pPrinterInfo = reinterpret_cast<PRINTER_INFO_2 *>(printerBuffer.data());
        if (EnumPrintersW(PRINTER_ENUM_LOCAL, nullptr, 2, reinterpret_cast<LPBYTE>(pPrinterInfo), bufferSize,
                          &bufferSize, &printerCount)) {
            for (DWORD i = 0; i < printerCount; ++i) {
                Local<Object> obj = mapperPrinterInfoNodeObjectW(isolate, context, &pPrinterInfo[i]);
                printList->Set(context, i, obj);
            }
        }
        FUNCTION_RETURN(args, printList)
    }

    void getPrinterInfo(const FunctionCallbackInfo<Value> &args) {
        Isolate *isolate = args.GetIsolate();
        // 创建一个新的V8对象
        Local<Context> context = isolate->GetCurrentContext();
        if (args.Length() < 1) {
            isolate->ThrowException(
                    String::NewFromUtf8(isolate, "Invalid number of arguments").ToLocalChecked());
            return;
        }
        LONG structSize = 0;
        HANDLE hPrinter = nullptr;
        DEVMODEW *devMode = nullptr;
        DWORD needed = 0;
        LONG ret = 0;

        v8::String::Utf8Value argsPrinterName(args.GetIsolate(), args[0]);
        const char* argsPrinterNamePoint = *argsPrinterName;
        // 将宽字符字符串赋值给LPWSTR类型
        LPWSTR printerName = ConvertToLPWSTR(argsPrinterNamePoint);
        WCHAR* printerNameW = printerName;
        BOOL ok = OpenPrinterW(printerName, &hPrinter, nullptr);
        if (!ok) {
            TRY_FUNCTION(isolate, "打印机名称不存在 001!!!")
        }
        GetPrinterW(hPrinter, 2, nullptr, 0, &needed);
        BYTE* printerBuffer  = new BYTE[needed];
        ok = GetPrinterW(hPrinter, 2, printerBuffer, needed, &needed);
        if (!ok || needed <= sizeof(PRINTER_INFO_2)) {
            TRY_FUNCTION(isolate, "获取打印机信息异常 001!!!")
        }
        auto* info = reinterpret_cast<PRINTER_INFO_2*>(printerBuffer);
        Local<Object> printerInfoNodeObject = mapperPrinterInfoNodeObjectW(isolate, context, info);
        /* ask for the size of DEVMODE struct */
        structSize = DocumentPropertiesW(nullptr, hPrinter, printerName, nullptr, nullptr, 0);
        if (structSize < sizeof(DEVMODEW)) {
            TRY_FUNCTION(isolate, "获取打印机信息异常 002!!!")
        }
        devMode = (DEVMODEW*)new BYTE[structSize];
        // Get the default DevMode for the printer and modify it for your needs.
        ret = DocumentPropertiesW(nullptr, hPrinter, printerName, devMode, nullptr, DM_OUT_BUFFER);
        if (IDOK != ret) {
            TRY_FUNCTION(isolate, "获取打印机信息异常 003!!!")
        }

        // 获取打印机纸张
        {
            DWORD n = DeviceCapabilitiesW(printerNameW, nullptr, DC_PAPERS, nullptr, nullptr);
            DWORD n2 = DeviceCapabilitiesW(printerNameW, nullptr, DC_PAPERNAMES, nullptr, nullptr);
            DWORD n3 = DeviceCapabilitiesW(printerNameW, nullptr, DC_PAPERSIZE, nullptr, nullptr);
            if (n != n2 || n != n3 || 0 == n || ((DWORD)-1 == n)) {
                TRY_FUNCTION(isolate, "获取打印机纸张类型异常 00010!!!")
            }
            OBJECT_SET_NUMBER(context, printerInfoNodeObject, "nPaperSizes", (int)n);
            int nPaperSizes = (int)n;
            size_t paperNameSize = 64;
            WORD *papers = new WORD[n];
            auto *paperNamesSeq = new WCHAR[paperNameSize * n + 1]; // +1 is "just in case"
            auto *paperSizes = new POINT[n];
            DeviceCapabilitiesW(printerNameW, nullptr, DC_PAPERS, (WCHAR*)papers, nullptr);
            DeviceCapabilitiesW(printerNameW, nullptr, DC_PAPERSIZE, (WCHAR*)paperSizes, nullptr);
            DeviceCapabilitiesW(printerNameW, nullptr, DC_PAPERNAMES, paperNamesSeq, nullptr);
            Local<Array> paperNames = Array::New(isolate);
            WCHAR* paperName = paperNamesSeq;
            for (int i = 0; i < (int)n; i++) {
                paperNames->Set(context, i, String::NewFromUtf8(isolate, WideCharToNarrow(paperName)).ToLocalChecked());
                paperName += paperNameSize;
            }
            OBJECT_SET_ARRAY(context, printerInfoNodeObject, "paperNames", paperNames)
        }

        {
            DWORD n = DeviceCapabilitiesW(printerNameW, nullptr, DC_BINS, nullptr, nullptr);
            DWORD n2 = DeviceCapabilitiesW(printerNameW, nullptr, DC_BINNAMES, nullptr, nullptr);
            if (n != n2 || ((DWORD)-1 == n)) {
                TRY_FUNCTION(isolate, "获取打印机纸张类型异常 0012!!!")
            }
//            printer->nBins = n;
            if (n > 0) {
                size_t binNameSize = 24;
                WORD * bins = new WORD(n);
                auto* binNamesSeq = new WCHAR[binNameSize * n + 1];
                DeviceCapabilitiesW(printerNameW, nullptr, DC_BINS, (WCHAR*)bins, nullptr);
                DeviceCapabilitiesW(printerNameW, nullptr, DC_BINNAMES, binNamesSeq, nullptr);
                WCHAR* binName = binNamesSeq;
                Local<Array> binNames = Array::New(isolate);
                for (int i = 0; i < (int)n; i++) {
                    binNames->Set(context, i, String::NewFromUtf8(isolate, WideCharToNarrow(binName)).ToLocalChecked());
                    binName += binNameSize;
                }
                OBJECT_SET_ARRAY(context, printerInfoNodeObject, "binNames", binNames)
                delete[] binNamesSeq;
            }
        }

        {
            DWORD n;

            n = DeviceCapabilitiesW(printerName, nullptr, DC_COLLATE, nullptr, nullptr);
            OBJECT_SET_BOOL(context, printerInfoNodeObject, "canCallate", (n > 0))

            n = DeviceCapabilitiesW(printerName, nullptr, DC_COLORDEVICE, nullptr, nullptr);
            OBJECT_SET_BOOL(context, printerInfoNodeObject, "isColor", (n > 0))

            n = DeviceCapabilitiesW(printerName, nullptr, DC_DUPLEX, nullptr, nullptr);
            OBJECT_SET_BOOL(context, printerInfoNodeObject, "isDuplex", (n > 0))

            n = DeviceCapabilitiesW(printerName, nullptr, DC_STAPLE, nullptr, nullptr);
            OBJECT_SET_BOOL(context, printerInfoNodeObject, "canStaple", (n > 0))

            n = DeviceCapabilitiesW(printerName, nullptr, DC_ORIENTATION, nullptr, nullptr);
            OBJECT_SET_BOOL(context, printerInfoNodeObject, "orientation", (n > 0))
        }

        FUNCTION_RETURN(args, printerInfoNodeObject)
    }

    void getPrinterJobList(const FunctionCallbackInfo<Value> &args) {
        Isolate *isolate = args.GetIsolate();
        v8::String::Utf8Value argsPrinterName(args.GetIsolate(), args[0]);
        if (argsPrinterName.length() == 0) {
            TRY_FUNCTION(isolate, "printerName不可为空")
        }
        // 创建一个新的V8对象
        Local<Context> context = isolate->GetCurrentContext();
        // 创建一个新的V8数组
        Local<Array> jobList = Array::New(isolate);
        const char* argsPrinterNamePoint = *argsPrinterName;
        HANDLE hPrinter = nullptr;
        BOOL ok = OpenPrinter(ConvertUtf8ToGbk(argsPrinterNamePoint), &hPrinter, nullptr);
        if (!ok) {
            TRY_FUNCTION(isolate, "打印机名称不存在!!!")
        }
        DWORD dwJobCount = 0;
        DWORD dwBytesNeededJob = 0;
        EnumJobs(hPrinter, 0, 100, 1, nullptr, 0, &dwBytesNeededJob, &dwJobCount);
        if (dwBytesNeededJob < 0) {
            TRY_FUNCTION(isolate, "打印机任务获取错误!!! 001")
        }
        JOB_INFO_1* pJobInfo = nullptr;
        pJobInfo = static_cast<JOB_INFO_1*>(malloc(dwBytesNeededJob));
        if (!pJobInfo) {
            TRY_FUNCTION(isolate, "打印机任务获取错误!!! 002")
        }
        if (EnumJobs(hPrinter, 0, 100, 1, reinterpret_cast<BYTE*>(pJobInfo), dwBytesNeededJob, &dwBytesNeededJob, &dwJobCount)) {
            for (DWORD j = 0; j < dwJobCount; ++j) {
                Local<Object> obj = Object::New(isolate);
                std::cout << "Job ID: " << pJobInfo[j].JobId << std::endl;
                std::cout << "Job Status: " << pJobInfo[j].Status << std::endl;
                OBJECT_SET_NUMBER(context, obj, "JobId", pJobInfo[j].JobId)
                OBJECT_SET_STRING(context, obj, "pPrinterName", gbkStringToUtf8String(pJobInfo[j].pPrinterName).c_str())
                OBJECT_SET_STRING(context, obj, "pMachineName", pJobInfo[j].pMachineName)
                OBJECT_SET_STRING(context, obj, "pUserName", pJobInfo[j].pUserName)
                OBJECT_SET_STRING(context, obj, "pDocument", gbkStringToUtf8String(pJobInfo[j].pDocument).c_str())
                OBJECT_SET_STRING(context, obj, "pDatatype", pJobInfo[j].pDatatype)
                OBJECT_SET_STRING(context, obj, "pStatus", pJobInfo[j].pStatus)
                OBJECT_SET_NUMBER(context, obj, "Status", pJobInfo[j].Status)
                OBJECT_SET_NUMBER(context, obj, "Priority", pJobInfo[j].Priority)
                OBJECT_SET_NUMBER(context, obj, "Position", pJobInfo[j].Position)
                OBJECT_SET_NUMBER(context, obj, "TotalPages", pJobInfo[j].TotalPages)
                OBJECT_SET_NUMBER(context, obj, "PagesPrinted", pJobInfo[j].PagesPrinted)
                jobList->Set(context, j, obj).Check();
            }
        }
        free(pJobInfo);
        ClosePrinter(hPrinter);
        FUNCTION_RETURN(args, jobList)
    }

    void Initialize(Local<Object> exports) {
        NODE_SET_METHOD(exports, "hello", Method);
        NODE_SET_METHOD(exports, "getPrinterList", getPrinterList);
        NODE_SET_METHOD(exports, "getPrinterInfo", getPrinterInfo);
        NODE_SET_METHOD(exports, "getPrinterJobList", getPrinterJobList);
        NODE_SET_METHOD(exports, "getWPrinterList", getPrinterList);
    }

    NODE_MODULE(NODE_GYP_MODULE_NAME, Initialize)

}  // namespace demo

