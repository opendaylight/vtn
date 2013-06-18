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
## JAVA_BUILD_LIBS:		JAR file path to be added to classpath only
##				on build. JAR files specified by this macro
##				are never written to "Class-Path" field in
##				MANIFEST.MF.
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
## JAVA_NO_CLASSPATH:		Don't put "Class-Path" field into MANIFEST.MF
##				if not empty.
##
## JAVA_TEXT_FILTERS:		Pairs of token filters to be applied on
##				text file copy. Each element in this macro
##				must be defined as "-f TOKEN VALUE".
##				All the occurrences of the token "%TOKEN%" in
##				resource files will be replaced with "VALUE".
##

ifndef	BUILD_JAVA_DEFS_MK_INCLUDED

BUILD_JAVA_DEFS_MK_INCLUDED	:= 1

ifdef	JAVA_CONFIG_MK

include $(JAVA_CONFIG_MK)

##
## Below are macros which can be overridden in Makefile.
##

# Java compiler's option.
JAVAC_ERRWARN	= -Werror
JAVAC_LINT	= -Xlint

# Let javac generate debugging information if true.
JAVAC_DEBUG	= $(JAVA_DEBUG_DEF)

# Encoding of Java source file.
JAVA_ENCODING	= $(JAVA_ENCODING_DEF)

# Library version number.
JAVA_LIBVERSION	= $(PFC_VERSION)

endif	# JAVA_CONFIG_MK

endif	# !BUILD_JAVA_DEFS_MK_INCLUDED
