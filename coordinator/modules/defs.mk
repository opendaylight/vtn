#
# Copyright (c) 2012-2013 NEC Corporation
# All rights reserved.
# 
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

##
## Common configurations to build UNC modules in pfcd.
##

# pfcd modules must be build using PFC-Core build configuration.
MODULE_SRCROOT	= ../../core
MODULE_BASEDIR	= ..

include $(MODULE_SRCROOT)/modules/defs.mk
include $(ODBC_DEFS_MK)
include $(MODULE_CONFIG_MK)
