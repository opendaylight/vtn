#
# Copyright (c) 2010-2013 NEC Corporation
# All rights reserved.
# 
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

##
## Common configurations to build executable.
## This makefile is designed to be included after config.mk is included.
##
## The following macros need to be defined in Makefile:
##
## EXEC_NAME:		Name of executable.
##
## C_SOURCES:		List of C source files to be compiled.
##
## CXX_SOURCES:		List of C++ source files to be compiled.
##
## AS_SOURCES:		List of assembly language source files to be compiled.
##

ifndef	BUILD_EXEC_DEFS_MK_INCLUDED

BUILD_EXEC_DEFS_MK_INCLUDED	:= 1

# Object files to be linked to the target.
OBJECTS		= $(C_SOURCES:.c=.o) $(CXX_SOURCES:.cc=.o) $(AS_SOURCES:.S=.o)

endif	# !BUILD_EXEC_DEFS_MK_INCLUDED
