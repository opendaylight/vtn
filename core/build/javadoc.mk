#
# Copyright (c) 2012-2013 NEC Corporation
# All rights reserved.
# 
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

##
## Makefile that drives all-in-one Java code documentation using javadoc.
## This makefile is designed to be included after config.mk is included.
##
## This makefile uses the following macros defined in Makefile:
##
## JAVA_SUBDIRS:		Sub directories which builds Java libraries.
##
## JAVADOC_TITLE:		The title of the document.
##

ifndef	BUILD_JAVADOC_MK_INCLUDED

BUILD_JAVADOC_MK_INCLUDED	:= 1

ifdef	JAVA_CONFIG_MK

DIRPATH_ARGS	:=
JAVA_SRCROOT	?= ..

JAVA_LIBNAME	= $(JAVADOC_TITLE)

include $(BLDDIR)/java-defs.mk
include $(BLDDIR)/ant-defs.mk

# A file which contains the list of source directories.
JAVA_SRCLIST_FILE	= $(OBJDIR)/srcfiles
MKJPROJECT_ARGS		+= --srclist $(JAVA_SRCLIST_FILE) --javadoc

ifdef	UNC_BUILD

# Add PFC-Core libraries to class path.
CORE_JAVALIBS	:= $(shell $(LISTFILE) -mc '^Makefile$$' $(CORE_SRCROOT)/java)
$(eval $(call LISTFILE_ASSERT,$(CORE_JAVALIBS)))

JAVA_LIBS	+= $(CORE_JAVALIBS:%=%.jar)

# Add all third-party Java libraries to class path.
LIST_EXTLIBS	=	\
	 $(LISTFILE) -mp '\.jar$$' -e '^junit\.jar$$' $(JAVA_OBJDIR)/ext
JAVA_EXTLIBS	:= $(shell $(LIST_EXTLIBS))
$(eval $(call LISTFILE_ASSERT,$(JAVA_EXTLIBS)))

endif	# !UNC_BUILD

# Invoke javadoc using Apache Ant.
doc javadoc document:	$(BUILD_XML) FRC
	@$(ANT_BUILD) || exit 1;				\
	$(RM) $(JAVA_SRCLIST_FILE)

# Create source directory list file.
srclist:	$(JAVA_SRCLIST_FILE)

$(JAVA_SRCLIST_FILE):	$(JAVA_SRCLIST_DEPS) FRC
	@echo "=== Collecting Java source directories.";		\
	$(MKDIR) -p $(OBJDIR) || exit 1;				\
	$(RM) $@ || exit 1;						\
	for d in $(JAVA_SUBDIRS); do					\
	    $(MAKE) -s -C $$d srclist					\
		JAVA_SRCLIST_FILE=$(JAVA_SRCLIST_FILE)|| exit 1;	\
	done

# Create Ant project file.
project:	$(BUILD_XML)

$(BUILD_XML):	$(JAVA_SRCLIST_FILE)
	@$(MKJPROJECT) -o $@ $(MKJPROJECT_ARGS)

else	# !JAVA_CONFIG_MK

doc javadoc document srclist project:

.PHONY:	doc javadoc document srclist

endif	# JAVA_CONFIG_MK

FRC:

.PHONY:	FRC

endif	# !BUILD_JAVADOC_MK_INCLUDED
