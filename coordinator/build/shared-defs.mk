#
# Copyright (c) 2012-2013 NEC Corporation
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
##			UNC_VERSION_MAJOR is used if omitted.
##
## SHLIB_MINOR:		Minor version of shared object.
##			UNC_VERSION_MINOR is used if omitted.
##
## C_SOURCES:		List of C source files to be compiled.
##
## CXX_SOURCES:		List of C++ source files to be compiled.
##
## AS_SOURCES:		List of assembly language source files to be compiled.
##

ifndef	UNC_BUILD_SHARED_DEFS_MK_INCLUDED

UNC_BUILD_SHARED_DEFS_MK_INCLUDED	:= 1

# Determine DSO version.
SHLIB_MAJOR	?= $(UNC_VERSION_MAJOR)
SHLIB_MINOR	?= $(UNC_VERSION_MINOR)

include $(CORE_BLDDIR)/shared-defs.mk

endif	# !UNC_BUILD_SHARED_DEFS_MK_INCLUDED
