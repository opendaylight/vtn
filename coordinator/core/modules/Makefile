#
# Copyright (c) 2010-2013 NEC Corporation
# All rights reserved.
# 
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

##
## Makefile that drives the production of all PFC modules.
##

include ../build/config.mk

# Collect sub directories which contain Makefile.
SUBDIRS		:= $(shell $(LISTFILE) -mc '^Makefile$$' .)
$(eval $(call LISTFILE_ASSERT,$(SUBDIRS)))

include ../build/subdirs.mk
