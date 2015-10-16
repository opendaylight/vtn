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


CXX_SOURCES = $(wildcard *.cc)
EXTRA_CPPFLAGS		= $(JSON_C_CPPFLAGS) $(LIBCURL_CPPFLAGS)
EXTRA_LIBDIRS		= $(JSON_C_LIBDIRS) $(LIBCURL_LIBDIRS)
EXTRA_LDLIBS		= $(JSON_C_LDFLAGS) $(LIBCURL_LDFLAGS)
EXTRA_RUNTIME_DIR	= $(JSON_C_RUNPATH) $(LIBCURL_RUNPATH)
CODE_GENERATE_SCRIPT =$(CURDIR)/code_gen.py
CODE_SPEC_SRC	=	$(wildcard *.rest)
GENERATE_CODE	=	$(CODE_SPEC_SRC:%.rest=$(OBJDIR)/%.hh)
$(OBJDIR)/%.hh: %.rest
	echo $(OBJDIR)
	python $(CODE_GENERATE_SCRIPT) $< $(OBJDIR)/

CLEANFILES	+=	$(OBJDIR)/*.hh

EXTRA_INCDIRS	=	$(OBJDIR)

include ../rules.mk
