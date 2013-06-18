#
# Copyright (c) 2010-2013 NEC Corporation
# All rights reserved.
# 
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

##
## Common rules to walk down all sub directories.
##
## The following macros needs to be defined in directory Makefile.
##
## SUBDIRS:		List of sub directories.
##
## SUBDIRS_NOBUILD:	List of sub directories which should not be built
##			by default.
##
## Note that dependencies of sub directory build may need to be defined for
## parallel build.
##

ifndef	BUILD_SUBDIRS_MK_INCLUDED

BUILD_SUBDIRS_MK_INCLUDED	:= 1

SUBDIRS_ALL	= $(SUBDIRS) $(SUBDIRS_NOBUILD)

all install:	$(SUBDIRS)

clean clobber:	$(SUBDIRS_ALL)

all:		SUBDIRS_TARGET = all
install:	SUBDIRS_TARGET = install
clean:		SUBDIRS_TARGET = clean
clobber:	SUBDIRS_TARGET = clobber

$(SUBDIRS_ALL):
	$(MAKE) -C $@ $(SUBDIRS_TARGET)

.PHONY: $(SUBDIRS_ALL) FRC

FRC:

endif	# !BUILD_SUBDIRS_MK_INCLUDED
