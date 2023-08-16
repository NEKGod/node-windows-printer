#include <locale>
#include <node.h>
#include <Windows.h>
#include <iostream>
#include <vector>
#include <node_api.h>
#include <codecvt>
using v8::Context;
using v8::FunctionCallbackInfo;
using v8::Isolate;
using v8::Local;
using v8::Object;
using v8::String;
using v8::Array;
using v8::Value;

#define OBJECT_SET_STRING(context, obj, field, value) \
        obj->Set( \
            context, \
            String::NewFromUtf8(isolate, field).ToLocalChecked(), \
            String::NewFromUtf8(isolate, value == nullptr ? "" : value).ToLocalChecked() \
        ).FromJust();

#define OBJECT_SET_NUMBER(context, obj, field, value) \
     obj->Set(context, \
            String::NewFromUtf8(isolate, field).ToLocalChecked(), \
            v8::Number::New(isolate,value) \
    ).FromJust();

#define OBJECT_SET_ARRAY(context, obj, field, value) \
     obj->Set(context, \
            String::NewFromUtf8(isolate, field).ToLocalChecked(), \
            value \
    ).FromJust();

#define OBJECT_SET_BOOL(context, obj, field, value) \
     obj->Set(context, \
            String::NewFromUtf8(isolate, field).ToLocalChecked(), \
            v8::Boolean::New(isolate, value) \
    ).FromJust();

#define FUNCTION_RETURN(args, value) \
    args.GetReturnValue().Set(value);  \
    return ;

#define TRY_FUNCTION(isolate, msg) \
    isolate->ThrowException( \
        String::NewFromUtf8(isolate, gbkStringToUtf8String(msg).c_str()).ToLocalChecked()); \
    return ;

char* WideCharToNarrow(const wchar_t* wideStr) {
    int bufferSize = WideCharToMultiByte(CP_UTF8, 0, wideStr, -1, nullptr, 0, nullptr, nullptr);
    char* narrowStr = new char[bufferSize];
    WideCharToMultiByte(CP_UTF8, 0, wideStr, -1, narrowStr, bufferSize, nullptr, nullptr);
    return narrowStr;
}

std::string gbkStringToUtf8String(const char* gbkString) {
    // 将 GBK 编码的 C 字符串转换为 UTF-16 编码的宽字符
    int utf16Size = MultiByteToWideChar(CP_ACP, 0, gbkString, -1, nullptr, 0);
    if (utf16Size == 0) {
        std::cerr << "Failed to calculate UTF-16 buffer size." << std::endl;
        return "";
    }
    auto utf16Buffer = new wchar_t[utf16Size];
    if (MultiByteToWideChar(CP_ACP, 0, gbkString, -1, utf16Buffer, utf16Size) == 0) {
        std::cerr << "GBK to UTF-16 conversion failed." << std::endl;
        delete[] utf16Buffer;
        return "";
    }
    // 将 UTF-16 编码的宽字符转换为 UTF-8 编码的字符串
    std::wstring utf16String(utf16Buffer);
    delete[] utf16Buffer;
    int utf8BufferSize = WideCharToMultiByte(CP_UTF8, 0, utf16String.c_str(), static_cast<int>(utf16String.length()), nullptr, 0, nullptr, nullptr);
    if (utf8BufferSize == 0) {
        std::cerr << "Failed to calculate UTF-8 buffer size." << std::endl;
        return "";
    }
    std::string utf8Buffer(utf8BufferSize, '\0');
    if (WideCharToMultiByte(CP_UTF8, 0, utf16String.c_str(), static_cast<int>(utf16String.length()), &utf8Buffer[0], utf8BufferSize, nullptr, nullptr) == 0) {
        std::cerr << "UTF-16 to UTF-8 conversion failed." << std::endl;
        return "";
    }
    return utf8Buffer;
}

LPSTR ConvertUtf8ToGbk(const std::string& utf8Str) {
    // 将 UTF-8 编码的字符串转换为宽字符字符串
    int wideStrLength = MultiByteToWideChar(CP_UTF8, 0, utf8Str.c_str(), -1, NULL, 0);
    std::wstring wideStrBuffer(wideStrLength, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, utf8Str.c_str(), -1, &wideStrBuffer[0], wideStrLength);

    // 将宽字符字符串转换为 GBK 编码的多字节字符字符串
    int gbkStrLength = WideCharToMultiByte(CP_ACP, 0, wideStrBuffer.c_str(), -1, NULL, 0, NULL, NULL);
    LPSTR gbkStrBuffer = new char[gbkStrLength];
    WideCharToMultiByte(CP_ACP, 0, wideStrBuffer.c_str(), -1, gbkStrBuffer, gbkStrLength, NULL, NULL);

    return gbkStrBuffer;
}

struct LPWSTRDeleter {
    void operator()(LPWSTR ptr) const {
        ::free(ptr);
    }
};

using UniqueLPWSTR = std::unique_ptr<wchar_t, LPWSTRDeleter>;

UniqueLPWSTR ConvertToLPWSTR_Unique(const std::string& utf8String) {
    int wideStrLength = MultiByteToWideChar(CP_UTF8, 0, utf8String.c_str(), -1, nullptr, 0);
    std::vector<wchar_t> wideStrBuffer(wideStrLength);
    MultiByteToWideChar(CP_UTF8, 0, utf8String.c_str(), -1, wideStrBuffer.data(), wideStrLength);
    LPWSTR lpwStr = _wcsdup(wideStrBuffer.data());
    return UniqueLPWSTR(lpwStr);
}

LPWSTR ConvertToLPWSTR(const std::string& utf8String) {
    int wideStrLength = MultiByteToWideChar(CP_UTF8, 0, utf8String.c_str(), -1, nullptr, 0);
    std::vector<wchar_t> wideStrBuffer(wideStrLength);
    MultiByteToWideChar(CP_UTF8, 0, utf8String.c_str(), -1, wideStrBuffer.data(), wideStrLength);
    LPWSTR lpwStr = _wcsdup(wideStrBuffer.data());
    return lpwStr;
}

v8::Local<v8::Object> mapperPrinterInfoNodeObject(Isolate *isolate, v8::Local<Context> context, PRINTER_INFO_2 *pPrinterItem)
{
    v8::Local<v8::Object> obj = v8::Object::New(isolate);
    OBJECT_SET_STRING(context, obj, "pServerName", pPrinterItem->pServerName == nullptr ? "":pPrinterItem->pServerName)
    OBJECT_SET_STRING(context, obj, "pPrinterName", gbkStringToUtf8String(pPrinterItem->pPrinterName).c_str())
    OBJECT_SET_STRING(context, obj, "pShareName", pPrinterItem->pShareName)
    OBJECT_SET_STRING(context, obj, "pPortName", pPrinterItem->pPortName)
    OBJECT_SET_STRING(context, obj, "pDriverName", pPrinterItem->pDriverName)
    OBJECT_SET_STRING(context, obj, "pComment", pPrinterItem->pComment)
    OBJECT_SET_STRING(context, obj, "pLocation", pPrinterItem->pLocation)
    OBJECT_SET_STRING(context, obj, "pSepFile", pPrinterItem->pSepFile)
    OBJECT_SET_STRING(context, obj, "pDatatype", pPrinterItem->pDatatype)
    OBJECT_SET_STRING(context, obj, "pPrintProcessor", pPrinterItem->pPrintProcessor)
    OBJECT_SET_STRING(context, obj, "pParameters", pPrinterItem->pParameters)
    OBJECT_SET_NUMBER(context, obj, "Attributes", pPrinterItem->Attributes)
    OBJECT_SET_NUMBER(context, obj, "Priority", pPrinterItem->Priority)
    OBJECT_SET_NUMBER(context, obj, "DefaultPriority", pPrinterItem->DefaultPriority)
    OBJECT_SET_NUMBER(context, obj, "StartTime", pPrinterItem->StartTime)
    OBJECT_SET_NUMBER(context, obj, "UntilTime", pPrinterItem->UntilTime)
    OBJECT_SET_NUMBER(context, obj, "Status", pPrinterItem->Status)
    OBJECT_SET_NUMBER(context, obj, "cJobs", pPrinterItem->cJobs)
    OBJECT_SET_NUMBER(context, obj, "AveragePPM", pPrinterItem->AveragePPM)
    return obj;
}