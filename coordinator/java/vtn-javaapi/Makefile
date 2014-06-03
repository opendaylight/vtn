#
# Copyright (c) 2012-2014 NEC Corporation
# All rights reserved.
#
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

##
## Makefile that drives the production of vtn-javaapi Java library.
##

include ../defs.mk

ifdef	JAVA_EXT_MK

# Attributes for manifest file.
JAVA_SPEC_TITLE		= VTN Service JavaAPI Library
JAVA_IMPL_TITLE		= $(JAVA_SPEC_TITLE)

# Use PFC-IPC and PFC-Core utility library.
JAVA_LIBS		= pfc_ipc.jar pfc_util.jar

# Define external libraries.
JAVA_EXTLIBS		= $(GOOGLE_GSON) $(COMMONS_NET) $(POSTGRESQL_JDBC)
JAVA_EXTLIBS		+= $(COMMONS_CODEC)

include ../rules.mk

else	# !JAVA_EXT_MK

all install clean clobber doc:

.PHONY:	all install clean clobber doc

endif	# JAVA_EXT_MK
