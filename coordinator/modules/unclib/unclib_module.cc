/*
 * Copyright (c) 2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*@brief   unclib
 *@file    unclib_module.cc
 *Desc:    This file contains the definition of UncLibModule module 
 */

#include <pfcxx/module.hh>
#include <uncxx/lib_unc.hh>
#include "unclib_module.hh"
using unc::unclib::UncLibModule;

UncLibModule* UncLibModule::unclib_module_ = NULL;

/**
 * @brief      UncLibModule constructor
 * @param[in]  const pfc_modattr_t * 
 */
UncLibModule::UncLibModule(const pfc_modattr_t *attr)
    :pfc::core::Module(attr) {
      /*  */
}

/**
 * @brief      UncLibModule destructor
 * @param[in]  None 
 */
UncLibModule::~UncLibModule() {
}

/**
 * @brief      UncLibModule get_unclib_module
 * @param[in]  None 
 * @param[out] unclib_module_
 */
UncLibModule* UncLibModule::get_unclib_module() {
  return unclib_module_;
}
/**
 * @brief      UncLibModule init()
 * @param[in]  None 
 */
pfc_bool_t UncLibModule::init(void) {
  pfc_log_info("UncLibModule init");
  unclib_module_ = this;
  return PFC_TRUE;
}

/**
 * @brief      UncLibModule fini()
 * @param[in]  None 
 */
pfc_bool_t UncLibModule::fini(void) {
  pfc_log_notice("Fini");
  return PFC_TRUE;
}

PFC_MODULE_DECL(unc::unclib::UncLibModule);
