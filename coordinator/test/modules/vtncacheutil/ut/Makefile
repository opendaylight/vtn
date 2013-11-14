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

EXEC_NAME :=  vtncacheutil_ut

MODULE_SRCROOT = $(GTEST_SRCROOT)/modules

VTNCACHEUTIL_SRCDIR = $(MODULE_SRCROOT)/vtncacheutil

# Define a list of directories that contain source files.
ALT_SRCDIRS += $(VTNCACHEUTIL_SRCDIR)

EXTRA_CXX_INCDIRS = $(MODULE_SRCROOT)
EXTRA_CXX_INCDIRS += $(VTNCACHEUTIL_SRCDIR)/include

VTNCACHEUTIL_SOURCES = keytree.cc
VTNCACHEUTIL_SOURCES += confignode.cc

UT_SOURCES = test_keytree.cc
UT_SOURCES += test_confignode.cc

CXX_SOURCES += $(UT_SOURCES)
CXX_SOURCES += $(VTNCACHEUTIL_SOURCES)

EXTRA_CXXFLAGS += -fprofile-arcs -ftest-coverage
EXTRA_CXXFLAGS += -Dprivate=public -Dprotected=public

UNC_LIBS = libpfc_util
EXTRA_LDLIBS += -lgcov

include ../../rules.mk
