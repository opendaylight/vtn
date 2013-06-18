#
# Copyright (c) 2010-2013 NEC Corporation
# All rights reserved.
# 
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

##
## Makefile that drives the production of perl scripts used by
## PFC build process.
##

include ../config.mk

PLFILES		=	\
	depfix.PL	\
	dirpath.PL	\
	gencopy.PL	\
	headexport.PL	\
	listfile.PL	\
	modtool.PL	\
	replace.PL

ifdef	JAVA_CONFIG_MK
include $(JAVA_CONFIG_MK)
PLFILES		+= ant.PL mkjproject.PL mkjavah.PL
ARGV_ant	= $(ANT_PATH) $(ANT_HOME) $(JAVA_HOME) $(JAVA_MODE)
ARGV_mkjavah	= $(JAVA_HOME)
endif	# JAVA_CONFIG_MK

SCRIPTS		= $(PLFILES:%.PL=%)

CLEANFILES	= $(SCRIPTS)
CLOBBERFILES	= not_exist
CLOBBER_DEPS	= clean

all:	$(SCRIPTS)

install:	all
	$(INSTALL_DIRS) $(TOOLBIN)
	$(INSTALL_PROGS) $(SCRIPTS) $(TOOLBIN)

distclean:	clobber

%:	%.PL
	$(PERL) $< $(ARGV_$*)

include ../../../build/rules.mk
