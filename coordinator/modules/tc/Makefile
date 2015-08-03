#
# Copyright (c) 2012-2015 NEC Corporation
# All rights reserved.
# 
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

##
## Makefile that drives the production of tc module.
##

include ../defs.mk

CXX_SOURCES	=	tc_operations.cc \
			tc_read_operations.cc \
			tc_read_status_operations.cc	\
			tc_config_operations.cc  \
			tc_startup_operations.cc \
			tc_db_operations.cc \
			tc_autosave_operations.cc \
			tc_candidate_operations.cc \
			tc_audit_operations.cc \
			tcmsg.cc \
			tcmsg_commit.cc \
			tcmsg_audit.cc \
			tc_lock.cc \
			tc_taskq.cc \
			tc_db_handler.cc \
			tc_lnc_event_handler.cc \
			tc_unc_states.cc \
			tc_module.cc 


PFC_LIBS += \
	  libtc_common 

USE_ODBC	= 1

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
