#
# Copyright (c) 2012-2013 NEC Corporation
# All rights reserved.
# 
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

##
## Makefile that drives the production of Java libraries.
##

NEED_OBJDIR	:= 1

include ../build/config.mk

SUBDIRS		= 	\
	pfc_ipc		\
	pfc_util

include $(BLDDIR)/subdirs.mk

# Include rules to build all-in-one code documentation.

JAVADOC_TITLE	= PFC-Core
JAVA_SUBDIRS	= $(SUBDIRS)

include $(BLDDIR)/javadoc.mk

# Directory build dependencies.
pfc_ipc:	pfc_util
