#
# Copyright (c) 2013 NEC Corporation
# All rights reserved.
# 
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

##
## Common rules to run unit tests under sub directories.
##

include $(TEST_SRCROOT)/build/config.mk

# Collect sub directories which contain Makefile.
SUBDIRS	:= $(shell $(LISTFILE) -mc '^Makefile$$' .)
$(eval $(call LISTFILE_ASSERT,$(SUBDIRS)))

include $(BLDDIR)/subdirs.mk

# Run tests.
test check:	FRC
	@for d in $(SUBDIRS); do					\
	    $(MAKE) -C $$d test || exit 1;				\
	done
