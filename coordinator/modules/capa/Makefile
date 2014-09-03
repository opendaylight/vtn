#
# Copyright (c) 2012-2014 NEC Corporation
# All rights reserved.
# 
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

##
## Makefile that drives the production of capa module.
##

include ../defs.mk

CXX_SOURCES = capa_module.cc \
              ctrlr_capability.cc \
              ctrlr_capa_defines.cc

CFDEF_FILES     += ctrlr_capa.cfdef

SYSCONF_FILES	= pfc_capa.conf vnp_capa.conf polc_capa.conf odc_capa.conf
SYSCONF_DESTDIR	= $(DESTDIR)$(INST_SYSCONFDIR)/capa
SYSCONF_TARGET	= $(SYSCONF_FILES:%=$(SYSCONF_DESTDIR)/%)
EXTRA_INSTALL	= install-sysconf


include ../rules.mk

# Install SYSCONF_FILES to INST_SYSCONFDIR/capa.
install-sysconf:	$(SYSCONF_TARGET)

$(SYSCONF_DESTDIR)/%:	% FRC
		@$(call CMD_INSTALL_FILE,$<,$@)

##
## Targets:
##
## make all (default target)
##	Build all binary distributions.
##
## make install
##	Execute "make all" and install binary distributions into the
##	installation directory.
##
## make clean
##	Remove binary distributions built by "make all".
##
## make clobber
##	Execute "make clean" and remove header dependency files.
##
