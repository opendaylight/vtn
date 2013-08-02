#
# Copyright (c) 2012-2013 NEC Corporation
# All rights reserved.
# 
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

##
## Common rules to build RPM package.
## This makefile is designed to be included after rpm-defs.mk is included.
##
## If RPM_NAME is not defined, this makefile provides stub targets that
## ignores RPM package build request.
##

ifndef	BUILD_RPM_RULES_MK_INCLUDED

BUILD_RPM_RULES_MK_INCLUDED	:= 1

ifeq	($(strip $(RPM_NAME)),)

# Define stub targets.
rpm setup-rpm install-rpm clean-rpm::

else	# RPM_NAME is not empty

# List of RPM package file path.
RPM_FILE	:= $(RPM_FILENAME:%=$(OBJDIR)/%)

# Directories required by rpmbuild.
RPM_ROOT	:= $(OBJDIR)/rpmbuild
RPM_BUILD	:= BUILD
RPM_BUILDROOT	:= BUILDROOT
RPM_RPMS	:= RPMS
RPM_SOURCES	:= SOURCES
RPM_SPECS	:= SPECS
RPM_SRPMS	:= SRPMS

RPM_DIRS	:= $(RPM_BUILD) $(RPM_BUILDROOT) $(RPM_RPMS) $(RPM_SOURCES)
RPM_DIRS	+= $(RPM_SPECS) $(RPM_SRPMS)

RPM_SOURCES_DIR	:= $(RPM_ROOT)/$(RPM_SOURCES)
RPM_SPECS_DIR	:= $(RPM_ROOT)/$(RPM_SPECS)
RPM_RPMS_DIR	:= $(RPM_ROOT)/$(RPM_RPMS)
RPM_SRPMS_DIR	:= $(RPM_ROOT)/$(RPM_SRPMS)

# Convert RPM_PROVIDES into "Provides:" lines in spec file.
DEFINE_PROVIDES	= \nProvides:\t$(1)
RPM_PROVIDES	= $(foreach p,$(RPM_PROVIDES_LIST),$(call DEFINE_PROVIDES,$(p)))

# Define suffix to be added RPM package name.
ifdef	RPM_NAME_SUFFIX
RPM_SUFFIX_DEFS	:= %define\trpm_name_suffix\t\t$(RPM_NAME_SUFFIX)
endif	# RPM_NAME_SUFFIX

# sed rules to generate spec file.
RPM_SPEC_RULES	+= -e 's,@RPM_NAME@,$(RPM_NAME),g'
RPM_SPEC_RULES	+= -e 's,@RPM_VERSION@,$(RPM_VERSION),g'
RPM_SPEC_RULES	+= -e 's,@RPM_RELEASE@,$(RPM_RELEASE),g'
RPM_SPEC_RULES	+= -e 's,@RPM_PREFIX@,$(RPM_PREFIX),g'
RPM_SPEC_RULES	+= -e 's,@RPM_ARCH@,$(RPM_ARCH),g'
RPM_SPEC_RULES	+= -e 's,@CONFIG_OS@,$(CONFIG_OS),g'
RPM_SPEC_RULES	+= -e 's,@CONFIG_ARCH@,$(CONFIG_ARCH),g'
RPM_SPEC_RULES	+= -e 's,@PFC_PRODUCT_NAME@,$(PFC_PRODUCT_NAME),g'
RPM_SPEC_RULES	+= -e 's,@RPM_NAME_SUFFIX@,$(RPM_SUFFIX_DEFS),g'
RPM_SPEC_RULES	+= -e 's,@RPM_PROVIDES@,$(RPM_PROVIDES),g'

RPM_CLEANFILES	= $(RPM_ROOT) $(RPM_FILE) $(RPM_SPEC_FILE)
CLEANFILES	+= $(RPM_CLEANFILES)

# Command to generate RPM package.
RPMBUILD_ARGS	= --define='_topdir $(RPM_ROOT)'
BUILD_RPM	=							\
	echo "=== Creating $@.";					\
	set -e;								\
	$(RM) $@;							\
	$(RPMBUILD) $(RPMBUILD_ARGS) $(1) $(RPM_SPEC_FILE);		\
	$(LN) $(2)/$(notdir $@) $@

ifdef	RPMBUILD

rpm::	$(RPM_FILE)

# Create binary package.
$(RPM_FILE):	$(RPM_FILE_DEPS) $(RPM_SPEC_FILE)
	@$(call BUILD_RPM,-bb,$(RPM_RPMS_DIR)/$(RPM_ARCH))

# Prepare rpmbuild environment.
setup-rpm::	$(RPM_SPEC_FILE)

# Prepare rpmbuild environment.
$(RPM_ROOT):
	@set -e;							\
	echo "=== Preparing rpmbuild environment.";			\
	$(RM) -rf $(RPM_ROOT);						\
	$(MKDIR) -p $(RPM_DIRS:%=$(RPM_ROOT)/%)

# Install RPM package to the RPM package directory.
RPM_INSTALL	=							\
	set -e;								\
	if [ ! -d $(RPM_PKGDIR) ]; then					\
	    echo "=== Installing $(RPM_PKGDIR)";			\
	    $(INSTALL_DIRS) $(RPM_PKGDIR);				\
	fi;								\
	echo "=== Installing $(RPM_PKGDIR)/$(1)";			\
	$(INSTALL_FILES) $(OBJDIR)/$(1) $(RPM_PKGDIR)

install-rpm::	$(RPM_FILE)
	@for f in $(RPM_FILENAME); do					\
	    $(call RPM_INSTALL,$$f) || exit 1;				\
	done

clean-rpm::
	$(RM) -rf $(RPM_CLEANFILES)

include $(BLDDIR)/sed-rules.mk

$(RPM_SPEC_FILE):	SED_RULES = $(RPM_SPEC_RULES)

else	# !RPMBUILD

rpm setup-rpm install-rpm clean-rpm::	FRC
	@echo "*** SKIP: rpmbuild is not installed."

endif	# RPMBUILD

endif	# RPM_NAME is empty

ifneq	($(strip $(RPM_SUBDIRS)),)

rpm setup-rpm install-rpm clean-rpm::	FRC
	@for d in $(RPM_SUBDIRS); do				\
	    $(MAKE) -C $$d $@ || exit 1;			\
	done

endif	# RPM_SUBDIRS is not empty

endif	# !BUILD_RPM_RULES_MK_INCLUDED
