#
# Copyright (c) 2012-2013 NEC Corporation
# All rights reserved.
# 
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

##
## Makefile that drives the production of dal module.
##

include ../defs.mk

EXTRA_INCDIRS	= ..

CXX_SOURCES = \
	dal_bind_column_info.cc \
	dal_bind_info.cc \
	dal_cursor.cc \
	dal_error_handler.cc \
	dal_odbc_mgr.cc \
	dal_query_builder.cc \
	dal_schema.cc \
	dal_module.cc

CFDEF_FILES     += dal_db.cfdef

USE_ODBC	= 1

include ../rules.mk
