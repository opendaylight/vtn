#
# Copyright (c) 2012-2013 NEC Corporation
# All rights reserved.
# 
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

##
## Common configurations to build mgmt module.
##

LIBS_BLDDIR	:= ../../../build

# Installation target directory.
#LIBS_TARGETDIR	= $(INST_LIBDIR)/clplugin
LIBS_TARGETDIR	= $(INST_LIBDIR)

# ***************
# Cluster plugin is linked to neither executable nor library.
#SHLIB_DLOPEN	:= 1

# ***************
# Don't use SO version for cluster plugin module.
#NO_SO_VERSION	:= 1

include ../../defs.mk
