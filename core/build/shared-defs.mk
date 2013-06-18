#
# Copyright (c) 2010-2013 NEC Corporation
# All rights reserved.
# 
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

##
## Common configurations to build shared library.
## This makefile is designed to be included after config.mk is included.
##
## The following macros need to be defined in Makefile:
##
## SHLIB_NAME:		Name of shared object.
##
## SHLIB_MAJOR:		Major version of shared object.
##			PFC_VERSION_MAJOR is used if omitted.
##
## SHLIB_MINOR:		Minor version of shared object.
##			PFC_VERSION_MINOR is used if omitted.
##
## C_SOURCES:		List of C source files to be compiled.
##
## CXX_SOURCES:		List of C++ source files to be compiled.
##
## AS_SOURCES:		List of assembly language source files to be compiled.
##

ifndef	BUILD_SHARED_DEFS_MK_INCLUDED

BUILD_SHARED_DEFS_MK_INCLUDED	:= 1

# Compiler options to generate position independent code.
PICFLAGS	= -fpic
CPPFLAGS	+= -DPIC
CFLAGS		+= $(PICFLAGS)
CXXFLAGS	+= $(PICFLAGS)

# Target file name.
SHLIB_MAJOR	?= $(PFC_VERSION_MAJOR)
SHLIB_MINOR	?= $(PFC_VERSION_MINOR)
SHLIB_SO	= $(SHLIB_NAME).so

# Official shared library name.
SONAME		= $(SHLIB_SO).$(SHLIB_MAJOR)

# Actual shared library file name.
SHLIB_FILE	= $(SONAME).$(SHLIB_MINOR)

# Symbolic links to shared library.
ifdef	SHLIB_DLOPEN
# This library is provided only for dlopen(). So "libXXX.so" is not needed.
SHLIB_LINKS	= $(SONAME)
else	# !SHLIB_DLOPEN
SHLIB_LINKS	= $(SONAME) $(SHLIB_SO)
endif	# SHLIB_DLOPEN

# All files to be created.
SHLIB_ALLFILES	= $(SHLIB_FILE) $(SHLIB_LINKS)

ifdef	NO_SO_VERSION

# Don't use SO version.
SONAME		= $(SHLIB_SO)
SHLIB_FILE	= $(SHLIB_SO)
SHLIB_LINKS	=

endif	# defined(NO_SO_VERSION)

# Version script.
VERS_SCRIPT	:= vers-script
SHLIB_VERS	:= $(shell [ -r $(VERS_SCRIPT) ] && echo $(VERS_SCRIPT))

# Linker option to generate shared object.
LDFLAGS		+= -shared -Wl,-h,$(SONAME)
LD_LDFLAGS	+= $(SHLIB_VERS:%=--version-script=%)

# Object files to be linked to the target.
OBJECTS		= $(C_SOURCES:.c=.o) $(CXX_SOURCES:.cc=.o) $(AS_SOURCES:.S=.o)

endif	# !BUILD_SHARED_DEFS_MK_INCLUDED
