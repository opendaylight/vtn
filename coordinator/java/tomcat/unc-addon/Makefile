#
# Copyright (c) 2012-2013 NEC Corporation
# All rights reserved.
# 
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

##
## Makefile that drives production of Tomcat server add-on for UNC.
##

JAVA_BLDDIR	:= ../../../build
NEED_OBJDIR	:= 1

include $(JAVA_BLDDIR)/config.mk

ifdef	JAVA_EXT_MK

include $(JAVA_BLDDIR)/java-defs.mk
include $(JAVA_BLDDIR)/tomcat-defs.mk

# Attributes for manifest file.
JAVA_SPEC_TITLE		= Tomcat add-ons for UNC.
JAVA_IMPL_TITLE		= $(JAVA_SPEC_TITLE)

# Specify class libraries.
JAVA_LIBS		= pfc_util.jar pfc_ipc.jar

JAVA_BUILD_LIBS		= $(TOMCAT_CATALINA_JAR)
JAVA_BUILD_LIBS		+= $(TOMCAT_JULI_JAR)

JAVA_NO_CLASSPATH	= 1

# Install JAR file as Tomcat common library.
JAVA_INSTALL_DIR	= $(TOMCAT_LIBDIR)

# Install properties of global resources.
GRES_PROP		= unc-gres.properties
DEST_GRES_PROP_DIR	= $(DESTDIR)$(TOMCAT_LIBDIR)
INST_GRES_PROP		= $(GRES_PROP:%=$(DEST_GRES_PROP_DIR)/%)
JAVA_EXTRA_INSTALL	= install-prop

include $(BLDDIR)/java-rules.mk

install-prop:	$(INST_GRES_PROP)

$(DEST_GRES_PROP_DIR)/%:	% FRC
	@$(call CMD_INSTALL_FILE,$<,$@)

else	# !JAVA_EXT_MK

all install clean clobber doc:

.PHONY:	all install clean clobber doc

endif	# JAVA_EXT_MK
