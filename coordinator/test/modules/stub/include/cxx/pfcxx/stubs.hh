/*
 * Copyright (c) 2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef STUB_SESS_HH
#define STUB_SESS_HH

#include<libpfc_ipc/ipc_impl.h>

#define MODULE_NAME "tc"

#undef PFC_ASSERT
#define PFC_ASSERT(ex)          ((void)0)

#undef __PFC_MODULE_SECT_DECL
#define __PFC_MODULE_SECT_DECL                                  \
        const uint8_t    __pfc_module_sect_text[] = {100};       \
        const uint8_t    __pfc_module_sect_rodata[] = {100};     \
        const uint8_t    __pfc_module_sect_data[] = {100};       \
        const uint8_t    __pfc_module_sect_bss[] = {100};        \
        const uint8_t    __pfc_module_sect_end[] = {100};


#define __PFC_MODULE_SECT_INITIALIZER   \
      {         \
            __pfc_module_sect_text,   \
            __pfc_module_sect_rodata, \
            __pfc_module_sect_data,   \
            __pfc_module_sect_bss,    \
            __pfc_module_sect_end,    \
          }

struct __pfc_ipcsrv {
  pfc_ipcid_t     isv_service;
  ipc_msg_t       isv_args;
//  pfc_mutex_t     isv_mutex;      /* mutex */
//  pfc_cond_t      isv_cond;       /* condition variable */
};  // duplicate


#endif


