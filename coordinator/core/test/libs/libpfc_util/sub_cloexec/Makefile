#
# Copyright (c) 2010-2013 NEC Corporation
# All rights reserved.
# 
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

##
## Makefile that drives the production of sub command for libpfc_util/cloexec.c
## testing.
##

GTEST_SRCROOT	:= ../../../..
NO_PTHREAD	:= 1
NEED_OBJDIR	:= 1

include $(GTEST_SRCROOT)/build/config.mk
include $(BLDDIR)/exec-defs.mk

EXEC_NAME	= sub_cloexec

C_SOURCES	= sub_cloexec.c

# Pass test object directory path to preprocessor.
TEST_OBJDIR	= $(abspath $(OBJDIR)/..)
CPPFLAGS	+= -DOBJDIR="\"$(TEST_OBJDIR)\""

include $(BLDDIR)/exec-rules.mk
