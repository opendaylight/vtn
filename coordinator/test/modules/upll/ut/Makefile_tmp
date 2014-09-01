#
# Copyright (c) 2013-2014 NEC Corporation
# All rights reserved.
#
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

##
## Makefile that run the unit tests for UPLL.
##

GTEST_SRCROOT	:= ../../../..
include ../../defs.mk

EXEC_NAME	:=  upll_ut

COMMON_STUB_PATH = ../..

TCLIB_STUBDIR	= $(COMMON_STUB_PATH)/stub/tclib_module
CAPA_STUBDIR	= $(COMMON_STUB_PATH)/stub/capa_module
DAL_STUBDIR	= $(COMMON_STUB_PATH)/stub/dal
MISC_STUBDIR	= $(COMMON_STUB_PATH)/stub/misc
CLSTAT_STUBDIR  = $(COMMON_STUB_PATH)/stub/clstat
VLANMAP_STUBDIR  = $(COMMON_STUB_PATH)/upll/ut

CLSTAT_SRCDIR	= $(MODULE_SRCROOT)/clstat
UPLL_SRCDIR	= $(MODULE_SRCROOT)/upll
CAPA_SRCDIR	= $(MODULE_SRCROOT)/capa
DAL_SRCDIR	= $(MODULE_SRCROOT)/dal
TCLIB_SRCDIR	= $(MODULE_SRCROOT)/tclib
ALARM_SRCDIR	= $(MODULE_SRCROOT)/alarm


# Define a list of directories that contain source files.
ALT_SRCDIRS	= $(CLSTAT_STUBDIR) $(UPLL_SRCDIR) $(CAPA_STUBDIR) $(DAL_STUBDIR)
ALT_SRCDIRS	+= $(TCLIB_STUBDIR) $(MISC_STUBDIR) $(CLSTAT_SRCDIR) $(VLANMAP_STUBDIR)

UT_INCDIRS_PREP		= ${COMMON_STUB_PATH} $(COMMON_STUB_PATH)/stub/include $(COMMON_STUB_PATH)/stub/include/core_include

UTXX_INCDIRS_PREP	= $(TCLIB_STUBDIR)
UTXX_INCDIRS_PREP	+= $(CAPA_STUBDIR)
UTXX_INCDIRS_PREP	+= $(CLSTAT_STUBDIR)
UTXX_INCDIRS_PREP	+= $(DAL_STUBDIR)
UTXX_INCDIRS_PREP	+= $(DAL_STUBDIR)/include
UTXX_INCDIRS_PREP	+= $(VLANMAP_STUBDIR)


EXTRA_CXX_INCDIRS	= $(MODULE_SRCROOT)
EXTRA_CXX_INCDIRS	+= $(CAPA_SRCDIR)/include
EXTRA_CXX_INCDIRS	+= $(CLSTAT_SRCDIR)/.
EXTRA_CXX_INCDIRS	+= $(CLSTAT_SRCDIR)/include
EXTRA_CXX_INCDIRS	+= $(UPLL_SRCDIR)
EXTRA_CXX_INCDIRS	+= $(UPLL_SRCDIR)/include
EXTRA_CXX_INCDIRS	+= $(DAL_SRCDIR)/.
EXTRA_CXX_INCDIRS	+= $(TCLIB_SRCDIR)/include
EXTRA_CXX_INCDIRS	+= $(ALARM_SRCDIR)/include

CPPFLAGS	+= -include ut_stub.h

UPLL_SOURCES	= ipc_util.cc
UPLL_SOURCES	+= ipct_st.cc 
UPLL_SOURCES	+= config_mgr.cc
UPLL_SOURCES	+= ctrlr_mgr.cc
UPLL_SOURCES	+= key_tree.cc
UPLL_SOURCES	+= tclib_intf_impl.cc
UPLL_SOURCES	+= momgr_intf.cc
UPLL_SOURCES	+= tx_mgr.cc
UPLL_SOURCES	+= config_lock.cc
UPLL_SOURCES	+= kt_util.cc
UPLL_SOURCES	+= vtn_momgr.cc
UPLL_SOURCES	+= vbr_momgr.cc
UPLL_SOURCES	+= vbr_if_momgr.cc
UPLL_SOURCES	+= momgr_impl.cc
UPLL_SOURCES	+= vlanmap_momgr.cc
UPLL_SOURCES	+= nwm_momgr.cc
UPLL_SOURCES	+= nwm_host_momgr.cc
UPLL_SOURCES	+= iproute_momgr.cc
UPLL_SOURCES	+= dhcprelay_if_momgr.cc
UPLL_SOURCES	+= dhcprelay_server_momgr.cc
UPLL_SOURCES	+= vrt_if_momgr.cc
UPLL_SOURCES	+= vunk_momgr.cc
UPLL_SOURCES	+= vunk_if_momgr.cc
UPLL_SOURCES	+= vnode_child_momgr.cc
UPLL_SOURCES	+= momgr_util.cc
UPLL_SOURCES	+= vnode_momgr.cc
UPLL_SOURCES	+= read_bulk.cc
UPLL_SOURCES	+= vlink_momgr.cc
UPLL_SOURCES	+= vrt_momgr.cc
UPLL_SOURCES	+= vtn_flowfilter_entry_momgr.cc
UPLL_SOURCES	+= vtn_flowfilter_momgr.cc
UPLL_SOURCES	+= flowlist_entry_momgr.cc
UPLL_SOURCES	+= flowlist_momgr.cc
UPLL_SOURCES	+= policingprofile_entry_momgr.cc
UPLL_SOURCES	+= policingprofile_momgr.cc
UPLL_SOURCES	+= vbr_flowfilter_entry_momgr.cc
UPLL_SOURCES	+= vbr_flowfilter_momgr.cc
UPLL_SOURCES	+= vbr_if_flowfilter_entry_momgr.cc
UPLL_SOURCES	+= vbr_if_flowfilter_momgr.cc
UPLL_SOURCES	+= vbr_if_policingmap_momgr.cc
UPLL_SOURCES	+= vbr_policingmap_momgr.cc
UPLL_SOURCES	+= vrt_if_flowfilter_entry_momgr.cc
UPLL_SOURCES	+= vrt_if_flowfilter_momgr.cc
UPLL_SOURCES	+= vtn_policingmap_momgr.cc
UPLL_SOURCES	+= vtep_momgr.cc
UPLL_SOURCES	+= vtep_if_momgr.cc
UPLL_SOURCES	+= vtep_grp_momgr.cc
UPLL_SOURCES	+= vtepgrp_mem_momgr.cc
UPLL_SOURCES	+= vtunnel_momgr.cc
UPLL_SOURCES	+= vtunnel_if_momgr.cc
UPLL_SOURCES  += dbconn_mgr.cc
UPLL_SOURCES  += vtn_dataflow_momgr.cc
UPLL_SOURCES  += ipc_client_handler.cc
UPLL_SOURCES  += config_svc.cc
UPLL_SOURCES  += vterm_if_momgr.cc
UPLL_SOURCES  += vterm_if_policingmap_momgr.cc
UPLL_SOURCES  += vterminal_momgr.cc
UPLL_SOURCES  += vterm_if_flowfilter_momgr.cc
UPLL_SOURCES  += vterm_if_flowfilter_entry_momgr.cc
UPLL_SOURCES  += ipc_event_queue.cc

CAPA_SOURCES	= capa_module_stub.cc
DAL_SOURCES	= dal_odbc_mgr.cc dal_bind_info.cc dal_schema.cc
TCLIB_SOURCES	= tclib_module.cc
MISC_SOURCES	= ipc_client.cc ipc_server.cc module.cc alarm.cc

UT_SOURCES	= vtn_momgr_ut.cc
UT_SOURCES	+= vbr_momgr_ut.cc
UT_SOURCES	+= vbr_if_momgr_ut.cc
UT_SOURCES  += vlanmap_ut.cc

CXX_SOURCES	= $(UT_SOURCES) util.cc
CXX_SOURCES	+= $(UPLL_SOURCES) $(CAPA_SOURCES) $(DAL_SOURCES) 
CXX_SOURCES	+= $(TCLIB_SOURCES) $(MISC_SOURCES)

EXTRA_CXXFLAGS	+= -fprofile-arcs -ftest-coverage
EXTRA_CXXFLAGS	+= -Dprivate=public -Dprotected=public

UNC_LIBS	= libpfc_util libpfc libpfc_ipcsrv libpfc_ipcclnt
UNC_LIBS	+= libpfcxx libpfcxx_ipcsrv libpfcxx_ipcclnt
EXTRA_LDLIBS	+= -lgcov

include ../../rules.mk
