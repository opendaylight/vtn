#
# Copyright (c) 2012-2013 NEC Corporation
# All rights reserved.
# 
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

##
## Common rules to build Java library.
## This makefile is designed to be included after java-defs.mk is included.
##

ifndef	BUILD_JAVA_RULES_MK_INCLUDED

BUILD_JAVA_RULES_MK_INCLUDED	:= 1

ifdef	JAVA_CONFIG_MK

ifdef	JUNIT_TEST

ifndef	JAVA_JUNIT
$(error JUnit is not installed.)
endif	# !JAVA_JUNIT

# Path to java executable.
JAVA		= $(JAVA_HOME)/bin/java

# Name of the target Java library.
JAVA_TESTNAME	:= $(notdir $(CURDIR))
JAVA_LIBNAME	:= $(JAVA_TESTNAME)_test

# Add the target to classpath.
JAVA_LIBS	+= $(JAVA_TESTNAME).jar
JAVA_EXTLIBS	+= junit.jar

# Need to specify LD_LIBRARY_PATH to load native library.
JUNIT_ENV	= LD_LIBRARY_PATH=$(LINK_LIBDIR)

else	# !JUNIT_TEST

# Library name must be identical to build directory name.
JAVA_LIBNAME	:= $(notdir $(CURDIR))

# Install Java library.
JAVA_DO_INSTALL	:= 1

endif	# JUNIT_TEST

# Java source directories.
JAVA_SRCDIR	= src $(JAVA_EXTRA_SRCDIR)

# Options to be passed to javac.
JAVAC_FLAGS	= $(JAVAC_ERRWARN) $(JAVAC_LINT) $(JAVAC_EXTRA_FLAGS)

# JAR file path.
JAVA_LIBFILE	= $(JAVA_LIBNAME).jar
JARFILE		= $(JAVA_OBJDIR)/$(JAVA_LIBFILE)

include $(BLDDIR)/ant-defs.mk

ifdef	JUNIT_TEST

JUNIT_TARGET	:= test
ifdef	ANT_JUNIT
MKJPROJECT_ARGS	+= -u $(JUNIT_TARGET) --jnidir $(LINK_JNILIBDIR)
endif	# ANT_JUNIT

# Class loader in JUnit does not see Class-Path in MANIFEST.MF.
JUNIT_CLASSPATH	= -classpath '$(JAVA_OBJDIR)/*:$(JAVA_OBJDIR)/ext/*'

endif	# JUNIT_TEST

JAVA_CLOBBERFILES	= $(JARFILE)
CLEANFILES		+= $(JAVA_CLOBBERFILES) $(OBJ_CLASSDIR)
CLOBBERFILES		+= $(JAVA_CLOBBERFILES)

JAVA_INSTALL_DIR	?= $(INST_JARDIR)
DEST_JAVADIR		= $(DESTDIR)$(JAVA_INSTALL_DIR)
DEST_JARFILE		= $(DEST_JAVADIR)/$(JAVA_LIBFILE)

CLEAN_DEPS	+= clean-jni

# Determine whether JAR file index should be created or not.
ifeq	($(strip $(JAVA_JAR_INDEX)),)
JAR_INDEX	:= off
else	# !empty(JAVA_JAR_INDEX)
JAR_INDEX	:= on
endif	# empty(JAVA_JAR_INDEX)

ANT_PROPS	+= pkg.jar.index=$(JAR_INDEX)

ifdef	ANT_JUNIT

# Ant property name to define test class name.
ANT_PROPS	+= pkg.test.name=$(JUNIT_TESTNAME)

endif	# ANT_JUNIT

JAVA_ALL_TARGET		?= jar
JAVA_INSTALL_TARGET	?= install-files

all:	$(JAVA_ALL_TARGET)
ifdef	JAVA_EXTRA_ALL
	$(MAKE) $(JAVA_EXTRA_ALL)
endif	# JAVA_EXTRA_ALL
	$(MAKE) all-jni

jar:	build
jar:	ANT_TARGET = jar

doc:	build
doc:	ANT_TARGET = doc

javadoc:	doc
document:	doc

build:	$(BUILD_XML)
	$(ANT_BUILD) $(ANT_TARGET)

ifdef	JAVA_DO_INSTALL

install:	$(JAVA_INSTALL_TARGET) $(JAVA_EXTRA_INSTALL)
	$(MAKE) install-jni

install-files:	$(DEST_JARFILE)

ifdef	JAVA_EXTRA_INSTALL
$(JAVA_EXTRA_INSTALL):	$(JAVA_INSTALL_TARGET)
endif	# JAVA_EXTRA_INSTALL

else	# !JAVA_DO_INSTALL

install:	all

endif	# JAVA_DO_INSTALL

include	$(BLDDIR)/rules.mk

# Create Ant project file.
project:	$(BUILD_XML)

$(BUILD_XML):	$(JAVA_DEPENDS) FRC
	@$(MKJPROJECT) -o $@ $(MKJPROJECT_ARGS)

# Install JAR files.
$(DEST_JAVADIR)/%:	$(JAVA_OBJDIR)/% FRC
	@$(call CMD_INSTALL_FILE,$<,$@)

$(JARFILE):	jar

# Build JNI build directories.
all-jni install-jni clean-jni:	$(JAVA_JNI_SUBDIRS)

ifneq	($(strip $(JAVA_JNI_SUBDIRS)),)

all-jni:	JNI_TARGET = all
install-jni:	JNI_TARGET = install
clean-jni:	JNI_TARGET = clean

$(JAVA_JNI_SUBDIRS):
	$(MAKE) -C $@ $(JNI_TARGET)

.PHONY:	all-jni install-jni clean-jni $(JAVA_JNI_SUBDIRS)

endif	# !empty(JAVA_JNI_SUBDIRS)

ifdef	JAVA_SRCLIST_FILE

# Put all source directories into JAVA_SRCLIST_FILE.

srclist:	$(JAVA_DEPENDS) FRC
	@for f in $(realpath $(JAVA_SRCDIR)); do		\
	    echo $$f >> $(JAVA_SRCLIST_FILE) || exit 1;		\
	done

endif	# JAVA_SRCLIST_FILE

ifdef	JUNIT_TEST

ifdef	JUNIT_NAME
JUNIT_TESTNAME	= $(JUNIT_PACKAGE)$(JUNIT_NAME)
endif	# JUNIT_NAME

ifdef	ANT_JUNIT

ifeq	($(strip $(JAVA_JNI_SUBDIRS)),)
JUNIT_DEPS	= $(BUILD_XML)
else	# !empty(JAVA_JNI_SUBDIRS)
JUNIT_DEPS	= all
endif	# empty(JAVA_JNI_SUBDIRS)

# Run JUnit tests via Ant.
$(JUNIT_TARGET) check:	$(JUNIT_DEPS)
	$(ANT_BUILD) $(JUNIT_TARGET)

$(JUNIT_TARGET) check:	ANT_JUNIT_ENV += $(JUNIT_ENV)

else	# !ANT_JUNIT

# Run JUnit tests via text UI.
JUNIT_JFLAGS	= $(JAVA_FLAGS) -Dpflow.core.libpath=$(LINK_JNILIBDIR)
TEXTUI_FLAGS	= $(JUNIT_JFLAGS) $(JUNIT_CLASSPATH)
JUNIT_TEXTUI	= junit.textui.TestRunner

$(JUNIT_TARGET) check:	all
	$(JUNIT_ENV) $(JAVA) $(TEXTUI_FLAGS) $(JUNIT_TEXTUI) $(JUNIT_TESTNAME)

endif	# ANT_JUNIT

ifdef	JUNIT_SWINGUI

SWINGUI_FLAGS	= $(JUNIT_JFLAGS) $(JUNIT_CLASSPATH)

# Run JUnit tests via swing UI provided by JUnit.jar.
swing:	all
	$(JAVA) $(SWINGUI_FLAGS) $(JUNIT_SWINGUI) $(JUNIT_TESTNAME)

else	# !JUNIT_SWINGUI

swing:
	@echo "*** SKIP: JUnit does not support Swing UI."

endif	# JUNIT_SWINGUI

endif	# JUNIT_TEST

else	# !JAVA_CONFIG_MK

all install clean clobber doc:	FRC

endif	# JAVA_CONFIG_MK

endif	# !BUILD_JAVA_RULES_MK_INCLUDED
