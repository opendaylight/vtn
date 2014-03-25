#
# Copyright (c) 2014 NEC Corporation
# All rights reserved.
# 
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

##
## Makefile that installs Apache Tomcat libraries.
##

include ../../../build/config.mk

ifdef	JAVA_EXT_MK

include $(JAVA_CONFIG_MK)
include $(JAVA_EXT_MK)
include $(BLDDIR)/tomcat-defs.mk

# Define files to be installed to TOMCAT_LIBDIR.
DEST_LIBDIR	= $(DESTDIR)$(TOMCAT_LIBDIR)
DEST_LIBFILES	= $(TOMCAT_LIBS:%=$(DEST_LIBDIR)/%)

# Define files to be installed to TOMCAT_BINDIR.
DEST_BINDIR	= $(DESTDIR)$(TOMCAT_BINDIR)
DEST_BINFILES	= $(TOMCAT_BOOTLIBS:%=$(DEST_BINDIR)/%)

DEST_DIRS	= $(DEST_LIBDIR) $(DEST_BINDIR)

all clean clobber:	FRC

install:	install-lib install-bin

# Install Tomcat libraries to TOMCAT_LIBDIR.
install-lib:	$(DEST_LIBFILES)

# Install Tomcat libraries to TOMCAT_BINDIR.
install-bin:	$(DEST_BINFILES)

$(DEST_DIRS):
	@echo "=== Installing $@";					\
	$(INSTALL_DIRS) $@

include $(BLDDIR)/rules.mk
include $(TOMCAT_RULES_MK)

else	# !JAVA_EXT_MK

all install clean clobber:

.PHONY:	all install clean clobber

endif	# JAVA_EXT_MK
