#
# Copyright (c) 2012-2014 NEC Corporation
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

ifndef	UNC_BUILD_JAVA_RULES_MK_INCLUDED

UNC_BUILD_JAVA_RULES_MK_INCLUDED	:= 1

ifneq	($(strip $(JAVA_WEBROOT)),)

include $(BLDDIR)/tomcat-defs.mk

# Create Ant project file for web application.
MKJPROJECT_EXTRA_ARGS	+= -w $(JAVA_WEBROOT)
MKJPROJECT_EXTRA_ARGS	+= -W $(TOMCAT_APPDIR)

# Tomcat uses Log4J for its logging, and it initializes Log4J on startup.
# So any Log4J configuration file in web application should not be shipped
# in order to prevent web application initializing Log4J.
JAVA_LOG4J_CONF		= '**/log4j.properties' '**/log4j.xml'
MKJPROJECT_EXTRA_ARGS	+= $(JAVA_LOG4J_CONF:%=-x %)

# Create web application by the default target.
JAVA_ALL_TARGET		= war

# Install web application on "make install".
JAVA_INSTALL_TARGET	= install-webapp

# Append Tomcat JAR files to classpath.
JAVA_BUILD_LIBS		+= $(TOMCAT_JARFILES)

# Specify installation directory of web application.
ANT_PROPS		+= pkg.webapp.base=$(DESTDIR)$(TOMCAT_APPDIR)

# Remove WAR file on "make clean".
CLEANFILES		+= $(OBJDIR)/$(JAVA_LIBNAME).war

endif	# !empty(JAVA_WEBROOT)

include $(CORE_BLDDIR)/java-rules.mk

ifneq	($(strip $(JAVA_WEBROOT)),)

# Rules to invoke Apache ant.
war:	build
war:	ANT_TARGET = war

install-webapp:	build
install-webapp:	ANT_TARGET = install-webapp

install-war:	build
install-war:	ANT_TARGET = install-war

endif	# !empty(JAVA_WEBROOT)

endif	# !UNC_BUILD_JAVA_RULES_MK_INCLUDED
