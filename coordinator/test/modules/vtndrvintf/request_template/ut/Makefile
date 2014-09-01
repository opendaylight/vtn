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

GTEST_SRCROOT := ../../../../../
include ../../../defs.mk

COMMON_STUB_PATH = ../../..

EXEC_NAME :=  vtndrvintf_ut

MODULE_SRCROOT = $(GTEST_SRCROOT)/modules

TCLIBMODULE_STUBDIR = $(COMMON_STUB_PATH)/stub/tclib_module
CONTROLLER_STUBDIR = $(COMMON_STUB_PATH)/stub/ContrllerFrameworkStub
MISC_STUBDIR = $(COMMON_STUB_PATH)/stub/misc
STUBDIR = $(COMMON_STUB_PATH)/stub

VTNCACHEUTIL_SRCDIR = $(GTEST_SRCROOT)/modules/vtncacheutil

# Define a list of directories that contain source files.
ALT_SRCDIRS =  $(VTNCACHEUTIL_SRCDIR) $(CONTROLLER_STUBDIR) $(TCLIBMODULE_STUBDIR)
ALT_SRCDIRS += $(MISC_STUBDIR)

CXX_INCDIRS += core/libs/
UT_INCDIRS_PREP = ${COMMON_STUB_PATH} $(COMMON_STUB_PATH)/stub/include $(COMMON_STUB_PATH)/stub/include/core_include $(COMMON_STUB_PATH)/stub/include/cxx

UTXX_INCDIRS_PREP = $(CONTROLLER_STUBDIR)/driver

EXTRA_CXX_INCDIRS = $(STUBDIR)
EXTRA_CXX_INCDIRS += $(MODULE_SRCROOT)
EXTRA_CXX_INCDIRS += $(TCLIBMODULE_STUBDIR)
EXTRA_CXX_INCDIRS += $(CONTROLLER_STUBDIR)
EXTRA_CXX_INCDIRS += $(VTNCACHEUTIL_SRCDIR)/include
EXTRA_CXX_INCDIRS += $(ALARM_SRCDIR)/include

CPPFLAGS += -include ut_stub.h

VTNCACHEUTIL_SOURCES = keytree.cc
VTNCACHEUTIL_SOURCES += confignode.cc
VTNCACHEUTIL_SOURCES += vtn_cache_mod.cc

CONTROLLER_STUB_SOURCES = vtn_drv_module.cc controller_fw.cc
MISC_STUB_SOURCES = ipc_client.cc ipc_server.cc module.cc audit_keytypes.cc
#TCLIBMODULE_STUBSOURCES = tclib_module.cc

UT_SOURCES = test_request_template.cc

CXX_SOURCES += $(UT_SOURCES)
CXX_SOURCES += $(VTNCACHEUTIL_SOURCES) $(CONTROLLER_STUB_SOURCES) $(TCLIBMODULE_STUBSOURCES)
CXX_SOURCES += $(MISC_STUB_SOURCES)

EXTRA_CXXFLAGS += -fprofile-arcs -ftest-coverage
EXTRA_CXXFLAGS += -Dprivate=public -Dprotected=public

UNC_LIBS = libpfc_util
EXTRA_LDLIBS += -lgcov

include ../../../rules.mk
