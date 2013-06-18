#
# Copyright (c) 2012-2013 NEC Corporation
# All rights reserved.
# 
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

##
## Makefile that drives the production of libsample2 library.
##

# Uncomment this if you don't want to link pthread library.
#NO_PTHREAD	:= 1

include ../defs.mk

# Define C++ source files.
# Suffix of the source file name must be ".cc".
CXX_SOURCES	=		\
 libtc_server_sess_utils.cc \
 libtc_client_sess_utils.cc 

# List of UNC or PFC libraries to be linked.
#UNC_LIBS	= libtccommon

# Additional cpp options.
#EXTRA_CPPFLAGS	= -D_YOUR_CPPFLAGS

# Additional compiler options.
#EXTRA_CXXFLAGS	= -pipe

# Additional linker options to link system libraries.
# Note that libpthread is always linked unless NO_PTHREAD is defined.
EXTRA_LDLIBS	= -lpfc_ipc -lpfc_ipcsrv -lpfc_ipcclnt -lpfc_util

# Define PFC-Core log identifier.
#USE_LOG_IDENT	= 1

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
