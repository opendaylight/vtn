#
# Copyright (c) 2012-2013 NEC Corporation
# All rights reserved.
# 
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

##
## Makefile that drives the production of usess module.
##

include ../defs.mk

# Define C++ source files.
# Suffix of the source file name must be ".cc".
CXX_SOURCES = \
	usess.cc \
	usess_sessions.cc \
	usess_session.cc \
	usess_users.cc \
	usess_user.cc \
	usess_enable.cc \
	usess_base_common.cc \
	usess_conf_common.cc \
	usess_conf_session.cc \
	usess_conf_user.cc \
	usess_conf_enable.cc


EXTRA_CXX_INCDIRS	= ../../libs/mgmt/libuncmgmtdb ..
EXTRA_LDLIBS = -lcrypt -luncmgmtdb

include ../rules.mk

