#
# Copyright (c) 2013 NEC Corporation
# All rights reserved.
#
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

##
## Common configurations to build unit tests for VTN Coordinator modules.
## GTEST_SRCROOT must be defined in advance.
##

include $(GTEST_SRCROOT)/test/build/gtest-defs.mk

MODULE_SRCROOT	= $(GTEST_SRCROOT)/modules

# Construct header include path.
ifeq	($(strip $(SUBARCH)),)
CC_INCDIRS	= core/include/arch/$(ARCH)
else	# !empty(SUBARCH)
CC_INCDIRS	= core/include/arch/$(SUBARCH) core/include/arch/$(ARCH)
endif	# empty(SUBARCH)

CC_INCDIRS	+= core/include/os/$(OSTYPE) core/include
CC_INCDIRS_PREP	= $(UT_INCDIRS_PREP) $(UNC_INCDIRS) $(CORE_EXP_INCDIR)

CXX_INCDIRS	= core/include/cxx

CXX_INCLUDES	= $(UT_INCDIRS_PREP:%=-I%) $(UTXX_INCDIRS_PREP:%=-I%)
CXX_INCLUDES	+= $(CXX_INCDIRS:%=-I$(SRCROOT)/%) $(BOOST_INCDIR:%=-I%)
CXX_INCLUDES	+= $(EXTRA_CXX_INCDIRS:%=-I%)
