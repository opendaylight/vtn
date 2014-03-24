#
# Copyright (c) 2012-2014 NEC Corporation
# All rights reserved.
# 
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

##
## Makefile that drives the tests for PFC core utilities.
##

# Use PFC daemon maintenance API.
USE_PFCD_MAINT	= 1

GTEST_SRCROOT	:= ../../..
include $(GTEST_SRCROOT)/test/build/gtest-defs.mk

EXEC_NAME	:= libpfc_util_test

CXX_SOURCES	=			\
	test_atomic.cc			\
	test_bitpos.cc			\
	test_byteorder.cc		\
	test_clock_linux.cc		\
	test_cloexec.cc			\
	test_conf.cc			\
	test_cred.cc			\
	test_ctype.cc			\
	test_flock.cc			\
	test_frotate.cc			\
	test_hash_cmn_hashops.cc	\
	test_hash_gen_basic.cc		\
	test_hash_gen_sync.cc		\
	test_hash_gen_mt.cc		\
	test_hash_gen_vref.cc		\
	test_hash_gen_hashops.cc	\
	test_hash_kref_basic.cc		\
	test_hash_kref_hashops.cc	\
	test_hash_kref_vref_hashops.cc	\
	test_hash_str_basic.cc		\
	test_hash_str_sync.cc		\
	test_hash_str_mt.cc		\
	test_hash_str_vref.cc		\
	test_hash_str_hashops.cc	\
	test_hash_str_vref_hashops.cc	\
	test_hash_u64_basic.cc		\
	test_hash_u64_sync.cc		\
	test_hash_u64_mt.cc		\
	test_hash_u64_vref.cc		\
	test_hash_opt_nolock.cc		\
	test_hash_opt_prime.cc		\
	test_hash_replace.cc		\
	test_hash_delete.cc		\
	test_hostaddr.cc		\
	test_list.cc			\
	test_listm_cmn_basic.cc		\
	test_listm_cmn_mt.cc		\
	test_listm_cmn_ref.cc		\
	test_listm_llist_basic.cc	\
	test_listm_llist_mt.cc		\
	test_listm_llist_ref.cc		\
	test_listm_vec_basic.cc		\
	test_listm_vec_mt.cc		\
	test_listm_vec_ref.cc		\
	test_listm_hash_basic.cc	\
	test_listm_hash_mt.cc		\
	test_listm_hash_ref.cc		\
	test_log.cc			\
	test_loginit.cc			\
	test_misc.cc			\
	test_mkdir.cc			\
	test_pwd.cc			\
	test_rbtree.cc			\
	test_rbtree_ex.cc		\
	test_refptr.cc			\
	test_rmpath.cc			\
	test_socket.cc			\
	test_string.cc			\
	test_strint.cc			\
	test_strtoint.cc		\
	test_sysconf.cc			\
	test_hash_util.cc		\
	test_pseudo_rand.cc		\
	test_pidfile.cc			\
	test_time.cc			\
	test_proc.cc

CXX_SOURCES	+=		\
	child.cc		\
	flock_subr.cc		\
	misc.cc			\
	pseudo_rand.cc		\
	random.cc		\
	signal_subr.cc		\
	thread_subr.cc		\
	tmpfile.cc

C_SOURCES	=		\
	init_fini.c		\
	rbtree_subr.c		\
	strint_refptr.c		\
	string_util.c		\
	hash_util.c		\
	listm_util.c

# Pass object directory path to preprocessor.
CPPFLAGS	+= -DOBJDIR="\"$(OBJDIR)\""

# Link command and control protocol libraries.
PFC_LIBS	+= libpfc_util libpfc libpfc_cmd libpfc_ctrl
LDLIBS		+= -lrt

# Import system library private header files.
EXTRA_INCDIRS	= $(PFC_LIBS:%=$(SRCROOT)/libs/%)

# Define PFC TEST configuration file format.
CFDEF_FILES	= test_conf.cfdef	\
		  test_loginit.cfdef

# Define file path to output hash report.
# If '-' is set, reports are written to stdout
HASH_REPORT_OUTPUT	?= -

## Sub commands used for testing.
SUBCMD_DIRS	= 	\
	sub_cloexec	\
	sub_flock

EXTRA_TARGET	+= $(SUBCMD_DIRS)
CLEAN_DEPS	= $(SUBCMD_DIRS)

##
## rules
##

include $(GTEST_BLDDIR)/gtest-rules.mk

$(SUBCMD_DIRS):
	$(MAKE) -C $@ $(SUBCMD_TARGET)

all:		SUBCMD_TARGET = all
clean:		SUBCMD_TARGET = clean

install:	all

.PHONY:	$(SUBCMD_DIRS)
