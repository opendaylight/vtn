#
# Copyright (c) 2013 NEC Corporation
# All rights reserved.
#
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html

##
## Makefile that drives the production of tc module.
##

include ../defs.mk

CXX_SOURCES = rest_client.cc \
              http_client.cc \
              json_build_parse.cc \
              rest_json_mod.cc \
              rest_util.cc

EXTRA_CPPFLAGS    = $(JSON_C_CPPFLAGS) $(LIBCURL_CPPFLAGS)
EXTRA_LIBDIRS     = $(JSON_C_LIBDIRS) $(LIBCURL_LIBDIRS)
EXTRA_LDLIBS      = $(JSON_C_LDFLAGS) $(LIBCURL_LDFLAGS)
EXTRA_RUNTIME_DIR = $(JSON_C_RUNPATH) $(LIBCURL_RUNPATH)

include ../rules.mk

##
## Targets:
##
## make all (default target)
##  Build all binary distributions.
##
## make installu
##  Execute "make all" and install binary distributions into the
##  installation directory.
##
## make clean
##  Remove binary distributions built by "make all".
##
## make clobber
##  Execute "make clean" and remove header dependency files.
##
