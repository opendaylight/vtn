#
# Copyright (c) 2012-2013 NEC Corporation
# All rights reserved.
# 
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

##
## Makefile that drives the production of JNI library for pfc_util.jar.
##

include ../../jni-defs.mk

SHLIB_NAME	= libpfc_util_jni

C_SOURCES	=	\
	hostaddr.c	\
	log.c		\
	misc.c		\
	uint.c

PFC_LIBS	= libpfc_util libpfc_jni

# Add header search path to include libpfc_jni internal header.
PFCLIB_INCDIRS	= libpfc_jni
EXTRA_INCDIRS	+= $(PFCLIB_INCDIRS:%=../../../libs/%)

# Generate JNI header file.
JAVAH_CLASSNAMES	= LogSystem TraceLogImpl
JAVAH_CLASSNAMES	+=  Logger$$TraceLogger
JAVAH_CLASSNAMES	+= HostAddress
JAVAH_CLASSNAMES	+= UnsignedInteger
JNI_JAVAH_CLASSES	= $(JAVAH_CLASSNAMES:%=org.opendaylight.vtn.core.util.%)

include ../../jni-rules.mk
