#
# Copyright (c) 2010-2013 NEC Corporation
# All rights reserved.
# 
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

##
## Common configurations to build PFC core libraries.
##

NEED_OBJDIR	:= 1

# Installation target directory.
LIBS_TARGETDIR	= $(INST_LIBDIR)

# Create symbolic links to library file under LINK_LIBDIR.
EXPORT_LIBS	:= 1

include ../../build/config.mk
include $(BLDDIR)/shared-defs.mk
