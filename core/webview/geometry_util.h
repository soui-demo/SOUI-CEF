#ifndef _BROWSER_GEOMETRY_UTIL_H_
#define _BROWSER_GEOMETRY_UTIL_H_
#pragma once

#include "include/internal/cef_types_wrappers.h"

namespace browser {

// Convert |value| from logical coordinates to device coordinates.
int LogicalToDevice(int value, float device_scale_factor);
CefRect LogicalToDevice(const CefRect& value, float device_scale_factor);

// Convert |value| from device coordinates to logical coordinates.
int DeviceToLogical(int value, float device_scale_factor);
void DeviceToLogical(CefMouseEvent& value, float device_scale_factor);

}  // namespace browser

#endif  // _BROWSER_GEOMETRY_UTIL_H_
