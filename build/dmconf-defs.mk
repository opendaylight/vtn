#
# Copyright (c) 2012-2013 NEC Corporation
# All rights reserved.
# 
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

##
## Common configurations to build daemon configuration file used by
## UNC daemon launcher.
##
## This makefile is designed to be included after config.mk is included.
##
## The following macros need to be defined in Makefile:
##
## DMCONF_IN:		The name of template file for daemon configuration
##			file. It must end with ".daemon.in".
##

# Name of daemon configuration file to be built.
DMCONF			= $(DMCONF_IN:%.in=%)
OBJ_DMCONF		= $(DMCONF:%=$(OBJDIR)/%)
CLEANFILES		+= $(OBJ_DMCONF)

SYS_EXTRA_TARGET	+= $(OBJ_DMCONF)
SYS_EXTRA_INSTALL	+= install-dmconf

# Default keywords to be replaced are listed below:
#
# %INST_SBINDIR%
#	Replaced with the installation directory for system admin commands.
#	(PREFIX/sbin)
#
# %INST_BINDIR%
#	Replaced with the installation directory for commands.
#	(PREFIX/bin)
DMCONF_RULES	= -p %INST_SBINDIR% '$(INST_SBINDIR)'
DMCONF_RULES	+= -p %INST_BINDIR% '$(INST_BINDIR)'
