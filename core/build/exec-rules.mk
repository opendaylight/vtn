#
# Copyright (c) 2010-2013 NEC Corporation
# All rights reserved.
# 
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

##
## Common rules to build executable.
## This makefile is designed to be included after exec-defs.mk is included.
##
## Command to link objects is determined by CXX_SOURCES macro. If it is empty,
## $(CC) is used as linker. If not empty, $(CXX) is used.
##

ifndef	BUILD_EXEC_RULES_MK_INCLUDED

BUILD_EXEC_RULES_MK_INCLUDED	:= 1

# Executable file name to be installed.
INST_EXEC_NAME	?= $(EXEC_NAME)

OBJ_EXEC	= $(OBJDIR)/$(EXEC_NAME)
OBJ_OBJECTS	= $(OBJECTS:%=$(OBJDIR)/%)

CLEANFILES	+= $(OBJ_EXEC) $(OBJ_OBJECTS)

# Install symbolic link to alternative command path.
INSTALL_ALTLINK	:= :

ifdef	ALTLINK_INSTALL
ifdef	ALTLINK_BINDIR
ifdef	ALTLINK_BINSRC

INSTALL_ALTLINK	:=							\
	for dir in $(ALTLINK_BINDIR); do				\
	    if [ ! -d $$dir ]; then					\
		echo "=== Installing $$dir";				\
		$(INSTALL_DIRS) $$dir;					\
	    fi;								\
	    src=$(ALTLINK_BINSRC)/$(INST_EXEC_NAME);			\
	    dst=$$dir/$(INST_EXEC_NAME);				\
	    echo "=== Installing $$dst";				\
	    $(LN_S) $$src $$dst;					\
	done

ALTLINK_CLEANFILES	:= $(ALTLINK_BINDIR:%=%/$(INST_EXEC_NAME))

endif	# ALTLINK_BINSRC
endif	# ALTLINK_BINDIR
endif	# ALTLINK_INSTALL

CMD_INSTALL	=							\
	set -e;								\
	bindir=$(DESTDIR)$(1);						\
	if [ ! -d $$bindir ]; then					\
	    echo "=== Installing $$bindir";				\
	    $(INSTALL_DIRS) $$bindir;					\
	fi;								\
	dst=$$bindir/$(INST_EXEC_NAME);					\
	echo "=== Installing $$dst";					\
	$(INSTALL_PROGS) $(OBJ_EXEC) $$dst;				\
	$(INSTALL_ALTLINK)

# Update IPC struct headers if needed.
EXTRA_INSTALL	+= ipc-header-install

all:	$(OBJ_EXEC) $(EXTRA_TARGET) $(SYS_EXTRA_TARGET)

include	$(BLDDIR)/rules.mk

LINK	= $(CC)
LD_MODE	= $(CC_MODE)

ifneq	($(strip $(CXX_SOURCES)),)

# Use CXX to link executable.
LINK	= $(CXX)
LD_MODE	= $(CXX_MODE)
LDLIBS	+= $(CXX_LDLIBS)

endif	# !empty(CXX_SOURCES)

# Let the linker search libraries under $(LINK_LIBDIR).
ifndef	NO_PFCLIBS
LDFLAGS	+= $(LINK_LIBDIR:%=-Wl,-rpath-link %)
endif	# !NO_PFCLIBS

# Rebuild target if any of archives is newer than the target.
PFCLIB_LINK_ARCHIVES	:= $(strip $(filter $(PFCLIB_ARCHIVES),$(PFC_LIBS)))
ifneq	($(PFCLIB_LINK_ARCHIVES),)
EXEC_DEPS	+=							\
	$(shell for i in $(PFCLIB_LINK_ARCHIVES); do			\
	    echo $(OBJROOT)/libs/$$i/$$i.a;				\
	  done)
endif	# !empty(PFCLIB_LINK_ARCHIVES)

CMDRULE_LINK_EXEC	=						\
	set -e;								\
	echo 'cd $(OBJDIR);'						\
	      $(LINK) $(LDFLAGS) -o $@ $(OBJECTS) $(LDLIBS);		\
	cd $(OBJDIR);							\
	$(LINK) $(LDFLAGS) -o $@ $(OBJECTS) $(LDLIBS)

$(OBJ_EXEC) $(OBJ_CMDDIR)/$(EXEC_NAME).cmd:	$(OBJ_OBJECTS) $(EXEC_DEPS) FRC
	@$(call CMD_EXECUTE,LINK_EXEC,$(OBJ_CMDDIR)/$(EXEC_NAME).cmd,$?)

altlink-uninstall:
ifdef	ALTLINK_CLEANFILES
	$(RM) $(ALTLINK_CLEANFILES)
endif	# ALTLINK_CLEANFILES

endif	# !BUILD_EXEC_RULES_MK_INCLUDED
