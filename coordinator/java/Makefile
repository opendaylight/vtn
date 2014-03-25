#
# Copyright (c) 2012-2014 NEC Corporation
# All rights reserved.
# 
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

##
## Makefile that drives the production of Java libraries.
##

NEED_OBJDIR	:= 1

include ../build/config.mk

# Name of the sub directory which contains third-party JAR files.
EXTDIR		= ext

# Name of the sub directory which installs Apache Tomcat binary distribution.
TOMCAT_DIR	= tomcat

# Name of the sub directory which installs system files.
FILES_DIR	= files

# Regular expression to eliminate sub directories from JAVA_SUBDIRS.
PAT_EXCLUDES	= '^($(EXTDIR)|$(TOMCAT_DIR)|$(FILES_DIR))$$'

# Collect sub directories which contain Makefile.
# Note that the JAR file name without ".jar" suffix must be the directory name.
JAVA_SUBDIRS	:= $(shell $(LISTFILE) -mc '^Makefile$$' -e $(PAT_EXCLUDES) .)
$(eval $(call LISTFILE_ASSERT,$(JAVA_SUBDIRS)))

SUBDIRS		= $(EXTDIR) $(TOMCAT_DIR) $(FILES_DIR) $(JAVA_SUBDIRS)

include $(CORE_BLDDIR)/subdirs.mk

# Include rules to build all-in-one code documentation.

JAVADOC_TITLE		= UNC
JAVA_BLDDIR		= $(CORE_BLDDIR)
JAVA_SRCLIST_DEPS	= $(EXTDIR)

include $(CORE_BLDDIR)/javadoc.mk

# Directory build dependencies.
ifneq	($(strip $(JAVA_SUBDIRS)),)
$(JAVA_SUBDIRS):	$(TOMCAT_DIR)
endif	# !empty(JAVA_SUBDIRS)

$(TOMCAT_DIR):	$(EXTDIR)

vtn-webapi:	vtn-javaapi
