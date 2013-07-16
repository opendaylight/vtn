#
# Copyright (c) 2012-2013 NEC Corporation
# All rights reserved.
# 
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

##
## Common rules to build JNI library.
## This makefile is designed to be included after jni-defs.mk is included.
##

ifndef	BUILD_JNI_RULES_MK_INCLUDED

BUILD_JNI_RULES_MK_INCLUDED	:= 1

ifdef	JAVA_CONFIG_MK

# Disallow unresolved symbol.
LD_LDFLAGS	+= $(LD_ZDEFS)

# Add JNI header search path.
EXTRA_INCDIRS	+= $(JNI_INCDIR)

# Allow to include pfc/jni.h.
EXTRA_CPPFLAGS	+= -DPFC_JNI_BUILD

# Create symbolic links to library file under LINK_JNILIBDIR.
EXPORT_LIBS	:= 1
EXPORT_LIBDIR	:= $(LINK_JNILIBDIR)

ifneq	($(strip $(JNI_JAVAH_CLASSES)),)
# Generate JNI header files.
JAVAH_INCDIR	= $(OBJDIR)/include
EXTRA_INCDIRS	+= $(JAVAH_INCDIR)
CLEANFILES	+= $(JAVAH_INCDIR)
endif	# !empty(JNI_JAVAH_CLASSES)

include $(BLDDIR)/shared-rules.mk

install:	all $(EXTRA_INSTALL)
	@$(call CMD_INSTALL,$(JNI_TARGETDIR))

ifdef	JAVAH_INCDIR

MKJAVAH		:= $(TOOLBIN)/mkjavah

# Create JNI header file names.
JNI_JAVAH_NAMES		:= $(subst $$,_,$(subst .,_,$(JNI_JAVAH_CLASSES)))
JNI_JAVAH_HEADERS	:= $(JNI_JAVAH_NAMES:%=$(JAVAH_INCDIR)/%.h)

# JAR file path to be passed to javah.
ifeq	($(strip $(JNI_JAVAH_JARFILE)),)
JNI_JAVA_LIBNAME	:= $(notdir $(realpath $(CURDIR)/..))
ifdef	JUNIT_TEST
JAVAH_JARFILE		:= $(JNI_JAVA_LIBNAME)_test.jar
else	# !JUNIT_TEST
JAVAH_JARFILE		:= $(JNI_JAVA_LIBNAME).jar
endif	# JUNIT_TEST
JNI_JAVAH_JARFILE	:= $(JAVA_OBJDIR)/$(JAVAH_JARFILE)
endif	# empty(JNI_JAVAH_JARFILE)

MKJAVAH_LOCK	= $(OBJDIR)/.mkjavah.lock
MKJAVAH_FLAGS	= -c $(JNI_JAVAH_JARFILE) -o $(JAVAH_INCDIR) -l $(MKJAVAH_LOCK)

# Create JNI header files.
$(JNI_JAVAH_HEADERS):	$(JNI_JAVAH_JARFILE)
	@$(MKJAVAH) $(MKJAVAH_FLAGS) $(JNI_JAVAH_CLASSES)

jni-headers:	$(JNI_JAVAH_HEADERS)

$(OBJ_OBJECTS):	$(JNI_JAVAH_HEADERS)

endif	# JAVAH_INCDIR

else	# !JAVA_CONFIG_MK

all install clean clobber:

.PHONY:	all install clean clobber

endif	# JAVA_CONFIG_MK

endif	# !BUILD_JNI_RULES_MK_INCLUDED
