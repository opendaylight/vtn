#
# Copyright (c) 2010-2013 NEC Corporation
# All rights reserved.
# 
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

##
## Makefile that drives the production of PFC system libraries.
##

include ../build/config.mk

IPCXX_SUBDIRS	= libpfcxx_ipcsrv libpfcxx_ipcclnt
IPC_SUBDIRS	= libpfc_ipc libpfc_ipcsrv libpfc_ipcclnt $(IPCXX_SUBDIRS)
SUBDIRS		= libpfc libpfc_util libpfc_cmd libpfc_ctrl
SUBDIRS		+= libpfcxx $(IPC_SUBDIRS)

LIBPFC_DEPS	= libpfc_util libpfc_ipcsrv libpfc_ipcclnt

ifdef	JAVA_CONFIG_MK
SUBDIRS		+= libpfc_jni
endif	# JAVA_CONFIG_MK

include $(BLDDIR)/subdirs.mk

# Export header files.
export-header:	$(EXPORT_HEADER_DIR)

ifdef	EXPORT_HEADER_DIR

HEAD_CPP	= $(CXX)
HEAD_CPPFLAGS	= -x c++ -E $(CC_MODE) $(CXX_CPPFLAGS) $(CPPFLAGS)
HEAD_PREFIX	= pfc pfcxx
HEAD_SRCDIR	= $(SRCROOT)/include $(OBJS_INCLUDE)
HEAD_FLAGS	= -c $(HEAD_CPP) $(HEAD_PREFIX:%=-p%) $(HEAD_CPPFLAGS:%=-C%)
HEAD_FLAGS	+= $(HEAD_SRCDIR:%=-s %)
HEAD_HEADERS	= $(SUBDIRS:%=%/HEADERS)

$(EXPORT_HEADER_DIR):	FRC
	$(HEADEXPORT) -o $@ $(HEAD_FLAGS) $(HEAD_HEADERS)

endif	# !EXPORT_HEADER_DIR

# Directory build dependencies.
libpfc:			$(LIBPFC_DEPS)
libpfc_cmd:		libpfc_util
libpfc_ipc:		libpfc_util
libpfc_ipcsrv:		libpfc_ipc
libpfc_ipcclnt:		libpfc_ipc
libpfcxx:		libpfc $(IPCXX_SUBDIRS)
libpfcxx_ipcsrv:	libpfc_ipcsrv
libpfcxx_ipcclnt:	libpfc_ipcclnt

ifdef	JAVA_CONFIG_MK
libpfc_jni:		libpfc_util
endif	# JAVA_CONFIG_MK
