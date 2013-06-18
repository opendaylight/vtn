#
# Copyright (c) 2012-2013 NEC Corporation
# All rights reserved.
# 
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

##
## Makefile that drives the production of vtn-webapi web application
##

include ../defs.mk

ifdef	JAVA_EXT_MK

# Attributes for manifest file.
JAVA_SPEC_TITLE         = VTN Service WEB API Library
JAVA_IMPL_TITLE         = $(JAVA_SPEC_TITLE)

# Use PFC-IPC and PFC-Core utility library.
JAVA_LIBS		= pfc_ipc.jar pfc_util.jar vtn-javaapi.jar

# Use both Google gson and org.json.
JAVA_EXTLIBS		= $(GOOGLE_GSON) $(ORG_JSON)

# Replace keywords in properties file.
JAVA_TEXT_FILTERS	= -f UNC_JAVA_LIBDIR '$(INST_JAVADIR)/jar'

include ../rules.mk

else	# !JAVA_EXT_MK

all install clean clobber doc:

.PHONY:	all install clean clobber doc

endif	# JAVA_EXT_MK
