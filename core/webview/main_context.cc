#include "main_context.h"
#include "include/base/cef_logging.h"

namespace browser
{

namespace {

MainContext* g_main_context = NULL;

}  // namespace

// static
MainContext* MainContext::Get() {
  DCHECK(g_main_context);
  return g_main_context;
}

MainContext::MainContext() {
  DCHECK(!g_main_context);
  g_main_context = this;

}

MainContext::~MainContext() {
  g_main_context = NULL;
}

}  // namespace browser
