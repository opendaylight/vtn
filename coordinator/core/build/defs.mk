#
# Copyright (c) 2010-2014 NEC Corporation
# All rights reserved.
#
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

##
## Common configurations for C/C++ build process.
## This makefile is designed to be included from config.mk.
##

ifndef	BUILD_DEFS_MK_INCLUDED

BUILD_DEFS_MK_INCLUDED	:= 1

# Path to build tools.
DEPFIX		:= $(TOOLBIN)/depfix
DIRPATH		:= $(TOOLBIN)/dirpath
MODTOOL		:= $(TOOLBIN)/modtool
CFDEFC		:= $(TOOLBIN)/cfdefc
GENCOPY		:= $(TOOLBIN)/gencopy
ELFSIGN		:= $(TOOLBIN)/elfsign
HEADEXPORT	:= $(TOOLBIN)/headexport
IPCTC		:= $(TOOLBIN)/ipctc
LISTFILE	:= $(TOOLBIN)/listfile
REPLACE		:= $(TOOLBIN)/replace

# The oldest year to be embedded into copyright notice.
OLDEST_YEAR	:= 2012

YEAR_RANGE_CMD	= $(GENCOPY) -Y $(OLDEST_YEAR:%=-O %)

# Assertions to verify output of "shell" function.
# These assertions are expected to be used via "eval" and "call" function.
SHELL_FUNC_ASSERT	=						\
	$(if $(1),$(if $(subst $(2),,$(firstword $(1))),,$(error $(1))))
DIRPATH_ASSERT	= $(call SHELL_FUNC_ASSERT,$(1),ERROR:)
MODTOOL_ASSERT	= $(call SHELL_FUNC_ASSERT,$(1),ERROR:)
LISTFILE_ASSERT	= $(call SHELL_FUNC_ASSERT,$(1),ERROR:)
GENCOPY_ASSERT	= $(call SHELL_FUNC_ASSERT,$(1),ERROR:)

# Name of directory which keeps dependencies.
DEPDIR		:= .deps

# Name of directory which keeps build command and its arguments.
CMDDIR		:= .cmds

# Object directory.
ifndef	NO_DIRPATH
ifdef	NEED_OBJDIR
DIRPATH_ARGS	?= -C $(CMDDIR) -D $(DEPDIR)
OBJDIR		:= $(shell $(DIRPATH) -mc $(DIRPATH_ARGS))
else	# !NEED_OBJDIR
OBJDIR		:= $(shell $(DIRPATH) -m)
endif	# NEED_OBJDIR
$(eval $(call DIRPATH_ASSERT,$(OBJDIR)))
endif	# !NO_DIRPATH

# Command to install directories.
PERM_DIR	= 0755
INSTALL_DIRS	= $(INSTALL) -d -m $(PERM_DIR)

# Command to install programs.
PERM_PROGS	= 0755
INSTALL_PROGS	= $(INSTALL) $(INSTALL_BIN_FLAGS) -m $(PERM_PROGS)

# Command to install shared libraries.
PERM_SHLIBS	= 0755
INSTALL_SHLIBS	= $(INSTALL) $(INSTALL_BIN_FLAGS) -m $(PERM_SHLIBS)

# Command to install system files.
PERM_FILES	= 0644
INSTALL_FILES	= $(INSTALL) -m $(PERM_FILES)

# Command to install system files, and required directories.
# arg1 is source file path, and arg2 is destination file path.
CMD_INSTALL_FILE	=					\
	dir=$(dir $(2));					\
	if [ ! -d $$dir ]; then					\
	    echo "=== Installing $$dir";			\
	    $(INSTALL_DIRS) $$dir || exit 1;			\
	fi;							\
	echo "=== Installing $(2)";				\
	$(INSTALL_FILES) $(1) $(2)

# Create symbolic link.
LN_S		= $(LN) -sf

# Create hard link.
LN_H		= $(LN) -f

ifndef	UNC_BUILD

# Directory path to place auto-generated header files.
OBJS_INCLUDE	= $(OBJROOT)/include

# C/C++ header include path under object directory.
OBJS_INCDIRS	= include include/openflow

endif	# !UNC_BUILD

# IPC struct template directory.
IPC_SRCROOT	?= $(SRCROOT)
IPC_DIRNAME	= ipc
IPC_TMPLDIR	= $(IPC_SRCROOT)/$(IPC_DIRNAME)

# Path to IPC struct information file.
IPC_STRUCT_BIN	= ipc_struct.bin
OBJ_IPC_BIN	= $(OBJROOT)/$(IPC_DIRNAME)/$(IPC_STRUCT_BIN)

# Relative path to IPC struct header.
IPC_STRUCT_H	= pfc/ipc_struct.h

# Relative path to C++ header which defines user-defined IPC struct accessors.
IPC_SERVER_PROTO_HH	= pfcxx/ipc_server_proto.hh
IPC_SERVER_INLINE_HH	= pfcxx/ipc_server_inline.hh
IPC_CLIENT_PROTO_HH	= pfcxx/ipc_client_proto.hh
IPC_CLIENT_INLINE_HH	= pfcxx/ipc_client_inline.hh

ifndef	UNC_BUILD

IPCTC_C_FILES		= $(IPC_STRUCT_H)
IPCTC_CXX_FILES		= $(IPC_SERVER_PROTO_HH) $(IPC_SERVER_INLINE_HH)
IPCTC_CXX_FILES		+= $(IPC_CLIENT_STRUCT_HH) $(IPC_CLIENT_INLINE_HH)

# Path to C and C++ language header files for IPC.
OBJ_STRUCT_H	= $(OBJS_INCLUDE)/$(IPC_STRUCT_H)
OBJ_SERVER_P_HH	= $(OBJS_INCLUDE)/$(IPC_SERVER_PROTO_HH)
OBJ_SERVER_I_HH	= $(OBJS_INCLUDE)/$(IPC_SERVER_INLINE_HH)
OBJ_CLIENT_P_HH	= $(OBJS_INCLUDE)/$(IPC_CLIENT_PROTO_HH)
OBJ_CLIENT_I_HH	= $(OBJS_INCLUDE)/$(IPC_CLIENT_INLINE_HH)
IPC_HEADERS	= $(OBJ_STRUCT_H) $(OBJ_SERVER_P_HH) $(OBJ_SERVER_I_HH)
IPC_HEADERS	+= $(OBJ_CLIENT_P_HH) $(OBJ_CLIENT_I_HH)

# C/C++ header include path.
ifeq	($(strip $(SUBARCH)),)
CC_INCDIRS	= include/arch/$(ARCH)
else	# !empty(SUBARCH)
CC_INCDIRS	= include/arch/$(SUBARCH) include/arch/$(ARCH)
endif	# empty(SUBARCH)

CC_INCDIRS	+= include/os/$(OSTYPE) include

# C++ header include path.
CXX_INCDIRS	= include/cxx

endif	# !UNC_BUILD

# Definitions for debugging.
CC_DEBUG_DEFS	+= -DPFC_DEBUG
ifdef	DEBUG_BUILD
CC_DEBUG_DEFS	+= -DPFC_VERBOSE_DEBUG
endif	# DEBUG_BUILD

# C/C++ preprocessor options.
# EXTRA_INCDIRS will be added to header search path for both C and C++ language,
# and EXTRA_CXX_INCDIRS only for C++ language.
# Note that CPPFLAGS is applied to both C and C++ compilation.
CC_DEFS		= $(CC_FEATURE_DEFS) $(CC_DEBUG_DEFS) $(EXTRA_CPPFLAGS)
CC_INCLUDES	= $(CC_INCDIRS_FIRST:%=-I%) $(CC_INCDIRS_PREP:%=-I%)
CC_INCLUDES	+= $(OBJS_INCDIRS:%=-I$(OBJROOT)/%)
CC_INCLUDES	+= $(CC_INCDIRS:%=-I$(SRCROOT)/%) $(OPENSSL_INCDIR:%=-I%)
CC_INCLUDES	+=  $(EXTRA_INCDIRS:%=-I%)
CPPFLAGS	= $(CPPFLAGS_ALL) $(CC_DEFS) $(CC_INCLUDES)
CXX_DEFS	= $(EXTRA_CXX_CPPFLAGS)
CXX_INCLUDES	= $(CXX_INCDIRS:%=-I$(SRCROOT)/%) $(BOOST_INCDIR:%=-I%)
CXX_INCLUDES	+= $(EXTRA_CXX_INCDIRS:%=-I%)
CXX_CPPFLAGS	= $(CXX_DEFS) $(CXX_INCLUDES)

# Build object for unit test.
# Never define UNIT_TEST for production build!
ifdef	UNIT_TEST
CPPFLAGS	+= -DPFC_UNIT_TEST
endif	# UNIT_TEST

# Disable some maintenance API for PFC daemon unless USE_PFCD_MAINT is defined.
ifndef	USE_PFCD_MAINT
CPPFLAGS	+= -D_PFC_PFCD_MAINT_DISABLE
endif	# !USE_PFCD_MAINT

# C compiler options.
CFLAGS		= $(CC_DEBUG) $(CC_OPT) $(CC_WARN) $(CC_MODE) $(EXTRA_CFLAGS)

# C++ compiler options.
CXXFLAGS	= $(CXX_DEBUG) $(CXX_OPT) $(CXX_WARN) $(CXX_MODE)	\
		  $(EXTRA_CXXFLAGS)

# CPPFLAGS used to compile assembly language source.
AS_CPPFLAGS	= $(CPPFLAGS) -D_PFC_ASM

# C compiler options used to compile assembly language source.
ASFLAGS		= -x assembler-with-cpp

# External libraries are linked if PFC_EXTLIBS is defined. PFC_EXTLIBS must
# contains directory name under $(SRCROOT)/ext.
PFC_EXTLIBDIRS	= $(PFC_EXTLIBS:%=$(OBJROOT)/ext/%)

# Library search path for runtime linker.
LD_RUNTIME_DIR	= $(LD_RUNTIME_DIR_FIRST) $(LD_RUNTIME_DIR_PREP) 
LD_RUNTIME_DIR	+= $(EXTRA_RUNTIME_DIR) $(OPENSSL_RUNPATH) $(INST_LIBDIR)
LD_RUNTIME_DIR	+= $(EXTERNAL_RUNPATH)
LD_RPATH	= $(filter-out $(DEFAULT_LIBPATH), $(abspath $(LD_RUNTIME_DIR)))

# Linker option to disallow unresolved symbol.
LD_ZDEFS	= -z defs

# Linker options.
# You can control this via following macros:
#
# OBJTREE_LIBDIRS:	Library search path ($(OBJROOT) relative)
# EXTRA_LIBDIRS:	Library search path (absolute path)
# EXTRA_LDLIBS:		Library option to be passed to linker (e.g. -lm)
# LD_LDFLAGS:		Linker options to be passed via "-Wl".
# EXTRA_LDFLAGS:	Extra flags to be passed to linker.
LD_LIBDIRS	= $(LINK_LIBDIR) $(OBJTREE_LIBDIRS:%=$(OBJROOT)/%)
LD_LIBDIRS	+= $(LD_LIBDIRS_FIRST) $(LD_LIBDIRS_PREP)
LD_LIBDIRS	+= $(OPENSSL_LIBDIR) $(PFC_EXTLIBDIRS)
LD_LIBDIRS	+= $(filter-out $(DEFAULT_LIBPATH),$(abspath $(EXTRA_LIBDIRS)))
LDFLAGS_RPATH	= $(LD_RPATH:%=-Wl,-rpath,%)
LDFLAGS		= $(LD_LIBDIRS:%=-L%) $(LDFLAGS_RPATH) $(LD_LDFLAGS:%=-Wl,%)
LDFLAGS		+= $(LD_MODE) $(EXTRA_LDFLAGS)
LDLIBS		= $(LDLIBS_PREP) $(PFC_LIBS:lib%=-l%)
LDLIBS		+= $(PFC_EXTLIBS:%=-l%) $(EXTRA_LDLIBS)

# Define POSIX thread options unless NO_PTHREAD is defined.
ifndef	NO_PTHREAD
CPPFLAGS	+= -pthread
LDFLAGS		+= -pthread
endif	# !NO_PTHREAD

# Define macros for cfdef compiler.
CFDEFC_CPP	= $(CC)
CFDEFC_CPPFLAGS	= -x c -E $(CC_MODE) $(CPPFLAGS_ALL) $(CC_WARN) $(CC_INCLUDES)
CFDEFC_CPPFLAGS	+= $(CC_DEFS) -D_PFC_IN_CFDEFC
CFDEFC_FLAGS	= $(CFDEF_MODE) -c $(CFDEFC_CPP) $(CFDEFC_CPPFLAGS:%=-C %)

CFDEF_PREFIX	= _cfdef_
CFDEF_SOURCES	= $(CFDEF_FILES:%.cfdef=$(OBJDIR)/$(CFDEF_PREFIX)%.c)
CFDEF_DEPFILES	= $(CFDEF_FILES:%=$(OBJ_DEPDIR)/%.d)

# Options for ar command to create archive.
ARFLAGS		= cru

# Check whether the specified file exists or not.
# If not, this function is evaluated as empty string.
TEST_FILE_EXISTS	= $(shell test -f $(1) && echo $(1))

# List of phony targets.
PHONY_TARGET	= FRC

# Files to be removed by "make clobber".
CLOBBERFILES	= $(OBJDIR)

# List of archive libraries under libs directory.
PFCLIB_ARCHIVES	:= libpfc_ctrl libpfc_licld

endif	# !BUILD_DEFS_MK_INCLUDED
