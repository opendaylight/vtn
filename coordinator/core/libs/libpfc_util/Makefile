#
# Copyright (c) 2010-2013 NEC Corporation
# All rights reserved.
# 
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

##
## Makefile that drives the production of PFC core utility.
##

# Use PFC daemon maintenance API.
USE_PFCD_MAINT	= 1

include ../defs.mk

SHLIB_NAME	= libpfc_util

C_SOURCES	=		\
	cloexec.c		\
	conf.c			\
	conf_lexer.c		\
	conf_map.c		\
	conf_parser.c		\
	cred.c			\
	ctype.c			\
	debug.c			\
	extcmd.c		\
	frotate.c		\
	flock.c			\
	hash.c			\
	hashlist.c		\
	hostaddr.c		\
	iostream.c		\
	libpfc_util.c		\
	linkedlist.c		\
	listmodel.c		\
	log_conf.c		\
	mkdir.c			\
	nonblock.c		\
	plaintext.c		\
	pwd.c			\
	rbtree.c		\
	refptr.c		\
	rmpath.c		\
	safepath.c		\
	socket.c		\
	string.c		\
	strtoint.c		\
	synch.c			\
	sysconf.c		\
	time.c			\
	vector.c		\
	clock_$(OSTYPE).c	\
	epoll_$(OSTYPE).c	\
	proc_$(OSTYPE).c	\
	synch_$(OSTYPE).c	\
	tid_$(OSTYPE).c		\
	log_$(SYSLOG_TYPE).c

ifeq	($(OSTYPE),linux)
C_SOURCES	+= closefrom_$(OSTYPE).c
endif	# OSTYPE == linux

ifeq	($(SUBARCH),i386)
AS_SOURCES	+= atomic_i386.S
endif	# SUBARCH == i386

LDLIBS		+= -lrt
EXTRA_CPPFLAGS	= -D_PFC_LIBPFC_UTIL_BUILD

# Define PFC system configuration file format.
CFDEF_FILES	= pfcd.cfdef

include ../rules.mk
