#
# Copyright (c) 2012-2013 NEC Corporation
# All rights reserved.
# 
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

##
## Makefile that drives the production of JUnit test for Java bindings of
## IPC framework APIs.
##

include ../defs.mk

JUNIT_PACKAGE	= org.opendaylight.vtn.core.ipc.

# Default class name to be invoked.
JUNIT_TESTNAME	= $(JUNIT_PACKAGE)AllTests

# Build JNI library for test.
JAVA_JNI_SUBDIRS	= jni

include ../rules.mk
