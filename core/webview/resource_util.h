#ifndef _BROWSER_RESOURCE_UTIL_H_
#define _BROWSER_RESOURCE_UTIL_H_
#pragma once

#include <string>
#include "include/cef_stream.h"

#if defined(OS_WIN)
#include "include/wrapper/cef_resource_manager.h"
#endif

namespace browser 
{


// Returns the contents of the CefRequest as a string.
std::string DumpRequestContents(CefRefPtr<CefRequest> request);

std::string GetTimeString(const CefTime& value);

std::string GetCertStatusString(cef_cert_status_t status);

std::string GetBinaryString(CefRefPtr<CefBinaryValue> value);

// Retrieve a resource as a string.
bool LoadBinaryResource(const char* resource_name, std::string& resource_data);

// Retrieve a resource as a steam reader.
CefRefPtr<CefStreamReader> GetBinaryResourceReader(const char* resource_name);

#if defined(OS_WIN)
// Create a new provider for loading binary resources.
CefResourceManager::Provider* CreateBinaryResourceProvider(
    const std::string& url_path);
#endif

}  // namespace browser

#endif  // _BROWSER_RESOURCE_UTIL_H_
