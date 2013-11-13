#
# Copyright (c) 2013 NEC Corporation
# All rights reserved.
#
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

##
## Makefile that run the unit tests for RESTJSONUTIL.
##

GTEST_SRCROOT	:= ../../../..
include ../../defs.mk

EXEC_NAME	:=  restjsonutil_ut

MODULE_SRCROOT	= $(GTEST_SRCROOT)/modules

STUBDIR	= stub

RESTJSONUTIL_SRCDIR	= $(MODULE_SRCROOT)/restjsonutil

# Define a list of directories that contain source files.
ALT_SRCDIRS	= $(RESTJSONUTIL_SRCDIR)

UT_INCDIRS_PREP		= stub/include stub/include/core_include

EXTRA_CXX_INCDIRS	= $(STUBDIR)
EXTRA_CXX_INCDIRS	+= $(MODULE_SRCROOT)
EXTRA_CXX_INCDIRS	+= $(RESTJSONUTIL_SRCDIR)/include

CPPFLAGS	+= -include ut_stub.h

RESTJSONUTIL_SOURCES	= http_client.cc 
RESTJSONUTIL_SOURCES	+= json_build_parse.cc 
RESTJSONUTIL_SOURCES	+= rest_client.cc 

UT_SOURCES	= jsonbuildparse_ut.cc
UT_SOURCES	+= restclient_ut.cc

CXX_SOURCES	+= $(UT_SOURCES)
CXX_SOURCES	+= $(RESTJSONUTIL_SOURCES)

EXTRA_CXXFLAGS	+= -fprofile-arcs -ftest-coverage
EXTRA_CXXFLAGS	+= -Dprivate=public -Dprotected=public

UNC_LIBS	= libpfc_util #libpfc libpfc_ipcsrv libpfc_ipcclnt
#UNC_LIBS	+= libpfcxx libpfcxx_ipcsrv libpfcxx_ipcclnt
EXTRA_LDLIBS	+= -lgcov -ljson

include ../../rules.mk
