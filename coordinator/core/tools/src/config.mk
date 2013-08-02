#
# Copyright (c) 2011-2013 NEC Corporation
# All rights reserved.
# 
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

##
## Makefile that defines PFC-Core build tool specific configuration.
##

# Place objects to current directory.
OBJDIR		:= .

# Don't use dirpath command to determine OBJDIR.
NO_DIRPATH	:= 1

# Don't link PFC libraries.
NO_PFCLIBS	:= 1

include ../../../build/config.mk
