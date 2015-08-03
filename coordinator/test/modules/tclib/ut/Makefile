#
# Copyright (c) 2013-2015 NEC Corporation
# All rights reserved.
#
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

##
## Makefile that run the unit tests for tclib.
##

GTEST_SRCROOT := ../../../..
include ../../defs.mk

COMMON_STUB_PATH = ../..

EXEC_NAME  :=  tclib_ut

MODULE_SRCROOT = $(GTEST_SRCROOT)/modules
TCLIB_SRCDIR = $(MODULE_SRCROOT)/tclib
MISC_STUBDIR = $(COMMON_STUB_PATH)/stub/misc
STUB_DIR = $(COMMON_STUB_PATH)/stub
#TCLIB_STUBDIR = $(COMMON_STUB_PATH)/stub/tclib_module

# Define a list of directories that contain source files.
ALT_SRCDIRS =  $(TCLIB_SRCDIR) $(STUB_DIR) $(MISC_STUBDIR) #$(TCLIB_STUBDIR)

CXX_INCDIRS  +=  core/include/cxx
CXX_INCDIRS  +=  core/libs/

UT_INCDIRS_PREP = ${COMMON_STUB_PATH}

EXTRA_CXX_INCDIRS = $(STUB_DIR)/include
EXTRA_CXX_INCDIRS = $(STUB_DIR)/include/cxx/pfcxx
#EXTRA_CXX_INCDIRS += $(TCLIB_STUBDIR)
EXTRA_CXX_INCDIRS += $(MODULE_SRCROOT)
EXTRA_CXX_INCDIRS += $(GTEST_SRCROOT)/include/unc
EXTRA_CXX_INCDIRS += $(GTEST_SRCROOT)/include/uncxx
EXTRA_CXX_INCDIRS += $(TCLIB_SRCDIR)/include

CPPFLAGS += -include ut_stub.h

TCLIB_SOURCES = tclib_module.cc
TCLIB_SOURCES += tclib_msg_util.cc

STUB_SOURCES = tclib_interface_stub.cc
STUB_SOURCES += stub_serversess.cc

MISC_STUB_SOURCES = module.cc

UT_SOURCES = tclib_common.cc
UT_SOURCES += audit_operations.cc
UT_SOURCES += commit_driver.cc
UT_SOURCES += api_test.cc
UT_SOURCES += validations.cc
UT_SOURCES += msg_util_test.cc

MAIN_SRC = gtest_main.cc

CXX_SOURCES += $(UT_SOURCES)
CXX_SOURCES += $(TCLIB_SOURCES)
CXX_SOURCES += $(STUB_SOURCES)
CXX_SOURCES += $(MISC_STUB_SOURCES)
CXX_SOURCES += $(MAIN_SRC)

EXTRA_CXXFLAGS = -fprofile-arcs -ftest-coverage
EXTRA_CXXFLAGS += -Dprivate=public -Dprotected=public

UNC_LIBS = libpfc_util
UNC_LIBS += libpfc_ipcsrv
EXTRA_LDLIBS += -lgcov

include ../../rules.mk
