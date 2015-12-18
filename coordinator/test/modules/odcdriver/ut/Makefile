#
# Copyright (c) 2013-2015 NEC Corporation
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

COMMON_STUB_PATH = ../..

EXEC_NAME :=  odcdriver_ut

MODULE_SRCROOT = $(GTEST_SRCROOT)/modules

RESTJSONUTIL_STUBDIR = $(COMMON_STUB_PATH)/stub/restjsonutil
VTNDRVINTF_STUBDIR = $(COMMON_STUB_PATH)/stub/ContrllerFrameworkStub
TCLIB_STUBDIR = $(COMMON_STUB_PATH)/stub/tclib_module
MISC_STUBDIR = $(COMMON_STUB_PATH)/stub/misc

ODCDRIVER_SRCDIR = $(MODULE_SRCROOT)/odcdriver
VTNCACHEUTIL_SRCDIR = $(MODULE_SRCROOT)/vtncacheutil
ALARM_SRCDIR = $(MODULE_SRCROOT)/alarm

# Define a list of directories that contain source files.
ALT_SRCDIRS = $(ODCDRIVER_SRCDIR) $(VTNCACHEUTIL_SRCDIR)  $(RESTJSONUTIL_STUBDIR) $(VTNDRVINTF_STUBDIR)
ALT_SRCDIRS += $(TCLIB_STUBDIR) $(MISC_STUBDIR)

CXX_INCDIRS += core/libs/
UT_INCDIRS_PREP = ${COMMON_STUB_PATH} $(COMMON_STUB_PATH)/stub/include $(COMMON_STUB_PATH)/stub/include/core_include $(COMMON_STUB_PATH)/stub/include/cxx

UTXX_INCDIRS_PREP = $(VTNDRVINTF_STUBDIR)/driver

EXTRA_CXX_INCDIRS = $(MODULE_SRCROOT)
EXTRA_CXX_INCDIRS += $(VTNDRVINTF_STUBDIR)
EXTRA_CXX_INCDIRS += $(RESTJSONUTIL_STUBDIR)
EXTRA_CXX_INCDIRS += $(ODCDRIVER_SRCDIR)/include
EXTRA_CXX_INCDIRS += $(VTNCACHEUTIL_SRCDIR)/include
EXTRA_CXX_INCDIRS += $(TCLIB_STUBDIR)
EXTRA_CXX_INCDIRS += $(ALARM_SRCDIR)/include
EXTRA_CXX_INCDIRS += $(OBJDIR)/../../../../modules/odcdriver/

CPPFLAGS += -include ut_stub.h

ODCDRIVER_SOURCES = odc_controller.cc
ODCDRIVER_SOURCES += odc_mod.cc
ODCDRIVER_SOURCES += odc_vbr.cc
ODCDRIVER_SOURCES += odc_vtn.cc
ODCDRIVER_SOURCES += odc_vbrif.cc
ODCDRIVER_SOURCES += odc_vbr_vlanmap.cc
ODCDRIVER_SOURCES += odc_switch.cc
ODCDRIVER_SOURCES += odc_port.cc
ODCDRIVER_SOURCES += odc_link.cc
ODCDRIVER_SOURCES += odc_rest.cc
ODCDRIVER_SOURCES += odc_util.cc
ODCDRIVER_SOURCES += odc_kt_utils.cc
ODCDRIVER_SOURCES += odc_vtnstation.cc
ODCDRIVER_SOURCES += odc_dataflow_util.cc
ODCDRIVER_SOURCES += odc_dataflow.cc
ODCDRIVER_SOURCES += odc_vterminal.cc
ODCDRIVER_SOURCES += odc_vterminal_if.cc
ODCDRIVER_SOURCES += odc_flowlist.cc
ODCDRIVER_SOURCES += odc_flow_conditions.cc
ODCDRIVER_SOURCES += odc_flow_filter.cc
ODCDRIVER_SOURCES += odc_ctr_dataflow.cc
ODCDRIVER_SOURCES += odc_vtn_dataflow.cc

VTNCACHEUTIL_SOURCES = keytree.cc
VTNCACHEUTIL_SOURCES += confignode.cc
VTNCACHEUTIL_SOURCES += vtn_cache_mod.cc
VTNDRVINTF_STUB_SOURCES += controller_fw.cc
VTNDRVINTF_STUB_SOURCES += vtn_drv_module.cc
#VTNDRVINTF_STUB_SOURCES += vtn_read_value_util.cc

TCLIB_SOURCES = tclib_module.cc
MISC_SOURCES  = ipc_client.cc ipc_server.cc module.cc

#UT_SOURCES  += odc_vbr_if_ut.cc
#UT_SOURCES  += odc_vbr_ut.cc
#UT_SOURCES  += odc_vtn_ut.cc
#UT_SOURCES  += odc_mod_ut.cc
#UT_SOURCES  += odc_controller_ut.cc
#UT_SOURCES  += odc_vbr_vlanmap_ut.cc
#UT_SOURCES  += odc_switch_ut.cc
#UT_SOURCES  += odc_port_ut.cc
#UT_SOURCES  += odc_link_ut.cc

CXX_SOURCES += $(UT_SOURCES)
CXX_SOURCES += $(ODCDRIVER_SOURCES) $(VTNCACHEUTIL_SOURCES) $(TCLIB_SOURCES) $(VTNDRVINTF_STUB_SOURCES)
CXX_SOURCES += $(MISC_SOURCES)

EXTRA_CXXFLAGS  += -fprofile-arcs -ftest-coverage
EXTRA_CXXFLAGS  += -Dprivate=public -Dprotected=public

UNC_LIBS = libpfc_util

EXTRA_CPPFLAGS    += $(JSON_C_CPPFLAGS)
EXTRA_LIBDIRS     += $(JSON_C_LIBDIRS)
EXTRA_LDLIBS      += -lgcov $(JSON_C_LDFLAGS)
EXTRA_RUNTIME_DIR += $(JSON_C_RUNPATH)

include ../../rules.mk
