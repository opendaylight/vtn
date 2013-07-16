#
# Copyright (c) 2012-2013 NEC Corporation
# All rights reserved.
# 
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

##
## Common rules to invoke Apache Ant.
## This makefile is designed to be included after java-defs.mk is included.
##

ifndef	BUILD_ANT_DEFS_MK_INCLUDED

BUILD_ANT_DEFS_MK_INCLUDED	:= 1

ANT		:= $(TOOLBIN)/ant
MKJPROJECT	:= $(TOOLBIN)/mkjproject

# Maximum heap size of Ant.
ANT_MAXHEAP	?= 256m

# Ant project file.
BUILD_XML	:= $(OBJDIR)/build.xml

# Directory to store class file.
OBJ_CLASSDIR	:= $(OBJDIR)/classdir

# Construct mkjproject arguments.
MKJPROJECT_ARGS	= $(JAVA_SRCDIR:%=-s %) -n $(JAVA_LIBNAME)
MKJPROJECT_ARGS	+= $(JAVA_MAIN_CLASS:%=-m %) -V $(JAVA_LIBVERSION)
MKJPROJECT_ARGS	+= -O $(OBJDIR) -J $(JAVA_OBJDIR) $(JAVA_ENCODING:%=-e %)
MKJPROJECT_ARGS	+= $(JAVA_LIBS:%=-c %) $(JAVA_EXTLIBS:%=-E %)
MKJPROJECT_ARGS	+= $(JAVA_BUILD_LIBS:%=-C %) $(JAVAC_FLAGS:%=-X %)
MKJPROJECT_ARGS	+= $(JAVA_TEXT_FILTERS) $(JAVA_TEXT_FILEMAT:%=-F %)

ifneq	($(strip $(JAVA_NO_CLASSPATH)),)
MKJPROJECT_ARGS	+= --no-classpath
endif	# !empty(JAVA_NO_CLASSPATH)

ifneq	($(strip $(JAVA_IMPL_TITLE)),)
MKJPROJECT_ARGS	+= -t '$(JAVA_IMPL_TITLE)'
endif	# !empty(JAVA_IMPL_TITLE)

ifneq	($(strip $(JAVA_SPEC_TITLE)),)
MKJPROJECT_ARGS	+= -T '$(JAVA_SPEC_TITLE)'
endif	# !empty(JAVA_SPEC_TITLE)

ifneq	($(strip $(JAVAC_DEBUG)),)
MKJPROJECT_ARGS	+= -g
endif	# !empty(JAVAC_DEBUG)

MKJPROJECT_ARGS	+= $(MKJPROJECT_EXTRA_ARGS)

# Macros to invoke Apache Ant.
ANT_ENV		= ANT_OPTS=$(ANT_OPTS) ANT_MAXHEAP=$(ANT_MAXHEAP)
ANT_ENV		+= $(ANT_JUNIT_ENV)
ANT_BUILD_FLAGS	= $(ANT_FLAGS) -f $(BUILD_XML) $(ANT_PROPS:%=-D%)
ANT_BUILD	= $(ANT_ENV) $(ANT) $(ANT_BUILD_FLAGS)

endif	# !BUILD_ANT_DEFS_MK_INCLUDED
