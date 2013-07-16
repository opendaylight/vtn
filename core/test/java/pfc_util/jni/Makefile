#
# Copyright (c) 2012-2013 NEC Corporation
# All rights reserved.
# 
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

##
## Makefile that drives the production of JNI library for pfc_util.jar test.
##

include ../../jni-defs.mk

SHLIB_NAME	= libpfc_util_test_jni

C_SOURCES	=	\
	buildconf.c	\
	misc.c		\
	uint.c

PFC_LIBS	= libpfc_util libpfc_jni

# Generate JNI header file.
JAVAH_CLASSNAMES	= TestBase
JAVAH_CLASSNAMES	+= UnsignedIntegerTest
JNI_JAVAH_CLASSES	= org.opendaylight.vtn.core.CoreSystemTest
JNI_JAVAH_CLASSES	+= $(JAVAH_CLASSNAMES:%=org.opendaylight.vtn.core.util.%)

include ../../jni-rules.mk
