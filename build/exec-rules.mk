#
# Copyright (c) 2010-2013 NEC Corporation
# All rights reserved.
# 
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

##
## Common rules to build executable.
## This makefile is designed to be included after exec-defs.mk is included.
##
## Command to link objects is determined by CXX_SOURCES macro. If it is empty,
## $(CC) is used as linker. If not empty, $(CXX) is used.
##

ifndef	UNC_BUILD_EXEC_RULES_MK_INCLUDED

UNC_BUILD_EXEC_RULES_MK_INCLUDED	:= 1

include	$(CORE_BLDDIR)/exec-rules.mk

endif	# !BUILD_EXEC_RULES_MK_INCLUDED
