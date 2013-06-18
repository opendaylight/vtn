#
# Copyright (c) 2010-2013 NEC Corporation
# All rights reserved.
# 
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

##
## Makefile that drives the installation of PFC system headers.
##

include ../build/config.mk

# C header files to be installed.
C_HEADERS	=			\
	pfc/base.h			\
	pfc/byteorder.h			\
	pfc/clock.h			\
	pfc/compiler/gcc.h		\
	pfc/conf.h			\
	pfc/conf_parser.h		\
	pfc/config.h			\
	pfc/debug.h			\
	pfc/event.h			\
	pfc/exstatus.h			\
	pfc/hash.h			\
	pfc/iostream.h			\
	pfc/ipc.h			\
	pfc/ipc_client.h		\
	pfc/ipc_server.h		\
	pfc/license.h			\
	pfc/list.h			\
	pfc/listmodel.h			\
	pfc/log.h			\
	pfc/module.h			\
	pfc/ofi.h			\
	pfc/path.h			\
	pfc/pfc.h			\
	pfc/pfcd_conf.h			\
	pfc/poller.h			\
	pfc/rbtree.h			\
	pfc/rc.h			\
	pfc/refptr.h			\
	pfc/socket.h			\
	pfc/strtoint.h			\
	pfc/syncblock.h			\
	pfc/synch.h			\
	pfc/taskq.h			\
	pfc/tc.h			\
	pfc/thread.h			\
	pfc/timer.h			\
	pfc/tpool.h			\
	pfc/util.h			\
	pfc/wdt.h			\
					\
	$(IPCTC_C_FILES)		\
					\
	pfc_licc.h			\
					\
	os/linux/pfc/atomic.h		\
	os/linux/pfc/bitpos.h		\
	os/linux/pfc/osdep.h		\
					\
	arch/x86/pfc/asm_base.h		\
	arch/x86/pfc/atomic_gcc.h	\
	arch/x86/pfc/byteorder_arch.h	\
	arch/x86/pfc/bitpos_gcc.h	\
					\
	arch/i386/pfc/atomic_gcc_arch.h	\
					\
	arch/x86_64/pfc/atomic_gcc_arch.h

# C++ header files to be installed.
CXX_HEADERS	=			\
	pfcxx/base.hh			\
	pfcxx/conf.hh			\
	pfcxx/event.hh			\
	pfcxx/ipc_server.hh		\
	pfcxx/ipc_client.hh		\
	pfcxx/module.hh			\
	pfcxx/pfc.hh			\
	pfcxx/poller.hh			\
	pfcxx/refptr.hh			\
	pfcxx/synch.hh			\
	pfcxx/task_queue.hh		\
	pfcxx/tdb.hh			\
	pfcxx/thread.hh			\
	pfcxx/thread_pool.hh		\
	pfcxx/timer.hh			\
	$(IPCTC_CXX_FILES)

HEADERS		= $(C_HEADERS) $(CXX_HEADERS:%=cxx/%)

HEADER_DIRS	= $(sort $(dir $(HEADERS)))

INST_INCROOT	= $(DESTDIR)$(INST_INCLUDEDIR)

ifeq	($(strip $(INST_INCLUDEDIR)),none)
INST_HEADERS	:=
INST_HEADDIRS	:=
else	# INST_INCLUDEDIR != none
INST_HEADERS	= $(HEADERS:%=$(INST_INCROOT)/%)
INST_HEADDIRS	= $(HEADER_DIRS:%=$(INST_INCROOT)/%)
endif	# INST_INCLUDEDIR == none

INSTALL_HEADER	=							\
	echo "=== Installing $@";					\
	$(INSTALL_FILES) $< $@

INSTALL_HEADDIRS	=						\
	echo "=== Installing $@";					\
	$(INSTALL_DIRS) $@

# Stub targets.
all clean clobber:

# Install public headers.
install:	$(INST_HEADDIRS) $(INST_HEADERS)

$(INST_INCROOT)/%:	%
	@$(INSTALL_HEADER)

$(INST_INCROOT)/%:	$(OBJS_INCLUDE)/%
	@$(INSTALL_HEADER)

$(INST_INCROOT)/cxx/%:	$(OBJS_INCLUDE)/%
	@$(INSTALL_HEADER)

$(INST_HEADDIRS):
	@$(INSTALL_HEADDIRS)

$(INST_HEADERS):	$(INST_HEADDIRS)
