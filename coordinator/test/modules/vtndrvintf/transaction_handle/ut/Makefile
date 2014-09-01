#
# Copyright (c) 2013 NEC Corporation
# All rights reserved.
#
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

##
## Makefile that run the unit tests for VTNDRVINTF.
##

GTEST_SRCROOT := ../../../../..
include ../../../defs.mk

COMMON_STUB_PATH = ../../..


EXEC_NAME := transaction_handler_ut

MODULE_SRCROOT = $(GTEST_SRCROOT)/modules

TCLIBMODULE_STUBDIR = $(COMMON_STUB_PATH)/stub/tclib_module
CONTROLLER_STUBDIR = $(COMMON_STUB_PATH)/stub/ContrllerFrameworkStub
MISC_STUBDIR = $(COMMON_STUB_PATH)/stub/misc

VTNCACHEUTIL_SRCDIR = $(MODULE_SRCROOT)/vtncacheutil
VTNDRVINTF_SRCDIR = $(MODULE_SRCROOT)/vtndrvintf
RESTJSONUTIL_SRCDIR = $(MODULE_SRCROOT)/restjsonutil

# Define a list of directories that contain source files.
ALT_SRCDIRS =  $(VTNCACHEUTIL_SRCDIR) $(RESTJSONUTIL_SRCDIR) $(CONTROLLER_STUBDIR) $(VTNDRVINTF_SRCDIR) $(TCLIBMODULE_STUBDIR)
ALT_SRCDIRS += $(MISC_STUBDIR)

CXX_INCDIRS += core/libs/
UT_INCDIRS_PREP = ${COMMON_STUB_PATH} $(COMMON_STUB_PATH)/stub/include $(COMMON_STUB_PATH)/stub/include/core_include $(COMMON_STUB_PATH)/stub/include/cxx

UTXX_INCDIRS_PREP = $(CONTROLLER_STUBDIR)/driver

EXTRA_CXX_INCDIRS += $(MODULE_SRCROOT)
EXTRA_CXX_INCDIRS += $(CONTROLLER_STUBDIR)
EXTRA_CXX_INCDIRS += $(TCLIBMODULE_STUBDIR)
EXTRA_CXX_INCDIRS += $(VTNCACHEUTIL_SRCDIR)/include
EXTRA_CXX_INCDIRS += $(RESTJSONUTIL_SRCDIR)/include
EXTRA_CXX_INCDIRS += $(VTNDRVINTF_SRCDIR)/include
EXTRA_CXX_INCDIRS += $(ALARM_SRCDIR)/include

CPPFLAGS += -include ut_stub.h

VTNCACHEUTIL_SOURCES = keytree.cc
VTNCACHEUTIL_SOURCES += confignode.cc
VTNCACHEUTIL_SOURCES += vtn_cache_mod.cc
VTNDRVINTF_SOURCES = vtn_drv_transaction_handle.cc
RESTJSONUTIL_SOURCES = http_client.cc json_build_parse.cc rest_client.cc
MISC_SOURCES = ipc_client.cc ipc_server.cc module.cc audit_keytypes.cc
CONTROLLER_STUB_SOURCES = vtn_drv_module.cc controller_fw.cc
TCLIBMODULE_STUBSOURCES = tclib_module.cc

UT_SOURCES = vtn_drv_transaction_handle_ut.cc

CXX_SOURCES += $(UT_SOURCES)
CXX_SOURCES += $(RESTJSONUTIL_SOURCES) $(VTNCACHEUTIL_SOURCES) $(CONTROLLER_STUB_SOURCES) $(VTNDRVINTF_SOURCES) $(TCLIBMODULE_STUBSOURCES)
CXX_SOURCES += $(MISC_SOURCES)

EXTRA_CXXFLAGS += -fprofile-arcs -ftest-coverage
EXTRA_CXXFLAGS += -Dprivate=public -Dprotected=public

UNC_LIBS = libpfc_util

EXTRA_CPPFLAGS += $(JSON_C_CPPFLAGS)
EXTRA_LIBDIRS += $(JSON_C_LIBDIRS)
EXTRA_LDLIBS += -lgcov $(JSON_C_LDFLAGS)
EXTRA_RUNTIME_DIR += $(JSON_C_RUNPATH)
EXTRA_LDLIBS += -lcurl
include ../../../rules.mk
