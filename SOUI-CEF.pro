TEMPLATE = subdirs
TARGET = soui-cef
CONFIG(x64){
TARGET = $$TARGET"64"
}

SUBDIRS += libcef_dll_wrapper
SUBDIRS += core
SUBDIRS += webprocess
SUBDIRS += main
