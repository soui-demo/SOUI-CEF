#ifndef _BROWSER_CUSTOMER_SCHEME_H_
#define _BROWSER_CUSTOMER_SCHEME_H_
#pragma once

#include "include/cef_scheme.h"

namespace browser {
namespace scheme {

// Create and register the custom scheme handler. See
// common/scheme_handler_common.h for registration of the custom scheme
// name/type which must occur in all processes. Called from test_runner.cc.
void RegisterSchemeHandlers();

// Register the custom scheme name/type. This must be done in all processes.
// handler which only occurs in the browser process. Called from
// client_app_delegates_common.cc.
void RegisterCustomSchemes(CefRawPtr<CefSchemeRegistrar> registrar, std::vector<CefString>& cookiable_schemes);

}  // namespace scheme
}  // namespace browser

#endif  // _BROWSER_CUSTOMER_SCHEME_H_
