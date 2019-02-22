#include <node.h>
#include <node_buffer.h>
#include <windows.h>
#include "windriver.h"
#include "install.c"

using namespace v8;
using namespace node;
using namespace std;

std::wstring string_to_wstring(const std::string& str) {
    return std::wstring(str.begin(), str.end());
}

Persistent<Function> WinDriver::constructor;

WinDriver::WinDriver(std::string driverName, std::string deviceName, std::string sysPath, std::string infPath) {
	_driverName = driverName;
	_deviceName = deviceName;
	_sysPath = sysPath;
	_infPath = infPath;
}

WinDriver::~WinDriver() {
	delete _handle;
}
static void freeCharArray (char * bu, void *hint) {
	delete [] bu;
}
void getBufferPointer(const FunctionCallbackInfo<Value>& args) 
{
    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope(isolate);
	if (args.Length() < 1) {
		isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "Wrong number of arguments.")));
		return;
	}
	char* jInputBuf = Buffer::Data(args[0]);
	char* OutputBuffer = new char[8];
	memset(OutputBuffer, 0, 8);
	memcpy(OutputBuffer, &jInputBuf, sizeof(jInputBuf));
	
	args.GetReturnValue().Set(Buffer::New(isolate, OutputBuffer, 8, freeCharArray, NULL).ToLocalChecked());
}

void WinDriver::Init(Local<Object> exports) {
	Isolate* isolate = Isolate::GetCurrent();
	Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);
	tpl->SetClassName(String::NewFromUtf8(isolate, "Driver"));
	tpl->InstanceTemplate()->SetInternalFieldCount(1);
 
	NODE_SET_METHOD(exports, "getBufferPointer", getBufferPointer);
	// Prototype
	NODE_SET_PROTOTYPE_METHOD(tpl, "load", Load);
	NODE_SET_PROTOTYPE_METHOD(tpl, "open", Open);
	NODE_SET_PROTOTYPE_METHOD(tpl, "ioctl", Ioctl);
	NODE_SET_PROTOTYPE_METHOD(tpl, "close", Close);
	NODE_SET_PROTOTYPE_METHOD(tpl, "unload", Unload);
	constructor.Reset(isolate, tpl->GetFunction());
	exports->Set(String::NewFromUtf8(isolate, "Driver"),tpl->GetFunction());
}

void WinDriver::New(const FunctionCallbackInfo<Value>& args) {
	Isolate* isolate = Isolate::GetCurrent();
	if (args.Length() < 2) {
		isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "Wrong number of arguments.")));
		return;
	}
	if (args.IsConstructCall()) {		// Invoked as constructor: `new Process(...)`
		v8::String::Utf8Value param1(args[0]->ToString());
		std::string driverName = std::string(*param1);
		
		v8::String::Utf8Value param2(args[1]->ToString());
		std::string deviceName = std::string(*param2);
		
		v8::String::Utf8Value param3(args[2]->ToString());
		std::string sysPath = std::string(*param3);
		
		std::string infPath;
		
		if(args.Length()>3){
			v8::String::Utf8Value param4(args[3]->ToString());
			infPath = std::string(*param4);
		}
	    WinDriver* obj = new WinDriver(driverName, deviceName, sysPath, infPath);
	    obj->Wrap(args.This());
	    args.GetReturnValue().Set(args.This());
	  } else {// Invoked as plain function `Process(...)`, turn into construct call.
	    const int argc = 4;
	    Local<Value> argv[argc] = { args[0], args[1], args[2], args[3]};
	    Local<Function> cons = Local<Function>::New(isolate, constructor);
	    args.GetReturnValue().Set(cons->NewInstance(argc, argv));
	  }
}

void WinDriver::Load(const FunctionCallbackInfo<Value>& args) 
{
    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope(isolate);
    WinDriver* obj = ObjectWrap::Unwrap<WinDriver>(args.Holder());
	
    DWORD    errNum = 0;
	BOOL     ok;
    HMODULE  library = NULL;
	char buffer[100];
	
    HANDLE   hDevice = CreateFile(obj->_deviceName.c_str(),GENERIC_READ | GENERIC_WRITE,0,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
    if(hDevice == INVALID_HANDLE_VALUE) {
        errNum = GetLastError();
        if (!(errNum == ERROR_FILE_NOT_FOUND ||
                errNum == ERROR_PATH_NOT_FOUND)) {
            sprintf(buffer, "CreateFile failed!  ERROR_FILE_NOT_FOUND = %d\n",errNum);
			isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, buffer)));
            return ;
        }
        library = LoadWdfCoInstaller();
        if (library == NULL) {
            isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "The WdfCoInstaller%s.dll library needs to be in same directory")));
			return;
        }
        ok = ManageDriver( obj->_driverName.c_str(),obj->_sysPath.c_str(),DRIVER_FUNC_INSTALL, string_to_wstring(obj->_infPath).c_str());
        if (!ok) {
            isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "Unable to install driver.")));
			ManageDriver( obj->_driverName.c_str(),obj->_sysPath.c_str(),DRIVER_FUNC_REMOVE, NULL );
            return;
        }

        hDevice = CreateFile( obj->_deviceName.c_str(),GENERIC_READ | GENERIC_WRITE,0,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL );
        if (hDevice == INVALID_HANDLE_VALUE) {
            sprintf(buffer, "Error: CreatFile Failed : %d\n", GetLastError());
			isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, buffer)));
			return;
        }
		if ( library ) {
			UnloadWdfCoInstaller( library );
		}
		CloseHandle ( hDevice );
		args.GetReturnValue().Set(Boolean::New(isolate, TRUE));
		return;
    }
	CloseHandle ( hDevice );
	args.GetReturnValue().Set(Boolean::New(isolate, false));
}

void WinDriver::Open(const FunctionCallbackInfo<Value>& args) 
{
    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope(isolate);
    WinDriver* obj = ObjectWrap::Unwrap<WinDriver>(args.Holder());
	obj->_handle = CreateFile(obj->_deviceName.c_str(),GENERIC_READ | GENERIC_WRITE,0,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL );
	args.GetReturnValue().Set(Boolean::New(isolate, TRUE));
}

void WinDriver::Ioctl(const FunctionCallbackInfo<Value>& args) 
{
	Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope(isolate);
    if (args.Length() < 5) {
		isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "Wrong number of arguments.")));
		return;
	}
	
	WinDriver* obj = ObjectWrap::Unwrap<WinDriver>(args.Holder());
	
	DWORD fileIoType = (DWORD) args[0]->IntegerValue();
	DWORD code = (DWORD) args[1]->IntegerValue();
	DWORD method = (DWORD) args[2]->IntegerValue();
	DWORD fileAccess = (DWORD) args[3]->IntegerValue();
		
	size_t jInputBufLength = Buffer::Length(args[4]);
	char* jInputBuf = Buffer::Data(args[4]);	
	
	size_t outBufSize = args.Length() > 5?(size_t)args[5]->IntegerValue():1000;
	
	DWORD controlCode = CTL_CODE( fileIoType, code, method, fileAccess  );

	char* OutputBuffer = new char[outBufSize];	//sizeof( OutputBuffer)

    BOOL bRc;
    ULONG bytesReturned;

    memset(OutputBuffer, 0, outBufSize);
    bRc = DeviceIoControl (obj->_handle,controlCode,jInputBuf,(DWORD) jInputBufLength,OutputBuffer,outBufSize,&bytesReturned,NULL);
    if ( !bRc )
    {
		isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "DeviceIoControl")));
        return;
    }
	args.GetReturnValue().Set(Buffer::New(isolate, OutputBuffer, bytesReturned, freeCharArray, NULL).ToLocalChecked());
}

void WinDriver::Close(const FunctionCallbackInfo<Value>& args) 
{
    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope(isolate);
    WinDriver* obj = ObjectWrap::Unwrap<WinDriver>(args.Holder());
	if(obj->_handle){
		CloseHandle(obj->_handle);
	}
	args.GetReturnValue().Set(Number::New(isolate, TRUE));
}

void WinDriver::Unload(const FunctionCallbackInfo<Value>& args) 
{
    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope(isolate);
    WinDriver* obj = ObjectWrap::Unwrap<WinDriver>(args.Holder());
    ManageDriver( obj->_driverName.c_str(),obj->_sysPath.c_str(),DRIVER_FUNC_REMOVE, NULL );
	args.GetReturnValue().Set(Number::New(isolate, TRUE));
}
NODE_MODULE(addon, WinDriver::Init)