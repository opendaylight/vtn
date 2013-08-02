#
# Copyright (c) 2012-2013 NEC Corporation
# All rights reserved.
# 
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

##
## Makefile that drives the production of libuncmgmtdb library.
##


##
## Makefile that drives the production of libuncsnmp_trap library.
##

# Uncomment this if you don't want to link pthread library.
#NO_PTHREAD	:= 1

include ../defs.mk

# Define C++ source files.
# Suffix of the source file name must be ".cc".
CXX_SOURCES	=		\
	mgmt_database.cc

# Define C source files.
# Suffix of the source file name must be ".c".
C_SOURCES = 

# List of UNC or PFC libraries to be linked.

# Additional cpp options.
EXTRA_CPPFLAGS	= \
	-DUNIX -DJP_UTF_8 \
	-D_THREAD_SAFE -D_LINUX -D_LINUX30 -DRHEL6

EXTRA_CFLAGS	= \
	-DUNIX -DJP_UTF_8 \
	-D_THREAD_SAFE -D_LINUX -D_LINUX30 -DRHEL6

# Additional compiler options.
#EXTRA_CXXFLAGS	= -pipe

# Additional linker options to link system libraries.
# Note that libpthread is always linked unless NO_PTHREAD is defined.
#EXTRA_CXX_INCDIRS	= ../../modules/usess/include ../../core/include/cxx/
EXTRA_LDLIBS = -lpfc_util

# Define PFC-Core log identifier.
#USE_LOG_IDENT	= 1
USE_ODBC		= 1

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

