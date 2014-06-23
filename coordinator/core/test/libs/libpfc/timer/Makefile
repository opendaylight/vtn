#
# Copyright (c) 2010-2014 NEC Corporation
# All rights reserved.
# 
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

##
## Makefile that drives the tests for libpfc timer.
##

GTEST_SRCROOT	:= ../../../..
include $(GTEST_SRCROOT)/test/build/gtest-defs.mk

EXEC_NAME	:= libpfc_timer_test

CXX_SOURCES	= $(wildcard *.cc)

# Link command and control protocol libraries.
PFC_LIBS	+= libpfc_util libpfc
LDLIBS		+= -lrt

# Import system library private header files.
EXTRA_INCDIRS	= $(PFC_LIBS:%=$(SRCROOT)/libs/%)

##
## rules
##

include $(GTEST_BLDDIR)/gtest-rules.mk

install:	all
