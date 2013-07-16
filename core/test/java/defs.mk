#
# Copyright (c) 2012-2013 NEC Corporation
# All rights reserved.
# 
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

##
## Common configurations to build JUnit tests for VTN-Core Java bindings.
##
## The name of the build directory must be equal to the target library name.
##
## In addition, the class name to be invoked by "make test" must be set to
## JUNIT_TESTNAME macro.
##

JAVA_BLDDIR	:= ../../../build
NEED_OBJDIR	:= 1
JUNIT_TEST	:= 1

include $(JAVA_BLDDIR)/config.mk
include $(JAVA_BLDDIR)/java-defs.mk
