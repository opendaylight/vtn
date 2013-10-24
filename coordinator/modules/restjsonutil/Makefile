#
# Copyright (c) 2012-2013 NEC Corporation
# All rights reserved.
# 
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

##
## Makefile that drives the production of tc module.
##

include ../defs.mk

CXX_SOURCES	=	rest_client.cc \
							http_client.cc \
							json_build_parse.cc	\
							restjsonmod.cc

UNC_LIBS   +=   -lcurl \
								-ljson

include ../rules.mk

##
## Targets:
##
## make all (default target)
##	Build all binary distributions.
##
## make installu
##	Execute "make all" and install binary distributions into the
##	installation directory.
##
## make clean
##	Remove binary distributions built by "make all".
##
## make clobber
##	Execute "make clean" and remove header dependency files.
##
