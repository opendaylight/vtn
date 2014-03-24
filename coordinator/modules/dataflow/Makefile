#
# Copyright (c) 2013-2014 NEC Corporation
# All rights reserved.
#
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

##
## Makefile that drives the production of capa module.
##

include ../defs.mk

CXX_SOURCES = dataflow.cc 

# Example of Boost library configuration.
BOOST_LIBS	= 

EXTRA_INCDIRS   = $(CURDIR)/../uppl/include $(CURDIR)/../upll $(CURDIR)/../upll/include

include ../rules.mk


##
## Targets:
##
## make all (default target)
##	Build all binary distributions.
##
## make install
##	Execute "make all" and install binary distributions into the
##	installation directory.
##
## make clean
##	Remove binary distributions built by "make all".
##
## make clobber
##	Execute "make clean" and remove header dependency files.
##
