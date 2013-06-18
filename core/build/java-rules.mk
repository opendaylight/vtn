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

# Library name must be identical to build directory name.
JAVA_LIBNAME	:= $(notdir $(CURDIR))

# Install Java library.
JAVA_DO_INSTALL	:= 1

# Java source directories.
JAVA_SRCDIR	= src $(JAVA_EXTRA_SRCDIR)

# Options to be passed to javac.
JAVAC_FLAGS	= $(JAVAC_ERRWARN) $(JAVAC_LINT) $(JAVAC_EXTRA_FLAGS)

# JAR file path.
JAVA_LIBFILE	= $(JAVA_LIBNAME).jar
JARFILE		= $(JAVA_OBJDIR)/$(JAVA_LIBFILE)

include $(BLDDIR)/ant-defs.mk

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

else	# !JAVA_CONFIG_MK

all install clean clobber doc:	FRC

endif	# JAVA_CONFIG_MK

endif	# !BUILD_JAVA_RULES_MK_INCLUDED
