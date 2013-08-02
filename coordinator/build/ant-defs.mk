#
# Copyright (c) 2012-2013 NEC Corporation
# All rights reserved.
# 
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

##
## Common rules to invoke Apache Ant.
## This makefile is designed to be included after java-defs.mk is included.
##

ifndef	UNC_BUILD_ANT_DEFS_MK_INCLUDED

UNC_BUILD_ANT_DEFS_MK_INCLUDED	:= 1

include $(CORE_BLDDIR)/ant-defs.mk

endif	# !UNC_BUILD_ANT_DEFS_MK_INCLUDED
