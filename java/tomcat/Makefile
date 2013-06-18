#
# Copyright (c) 2012-2013 NEC Corporation
# All rights reserved.
# 
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

##
## Makefile that installs Apache Tomcat binary distribution.
##

NEED_OBJDIR	:= 1

include ../../build/config.mk

ifdef	JAVA_EXT_MK

include $(JAVA_CONFIG_MK)
include $(JAVA_EXT_MK)
include $(BLDDIR)/tomcat-defs.mk

SUBDIRS		= unc-addon

# Install common libraries to TOMCAT_LIBDIR.
DEST_LIBDIR	= $(DESTDIR)$(TOMCAT_LIBDIR)
COMMON_LIBS	= pfc_util.jar pfc_ipc.jar
DEST_LIBFILES	= $(COMMON_LIBS:%=$(DEST_LIBDIR)/%)

# Install shared libraries to TOMCAT_SHLIBDIR.
DEST_SHLIBDIR	= $(DESTDIR)$(TOMCAT_SHLIBDIR)
DEST_EXTDIR	= $(DEST_SHLIBDIR)
DEST_SHLIBFILES	= $(JAVA_EXT_LIBS:%=$(DEST_SHLIBDIR)/%)

all:	all-subdirs

install:	install-lib install-shlib
	$(MAKE) install-subdirs

# Install JAR file links to TOMCAT_LIBDIR.
install-lib:	$(DEST_LIBFILES)

$(DEST_LIBDIR):
	@echo "=== Installing $@";					\
	$(INSTALL_DIRS) $@

$(DEST_LIBDIR)/%:	$(DEST_LIBDIR) FRC
	@echo "=== Installing $@";					\
	$(LN_S) $(TOMCAT_RJARDIR)/$* $@

# Install JAR file links to TOMCAT_SHLIBDIR.
install-shlib:	$(DEST_SHLIBFILES)

# Build sub directories.
$(SUBDIRS):	FRC
	$(MAKE) -C $@ $(SUBDIR_TARGET)

all-subdirs install-subdirs clean-subdirs:	$(SUBDIRS)

all-subdirs:		SUBDIR_TARGET = all
install-subdirs:	SUBDIR_TARGET = install
clean-subdirs:		SUBDIR_TARGET = install

include $(BLDDIR)/rules.mk
include $(JAVA_EXT_RULES_MK)

else	# !JAVA_EXT_MK

all install clean clobber doc:

.PHONY:	all install clean clobber doc

endif	# JAVA_EXT_MK
