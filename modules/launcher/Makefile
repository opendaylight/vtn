#
# Copyright (c) 2012-2013 NEC Corporation
# All rights reserved.
# 
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

##
## Makefile that drives the production of launcher module.
##

include ../defs.mk

C_SOURCES	=	\
	cluster.c	\
	daemon.c	\
	event.c		\
	ipc.c		\
	launcher.c

UNC_LIBS	= libunc_launcher
EXTRA_LDLIBS	= -ldl

# Use clstat internal header files.
EXTRA_INCDIRS	= ../clstat

include ../rules.mk

##
## Targets:
##
## make all (default target)
##	Build all binary distributions.
##
## make install
##	Execute "make all" and install binary distributions into the
##	installation directory.
##
## make clean
##	Remove binary distributions built by "make all".
##
## make clobber
##	Execute "make clean" and remove header dependency files.
##
