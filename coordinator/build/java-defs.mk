#
# Copyright (c) 2012-2013 NEC Corporation
# All rights reserved.
# 
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

##
## Common configurations to build Java library.
## This makefile is designed to be included after config.mk is included.
## Note that the Java library name must be identical to build directory name.
##
## This makefile uses the following macros defined in Makefile:
##
## JAVA_LIBS:			JAR file name to be added to classpath.
##
## JAVA_EXTLIBS:		Third-party JAR file name to be added to
##				classpath.
##
## JAVA_EXTRA_SRCDIR:		Path to additional Java source root directory.
##
## JAVA_IMPL_TITLE:		Implementation-Title value in MANIFEST.MF.
##				The library name is used if omitted.
##
## JAVA_SPEC_TITLE:		Specification-Title value in MANIFEST.MF.
##				The library name is used if omitted.
##
## JAVA_MAIN_CLASS:		Main-Class value in MANIFEST.MF.
##				Main-Class tag is not set if omitted.
##
## JAVA_DEPENDS:		Make target list which must be invoked before
##				invoking Ant.
##
## JAVA_EXTRA_ALL:		Make target list which must be invoked after
##				"make all".
##
## JAVA_EXTRA_INSTALL:		Make target list which must be invoked after
##				"make install".
##
## JAVA_JNI_SUBDIRS:		List of JNI build directories.
##
## JAVA_JAR_INDEX:		Create JAR file with class index if 1 is
##				set to this macro. Note that all JAR files
##				used by the library must be specified to
##				JAVA_LIBS if JAR file index is enabled.
##

ifndef	UNC_BUILD_JAVA_DEFS_MK_INCLUDED

UNC_BUILD_JAVA_DEFS_MK_INCLUDED	:= 1

include $(CORE_BLDDIR)/java-defs.mk

# Library version number.
JAVA_LIBVERSION	=		\
	 $(UNC_VERSION_MAJOR).$(UNC_VERSION_MINOR).$(UNC_VERSION_REVISION)

# Name of web application root directory in build directory.
JAVA_WEBAPP_DIR	= webapp

# Absolute path to web application root directory.
JAVA_WEBROOT	:= $(realpath $(JAVA_WEBAPP_DIR))

ifdef	JAVA_EXT_MK
include $(JAVA_EXT_MK)
endif	# JAVA_EXT_MK

endif	# !UNC_BUILD_JAVA_DEFS_MK_INCLUDED
