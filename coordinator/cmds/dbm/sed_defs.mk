#
# Copyright (c) 2012-2014 NEC Corporation
# All rights reserved.
#
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

NEED_OBJDIR	:= 1
CLM_ALLOWED_UID	= 0

# sed(1) rules to generate shell scripts.
SED_RULE_LIST	= 's,%INST_BINDIR%,$(INST_BINDIR),g'
SED_RULE_LIST	+= 's,%INST_DATADIR%,$(INST_DATADIR),g'
SED_RULE_LIST	+= 's,%INST_JARDIR%,$(INST_JARDIR),g'
SED_RULE_LIST	+= 's,%INST_LIBEXECDIR%,$(INST_LIBEXECDIR),g'
SED_RULE_LIST	+= 's,%INST_LOCALSTATEDIR%,$(INST_LOCALSTATEDIR),g'
SED_RULE_LIST	+= 's,%INST_SBINDIR%,$(INST_SBINDIR),g'
SED_RULE_LIST	+= 's,%INST_SYSCONFDIR%,$(INST_SYSCONFDIR),g'
SED_RULE_LIST	+= 's,%INST_SYSSCRIPTDIR%,$(INST_SYSSCRIPTDIR),g'
SED_RULE_LIST	+= 's,%INST_UNCWORKDIR%,$(INST_UNCWORKDIR),g'
SED_RULE_LIST	+= 's,%INST_SQLDIR%,$(INST_SQLDIR),g'
SED_RULE_LIST	+= 's,%PATH_SCRIPT%,$(PATH_SCRIPT),g'
SED_RULE_LIST	+= 's,%SHELL_PATH%,$(SHELL_PATH),g'
SED_RULE_LIST	+= 's,%MULTIARCH%,$(MULTIARCH),g'
