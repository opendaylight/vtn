#
# Copyright (c) 2015 NEC Corporation
# All rights reserved.
#
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

OUTDIR			= target
DEFS_MK			= $(OUTDIR)/defs.mk
CONFIG_DONE		= $(OUTDIR)/config.done
CONFIGURE		= configure

VSEM_DIR		= $(OUTDIR)/odl_vsem_provider
VSEM_PROVIDER_DIR	= $(VSEM_DIR)/ODL_VSEM_PROVIDER
VSEM_PROVIDER_UI_DIR	= $(VSEM_DIR)/ODL_VSEM_PROVIDER_UI

ODL_SCVMM_PROVIDER	= ODL_SCVMM_PROVIDER
DIST_DIR		= $(VSEM_DIR)/$(ODL_SCVMM_PROVIDER)
DIST_REGISTER_DIR	= $(DIST_DIR)/Register_settings
SCVMM_PROVIDER_ZIP	= $(ODL_SCVMM_PROVIDER).zip
DIST_ODL_SCVMM_PROVIDER	= $(OUTDIR)/$(SCVMM_PROVIDER_ZIP)

VSEM_ODL_PROVIDER_SOURCES	:= $(shell find VSEMOdlProvider -name '*.cs')
VSEM_ODL_DLL			= $(VSEM_PROVIDER_DIR)/VSEMOdl.dll

VSEM_ODL_PROVIDER_UI_SOURCES	:= $(shell find VSEMOdlUI -name '*.cs')
VSEM_ODL_PROVIDER_UI_DLL	= $(VSEM_PROVIDER_UI_DIR)/VSEMOdlUI.dll

-include $(DEFS_MK)

all:	vsemodlui

install:	$(DIST_ODL_SCVMM_PROVIDER)

run_configure:	$(CONFIG_DONE)

$(CONFIG_DONE):	$(CONFIGURE)
	bash $(CONFIGURE)

$(DIST_DIR):	vsemodlui
	mkdir -p $@
	cp -a $(VSEM_PROVIDER_DIR) $@
	cp -a $(VSEM_PROVIDER_UI_DIR) $@
	mkdir -p $(DIST_REGISTER_DIR)
	cp -a reg_entry.reg $(DIST_REGISTER_DIR)

$(DIST_ODL_SCVMM_PROVIDER):	$(DIST_DIR)
	out=$(abspath $@); cd $(VSEM_DIR); \
	$(ZIP) -r $$out $(ODL_SCVMM_PROVIDER)

vsemodlprovider:	$(VSEM_ODL_DLL)

$(VSEM_ODL_DLL):	$(VSEM_ODL_PROVIDER_SOURCES)
	mkdir -p $(VSEM_PROVIDER_DIR)
	$(MCS) $(CS_OPTIONS) "-out:$@" $(VSEM_ODL_PROVIDER_SOURCES) $(REF_STRING)

vsemodlui:	$(VSEM_ODL_PROVIDER_UI_DLL)

$(VSEM_ODL_PROVIDER_UI_DLL):	$(VSEM_ODL_PROVIDER_UI_SOURCES) $(VSEM_ODL_DLL)
	mkdir -p $(VSEM_PROVIDER_UI_DIR)
	$(MCS) $(CS_OPTIONS) "-out:$@" $(VSEM_ODL_PROVIDER_UI_SOURCES) $(REF_STRING) "-r:$(VSEM_ODL_DLL)"

clean:
	$(RM) -f $(OUTDIR)
