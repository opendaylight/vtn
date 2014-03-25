#
# Copyright (c) 2013-2014 NEC Corporation
# All rights reserved.
# 
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

##
## Makefile that drives the production of VTN Coordinator binary distribution.
##

# Path to source tree root directory.
SRCDIR		= ../..

# Build target directory.
TARGETDIR	:= $(realpath ../target)

# Import build configuration.
CONFIG_MK	= $(TARGETDIR)/config.mk
-include $(CONFIG_MK)

# Directory to store objects.
VTN_DESTDIR	?= root
OBJDIR		= $(TARGETDIR)/objs
DESTROOT	= $(TARGETDIR)/$(VTN_DESTDIR)

JAVA_LIBDIR	= $(TARGETDIR)/java

VERSION_FILE	= $(TARGETDIR)/VERSION
CONFIGURE	= $(SRCDIR)/configure
CONFIG_STATUS	= $(OBJDIR)/config.status

# Options for configure script.
CONFIGURE_ARGS	= $(VTN_PREFIX:%=--prefix=%)
CONFIGURE_ARGS	+= --with-version-file=$(VERSION_FILE)
CONFIGURE_ARGS	+= --with-objdir=$(OBJDIR)
CONFIGURE_ARGS	+= --includedir=none
CONFIGURE_ARGS	+= --with-java-libdir=$(JAVA_LIBDIR)
CONFIGURE_ARGS	+= --with-tomcat-libdir=$(JAVA_LIBDIR)

# Determine JAVA_HOME.
STRIP_VTN_JAVA_HOME	:= $(strip $(VTN_JAVA_HOME))
ifeq	($(STRIP_VTN_JAVA_HOME),)
STRIP_VTN_JAVA_HOME	:= $(strip $(JAVA_HOME))
endif	# empty(VTN_JAVA_HOME)
ifneq	($(STRIP_VTN_JAVA_HOME),)
CONFIGURE_ARGS	+= --with-java-home="$(STRIP_VTN_JAVA_HOME)"
endif	# !empty(VTN_JAVA_HOME)

ifneq	($(strip $(VTN_DEBUG)),true)
CONFIGURE_ARGS	+= --disable-debug
endif	# VTN_DEBUG != true

STRIP_VTN_BUILD_ID	:= $(strip $(VTN_BUILD_ID))
ifneq	($(STRIP_VTN_BUILD_ID),)
CONFIGURE_ARGS	+= --with-build-id="$(STRIP_VTN_BUILD_ID)"
endif	# !empty(VTN_BUILD_ID)

STRIP_VTN_SCM_REVISION	:= $(strip $(VTN_SCM_REVISION))
ifneq	($(STRIP_VTN_SCM_REVISION),)
CONFIGURE_ARGS	+= --with-scm-revision="$(STRIP_VTN_SCM_REVISION)"
endif	# !empty(VTN_SCM_REVISION)

all clean clobber:	$(CONFIG_STATUS) FRC
	$(MAKE) -C $(SRCDIR) $@

configure:	$(CONFIG_STATUS) FRC

install:	$(CONFIG_STATUS) FRC
	$(RM) -rf $(DESTROOT)
	$(MAKE) -C $(SRCDIR) $@ DESTDIR=$(DESTROOT)

ifeq	($(strip $(VTN_SKIP_TEST_BUILD)),true)

test-all test-test:	FRC
	@echo "=== Skip compiling tests."

.PHONY:

else	# VTN_SKIP_TEST_BUILD != true

test-all:	$(CONFIG_STATUS) FRC
	$(MAKE) -C $(SRCDIR) $@

ifeq	($(strip $(VTN_SKIP_TEST)),true)

test-test:	FRC
	@echo "=== Skip tests."

else	# VTN_SKIP_TEST != true

test-test:	$(CONFIG_STATUS) FRC
	$(MAKE) -C $(SRCDIR) $@

endif	# VTN_SKIP_TEST == true

endif	# VTN_SKIP_TEST_BUILD == true

distclean:	FRC
	@-if [ -f $(CONFIG_STATUS) ]; then				\
	    $(MAKE) -k -C $(SRCDIR) distclean;				\
	fi

# Configure build environment.
$(CONFIG_STATUS):	$(CONFIGURE) $(VERSION_FILE)
	$(CONFIGURE) $(CONFIGURE_ARGS)

$(VERSION_FILE):	$(CONFIG_MK)
	@echo "--- Install new version: $(VTN_VERSION)";		\
	echo $(VTN_VERSION) > $(VERSION_FILE) || exit 1;		\
	$(RM) -rf $(DESTROOT)

FRC:

.PHONY:	FRC
