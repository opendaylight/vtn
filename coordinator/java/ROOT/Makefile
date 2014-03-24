#
# Copyright (c) 2013-2014 NEC Corporation
# All rights reserved.
# 
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

##
## Makefile that drives the production of ROOT context
##

include ../defs.mk

ifdef	JAVA_EXT_MK

# Attributes for manifest file.
JAVA_SPEC_TITLE         = VTN Service WEB API redirection for OpenStack
JAVA_IMPL_TITLE         = $(JAVA_SPEC_TITLE)

include ../rules.mk

else	# !JAVA_EXT_MK

all install clean clobber doc:

.PHONY:	all install clean clobber doc

endif	# JAVA_EXT_MK
