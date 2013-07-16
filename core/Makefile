#
# Copyright (c) 2010-2013 NEC Corporation
# All rights reserved.
# 
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

##
## Makefile that drives the production of PFC binary distribution.
##

SUBDIRS		= ipc include libs cmds modules
RPM_SUBDIRS	= cmds

# Don't use dirpath command.
NO_DIRPATH	:= 1

CONFIGURE	= ./configure

ifeq	($(realpath build/config.mk),)
$(error Run "$(CONFIGURE)" at first.)
endif	# build/config.mk does not exist

include build/config.mk

ifdef	JAVA_CONFIG_MK
SUBDIRS		+= java
endif	# JAVA_CONFIG_MK

include build/subdirs.mk
include build/rpm-rules.mk
include tools/version.mk

CONFIG_STATUS	= $(OBJROOT)/config.status

$(CONFIG_STATUS):	$(CONFIGURE)
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

# Preserve tools version.
TOOLS_VERSION_FILE	= $(OBJROOT)/tools-version

# Check to see whether build tools should be rebuilt.
tools-check:	$(CONFIG_STATUS) FRC
	@set -e;							\
	if [ -r "$(TOOLS_VERSION_FILE)" ]; then				\
	    current=`$(CAT) $(TOOLS_VERSION_FILE)`;			\
	fi;								\
	if [ "$$current" = "$(TOOLS_VERSION)" ]; then			\
	    exit 0;							\
	fi;								\
	echo "*** Build tools must be rebuilt.";			\
	$(MAKE) -C tools DESTDIR= distclean;				\
	$(MAKE) -C tools DESTDIR= install;				\
	$(MKDIR) -p $(OBJROOT);						\
	echo $(TOOLS_VERSION) > $(TOOLS_VERSION_FILE)

tools-rebuild:	FRC
	@$(RM) $(TOOLS_VERSION_FILE)
	$(MAKE) tools-check

DISTCLEANFILES	:= $(OBJROOT)
DISTCLEANFILES	+= build/config.mk build/config.pl

# Clean up all build outputs for source distribution.
distclean:	FRC
	$(MAKE) -C tools $@
	$(RM) -rf $(DISTCLEANFILES)

# No need to update IPC struct headers on full build.
SKIP_IPC_UPDATE	= 1
export	SKIP_IPC_UPDATE

# Directory build dependencies.
ipc:		tools-check
include:	ipc
libs:		include
cmds:		libs
modules:	libs

ifdef	JAVA_CONFIG_MK
java:		libs
endif	# JAVA_CONFIG_MK
