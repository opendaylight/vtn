#
# Copyright (c) 2012-2013 NEC Corporation
# All rights reserved.
# 
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

##
## Common configurations to build JNI library.
## This makefile is designed to be included after config.mk is included.
## This makefile includes shared-defs.mk, so macros required by shared-defs.mk
## need to be defined in Makefile.
##
## This makefile uses the following macros defined in Makefile:
##
## JNI_JAVAH_CLASSES:	List of fully qualified class names to be passed to
##			avah. If this macro is defined, rules to generate JNI
##			header files are defined. Header files are always
##			generated under $(OBJDIR)/include.
##

ifndef	BUILD_JNI_DEFS_MK_INCLUDED

BUILD_JNI_DEFS_MK_INCLUDED	:= 1

ifdef	JAVA_CONFIG_MK

# Installation target directory.
JNI_TARGETDIR	= $(INST_JAVADIR)/jni

# JNI library is never linked to executable.
SHLIB_DLOPEN	:= 1

include $(BLDDIR)/shared-defs.mk
include $(JAVA_CONFIG_MK)

endif	# JAVA_CONFIG_MK

endif	# !BUILD_JNI_DEFS_MK_INCLUDED
