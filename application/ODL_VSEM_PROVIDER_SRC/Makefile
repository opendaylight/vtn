#
# Copyright (c) 2015 NEC Corporation
# All rights reserved.
#
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#


VSEM_PROVIDER_OUT_DIR     := dist/odl_vsem_provider/ODL_VSEM_PROVIDER
VSEM_PROVIDER_UI_OUT_DIR  := dist/odl_vsem_provider/ODL_VSEM_PROVIDER_UI
CONFIGURE_OUT_DIR         := dist/odl_vsem_provider
CONFIGURE 		  := "./configure"
ODL_SCVMM_PROVIDER        := dist/odl_vsem_provider/ODL_SCVMM_PROVIDER
EXECUTE			  := $(shell bash configure)


ifneq  ($(realpath $(CONFIGURE_OUT_DIR)/defs.mk),)
include $(CONFIGURE_OUT_DIR)/defs.mk
endif # defs.mk does not exist

all:run_configure vsemodlprovider vsemodlui Register_entry install

run_configure:
	bash configure	

install:
	mkdir -p $(ODL_SCVMM_PROVIDER);
	$(ZIP) -r $(VSEM_PROVIDER_OUT_DIR).zip $(VSEM_PROVIDER_OUT_DIR)
	$(ZIP) -r $(VSEM_PROVIDER_UI_OUT_DIR).zip $(VSEM_PROVIDER_UI_OUT_DIR)
	cd dist/odl_vsem_provider;rm -rf ODL_VSEM_PROVIDER.zip;mv ODL_VSEM_PROVIDER ODL_SCVMM_PROVIDER;
	cd dist/odl_vsem_provider;rm -rf ODL_VSEM_PROVIDER_UI.zip;mv ODL_VSEM_PROVIDER_UI ODL_SCVMM_PROVIDER;
	cd dist/odl_vsem_provider;mkdir -p Register_settings;mv reg_entry.reg Register_settings;mv Register_settings ODL_SCVMM_PROVIDER;
	cd dist/odl_vsem_provider;$(ZIP) -r ODL_SCVMM_PROVIDER.zip ODL_SCVMM_PROVIDER
	
VSEM_ODL_PROVIDER_SOURCES = $(shell find VSEMOdlProvider -name '*.cs')
VSEM_ODL_PROVIDER_UI_SOURCES = $(shell find VSEMOdlUI -name '*.cs')

vsemodlprovider:
	mkdir -p $(VSEM_PROVIDER_OUT_DIR)
	$(MCS) $(CS_OPTIONS) "-out:$(VSEM_PROVIDER_OUT_DIR)/VSEMOdl.dll" $(VSEM_ODL_PROVIDER_SOURCES) $(REF_STRING) 

vsemodlui: vsemodlprovider
	mkdir -p $(VSEM_PROVIDER_UI_OUT_DIR)
	$(MCS) $(CS_OPTIONS) "-out:$(VSEM_PROVIDER_UI_OUT_DIR)/VSEMOdlUI.dll" $(VSEM_ODL_PROVIDER_UI_SOURCES) $(REF_STRING) "-r:$(VSEM_PROVIDER_OUT_DIR)/VSEMOdl.dll"

Register_entry:
	cp reg_entry.reg dist/odl_vsem_provider

clean:
	rm -rf dist
