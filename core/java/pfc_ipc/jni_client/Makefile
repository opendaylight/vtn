#
# Copyright (c) 2012-2013 NEC Corporation
# All rights reserved.
# 
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

##
## Makefile that drives the production of JNI library for IPC client.
##

include ../../jni-defs.mk

SHLIB_NAME	= libpfc_ipcclnt_jni

C_SOURCES	=		\
	client.c		\
	connection.c		\
	event.c			\
	misc.c			\
	pdu.c			\
	session.c		\
	struct.c

PFC_LIBS	= libpfc_util libpfc_jni libpfc_ipc libpfc_ipcclnt

# Add header search path to include libpfc_jni internal header.
PFCLIB_INCDIRS	= libpfc_jni libpfc_ipc libpfc_ipcclnt
EXTRA_INCDIRS	+= $(PFCLIB_INCDIRS:%=../../../libs/%)

# Define log identifier.
EXTRA_CPPFLAGS	+= -DPFC_LOG_IDENT='"$(SHLIB_NAME)"'

# Generate JNI header file.
JAVAH_CLASSNAMES	= IpcConnection
JAVAH_CLASSNAMES	+= DefaultConnection
JAVAH_CLASSNAMES	+= AltConnection
JAVAH_CLASSNAMES	+= ConnectionPool
JAVAH_CLASSNAMES	+= ClientLibrary
JAVAH_CLASSNAMES	+= ClientSession
JAVAH_CLASSNAMES	+= IpcDataUnit
JAVAH_CLASSNAMES	+= IpcInt8
JAVAH_CLASSNAMES	+= IpcInt16
JAVAH_CLASSNAMES	+= IpcInt32
JAVAH_CLASSNAMES	+= IpcInt64
JAVAH_CLASSNAMES	+= IpcUint8
JAVAH_CLASSNAMES	+= IpcUint16
JAVAH_CLASSNAMES	+= IpcUint32
JAVAH_CLASSNAMES	+= IpcUint64
JAVAH_CLASSNAMES	+= IpcFloat
JAVAH_CLASSNAMES	+= IpcDouble
JAVAH_CLASSNAMES	+= IpcInet4Address
JAVAH_CLASSNAMES	+= IpcInet6Address
JAVAH_CLASSNAMES	+= IpcString
JAVAH_CLASSNAMES	+= IpcBinary
JAVAH_CLASSNAMES	+= IpcNull
JAVAH_CLASSNAMES	+= IpcStruct
JAVAH_CLASSNAMES	+= IpcHostSet
JAVAH_CLASSNAMES	+= IpcEvent
JAVAH_CLASSNAMES	+= IpcEventSystem
JAVAH_CLASSNAMES	+= IpcEventAttribute

JNI_JAVAH_CLASSES	= $(JAVAH_CLASSNAMES:%=org.opendaylight.vtn.core.ipc.%)

include ../../jni-rules.mk
