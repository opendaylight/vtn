#
# Copyright (c) 2014 NEC Corporation
# All rights reserved.
# 
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

##
## Makefile that drives the production of Apache Tomcat bootstrap JAR file.
##

NEED_OBJDIR	:= 1

include ../../../build/config.mk

ifdef	JAVA_EXT_MK

include $(JAVA_CONFIG_MK)
include $(JAVA_EXT_MK)
include $(BLDDIR)/tomcat-defs.mk
include $(CORE_BLDDIR)/ant-defs.mk

BOOTSTRAP_JAR	= bootstrap.jar
OBJ_BOOTSTRAP	= $(OBJDIR)/$(BOOTSTRAP_JAR)
CLASSDIR	= $(OBJDIR)/classes
CLEANFILES	+= $(OBJ_BOOTSTRAP) $(CLASSDIR)

# Ant flags used to build bootstrap.jar
ANT_PROPS	= classdir=$(CLASSDIR)
ANT_PROPS	+= bootstrap.jar=$(OBJ_BOOTSTRAP)
ANT_PROPS	+= catalina.jar=$(TOMCAT_CATALINA_JAR)
BUILD_XML	= build.xml

# bootstrap.jar needs to be installed to TOMCAT_BINDIR.
DEST_BINDIR	= $(DESTDIR)$(TOMCAT_BINDIR)
DEST_BINFILES	= $(BOOTSTRAP_JAR:%=$(DEST_BINDIR)/%)

all:	$(OBJ_BOOTSTRAP)

$(OBJ_BOOTSTRAP):	$(TOMCAT_CATALINA_JAR)
	$(ANT_BUILD)

install:	$(DEST_BINFILES)

$(DEST_BINDIR)/%:	$(OBJDIR)/% $(DEST_BINDIR) FRC
	@echo "=== Installing $@";					\
	$(INSTALL_FILES) $< $@

$(DEST_BINDIR):
	@echo "=== Installing $@";					\
	$(INSTALL_DIRS) $@

include $(BLDDIR)/rules.mk

else	# !JAVA_EXT_MK

all install clean clobber:

.PHONY:	all install clean clobber

endif	# JAVA_EXT_MK
