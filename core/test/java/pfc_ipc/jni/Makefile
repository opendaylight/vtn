#
# Copyright (c) 2012-2013 NEC Corporation
# All rights reserved.
# 
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

##
## Makefile that drives the production of JNI library for pfc_ipc.jar test.
##

include ../../jni-defs.mk

SHLIB_NAME	= libpfc_ipc_test_jni

C_SOURCES	=	\
	misc.c		\
	ut_struct.c

# Generate JNI header file.
JAVAH_CLASSNAMES	= IpcStructTest
JAVAH_CLASSNAMES	+= TestStruct
JAVAH_CLASSNAMES	+= TestStruct1
JAVAH_CLASSNAMES	+= TestStruct2
JAVAH_CLASSNAMES	+= TestStruct3
JAVAH_CLASSNAMES	+= TestStruct4
JAVAH_CLASSNAMES	+= TestBase
JAVAH_CLASSNAMES	+= ClientLibraryTest
JNI_JAVAH_CLASSES	= $(JAVAH_CLASSNAMES:%=org.opendaylight.vtn.core.ipc.%)

PFC_LIBS	= libpfc_util libpfc_jni libpfc_ipc libpfc_ipcclnt

# Add header search path to include libpfc_jni internal header.
PFCLIB_INCDIRS	=libpfc_ipc libpfc_ipcsrv libpfc_ipcclnt
EXTRA_INCDIRS	+= $(PFCLIB_INCDIRS:%=$(SRCROOT)/libs/%)

UT_STRUCT_IPCT	:= ut_struct.ipct
UT_STRUCT_H	:= $(UT_STRUCT_IPCT:%.ipct=$(OBJDIR)/include/%.h)
UT_STRUCT_BIN	:= $(UT_STRUCT_IPCT:%.ipct=$(OBJDIR)/%.bin)

IPC_STRUCT_BIN	:= $(OBJROOT)/ipc/ipc_struct.bin

EXTRA_CPPFLAGS	+= -DIPC_STRUCT_BIN='"$(IPC_STRUCT_BIN)"'
EXTRA_CPPFLAGS	+= -DUT_STRUCT_BIN='"$(UT_STRUCT_BIN)"'

CLEANFILES	+= $(UT_STRUCT_BIN) $(UT_STRUCT_H)

include ../../jni-rules.mk

$(UT_STRUCT_H):	$(UT_STRUCT_BIN)

$(UT_STRUCT_BIN):	$(UT_STRUCT_IPCT)
	@$(IPCTC) -h $(UT_STRUCT_H) -i $@ $(UT_STRUCT_IPCT)

$(OBJ_OBJECTS):	$(UT_STRUCT_H)
