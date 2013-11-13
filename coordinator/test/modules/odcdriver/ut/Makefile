#
# Copyright (c) 2013 NEC Corporation
# All rights reserved.
#
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

##
## Makefile that run the unit tests for ODCDRIVER.
##

GTEST_SRCROOT := ../../../..
include ../../defs.mk

EXEC_NAME :=  odcdriver_ut

MODULE_SRCROOT = $(GTEST_SRCROOT)/modules

RESTJSONUTIL_STUBDIR = stub/restjsonutil
VTNDRVINTF_STUBDIR = stub/vtndrvintf
TCLIB_STUBDIR = stub/tclib_module
MISC_STUBDIR = stub/misc
STUBDIR = stub

ODCDRIVER_SRCDIR = $(MODULE_SRCROOT)/odcdriver
VTNCACHEUTIL_SRCDIR = $(MODULE_SRCROOT)/vtncacheutil
RESTJSONUTIL_SRCDIR = $(MODULE_SRCROOT)/restjsonutil
TCLIB_SRCDIR = $(MODULE_SRCROOT)/tclib
ALARM_SRCDIR = $(MODULE_SRCROOT)/alarm

# Define a list of directories that contain source files.
ALT_SRCDIRS = $(ODCDRIVER_SRCDIR) $(VTNCACHEUTIL_SRCDIR)  $(RESTJSONUTIL_STUBDIR) $(VTNDRVINTF_STUBDIR)
ALT_SRCDIRS += $(TCLIB_STUBDIR) $(MISC_STUBDIR)

UT_INCDIRS_PREP = stub/include stub/include/core_include

EXTRA_CXX_INCDIRS = $(MODULE_SRCROOT)
EXTRA_CXX_INCDIRS += $(STUBDIR)
EXTRA_CXX_INCDIRS += $(VTNDRVINTF_STUBDIR)
EXTRA_CXX_INCDIRS += $(RESTJSONUTIL_STUBDIR)
EXTRA_CXX_INCDIRS += $(ODCDRIVER_SRCDIR)/include
EXTRA_CXX_INCDIRS += $(VTNCACHEUTIL_SRCDIR)/include
EXTRA_CXX_INCDIRS += $(TCLIB_SRCDIR)/include
EXTRA_CXX_INCDIRS += $(ALARM_SRCDIR)/include

CPPFLAGS += -include ut_stub.h

ODCDRIVER_SOURCES = odc_controller.cc
ODCDRIVER_SOURCES += odc_mod.cc
ODCDRIVER_SOURCES += odc_vbr.cc
ODCDRIVER_SOURCES += odc_vtn.cc
ODCDRIVER_SOURCES += odc_vbrif.cc

VTNCACHEUTIL_SOURCES = keytree.cc
VTNCACHEUTIL_SOURCES += confignode.cc

TCLIB_SOURCES = tclib_module.cc
MISC_SOURCES  = ipc_client.cc ipc_server.cc module.cc

UT_SOURCES  += odc_vbr_if_ut.cc
UT_SOURCES  += odc_vbr_ut.cc
UT_SOURCES  += odc_vtn_ut.cc
UT_SOURCES  += odc_mod_ut.cc
UT_SOURCES  += odc_controller_ut.cc

CXX_SOURCES += $(UT_SOURCES)
CXX_SOURCES += $(ODCDRIVER_SOURCES) $(VTNCACHEUTIL_SOURCES) $(TCLIB_SOURCES)
CXX_SOURCES += $(MISC_SOURCES)

EXTRA_CXXFLAGS  += -fprofile-arcs -ftest-coverage
EXTRA_CXXFLAGS  += -Dprivate=public -Dprotected=public

UNC_LIBS = libpfc_util
EXTRA_LDLIBS += -lgcov -ljson

include ../../rules.mk
