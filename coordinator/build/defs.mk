#
# Copyright (c) 2012-2013 NEC Corporation
# All rights reserved.
# 
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

##
## Common configurations for C/C++ build process.
## This makefile is designed to be included from config.mk.
##

ifndef	UNC_BUILD_DEFS_MK_INCLUDED

UNC_BUILD_DEFS_MK_INCLUDED	:= 1

# Declare that this is UNC build environment.
UNC_BUILD	:= 1

# Path to UNC-Core build directory.
CORE_BLDDIR	:= $(CORE_SRCROOT)/build

# Prepend UNC include directories and public PFC-Core include directory
# to header file search path.
CC_INCDIRS_PREP	= $(UNC_INCDIRS) $(CORE_EXP_INCDIR)

# Prepend libraries defined in UNC_LIBS to LDLIBS.
LDLIBS_PREP	= $(UNC_LIBS:lib%=-l%)

# Import common definition for PFC-Core build environment.
include $(CORE_BLDDIR)/defs.mk

#
# Overwrite variables defined by PFC-Core build environment.
# 

# The oldest year to be embedded into copyright notice.
OLDEST_YEAR	:= 2012

# IPC struct template directory.
IPC_TMPLDIR	= $(CORE_SRCROOT)/$(IPC_DIRNAME)

# Path to IPC struct information file.
OBJ_IPC_BIN	= $(CORE_OBJROOT)/$(IPC_DIRNAME)/$(IPC_STRUCT_BIN)

# Definitions for debugging.
CC_DEBUG_DEFS	+= -DUNC_DEBUG
ifdef	DEBUG_BUILD
CC_DEBUG_DEFS	+= -DUNC_VERBOSE_DEBUG
endif	# DEBUG_BUILD

# Build object for unit test.
# Never define UNIT_TEST for production build!
ifdef	UNIT_TEST
CPPFLAGS	+= -DUNC_UNIT_TEST
endif	# UNIT_TEST

# CPPFLAGS used to compile assembly language source.
AS_CPPFLAGS	+= -D_UNC_ASM

# C preprocessor flags for cfdef compiler.
CFDEFC_CPPFLAGS	+= -D_UNC_IN_CFDEFC

# Paths to auto-generated header files for IPC framework.
IPC_HEADER_NAMES	= $(IPC_STRUCT_H)
IPC_HEADER_NAMES	+= $(IPC_SERVER_PROTO_HH) $(IPC_SERVER_INLINE_HH)
IPC_HEADER_NAMES	+= $(IPC_CLIENT_PROTO_HH) $(IPC_CLIENT_INLINE_HH)
IPC_HEADERS		= $(IPC_HEADER_NAMES:%=$(CORE_EXP_INCDIR)/%)

endif	# !UNC_BUILD_DEFS_MK_INCLUDED
