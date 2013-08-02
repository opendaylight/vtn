#
# Copyright (c) 2012-2013 NEC Corporation
# All rights reserved.
# 
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

##
## Common rules for build.
## This makefile is designed to be included after config.mk is included.
##

ifndef	UNC_BUILD_RULES_MK_INCLUDED

UNC_BUILD_RULES_MK_INCLUDED	:= 1

include $(ODBC_RULES_MK)
include $(CORE_BLDDIR)/rules.mk

endif	# !UNC_BUILD_RULES_MK_INCLUDED
