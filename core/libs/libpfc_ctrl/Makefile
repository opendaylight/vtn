#
# Copyright (c) 2010-2013 NEC Corporation
# All rights reserved.
# 
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

##
## Makefile that drives the production of PFC control library.
##
## Remarks:
##	libpfc_ctrl is built as archive.
##

include ../archive-defs.mk

LIB_NAME	= libpfc_ctrl

C_SOURCES	=		\
	client.c

include ../archive-rules.mk

# client.c requires libpfc_cmd internal header.
$(OBJDIR)/client.o:	EXTRA_INCDIRS = ../libpfc_cmd
