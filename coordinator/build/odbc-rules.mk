#
# Copyright (c) 2012-2013 NEC Corporation
# All rights reserved.
# 
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

##
## Common rules to compile source files with ODBC library.
## This makefile is designed to be included after odbc-defs.mk is included.
##

ifndef	UNC_BUILD_ODBC_RULES_MK_INCLUDED

UNC_BUILD_ODBC_RULES_MK_INCLUDED	:= 1

ifneq	($(strip $(USE_ODBC)),)

ifndef	UNC_ODBC_DEFS_MK_INCLUDED
$(error odbc-defs.mk is not included)
endif	# !UNC_ODBC_DEFS_MK_INCLUDED

EXTRA_CPPFLAGS		+= $(ODBC_CPPFLAGS)
EXTRA_LDLIBS		+= $(ODBC_LDFLAGS)
EXTRA_RUNTIME_DIR	+= $(ODBC_RUNPATH)

endif	# !empty(USE_ODBC)

endif	# !UNC_BUILD_ODBC_RULES_MK_INCLUDED
