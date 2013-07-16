#
# Copyright (c) 2013 NEC Corporation
# All rights reserved.
# 
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

##
## Common configurations for unit test build environment using Google Test.
##
## GTEST_SRCROOT, which points path to source tree root, must be defined
## before this file is included.
##

NEED_OBJDIR	:= 1
GTEST_BLDDIR	:= $(GTEST_SRCROOT)/test/build

include $(GTEST_SRCROOT)/build/config.mk
include $(CORE_SRCROOT)/test/build/gtest-defs.mk
