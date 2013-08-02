#
# Copyright (c) 2012-2013 NEC Corporation
# All rights reserved.
# 
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

##
## Common configurations to build PFC core JNI libraries.
##

JNI_BLDDIR	?= ../../../build
NEED_OBJDIR	:= 1

include $(JNI_BLDDIR)/config.mk
include $(JNI_BLDDIR)/jni-defs.mk
