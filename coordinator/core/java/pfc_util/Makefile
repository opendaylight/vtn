#
# Copyright (c) 2012-2013 NEC Corporation
# All rights reserved.
# 
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

##
## Makefile that drives the production of Java bindings of PFC-Core utilities.
##

include ../defs.mk

# Rules to create compile-time configuration.
ifdef	DEBUG_BUILD
PFC_BUILD_TYPE_SUFFIX	:= -debug
PFC_DEBUG		:= true
else	# !DEBUG_BUILD
PFC_DEBUG		:= false
endif	# DEBUG_BUILD

PFC_JAVA_LIBDIR		:= $(INST_JAVADIR)/jar
PFC_JNI_LIBDIR		:= $(INST_JAVADIR)/jni

# Replace keywords in property file.
JAVA_TEXT_FILTERS	= -f PFC_PRODUCT_NAME '$(PFC_PRODUCT_NAME)'
JAVA_TEXT_FILTERS	+= -f PFC_VERSION_STRING '$(PFC_VERSION_STRING)'
JAVA_TEXT_FILTERS	+= -f PFC_VERSION_MAJOR '$(PFC_VERSION_MAJOR)'
JAVA_TEXT_FILTERS	+= -f PFC_VERSION_MINOR '$(PFC_VERSION_MINOR)'
JAVA_TEXT_FILTERS	+= -f PFC_VERSION_REVISION '$(PFC_VERSION_REVISION)'
JAVA_TEXT_FILTERS	+= -f PFC_VERSION_PATCHLEVEL '$(PFC_VERSION_PATCHLEVEL)'
JAVA_TEXT_FILTERS	+= -f PFC_BUILD_TYPE_SUFFIX '$(PFC_BUILD_TYPE_SUFFIX)'
JAVA_TEXT_FILTERS	+= -f PFC_DEBUG '$(PFC_DEBUG)'
JAVA_TEXT_FILTERS	+= -f PREFIX '$(PREFIX)'
JAVA_TEXT_FILTERS	+= -f INST_BINDIR '$(INST_BINDIR)'
JAVA_TEXT_FILTERS	+= -f INST_DATADIR '$(INST_DATADIR)'
JAVA_TEXT_FILTERS	+= -f INST_LIBDIR '$(INST_LIBDIR)'
JAVA_TEXT_FILTERS	+= -f INST_LIBEXECDIR '$(INST_LIBEXECDIR)'
JAVA_TEXT_FILTERS	+= -f INST_LOCALSTATEDIR '$(INST_LOCALSTATEDIR)'
JAVA_TEXT_FILTERS	+= -f INST_SBINDIR '$(INST_SBINDIR)'
JAVA_TEXT_FILTERS	+= -f INST_SYSCONFDIR '$(INST_SYSCONFDIR)'
JAVA_TEXT_FILTERS	+= -f PFC_JAVA_LIBDIR '$(PFC_JAVA_LIBDIR)'
JAVA_TEXT_FILTERS	+= -f PFC_JNI_LIBDIR '$(PFC_JNI_LIBDIR)'

# Attributes for manifest file.
JAVA_SPEC_TITLE		= PFC-Core utility classes
JAVA_IMPL_TITLE		= Java bindings for PFC-Core utility

# Build JNI library.
JAVA_JNI_SUBDIRS	= jni

# Create JAR file index.
JAVA_JAR_INDEX		= 1

include ../rules.mk
