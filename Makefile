#
# Copyright (c) 2012-2013 NEC Corporation
# All rights reserved.
# 
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

##
## Makefile that drives the production of UNC system.
##

SUBDIRS		= core libs cmds modules java scripts sql

# Don't use dirpath command.
NO_DIRPATH	:= 1

CONFIGURE	= ./configure
CORE_CONFIGURE	= ./core/configure

ifeq	($(realpath build/config.mk),)
$(error Run "$(CONFIGURE)" at first.)
endif	# build/config.mk does not exist

include build/config.mk
include $(CORE_BLDDIR)/subdirs.mk

# No need to update IPC struct headers on full build.
SKIP_IPC_UPDATE	= 1
export	SKIP_IPC_UPDATE

CONFIG_STATUS	= $(OBJROOT)/config.status
VERSION_FILE	= VERSION

$(CONFIG_STATUS):	$(CONFIGURE) $(CORE_CONFIGURE) $(VERSION_FILE)
	@if [ ! -r "$@" ]; then						\
	    echo '*** ERROR: Run "$(CONFIGURE)" at first.';		\
	    exit 1;							\
	fi;								\
	echo "=== Recheck configuration.";				\
	$(CONFIGURE) --recheck=$@;					\
	status=$$?;							\
	if [ $$status -ne 0 ]; then					\
	    $(RM) $@;							\
	    echo "*** ERROR: $(CONFIGURE) failed.";			\
	    exit $$status;						\
	fi

DISTCLEANFILES	:= $(OBJROOT)
DISTCLEANFILES	+= build/config.mk

env:	$(CONFIG_STATUS)

# Clean up all build outputs for source distribution.
distclean:	FRC
	$(MAKE) -C core $@
	$(RM) -rf $(DISTCLEANFILES)

# Export public PFC-Core header files by force.
core-headers:
	$(MAKE) -C core/libs EXPORT_HEADER_DIR=$(CORE_EXP_INCDIR) export-header

# Run unit tests.
test-%:	FRC
	$(MAKE) -C core/test $*
	$(MAKE) -C test $*

# Directory build dependencies.
core:		env
libs:		core
cmds:		libs
modules:	libs
java:		libs
scripts:	core
sql:		core
