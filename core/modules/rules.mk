#
# Copyright (c) 2010-2013 NEC Corporation
# All rights reserved.
# 
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

##
## Common rules to build PFC modules.
##

ifeq	($(strip $(MODULE_NAME)),)
$(error MODULE_NAME must be defined.)
endif	# empty(MODULE_NAME)

# SONAME is identical to module name.
SHLIB_NAME	= $(MODULE_NAME)

EXTRA_INCDIRS	+= include
CPPFLAGS	+= -DPFC_MODULE_BUILD
CPPFLAGS	+= -DMODULE_NAME='"$(MODULE_NAME)"'
CPPFLAGS	+= $(MODULE_VERSION:%=-DMODULE_VERSION=%)

# Import header files of the module which depends on.
CPPFLAGS	+= $(MODULE_DEPENDS:%=-I%/include)

# Link PFC core libraries.
PFC_CORE_LIBS	:= libpfc_util libpfc libpfc_ipcsrv libpfc_ipcclnt
ifneq	($(strip $(CXX_SOURCES)),)
PFC_CORE_LIBS	+= libpfcxx libpfcxx_ipcsrv libpfcxx_ipcclnt
endif	# !empty(CXX_SOURCES)

PFC_LIBS	:= $(PFC_CORE_LIBS) $(filter-out $(PFC_CORE_LIBS),$(PFC_LIBS))

ifneq	($(strip $(MODULE_CFDEF)),)

# Build configuration definition file.
CFDEF_FILES	+= $(MODULE_CFDEF)

# Symbol name and visibility of module configuration file definition is
# controlled by the system. Ignore cf_name and cf_visibility directive
# in MODULE_CFDEF.
MODULE_CFDEF_NAME	= "__pfc_$(MODULE_NAME)_cfdef"
MODULE_CFDEFC_FLAGS	= -n $(MODULE_CFDEF_NAME) -v hidden
MODULE_CFDEF_SOURCE	= $(OBJDIR)/$(CFDEF_PREFIX)$(MODULE_NAME).c
CPPFLAGS		+= -DMODULE_CFDEF_NAME=$(MODULE_CFDEF_NAME)

endif	# !empty(MODULE_CFDEF)

include $(BLDDIR)/shared-rules.mk

$(MODULE_CFDEF_SOURCE):	CFDEFC_FLAGS += $(MODULE_CFDEFC_FLAGS)

install:	all $(EXTRA_INSTALL)
	@$(call CMD_INSTALL,$(INST_MODULEDIR)) || exit 1;		\
	install_modconf() {						\
	    name=$$1;							\
	    dir=$$2;							\
	    cffile=;							\
	    if [ -f $(OBJDIR)/$$name ]; then				\
		cffile=$(OBJDIR)/$$name;				\
	    elif [ -f $$name ]; then					\
		cffile=$$name;						\
	    fi;								\
	    if [ -n "$$cffile" ]; then					\
		if [ ! -d $$dir ]; then					\
		    echo "==== Installing $$dir";			\
		    $(INSTALL_DIRS) $$dir || exit 1;			\
		fi;							\
		echo "=== Installing $$dir/$(MODULE_CONF)";		\
		$(INSTALL_FILES) $$cffile $$dir/$(MODULE_CONF) ||	\
		    exit 1;						\
	    fi;								\
	};								\
	depfile=$(MODULE_NAME).dep;					\
	moddir=$(DESTDIR)$(INST_MODULEDIR);				\
	if [ -f $$depfile ]; then					\
	    echo "=== Installing $$moddir/$$depfile";			\
	    $(INSTALL_FILES) $$depfile $$moddir || exit 1;		\
	fi;								\
	install_modconf $(MODULE_CONF) $$moddir || exit 1;		\
	install_modconf $(MODULE_PUBLIC_CONF)				\
	    $(DESTDIR)$(INST_MODCONFDIR) || exit 1;			\
	if [ -n "$(MODULE_EXTRA_FILES)" ]; then				\
	    echo "=== Installing extra module files: $(MODULE_EXTRA_FILES)"; \
	   $(INSTALL_FILES) $(MODULE_EXTRA_FILES) $$moddir || exit 1;	\
	fi
