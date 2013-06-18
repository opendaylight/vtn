#
# Copyright (c) 2012-2013 NEC Corporation
# All rights reserved.
# 
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

##
## Common configurations to build UNC libraries.
##

LIBS_BLDDIR	?= ../../build
NEED_OBJDIR	:= 1

# Installation target directory.
LIBS_TARGETDIR	?= $(INST_LIBDIR)

ifndef	SHLIB_DLOPEN
# Create symbolic links to library file under LINK_LIBDIR.
EXPORT_LIBS	:= 1
endif	# !SHLIB_DLOPEN

include $(LIBS_BLDDIR)/config.mk
include $(BLDDIR)/shared-defs.mk
