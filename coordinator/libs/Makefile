#
# Copyright (c) 2012-2014 NEC Corporation
# All rights reserved.
# 
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

##
## Makefile that drives the production of UNC system libraries.
##

include ../build/config.mk

# Collect sub directories which contain Makefile.
# Note that the command name must be the directory name.
SUBDIRS		:= $(shell $(LISTFILE) -mc '^Makefile$$' .)
$(eval $(call LISTFILE_ASSERT,$(SUBDIRS)))

include $(CORE_BLDDIR)/subdirs.mk

# Directory build dependencies.
mgmt:		libunc_odbc
