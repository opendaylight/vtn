#
# Copyright (c) 2011-2015 NEC Corporation
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

ifdef	GTEST_CONFIG

include $(BLDDIR)/exec-defs.mk

GTEST_INCDIR	:= $(shell $(GTEST_CONFIG) --includedir)
GTEST_INCDIR	:= $(filter-out /usr/include, $(abspath $(GTEST_INCDIR)))
CPPFLAGS	+= $(GTEST_INCDIR:%=-I%) -DHEAVY_LOAD_TEST

GTEST_CXXFLAGS	:= $(shell $(GTEST_CONFIG) --cxxflags)
CXXFLAGS	+= $(GTEST_CXXFLAGS)

EXTRA_RUNTIME_DIR	= $(LINK_LIBDIR)

GTEST_LIBDIR	:= $(shell $(GTEST_CONFIG) --libdir)
GTEST_LIBDIR	:= $(filter-out $(DEFAULT_LIBPATH), $(abspath $(GTEST_LIBDIR)))
LDFLAGS		+= $(GTEST_LIBDIR:%=-L%)
LD_RUNTIME_DIR	+= $(GTEST_LIBDIR)

GTEST_LDLIBS	:= $(shell $(GTEST_CONFIG) --libs)
LDLIBS		+= $(GTEST_LDLIBS) -lgtest_main

# Google test parameters.
NAME		?=
SUBNAME		?= *
REPEAT		?=

TERM		:= $(strip $(TERM))

ifeq	($(TERM),kterm)
COLOR		?= yes
else ifeq ($(TERM),xterm)
COLOR		?= yes
else ifeq ($(TERM),xterm-color)
COLOR		?= yes
else ifeq ($(TERM),xterm-256color)
COLOR		?= yes
else ifeq ($(TERM),screen)
COLOR		?= yes
else ifeq ($(TERM),screen-256color)
COLOR		?= yes
else ifeq ($(TERM),linux)
COLOR		?= yes
else ifeq ($(TERM),cygwin)
COLOR		?= yes
endif	# TERM

ifneq	($(strip $(JOB_NAME)),)
COLOR		= no
endif	# !empty(JOB_NAME)

GTEST_FILTER	= $(NAME:%=--gtest_filter='%.$(SUBNAME)')
GTEST_REPEAT	= $(REPEAT:%=--gtest_repeat=%)
GTEST_COLOR	= $(COLOR:%=--gtest_color=%)
GTEST_FLAGS	= $(GTEST_FILTER) $(GTEST_REPEAT) $(GTEST_COLOR)

ifdef	BROKEN_TEST
GTEST_FLAGS	+= --gtest_also_run_disabled_tests
endif	# BROKEN_TEST

endif	# GTEST_CONFIG
