#
# Copyright (c) 2010-2013 NEC Corporation
# All rights reserved.
# 
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

##
## Common rules to generate target file using sed(1).
##
## Name of source file must have ".in" as suffix. If the source file name is
## XXX.in, the target file path must be $(OBJDIR)/XXX.
##
## Command line arguments for sed(1) must be specfified by makefile variable,
## SED_RULES.
##

ifndef	BUILD_SED_RULES_MK_INCLUDED

BUILD_SED_RULES_MK_INCLUDED	:= 1

.DELETE_ON_ERROR:

# Generate target file under the object directory.
CMDRULE_SED	=							\
	set -e;								\
	echo "=== Generating $@";					\
	$(SED) $(SED_RULES) $< > $@

$(OBJDIR)/%:	%.in FRC
	@$(call CMD_EXECUTE,SED,$(OBJ_CMDDIR)/$*.cmd,$?)

endif	# !BUILD_SED_RULES_MK_INCLUDED
