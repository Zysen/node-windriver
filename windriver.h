#ifndef Driver_H
#define Driver_H
 
#include <windows.h>
 
#include <node.h>
#include <node_object_wrap.h>
 
class WinDriver : public node::ObjectWrap {
 public:
  static void Init(v8::Handle<v8::Object> exports);
 
 private:
  explicit WinDriver(std::string driverName, std::string deviceName, std::string sysPath, std::string infPath);
  ~WinDriver();
 
  HANDLE _handle;
  std::string _driverName;
  std::string _deviceName;
  std::string _sysPath;
  std::string _infPath;
  
  static void New(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void Load(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void Open(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void Ioctl(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void Close(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void Unload(const v8::FunctionCallbackInfo<v8::Value>& args);
  static v8::Persistent<v8::Function> constructor;
  
};
 
#endif