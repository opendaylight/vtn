#
# Copyright (c) 2010-2013 NEC Corporation
# All rights reserved.
# 
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

##
## Common rules to build shared library.
## This makefile is designed to be included after shared-defs.mk is included.
##
## Command to link objects is determined by CXX_SOURCES macro. If it is empty,
## $(CC) is used as linker. If not empty, $(CXX) is used.
##

ifndef	BUILD_SHARED_RULES_MK_INCLUDED

BUILD_SHARED_RULES_MK_INCLUDED	:= 1

OBJ_SHLIB_SO	= $(OBJDIR)/$(SHLIB_SO)
OBJ_SONAME	= $(OBJDIR)/$(SONAME)
OBJ_SHLIB_FILE	= $(OBJDIR)/$(SHLIB_FILE)
OBJ_OBJECTS	= $(OBJECTS:%=$(OBJDIR)/%)

ifdef	EXPORT_LIBS
EXPORT_LIBDIR	?= $(LINK_LIBDIR)
LINK_OBJFILES	= $(SHLIB_ALLFILES:%=$(EXPORT_LIBDIR)/%)
SHLIB_TARGET	= $(LINK_OBJFILES)
else	# !EXPORT_LIBS
ifdef	SHLIB_DLOPEN
SHLIB_TARGET	= $(OBJ_SONAME)
OBJ_SHLIB_SO	=
else	# !SHLIB_DLOPEN
SHLIB_TARGET	= $(OBJ_SHLIB_SO)
endif	# SHLIB_DLOPEN
endif	# EXPORT_LIBS

CLEANFILES	+= $(sort $(SHLIB_ALLFILES:%=$(OBJDIR)/%))
CLEANFILES	+= $(LINK_OBJFILES) $(OBJ_OBJECTS)

CMD_DEST_INSTALL	=						\
	libdir=$(2)$(1);						\
	if [ ! -d $$libdir ]; then					\
	    echo "=== Installing $$libdir";				\
	    $(INSTALL_DIRS) $$libdir || exit 1;				\
	fi;								\
	echo "=== Installing $$libdir/$(SHLIB_FILE)";			\
	$(INSTALL_SHLIBS) $(OBJ_SHLIB_FILE) $$libdir || exit 1;		\
	for link in $(SHLIB_LINKS); do					\
	    dst="$$libdir/$$link";					\
	    echo "=== Installing $$dst";				\
	    $(LN) -sf $(SHLIB_FILE) $$dst || exit 1;			\
	done

CMD_INSTALL	= $(call CMD_DEST_INSTALL,$(1),$(DESTDIR))

# Update IPC struct headers if needed.
EXTRA_INSTALL	+= ipc-header-install

#all:	$(SHLIB_TARGET) $(EXTRA_TARGET) $(SYS_EXTRA_TARGET)
all:   $(GENERATE_CODE) $(SHLIB_TARGET) $(EXTRA_TARGET) $(SYS_EXTRA_TARGET)

ifndef	NO_SO_VERSION

ifdef	OBJ_SHLIB_SO
$(OBJ_SHLIB_SO):	$(OBJ_SONAME)
	$(LN) -sf $(SONAME) $@
endif	# OBJ_SHLIB_SO

$(OBJ_SONAME):	$(OBJ_SHLIB_FILE)
	$(LN) -sf $(SHLIB_FILE) $@

endif	# !defined(NO_SO_VERSION)

include	$(BLDDIR)/rules.mk

SHLIB_CMD	= $(OBJ_CMDDIR)/$(SHLIB_SO).cmd

ifdef	SHLIB_BINARY

CMDRULE_COPY_SO	=							\
	echo "=== Copying $< to $@";					\
	$(INSTALL_SHLIBS) $< $@

ifeq	($(strip $(realpath $(SHLIB_BINARY))),)
$(error FATAL: $(SHLIB_NAME) does not support your target host)
endif	# $(SHLIB_BINARY) is not found

$(OBJ_SHLIB_FILE) $(SHLIB_CMD):	$(SHLIB_BINARY) FRC
	@$(call CMD_EXECUTE,COPY_SO,$(SHLIB_CMD),$?)

else	# !SHLIB_BINARY

LINK	= $(CC)
LD_MODE	= $(CC_MODE)

ifneq	($(strip $(CXX_SOURCES)),)

# Use CXX to link shared object.
LINK	= $(CXX)
LD_MODE	= $(CXX_MODE)
LDLIBS	+= $(CXX_LDLIBS)

endif	# !empty(CXX_SOURCES)

# Rebuild target if any of archives is newer than the target.
PFCLIB_LINK_ARCHIVES	:= $(strip $(filter $(PFCLIB_ARCHIVES),$(PFC_LIBS)))
ifneq	($(PFCLIB_LINK_ARCHIVES),)
SHLIB_DEPS	+=							\
	$(shell for i in $(PFCLIB_LINK_ARCHIVES); do			\
	    echo $(OBJROOT)/libs/$$i/$$i.a;				\
	  done)
endif	# !empty(PFCLIB_LINK_ARCHIVES)

CMDRULE_LINK_SO	=							\
	set -e;								\
	echo 'cd $(OBJDIR);'						\
	      $(LINK) $(LDFLAGS) -o $(SHLIB_FILE) $(OBJECTS) $(LDLIBS);	\
	cd $(OBJDIR);							\
	$(LINK) $(LDFLAGS) -o $(SHLIB_FILE) $(OBJECTS) $(LDLIBS)

$(OBJ_SHLIB_FILE) $(SHLIB_CMD):	$(OBJ_OBJECTS) $(SHLIB_DEPS) FRC
	@$(call CMD_EXECUTE,LINK_SO,$(SHLIB_CMD),$?)

endif	# SHLIB_BINARY

endif	# !BUILD_SHARED_RULES_MK_INCLUDED
