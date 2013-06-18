#
# Copyright (c) 2010-2013 NEC Corporation
# All rights reserved.
# 
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

##
## Makefile that drives the production of cfdef package.
##

include ../config.mk

SRCDIR		= src
CFDEF_MAKEFILE	= $(SRCDIR)/Makefile

MKARGS	= BASEDIR=$(TOOLS_DIR) INC="$(CC_INCLUDES)"

all install:	$(CFDEF_MAKEFILE)
	$(MAKE) -j1 -C $(SRCDIR) $@

clean clobber distclean:
	@if [ -f $(CFDEF_MAKEFILE) ]; then				\
	    $(MAKE) -j1 -C $(SRCDIR) clean;				\
	    $(RM) $(CFDEF_MAKEFILE) $(CFDEF_MAKEFILE).old;		\
	fi

# Generate makefile using ExtUtils::MakeMaker.
$(SRCDIR)/%:	$(SRCDIR)/%.PL
	cd $(SRCDIR); $(PERL) $*.PL $(MKARGS)
