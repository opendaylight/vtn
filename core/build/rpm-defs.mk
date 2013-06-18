#
# Copyright (c) 2012-2013 NEC Corporation
# All rights reserved.
# 
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

##
## Common configurations to build RPM package.
## This makefile is designed to be included after config.mk is included.
##
## The following macros need to be defined in Makefile:
##
## RPM_NAME:		Name of RPM package.
##
## RPM_REVISION:	Release number of the RPM package.
##			0 is used if omitted.
##
## RPM_PROVIDES_LIST:	List of additional package names provided by
##			this RPM package.
##
## RPM_FILE_DEPS:	Dependency list for the target that builds
##			RPM package.
##

ifndef	BUILD_RPM_DEFS_MK_INCLUDED

BUILD_RPM_DEFS_MK_INCLUDED	:= 1

# Actual name of RPM package.
RPM_PKGNAME	= $(RPM_NAME)$(RPM_NAME_SUFFIX)

# Version of RPM package.
RPM_REVISION	= 0
RPM_VERSION	= $(PFC_VERSION)
RPM_RELEASE	= $(RPM_REVISION).$(LINUX_DIST)

# Installation prefix.
RPM_PREFIX	?= $(PREFIX)

# RPM spec file for pfc_licgen package.
RPM_SPEC_NAME	= $(RPM_PKGNAME).spec
RPM_SPEC_FILE	= $(OBJDIR)/$(RPM_SPEC_NAME)

# List of RPM filenames.
RPM_FILENAME	= $(RPM_NAME)-$(RPM_VERSION)-$(RPM_RELEASE).$(RPM_ARCH).rpm

endif	# !BUILD_RPM_DEFS_MK_INCLUDED
