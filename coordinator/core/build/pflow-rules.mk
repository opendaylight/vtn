#
# Copyright (c) 2012-2013 NEC Corporation
# All rights reserved.
# 
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

##
## Rules to build sources which uses PFLOW vendor specific message.
##
## Remarks:
##	Rules in this file affects only sources that include pfc/pflow.h.
##

ifndef	BUILD_PFLOW_RULES_MK_INCLUDED

# Include PFS-1.0 header file to build PFS1_0_C_SOURCES and
# PFS1_0_CXX_SOURCES.
PFS1_0_OBJECTS	= $(PFS1_0_C_SOURCES:.c=.o) $(PFS1_0_CXX_SOURCES:.cc=.o)

ifneq	($(strip $(PFS1_0_OBJECTS)),)

ifdef	BUILD_RULES_MK_INCLUDED
PFS1_0_OBJS	= $(PFS1_0_OBJECTS:%=$(OBJDIR)/%)
else	# !BUILD_RULES_MK_INCLUDED
PFS1_0_OBJS	= $(PFS1_0_OBJECTS)
endif	# BUILD_RULES_MK_INCLUDED

$(PFS1_0_OBJS):	CPPFLAGS += -D__PFC_PFS_1_0_SOURCE

endif	# !empty(PFS1_0_OBJECTS)

# Include PFS-2.0 header file to build PFS2_0_C_SOURCES and
# PFS2_0_CXX_SOURCES.
PFS2_0_OBJECTS	= $(PFS2_0_C_SOURCES:.c=.o) $(PFS2_0_CXX_SOURCES:.cc=.o)

ifneq	($(strip $(PFS2_0_OBJECTS)),)

ifdef	BUILD_RULES_MK_INCLUDED
PFS2_0_OBJS	= $(PFS2_0_OBJECTS:%=$(OBJDIR)/%)
else	# !BUILD_RULES_MK_INCLUDED
PFS2_0_OBJS	= $(PFS2_0_OBJECTS)
endif	# BUILD_RULES_MK_INCLUDED

$(PFS2_0_OBJS):	CPPFLAGS += -D__PFC_PFS_2_0_SOURCE

endif	# !empty(PFS2_0_OBJECTS)

# Include PFS-3.0 header file to build PFS3_0_C_SOURCES and
# PFS3_0_CXX_SOURCES.
PFS3_0_OBJECTS	= $(PFS3_0_C_SOURCES:.c=.o) $(PFS3_0_CXX_SOURCES:.cc=.o)

ifneq	($(strip $(PFS3_0_OBJECTS)),)

ifdef	BUILD_RULES_MK_INCLUDED
PFS3_0_OBJS	= $(PFS3_0_OBJECTS:%=$(OBJDIR)/%)
else	# !BUILD_RULES_MK_INCLUDED
PFS3_0_OBJS	= $(PFS3_0_OBJECTS)
endif	# BUILD_RULES_MK_INCLUDED

$(PFS3_0_OBJS):	CPPFLAGS += -D__PFC_PFS_3_0_SOURCE

endif	# !empty(PFS3_0_OBJECTS)

endif	# !BUILD_PFLOW_RULES_MK_INCLUDED
