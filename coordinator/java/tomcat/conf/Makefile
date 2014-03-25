#
# Copyright (c) 2014 NEC Corporation
# All rights reserved.
# 
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

##
## Makefile that installs configuration files for Apache Tomcat.
##

include ../../../build/config.mk

ifdef	JAVA_EXT_MK

include $(BLDDIR)/tomcat-defs.mk

# Configuration files to be installed.
CONF_FILES	= catalina.properties context.xml
CONF_FILES	+= server.xml web.xml tomcat-env.sh
DEST_CONFDIR	= $(DESTDIR)$(TOMCAT_CONFDIR)
DEST_FILES	= $(CONF_FILES:%=$(DEST_CONFDIR)/%)
DEST_DIRS	= $(DEST_CONFDIR)

# Log4j configuration file needs to be installed to TOMCAT_LIBDIR.
LIB_FILES	= log4j.properties
DEST_LIBDIR	= $(DESTDIR)$(TOMCAT_LIBDIR)
DEST_FILES	+= $(LIB_FILES:%=$(DEST_LIBDIR)/%)
DEST_DIRS	+= $(DEST_LIBDIR)

all clean clobber:	FRC

install:	$(DEST_FILES)

$(DEST_CONFDIR)/%:	% $(DEST_CONFDIR) FRC
	@echo "=== Installing $@";					\
	$(INSTALL_FILES) $< $@

$(DEST_LIBDIR)/%:	% $(DEST_LIBDIR) FRC
	@echo "=== Installing $@";					\
	$(INSTALL_FILES) $< $@

$(DEST_DIRS):
	@echo "=== Installing $@";					\
	$(INSTALL_DIRS) $@

include $(BLDDIR)/rules.mk

else	# !JAVA_EXT_MK

all install clean clobber:

.PHONY:	all install clean clobber

endif	# JAVA_EXT_MK
