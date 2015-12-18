#
# Copyright (c) 2013-2015 NEC Corporation
# All rights reserved.
#
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html

##
## Makefile that drives the production of controller module.
##

include ../defs.mk

CXX_SOURCES		= $(wildcard *.cc)
EXTRA_CPPFLAGS		= $(JSON_C_CPPFLAGS) $(LIBCURL_CPPFLAGS)
EXTRA_INCDIRS		= $(OBJDIR)
EXTRA_LIBDIRS		= $(JSON_C_LIBDIRS) $(LIBCURL_LIBDIRS)
EXTRA_LDLIBS		= $(JSON_C_LDFLAGS) $(LIBCURL_LDFLAGS)
EXTRA_RUNTIME_DIR	= $(JSON_C_RUNPATH) $(LIBCURL_RUNPATH)

CODE_GENERATE_SCRIPT	= $(CURDIR)/code_gen.py
REST_SOURCES		= $(wildcard *.rest)
REST_HEADERS		= $(REST_SOURCES:%.rest=$(OBJDIR)/%.hh)
CLEANFILES		+= $(REST_HEADERS)
CFLAGS  = -I'/../../dist/target/objs/modules/odcdriver/'

include ../rules.mk

$(OBJ_OBJECTS):	$(REST_HEADERS)

$(OBJDIR)/%.hh: %.rest
	echo $(OBJDIR)
	python -B $(CODE_GENERATE_SCRIPT) $< $(OBJDIR)/
