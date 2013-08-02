#
# Copyright (c) 2012-2013 NEC Corporation
# All rights reserved.
# 
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

##
## Common rules to generate target file using "replace".
##
## Name of source file must have ".in" as suffix. If the source file name is
## XXX.in, the target file path must be $(OBJDIR)/XXX.
##
## Command line arguments for 'replace" must be specfified by REPL_RULES
## makefile variable.
##
## Syntax of REPL_RULES:
##
## -p <keyword> <replacement>
##	Replace all <keyword> in the template file with <replacement>.
##
## -a <keyword> string
##	Indicate that <keyword> specified by -p should be replaced with a
##	quoted string. <replacement> specified by -p will be converted into
##	a quoted string.
##

ifndef	BUILD_REPL_RULES_MK_INCLUDED

BUILD_REPL_RULES_MK_INCLUDED	:= 1

# Generate target file under the object directory.
CMDRULE_REPLACE	=							\
	set -e;								\
	echo "=== Generating $@";					\
	$(REPLACE) -o $@ $(REPL_RULES) $<

$(OBJDIR)/%:	%.in FRC
	@$(call CMD_EXECUTE,REPLACE,$(OBJ_CMDDIR)/$*.cmd,$?)

endif	# !BUILD_REPL_RULES_MK_INCLUDED
