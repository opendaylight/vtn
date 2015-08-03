#
# Copyright (c) 2013-2015 NEC Corporation
# All rights reserved.
#
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

##
## Makefile that run the unit tests for tc.
##

GTEST_SRCROOT := ../../../..
include ../../defs.mk

COMMON_STUB_PATH = ../..

EXEC_NAME := tc_ut

MODULE_SRCROOT = $(GTEST_SRCROOT)/modules
TC_SRCDIR = $(MODULE_SRCROOT)/tc
ALARM_SRCDIR = $(MODULE_SRCROOT)/alarm
STUB_DIR = $(COMMON_STUB_PATH)/stub

# Define a list of directories that contain source files.
ALT_SRCDIRS = $(TC_SRCDIR) $(STUB_DIR) $(ALARM_SRCDIR)

CXX_INCDIRS += core/include/cxx
CXX_INCDIRS += core/libs/

UT_INCDIRS_PREP = ${COMMON_STUB_PATH}

EXTRA_CXX_INCDIRS = $(STUB_DIR)/include
EXTRA_CXX_INCDIRS += $(MODULE_SRCROOT)
EXTRA_CXX_INCDIRS += $(MODULE_SRCROOT)/alarm/include
EXTRA_CXX_INCDIRS += $(MODULE_SRCROOT)/launcher/include
EXTRA_CXX_INCDIRS += $(MODULE_SRCROOT)/clstat/include
EXTRA_CXX_INCDIRS += $(GTEST_SRCROOT)/include/unc
EXTRA_CXX_INCDIRS += $(GTEST_SRCROOT)/include/
EXTRA_CXX_INCDIRS += $(GTEST_SRCROOT)/include/uncxx
EXTRA_CXX_INCDIRS += $(TC_SRCDIR)/include

CPPFLAGS += -include ut_stub.h

TC_SOURCES = tcmsg.cc
TC_SOURCES += tcmsg_audit.cc
TC_SOURCES += tcmsg_commit.cc
TC_SOURCES += tc_operations.cc
TC_SOURCES += tc_startup_operations.cc
TC_SOURCES += tc_lock.cc
TC_SOURCES += tc_read_operations.cc
TC_SOURCES += tc_audit_operations.cc
TC_SOURCES += tc_autosave_operations.cc
TC_SOURCES += tc_config_operations.cc
TC_SOURCES += tc_db_operations.cc
TC_SOURCES += tc_db_handler.cc
TC_SOURCES += tc_taskq.cc
TC_SOURCES += tc_candidate_operations.cc
TC_SOURCES += tc_read_status_operations.cc
ALARM_SOURCES = alarm.cc

STUB_SOURCES = stub_oper_utils.cc
STUB_SOURCES += stub_tcmsg_utils.cc

UT_SOURCES = test_tcdboperations.cc
UT_SOURCES += test_tcautosaveoperations.cc
UT_SOURCES += test_tcauditoperations.cc
UT_SOURCES += test_tcstartupoperations.cc
UT_SOURCES += test_tcconfigoperations.cc
UT_SOURCES += test_tcmsg.cc
UT_SOURCES += test_tcoperations.cc
UT_SOURCES += test_tcreadoperations.cc

MAIN_SRC = gtest_main.cc

CXX_SOURCES += $(UT_SOURCES)
CXX_SOURCES += $(TC_SOURCES)
CXX_SOURCES += $(ALARM_SOURCES)
CXX_SOURCES += $(STUB_SOURCES)
CXX_SOURCES += $(MAIN_SRC)

EXTRA_CXXFLAGS = -fprofile-arcs -ftest-coverage
EXTRA_CXXFLAGS += -Dprivate=public -Dprotected=public

UNC_LIBS = libpfc_util
UNC_LIBS += libpfcxx
UNC_LIBS += libpfc
UNC_LIBS += libpfc_ipcsrv
UNC_LIBS += libpfc_ipcclnt
UNC_LIBS += libpfcxx_ipcsrv
UNC_LIBS += libpfcxx_ipcclnt
UNC_LIBS += libtc_common 
EXTRA_LDLIBS += -lgcov -lodbc

include ../../rules.mk
