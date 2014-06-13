#
# Copyright (c) 2013 NEC Corporation
# All rights reserved.
#
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

##
## Makefile that run the unit tests for VTNCACHEUTIL.
##

GTEST_SRCROOT := ../../../..
include ../../defs.mk

COMMON_STUB_PATH = ../..
EXEC_NAME :=  vtncacheutil_ut

MODULE_SRCROOT = $(GTEST_SRCROOT)/modules

MISC_STUBDIR = $(COMMON_STUB_PATH)/stub/misc

VTNCACHEUTIL_SRCDIR = $(MODULE_SRCROOT)/vtncacheutil

# Define a list of directories that contain source files.
ALT_SRCDIRS += $(VTNCACHEUTIL_SRCDIR) $(MISC_STUBDIR)


UT_INCDIRS_PREP = ${COMMON_STUB_PATH} $(COMMON_STUB_PATH)/stub/include $(COMMON_STUB_PATH)/stub/include/cxx

EXTRA_CXX_INCDIRS = $(MODULE_SRCROOT)
EXTRA_CXX_INCDIRS += $(VTNCACHEUTIL_SRCDIR)/include

CPPFLAGS += -include ut_stub.h

VTNCACHEUTIL_SOURCES = keytree.cc
VTNCACHEUTIL_SOURCES += confignode.cc
VTNCACHEUTIL_SOURCES += vtn_cache_mod.cc

UT_SOURCES = test_keytree.cc
UT_SOURCES += test_confignode.cc

MISC_SOURCES  = module.cc

CXX_SOURCES += $(UT_SOURCES)
CXX_SOURCES += $(VTNCACHEUTIL_SOURCES)
CXX_SOURCES += $(MISC_SOURCES)

EXTRA_CXXFLAGS += -fprofile-arcs -ftest-coverage
EXTRA_CXXFLAGS += -Dprivate=public -Dprotected=public

UNC_LIBS = libpfc_util
EXTRA_LDLIBS += -lgcov

include ../../rules.mk
