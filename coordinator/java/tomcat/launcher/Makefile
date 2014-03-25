#
# Copyright (c) 2014 NEC Corporation
# All rights reserved.
# 
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

##
## Makefile that installs Apache Tomcat launcher.
##

NEED_OBJDIR	:= 1

include ../../../build/config.mk

ifdef	JAVA_EXT_MK

include $(BLDDIR)/tomcat-defs.mk
include $(BLDDIR)/dmconf-defs.mk
include $(JAVA_EXT_MK)

# Define files to be installed to  TOMCAT_BINDIR.
SCRIPTS_IN	= tomcat.in
SCRIPTS		= $(SCRIPTS_IN:%.in=%)
OBJ_SCRIPTS	= $(SCRIPTS:%=$(OBJDIR)/%)
DEST_BINDIR	= $(DESTDIR)$(TOMCAT_BINDIR)
DEST_BINFILES	= $(SCRIPTS:%=$(DEST_BINDIR)/%)
CLEANFILES	+= $(OBJ_SCRIPTS)

# Rules to replace keywords in shell scripts.
REPL_RULES	= -p %INST_DATADIR% '$(INST_DATADIR)'
REPL_RULES	+= -p %INST_JAVADIR% '$(INST_JAVADIR)'
REPL_RULES	+= -p %TOMCAT_ROOT% '$(TOMCAT_ROOT)'
REPL_RULES	+= -p %TOMCAT_VARDIR% '$(INST_LOCALSTATEDIR)/tomcat'
REPL_RULES	+= -p %LOG4J% '$(LOG4J)'
REPL_RULES	+= -p %SHELL_PATH% '$(SHELL_PATH)'

# Launcher configuration file.
DMCONF_IN	= tomcat.daemon.in

# Rules to replace keywords in launcher configuration file.
DMCONF_RULES	+= -p %TOMCAT_BINDIR% '$(TOMCAT_BINDIR)'

all:	scripts dmconf

scripts:	$(OBJ_SCRIPTS)

dmconf:		$(SYS_EXTRA_TARGET)

install:	$(DEST_BINFILES) $(SYS_EXTRA_INSTALL)

$(DEST_BINDIR)/%:	$(OBJDIR)/% $(DEST_BINDIR) FRC
	@echo "=== Installing $@";					\
	$(INSTALL_FILES) $< $@

$(DEST_BINDIR):
	@echo "=== Installing $@";					\
	$(INSTALL_DIRS) $@

$(DEST_BINFILES):	PERM_FILES = $(PERM_PROGS)

include $(BLDDIR)/rules.mk
include $(BLDDIR)/dmconf-rules.mk

else	# !JAVA_EXT_MK

all install clean clobber:

.PHONY:	all install clean clobber

endif	# JAVA_EXT_MK
