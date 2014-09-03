#
# Copyright (c) 2012-2014 NEC Corporation
# All rights reserved.
# 
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

##
## Common makefile to build UNC daemon using pfcd.
##
## The following macros need to be defined in Makefile:
##

NEED_OBJDIR	:= 1

include ../../build/config.mk

# The name of the daemon process.
ifndef	DAEMON_NAME
DAEMON_NAME	:= $(notdir $(CURDIR))
endif	# !DAEMON_NAME

ifndef	DAEMON_DESC
$(error DAEMON_DESC must be defined.)
endif	# !DAEMON_DESC

ifeq	($(strip $(DAEMON_DESC)),)
$(error DAEMON_DESC must not be empty.)
endif	# empty(DAEMON_DESC)

ifneq	($(DAEMON_NAME),uncd)

# Check mandatory parameters.
ifeq	($(strip $(PROCESS_TYPE)),)
$(error PROCESS_TYPE must be defined.)
endif	# empty(PROCESS_TYPE)

ifeq	($(strip $(START_ORDER)),)
$(error START_ORDER must be defined.)
endif	# empty(START_ORDER)

ifeq	($(strip $(STOP_ORDER)),)
$(error STOP_ORDER must be defined.)
endif	# empty(STOP_ORDER)

ifeq	($(strip $(CLEV_ORDER_ACT)),)
$(error CLEV_ORDER_ACT must be defined.)
endif	# empty(CLEV_ORDER_ACT)

include $(BLDDIR)/dmconf-defs.mk

# Use common template for the daemon configuration file.
ifndef	DMCONF_IN
COMMON_DMCONF	= 1
DMCONF_IN	= ../common/launcher.daemon.in
DMCONF		= $(DAEMON_NAME).daemon
OBJ_DMCONF	= $(DMCONF:%=$(OBJDIR)/%)
endif	# !DMCONF_IN

# Create hardlink to uncd and related commands.
DAEMON_HARDLINK	:= 1

endif	# DAEMON_NAME != uncd

# Identifier for syslog(3).
LOG_IDENT	= UNC.$(DAEMON_NAME)

# Determine module names to be written into the configuration file.
# Note that every pfcd-clone daemon must load "clstat" module.
DAEMON_MODULES	+= clstat
LOAD_MODULES	:= $(shell $(MODTOOL) -m -L $(DAEMON_MODULES))
$(eval $(call MODTOOL_ASSERT,$(LOAD_MODULES)))

# The name of the configuration file.
DAEMON_CONF	= $(DAEMON_NAME).conf
DAEMON_ATTR	= $(DAEMON_NAME).attr
DAEMON_CONF_IN	?= ../common/daemon.conf.in
DAEMON_ATTR_IN	?= ../common/daemon.attr.in
OBJ_DAEMON_CONF	= $(OBJDIR)/$(DAEMON_CONF)
OBJ_DAEMON_ATTR	= $(OBJDIR)/$(DAEMON_ATTR)
ALL_TARGET	= $(OBJ_DAEMON_CONF) $(OBJ_DAEMON_ATTR) $(SYS_EXTRA_TARGET)
CLEANFILES	= $(ALL_TARGET)

# Prefix of commands associated with this daemon.
CMD_PREFIX	:= $(shell echo $(DAEMON_NAME) | $(SED) -e 's,d$$,,')

# Set '#' to HASH_SIGN using perl because make can not treat '#'.
HASH_SIGN	:= $(shell $(PERL) -e 'print "\x23";')

# Installation path for the configuration file.
DEST_SYSCONFDIR		= $(DESTDIR)$(INST_SYSCONFDIR)
DEST_DAEMON_CONF	= $(DEST_SYSCONFDIR)/$(DAEMON_CONF)

# Installation path for the attributes file.
DEST_DAEMON_ATTR_DIR	= $(DESTDIR)$(INST_DATADIR)/daemon
DEST_DAEMON_ATTR	= $(DEST_DAEMON_ATTR_DIR)/$(DAEMON_ATTR)

ifdef	DAEMON_HARDLINK

# Path to executable files.
INST_UNCD	= $(INST_SBINDIR)/uncd
INST_DAEMON	= $(INST_SBINDIR)/$(DAEMON_NAME)
DEST_UNCD	= $(DESTDIR)$(INST_UNCD)
DEST_DAEMON	= $(DESTDIR)$(INST_DAEMON)

DAEMON_CONTROL		= $(CMD_PREFIX)_control
INST_UNC_CONTROL	= $(INST_BINDIR)/unc_control
INST_DAEMON_CONTROL	= $(INST_BINDIR)/$(DAEMON_CONTROL)
DEST_UNC_CONTROL	= $(DESTDIR)$(INST_UNC_CONTROL)
DEST_DAEMON_CONTROL	= $(DESTDIR)$(INST_DAEMON_CONTROL)

INST_UNC_MODCACHE	= $(INST_BINDIR)/unc_modcache
INST_DAEMON_MODCACHE	= $(INST_BINDIR)/$(CMD_PREFIX)_modcache
DEST_UNC_MODCACHE	= $(DESTDIR)$(INST_UNC_MODCACHE)
DEST_DAEMON_MODCACHE	= $(DESTDIR)$(INST_DAEMON_MODCACHE)

INSTALL_TARGET	+= $(DEST_DAEMON) $(DEST_DAEMON_CONTROL) $(DEST_DAEMON_MODCACHE)

endif	# DAEMON_HARDLINK

INSTALL_TARGET	+= $(DEST_DAEMON_CONF) $(DEST_DAEMON_ATTR) $(SYS_EXTRA_INSTALL)

# Path to daemon's working directory.
INST_WORKDIR	= $(INST_UNCWORKDIR)/$(DAEMON_NAME)

# Path to volatile file directory.
INST_RUNDIR	= $(INST_LOCALSTATEDIR)/run

# Path to module cache directory.
INST_MODCHDIR	= $(INST_UNCWORKDIR)/modules

# Year range to be embeded in copyright notice.
YEAR_RANGE	:= $(shell $(YEAR_RANGE_CMD))
$(eval $(call GENCOPY_ASSERT,$(YEAR_RANGE)))

# Rules to generate configuration file.
CONF_REPL_RULES	= -p %INST_WORKDIR% '$(INST_WORKDIR)'
CONF_REPL_RULES	+= -p %INST_RUNDIR% '$(INST_RUNDIR)'
CONF_REPL_RULES	+= -p %INST_MODULEDIR% '$(INST_MODULEDIR)'
CONF_REPL_RULES	+= -p %INST_MODCHDIR% '$(INST_MODCHDIR)'
CONF_REPL_RULES	+= -p %DAEMON_NAME% '$(DAEMON_NAME)'
CONF_REPL_RULES	+= -p %DAEMON_DESC% $(DAEMON_DESC)
CONF_REPL_RULES	+= -p %LOAD_MODULES% '$(LOAD_MODULES)'
CONF_REPL_RULES	+= -p %YEAR_RANGE% '$(YEAR_RANGE)'

ifeq	($(strip $(UNCD_CTRL_PERM)),)
# Comment out options.ctrl_perm.
CONF_REPL_RULES	+= -p %CTRL_PERM% '0700'
CONF_REPL_RULES	+= -p %CTRL_PERM_COMM% '$(HASH_SIGN)'
else	# !empty(UNCD_CTRL_PERM)
# Set permission bits to options.ctrl_perm.
CONF_REPL_RULES	+= -p %CTRL_PERM% '$(UNCD_CTRL_PERM)'
CONF_REPL_RULES	+= -p %CTRL_PERM_COMM% ''
endif	# empty(UNCD_CTRL_PERM)

ifeq	($(strip $(UNCD_USER)),)
# Comment out options.user.
CONF_REPL_RULES	+= -p %UNCD_USER% 'user'
CONF_REPL_RULES	+= -p %UNCD_USER_COMM% '$(HASH_SIGN)'
else	# !empty(UNCD_USER)
# Set user name to options.user.
CONF_REPL_RULES	+= -p %UNCD_USER% '$(UNCD_USER)'
CONF_REPL_RULES	+= -p %UNCD_USER_COMM% ''
endif	# empty(UNCD_USER)

ifeq	($(strip $(UNCD_GROUP)),)
# Comment out options.group.
CONF_REPL_RULES	+= -p %UNCD_GROUP% 'group'
CONF_REPL_RULES	+= -p %UNCD_GROUP_COMM% '$(HASH_SIGN)'
else	# !empty(UNCD_GROUP)
# Set group name to options.group.
CONF_REPL_RULES	+= -p %UNCD_GROUP% '$(UNCD_GROUP)'
CONF_REPL_RULES	+= -p %UNCD_GROUP_COMM% ''
endif	# empty(UNCD_GROUP)

ifeq	($(strip $(ADMIN_USER)),)
# Comment out options.admin_user.
CONF_REPL_RULES	+= -p %ADMIN_USER% 'user'
CONF_REPL_RULES	+= -p %ADMIN_USER_COMM% '$(HASH_SIGN)'
else	# !empty(ADMIN_USER)
# Set user name to options.admin_user.
CONF_REPL_RULES	+= -p %ADMIN_USER% '$(ADMIN_USER)'
CONF_REPL_RULES	+= -p %ADMIN_USER_COMM% ''
endif	# empty(ADMIN_USER)

ifeq	($(strip $(ADMIN_GROUP)),)
# Comment out options.admin_group.
CONF_REPL_RULES	+= -p %ADMIN_GROUP% 'group'
CONF_REPL_RULES	+= -p %ADMIN_GROUP_COMM% '$(HASH_SIGN)'
else	# !empty(ADMIN_GROUP)
# Set group name to options.admin_group.
CONF_REPL_RULES	+= -p %ADMIN_GROUP% '$(ADMIN_GROUP)'
CONF_REPL_RULES	+= -p %ADMIN_GROUP_COMM% ''
endif	# empty(ADMIN_GROUP)

CONF_REPL_RULES	+= -a %UNCD_USER% string,min=1,max=31
CONF_REPL_RULES	+= -a %UNCD_GROUP% string,min=1,max=31
CONF_REPL_RULES	+= -a %ADMIN_USER% string,min=1,max=31
CONF_REPL_RULES	+= -a %ADMIN_GROUP% string,min=1,max=31

# Set ipc_event.idle_timeout.
ifeq	($(strip $(IPCEVENT_IDLE_TIMEOUT)),)
CONF_REPL_RULES	+= -p %IPCEVENT_IDLE_TIMEOUT% '1000'
CONF_REPL_RULES	+= -p %IPCEVENT_IDLE_TIMEOUT_COMM% '$(HASH_SIGN)'
else	# !empty(IPCEVENT_IDLE_TIMEOUT)
CONF_REPL_RULES	+= -p %IPCEVENT_IDLE_TIMEOUT% '$(IPCEVENT_IDLE_TIMEOUT)'
CONF_REPL_RULES	+= -p %IPCEVENT_IDLE_TIMEOUT_COMM% ''
endif	# empty(IPCEVENT_IDLE_TIMEOUT)

# Set ipc_event.maxthreads.
ifeq	($(strip $(IPCEVENT_MAXTHREADS)),)
CONF_REPL_RULES	+= -p %IPCEVENT_MAXTHREADS% '32'
CONF_REPL_RULES	+= -p %IPCEVENT_MAXTHREADS_COMM% '$(HASH_SIGN)'
else	# !empty(IPCEVENT_MAXTHREADS)
CONF_REPL_RULES	+= -p %IPCEVENT_MAXTHREADS% '$(IPCEVENT_MAXTHREADS)'
CONF_REPL_RULES	+= -p %IPCEVENT_MAXTHREADS_COMM% ''
endif	# empty(IPCEVENT_MAXTHREADS)

# Set ipc_event.conn_interval.
ifeq	($(strip $(IPCEVENT_CONN_INTERVAL)),)
CONF_REPL_RULES	+= -p %IPCEVENT_CONN_INTERVAL% '60000'
CONF_REPL_RULES	+= -p %IPCEVENT_CONN_INTERVAL_COMM% '$(HASH_SIGN)'
else	# !empty(IPCEVENT_CONN_INTERVAL)
CONF_REPL_RULES	+= -p %IPCEVENT_CONN_INTERVAL% '$(IPCEVENT_CONN_INTERVAL)'
CONF_REPL_RULES	+= -p %IPCEVENT_CONN_INTERVAL_COMM% ''
endif	# empty(IPCEVENT_CONN_INTERVAL)

# Set ipc_event.keep_interval.
ifeq	($(strip $(IPCEVENT_KEEP_INTERVAL)),)
CONF_REPL_RULES	+= -p %IPCEVENT_KEEP_INTERVAL% '60000'
CONF_REPL_RULES	+= -p %IPCEVENT_KEEP_INTERVAL_COMM% '$(HASH_SIGN)'
else	# !empty(IPCEVENT_KEEP_INTERVAL)
CONF_REPL_RULES	+= -p %IPCEVENT_KEEP_INTERVAL% '$(IPCEVENT_KEEP_INTERVAL)'
CONF_REPL_RULES	+= -p %IPCEVENT_KEEP_INTERVAL_COMM% ''
endif	# empty(IPCEVENT_KEEP_INTERVAL)

# Set ipc_event.keep_timeout.
ifeq	($(strip $(IPCEVENT_KEEP_TIMEOUT)),)
CONF_REPL_RULES	+= -p %IPCEVENT_KEEP_TIMEOUT% '60000'
CONF_REPL_RULES	+= -p %IPCEVENT_KEEP_TIMEOUT_COMM% '$(HASH_SIGN)'
else	# !empty(IPCEVENT_KEEP_TIMEOUT)
CONF_REPL_RULES	+= -p %IPCEVENT_KEEP_TIMEOUT% '$(IPCEVENT_KEEP_TIMEOUT)'
CONF_REPL_RULES	+= -p %IPCEVENT_KEEP_TIMEOUT_COMM% ''
endif	# empty(IPCEVENT_KEEP_TIMEOUT)

# Set ipc_event.timeout.
ifeq	($(strip $(IPCEVENT_TIMEOUT)),)
CONF_REPL_RULES	+= -p %IPCEVENT_TIMEOUT% '60000'
CONF_REPL_RULES	+= -p %IPCEVENT_TIMEOUT_COMM% '$(HASH_SIGN)'
else	# !empty(IPCEVENT_TIMEOUT)
CONF_REPL_RULES	+= -p %IPCEVENT_TIMEOUT% '$(IPCEVENT_TIMEOUT)'
CONF_REPL_RULES	+= -p %IPCEVENT_TIMEOUT_COMM% ''
endif	# empty(IPCEVENT_TIMEOUT)

# Rules to generate static attributes file.
ATTR_REPL_RULES	= -p %DAEMON_DESC% $(DAEMON_DESC)
ATTR_REPL_RULES	+= -p %DAEMON_QDESC% $(DAEMON_DESC)
ATTR_REPL_RULES	+= -a %DAEMON_QDESC% string,min=1
ATTR_REPL_RULES	+= -p %LOG_IDENT% $(LOG_IDENT)
ATTR_REPL_RULES	+= -a %LOG_IDENT% string,min=1,max=63
ATTR_REPL_RULES	+= -p %YEAR_RANGE% '$(YEAR_RANGE)'

ifdef	DMCONF_IN
# Rules to generate daemon configuration file.
DMCONF_RULES	+= -p %DAEMON_DESC% $(DAEMON_DESC)
DMCONF_RULES	+= -p %DAEMON_QDESC% $(DAEMON_DESC)
DMCONF_RULES	+= -a %DAEMON_QDESC% string,min=1
DMCONF_RULES	+= -p %DAEMON_NAME% '$(DAEMON_NAME)'
DMCONF_RULES	+= -p %DAEMON_QNAME% '$(DAEMON_NAME)'
DMCONF_RULES	+= -a %DAEMON_QNAME% string,min=1
DMCONF_RULES	+= -p %DAEMON_CONTROL% '$(DAEMON_CONTROL)'
DMCONF_RULES	+= -p %PROCESS_TYPE% '$(PROCESS_TYPE)'
DMCONF_RULES	+= -p %START_ORDER% '$(START_ORDER)'
DMCONF_RULES	+= -p %STOP_ORDER% '$(STOP_ORDER)'
DMCONF_RULES	+= -p %CLEV_ORDER_ACT% '$(CLEV_ORDER_ACT)'
DMCONF_RULES	+= -p %YEAR_RANGE% '$(YEAR_RANGE)'
endif	# !DMCONF_IN

# Create a hard link file.
# arg1 is source file path, and arg2 is destination file path.
CMD_LN_H	=						\
	dir=$(dir $(2));					\
	if [ ! -d $$dir ]; then					\
	    echo "=== Installing $$dir";			\
	    $(INSTALL_DIRS) $$dir || exit 1;			\
	fi;							\
	echo "=== Installing $(2)";				\
	$(LN_H) $(1) $(2)

all:		$(ALL_TARGET)

install:	$(INSTALL_TARGET)

include $(BLDDIR)/rules.mk
include $(CORE_BLDDIR)/repl-rules.mk

ifdef	DMCONF_IN

ifdef	COMMON_DMCONF

$(OBJ_DMCONF):	$(DMCONF_IN) FRC
	@$(call CMD_EXECUTE,REPLACE,$(OBJ_CMDDIR)/$*.cmd,$?)

endif	# COMMON_DMCONF

include $(BLDDIR)/dmconf-rules.mk

endif	# !DMCONF_IN

$(OBJ_DAEMON_CONF):	$(DAEMON_CONF_IN) FRC
	@$(call CMD_EXECUTE,REPLACE,$(OBJ_CMDDIR)/$(DAEMON_CONF).cmd,$?)

$(OBJ_DAEMON_CONF):	REPL_RULES = $(CONF_REPL_RULES)

$(OBJ_DAEMON_ATTR):	$(DAEMON_ATTR_IN) FRC
	@$(call CMD_EXECUTE,REPLACE,$(OBJ_CMDDIR)/$(DAEMON_ATTR).cmd,$?)

$(OBJ_DAEMON_ATTR):	REPL_RULES = $(ATTR_REPL_RULES)

$(DEST_SYSCONFDIR)/%:	$(OBJDIR)/% FRC
	@$(call CMD_INSTALL_FILE,$<,$@)

$(DEST_DAEMON_ATTR_DIR)/%:	$(OBJDIR)/% FRC
	@$(call CMD_INSTALL_FILE,$<,$@)

$(DEST_DAEMON):	$(DEST_UNCD) FRC
	@$(call CMD_LN_H,$<,$@)

$(DEST_DAEMON_CONTROL):	$(DEST_UNC_CONTROL) FRC
	@$(call CMD_LN_H,$<,$@)

$(DEST_DAEMON_MODCACHE):	$(DEST_UNC_MODCACHE) FRC
	@$(call CMD_LN_H,$<,$@)
