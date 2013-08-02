#
# Copyright (c) 2012-2013 NEC Corporation
# All rights reserved.
# 
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

##
## Makefile that drives the production of Java bindings for IPC framework.
##

include ../defs.mk

# Attributes for manifest file.
JAVA_SPEC_TITLE		= PFC IPC framework classes
JAVA_IMPL_TITLE		= Java bindings for PFC IPC framework

# Build JNI library.
JAVA_JNI_SUBDIRS	= jni_client

# This library uses PFC-Core utilities.
JAVA_LIBS		= pfc_util.jar

# Create JAR file index.
JAVA_JAR_INDEX		= 1

include ../rules.mk
