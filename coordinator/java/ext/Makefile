#
# Copyright (c) 2012-2013 NEC Corporation
# All rights reserved.
# 
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

##
## Makefile that installs third-party JAR files.
##

include ../../build/config.mk

ifdef	JAVA_EXT_MK

include $(JAVA_CONFIG_MK)
include $(JAVA_EXT_MK)

# Path to installation directory.
DEST_EXTDIR	= $(DEST_EXTJARDIR)
DEST_JARFILES	= $(JAVA_EXT_LIBS:%=$(DEST_EXTDIR)/%)

# Nothing to do on "make all".
all:

# Install third-party JAR files to the installation root directory.
install:	$(DEST_JARFILES)

include $(BLDDIR)/rules.mk
include $(JAVA_EXT_RULES_MK)

else	# !JAVA_EXT_MK

all install clean clobber doc:

.PHONY:	all install clean clobber doc

endif	# JAVA_EXT_MK
