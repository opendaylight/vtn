#
# Copyright (c) 2013-2015 NEC Corporation
# All rights reserved.
#
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

##
## Makefile that run the unit tests for UPPL.
##

GTEST_SRCROOT	:= ../../../..
include ../../defs.mk

EXEC_NAME	:=  uppl_ut

COMMON_STUB_PATH = ../..

MODULE_SRCROOT	= $(GTEST_SRCROOT)/modules

ODBC_STUBDIR	  = $(COMMON_STUB_PATH)/stub/ODBC
CLSTAT_STUBDIR	= $(COMMON_STUB_PATH)/stub/clstat
TCLIB_STUBDIR	  = $(COMMON_STUB_PATH)/stub/tclib_module
MISC_STUBDIR	  = $(COMMON_STUB_PATH)/stub/misc

UPPL_SRCDIR	    = $(MODULE_SRCROOT)/uppl
TCLIB_SRCDIR	  = $(MODULE_SRCROOT)/tclib
ALARM_SRCDIR	  = $(MODULE_SRCROOT)/alarm
CAPA_SRCDIR	    = $(MODULE_SRCROOT)/capa

# Define a list of directories that contain source files.
ALT_SRCDIRS	= $(UPPL_SRCDIR) $(ODBC_STUBDIR) $(TCLIB_STUBDIR)
ALT_SRCDIRS	+= $(MISC_STUBDIR)

CXX_INCDIRS += core/libs/
UT_INCDIRS_PREP  =  ${COMMON_STUB_PATH} $(COMMON_STUB_PATH)/stub/include $(COMMON_STUB_PATH)/stub/include/core_include
UT_INCDIRS_PREP += $(CLSTAT_STUBDIR)

UTXX_INCDIRS_PREP	= $(TCLIB_STUBDIR)
UTXX_INCDIRS_PREP	+= $(ODBC_STUBDIR)/include

EXTRA_CXX_INCDIRS	= $(MODULE_SRCROOT)
EXTRA_CXX_INCDIRS	+= $(UPPL_SRCDIR)/include
EXTRA_CXX_INCDIRS	+= $(TCLIB_SRCDIR)/include
EXTRA_CXX_INCDIRS	+= $(ALARM_SRCDIR)/include
EXTRA_CXX_INCDIRS	+= $(CAPA_SRCDIR)/.
EXTRA_CXX_INCDIRS	+= $(CAPA_SRCDIR)/include

CPPFLAGS	+= -include ut_stub.h

# Define a list of cfdef files to be compiled.
#ALT_CFDEF_FILES	= $(UPPL_SRCDIR)/uppl_ctr_capability.cfdef

UPPL_SOURCES	= ipc_client_configuration_handler.cc 
UPPL_SOURCES	+= ipc_client_logical_handler.cc 
UPPL_SOURCES	+= ipc_connection_manager.cc 
UPPL_SOURCES	+= ipc_server_handler.cc 
UPPL_SOURCES	+= ipct_util.cc 
UPPL_SOURCES	+= itc_audit_request.cc 
UPPL_SOURCES	+= itc_configuration_request.cc 
UPPL_SOURCES	+= itc_db_config.cc 
UPPL_SOURCES	+= itc_import_request.cc 
UPPL_SOURCES	+= itc_kt_base.cc 
UPPL_SOURCES	+= itc_kt_boundary.cc 
UPPL_SOURCES	+= itc_kt_controller.cc 
UPPL_SOURCES	+= itc_kt_ctr_domain.cc 
UPPL_SOURCES	+= itc_kt_link.cc 
UPPL_SOURCES	+= itc_kt_logical_member_port.cc 
UPPL_SOURCES	+= itc_kt_logicalport.cc 
UPPL_SOURCES	+= itc_kt_port.cc 
UPPL_SOURCES	+= itc_kt_root.cc 
UPPL_SOURCES	+= itc_kt_state_base.cc 
UPPL_SOURCES	+= itc_kt_switch.cc 
UPPL_SOURCES	+= itc_notification_request.cc 
UPPL_SOURCES	+= itc_read_request.cc 
UPPL_SOURCES	+= itc_state_change.cc 
UPPL_SOURCES	+= itc_transaction_request.cc 
UPPL_SOURCES	+= odbcm_db_tableschema.cc
UPPL_SOURCES	+= phy_util.cc 
UPPL_SOURCES	+= physical_core.cc 
UPPL_SOURCES	+= physical_itc.cc 
UPPL_SOURCES	+= physical_notification_manager.cc 
UPPL_SOURCES	+= unc_state_handler.cc 
UPPL_SOURCES	+= physicallayer.cc
UPPL_SOURCES  += itc_kt_ctr_dataflow.cc
UPPL_SOURCES  += itc_kt_dataflow.cc
UPPL_SOURCES  += physical_taskq.cc
UPPL_SOURCES  += itc_kt_port_neighbor.cc
UPPL_SOURCES  += itc_kt_dataflow_v2.cc
ODBC_SOURCES	= odbcm_mgr_stub.cc odbcm_utils_stub.cc
TCLIB_SOURCES	= tclib_module.cc
MISC_SOURCES	= ipc_client.cc ipc_server.cc module.cc alarm.cc

UT_SOURCES	= Boundary_ut.cc
UT_SOURCES	+= Controller_ut.cc
UT_SOURCES	+= Domain_ut.cc
UT_SOURCES	+= Link_ut.cc
UT_SOURCES	+= LogicalMemberPort_ut.cc
UT_SOURCES	+= LogicalPort_ut.cc
UT_SOURCES	+= Port_ut.cc
UT_SOURCES	+= Switch_ut.cc

CXX_SOURCES	+= $(UT_SOURCES) PhysicalLayerStub.cc util.cc
CXX_SOURCES	+= $(UPPL_SOURCES) $(ODBC_SOURCES) $(TCLIB_SOURCES)
CXX_SOURCES	+= $(MISC_SOURCES)

EXTRA_CXXFLAGS	+= -fprofile-arcs -ftest-coverage
EXTRA_CXXFLAGS	+= -Dprivate=public -Dprotected=public

UNC_LIBS	= libpfc_util libpfc libpfc_ipcsrv libpfc_ipcclnt
UNC_LIBS	+= libpfcxx libpfcxx_ipcsrv libpfcxx_ipcclnt
EXTRA_LDLIBS	+= -lgcov

include ../../rules.mk
