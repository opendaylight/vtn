#
# Copyright (c) 2012-2015 NEC Corporation
# All rights reserved.
# 
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

##
## Makefile that drives the production of uppl module.
##

include ../defs.mk

CXX_SOURCES	= physicallayer.cc \
		odbcm_mgr.cc \
                odbcm_connection.cc \
		odbcm_mgr_dboperations.cc \
		unc_state_handler.cc \
		physical_core.cc \
		phy_util.cc \
		ipct_util.cc \
		physical_itc.cc \
		itc_configuration_request.cc \
		itc_read_request.cc \
		itc_kt_base.cc \
		itc_kt_state_base.cc \
		itc_kt_root.cc \
		itc_kt_controller.cc \
		itc_kt_dataflow.cc \
		itc_kt_dataflow_v2.cc \
		itc_kt_ctr_dataflow.cc \
		itc_kt_ctr_domain.cc \
		itc_kt_logicalport.cc \
		itc_kt_logical_member_port.cc \
		itc_kt_switch.cc \
		itc_kt_port.cc \
		itc_kt_port_neighbor.cc \
		itc_kt_link.cc \
		itc_kt_boundary.cc \
		itc_transaction_request.cc  \
		itc_import_request.cc \
		itc_audit_request.cc \
		itc_db_config.cc \
		itc_state_change.cc \
		ipc_connection_manager.cc \
		ipc_client_logical_handler.cc \
		ipc_server_handler.cc \
		ipc_client_configuration_handler.cc \
		physical_notification_manager.cc \
		itc_notification_request.cc \
		physical_taskq.cc \
		odbcm_query_factory.cc \
		odbcm_db_tableschema.cc \
		odbcm_db_varbind.cc \
                odbcm_bind_controller.cc \
                odbcm_bind_domain.cc \
                odbcm_bind_logicalmemberport.cc \
                odbcm_bind_logicalport.cc \
                odbcm_bind_switch.cc \
                odbcm_bind_port.cc \
                odbcm_bind_link.cc \
                odbcm_bind_boundary.cc \
		odbcm_query_processor.cc \
		odbcm_utils.cc

# Use ODBC library.
USE_ODBC	= 1

CFDEF_FILES = odbcm.cfdef

MODULE_EXTRA_FILES = odbcm.conf

EXTRA_INCDIRS   = $(CURDIR)/ \
                  $(CURDIR)/../capa/include \
                  $(CURDIR)/../capa \
                  $(CURDIR)/../unclib

include ../rules.mk

##
## Targets:
##
## make all (default target)
##	Build all binary distributions.
##
## make install
##	Execute "make all" and install binary distributions into the
##	installation directory.
##
## make clean
##	Remove binary distributions built by "make all".
##
## make clobber
##	Execute "make clean" and remove header dependency files.
##
