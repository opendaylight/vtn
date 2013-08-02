#
# Copyright (c) 2012-2013 NEC Corporation
# All rights reserved.
# 
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

##
## Makefile that drives the production of PFC-Core JNI helper library.
##

include ../defs.mk

ifdef	JAVA_CONFIG_MK

include $(JAVA_CONFIG_MK)

SHLIB_NAME	= libpfc_jni

C_SOURCES	=		\
	jni.c			\
	log.c

EXTRA_INCDIRS	= $(JNI_INCDIR)
EXTRA_CPPFLAGS	= -DPFC_JNI_BUILD -D_PFC_LIBPFC_JNI_BUILD
PFC_LIBS	= libpfc_util

# Define log identifier.
USE_LOG_IDENT	= 1

include ../rules.mk

else	# !JAVA_CONFIG_MK

all install clean clobber:

.PHONY:	all install clean clobber

endif	# JAVA_CONFIG_MK
