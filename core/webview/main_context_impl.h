#ifndef _BROWSER_MAIN_CONTEXT_IMPL_H_
#define _BROWSER_MAIN_CONTEXT_IMPL_H_
#pragma once

#include "include/base/cef_scoped_ptr.h"
#include "include/base/cef_thread_checker.h"
#include "include/cef_app.h"
#include "include/cef_command_line.h"
#include "main_context.h" 

namespace browser
{

// Used to store global context in the browser process.
class MainContextImpl : public MainContext {
 public:
  MainContextImpl(bool terminate_when_all_windows_closed);

  // MainContext members.
    
  IBrowserClient *GetDevToolsClient() OVERRIDE;
  void SetDevToolsClient(IBrowserClient *pBrowserClient) OVERRIDE;
  std::string GetConsoleLogPath() OVERRIDE;
  std::string GetDownloadPath(const std::string& file_name) OVERRIDE;
  std::string GetAppWorkingDirectory() OVERRIDE;
  std::string GetMainURL() OVERRIDE;   
 

  //return the opened browser count
  int GetOpenedBrowserCount() OVERRIDE; 

  int  AddOpenedBrowserCount() OVERRIDE;

  int  DelOpenedBrowserCount() OVERRIDE;



  // Initialize CEF and associated main context state. This method must be
  // called on the same thread that created this object.
  bool Initialize(const CefMainArgs& main_args, CefRefPtr<CefApp> app, void* windows_sandbox_info);
  // Shut down CEF and associated context state. This method must be called on
  // the same thread that created this object.
  void Shutdown();

 private:
  // Allow deletion via scoped_ptr only.
  friend struct base::DefaultDeleter<MainContextImpl>;

  ~MainContextImpl();

  // Returns true if the context is in a valid state (initialized and not yet
  // shut down).
  bool InValidState() const {
    return initialized_ && !shutdown_;
  }
 
  const bool terminate_when_all_windows_closed_;

  // Track context state. Accessing these variables from multiple threads is
  // safe because only a single thread will exist at the time that they're set
  // (during context initialization and shutdown).
  bool initialized_;
  bool shutdown_;

  std::string main_url_;  

  // Used to verify that methods are called on the correct thread.
  base::ThreadChecker thread_checker_;

  int browser_opened_;

  HWND dev_tools_hwnd_;
  IBrowserClient *dev_tools_client_;

  DISALLOW_COPY_AND_ASSIGN(MainContextImpl);
};

}  // namespace browser

#endif  // _BROWSER_MAIN_CONTEXT_IMPL_H_
