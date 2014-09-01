#
# Copyright (c) 2013 NEC Corporation
# All rights reserved.
#
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

##
## Makefile that run the unit tests for CONTROLLER.
##

GTEST_SRCROOT := ../../../../..
include ../../../defs.mk

COMMON_STUB_PATH = ../../..

EXEC_NAME :=  controller_ut

MODULE_SRCROOT = $(GTEST_SRCROOT)/modules

TCLIBMODULE_STUBDIR = $(COMMON_STUB_PATH)/stub/tclib_module
ODCDRIVER_STUBDIR = $(COMMON_STUB_PATH)/stub/odcdriver
VTNCACHEUTIL_STUBDIR = $(COMMON_STUB_PATH)/stub/vtncacheutil
MISC_STUBDIR = $(COMMON_STUB_PATH)/stub/misc

CONTROLLER_SRCDIR = $(MODULE_SRCROOT)/vtndrvintf
ALARM_SRCDIR = $(MODULE_SRCROOT)/alarm

# Define a list of directories that contain source files.
ALT_SRCDIRS = $(CONTROLLER_SRCDIR) $(TCLIBMODULE_STUBDIR) $(ODCDRIVER_STUBDIR) $(VTNCACHEUTIL_STUBDIR)
ALT_SRCDIRS += $(MISC_STUBDIR)

CXX_INCDIRS += core/libs/
UT_INCDIRS_PREP = ${COMMON_STUB_PATH} $(COMMON_STUB_PATH)/stub/include $(COMMON_STUB_PATH)/stub/include/core_include $(COMMON_STUB_PATH)/stub/include/cxx

EXTRA_CXX_INCDIRS = $(MODULE_SRCROOT)
EXTRA_CXX_INCDIRS += $(GTEST_SRCROOT)/include/uncxx
EXTRA_CXX_INCDIRS += $(TCLIBMODULE_STUBDIR)
EXTRA_CXX_INCDIRS += $(ODCDRIVER_STUBDIR)
EXTRA_CXX_INCDIRS += $(CONTROLLER_SRCDIR)/include
EXTRA_CXX_INCDIRS += $(CONTROLLER_SRCDIR)/include/driver
EXTRA_CXX_INCDIRS += $(VTNCACHEUTIL_STUBDIR)/include
EXTRA_CXX_INCDIRS += $(ALARM_SRCDIR)/include

CPPFLAGS += -include ut_stub.h

CONTROLLER_SOURCES = controller_fw.cc

#TCLIBMODULE_STUBSOURCES += tclib_module.cc
ODCDRIVER_STUB_SOURCES = odc_controller.cc odc_drv.cc
VTNCACHEUTIL_STUB_SOURCES += keytree.cc confignode.cc
MISC_SOURCES = ipc_client.cc ipc_server.cc module.cc

UT_SOURCES = controller_fw_ut.cc

CXX_SOURCES += $(UT_SOURCES)
CXX_SOURCES += $(CONTROLLER_SOURCES) $(TCLIBMODULE_STUBSOURCES) $(ODCDRIVER_STUB_SOURCES) $(VTNCACHEUTIL_STUB_SOURCES)
CXX_SOURCES += $(MISC_SOURCES)

EXTRA_CXXFLAGS += -fprofile-arcs -ftest-coverage
EXTRA_CXXFLAGS += -Dprivate=public -Dprotected=public

UNC_LIBS = libpfc_util
EXTRA_LDLIBS += -lgcov

include ../../../rules.mk
