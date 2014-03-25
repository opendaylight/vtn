#
# Copyright (c) 2014 NEC Corporation
# All rights reserved.
# 
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

##
## Makefile that installs external libraries used by web applications.
##

include ../../../build/config.mk

ifdef	JAVA_EXT_MK

include $(JAVA_CONFIG_MK)
include $(JAVA_EXT_MK)
include $(BLDDIR)/tomcat-defs.mk

# Relative path from TOMCAT_SHLIBDIR to INST_JARDIR/ext.
JAVA_REL_EXTDIR	= ../../../lib/java/jar/ext

# Define files to be installed to TOMCAT_SHLIBDIR.
DEST_SHLIBDIR	= $(DESTDIR)$(TOMCAT_SHLIBDIR)
DEST_EXTDIR	= $(DEST_SHLIBDIR)
DEST_SHLIBFILES	= $(TOMCAT_EXT_LIBS:%=$(DEST_SHLIBDIR)/%)

DEST_DIRS	= $(DEST_SHLIBDIR)

all clean clobber:	FRC

install:	$(DEST_SHLIBFILES)

$(DEST_SHLIBFILES):	$(DEST_SHLIBDIR)
	@echo "=== Installing $@";					\
	$(LN_S) $(JAVA_REL_EXTDIR)/$(notdir $@) $@

$(DEST_DIRS):
	@echo "=== Installing $@";					\
	$(INSTALL_DIRS) $@

include $(BLDDIR)/rules.mk

else	# !JAVA_EXT_MK

all install clean clobber:

.PHONY:	all install clean clobber

endif	# JAVA_EXT_MK
