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

GTEST_SRCROOT := ../../../..
include ../../defs.mk

EXEC_NAME :=  vtndrvintf_ut

MODULE_SRCROOT = $(GTEST_SRCROOT)/modules

CONTROLLER_STUBDIR = stub/ContrllerFrameworkStub
MISC_STUBDIR = stub/ipc_client_server
STUBDIR = stub

VTNCACHEUTIL_SRCDIR = $(GTEST_SRCROOT)/modules/vtncacheutil

# Define a list of directories that contain source files.
ALT_SRCDIRS =  $(VTNCACHEUTIL_SRCDIR) $(CONTROLLER_STUBDIR)
ALT_SRCDIRS += $(MISC_STUBDIR)

UT_INCDIRS_PREP = stub/include stub/include/core_include

UTXX_INCDIRS_PREP = $(CONTROLLER_STUBDIR)/driver

EXTRA_CXX_INCDIRS = $(STUBDIR)
EXTRA_CXX_INCDIRS += $(MODULE_SRCROOT)
EXTRA_CXX_INCDIRS += $(CONTROLLER_STUBDIR)
EXTRA_CXX_INCDIRS += $(VTNCACHEUTIL_SRCDIR)/include
EXTRA_CXX_INCDIRS += $(ALARM_SRCDIR)/include

CPPFLAGS += -include ut_stub.h

VTNCACHEUTIL_SOURCES = keytree.cc
VTNCACHEUTIL_SOURCES += confignode.cc

CONTROLLER_STUB_SOURCES = controller_fw.cc
MISC_STUB_SOURCES = ipc_client.cc ipc_server.cc module.cc

UT_SOURCES = test_request_template.cc

CXX_SOURCES += $(UT_SOURCES)
CXX_SOURCES += $(VTNCACHEUTIL_SOURCES) $(CONTROLLER_STUB_SOURCES)
CXX_SOURCES += $(MISC_STUB_SOURCES)

EXTRA_CXXFLAGS += -fprofile-arcs -ftest-coverage
EXTRA_CXXFLAGS += -Dprivate=public -Dprotected=public

UNC_LIBS = libpfc_util
EXTRA_LDLIBS += -lgcov

include ../../rules.mk
