/*
 * Copyright (c) 2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <pfc/conf_parser.h>

static const pfc_cfdef_param_t  cfdef_params_block_version_list[] = {
    {
     .cfdp_name  = "version",
     .cfdp_min  = (uint64_t)0ULL,
     .cfdp_max  = (uint64_t)1023ULL,
     .cfdp_type  = PFC_CFTYPE_STRING,
     .cfdp_nelems  = PFC_CFPARAM_NELEMS_VARLEN,
     .cfdp_flags  = (0),
    }
};

static const pfc_cfdef_param_t  _cfdef_params_block_kt_cap_map_list[] = {
  {
    .cfdp_name  = "kt_map_name",
    .cfdp_min  = (uint64_t)0ULL,
    .cfdp_max  = (uint64_t)1023ULL,
    .cfdp_type  = PFC_CFTYPE_STRING,
    .cfdp_nelems  = PFC_CFPARAM_NELEMS_VARLEN,
    .cfdp_flags  = (0),
  }
};

static const pfc_cfdef_param_t  _cfdef_params_map_kt_cap[] = {
  {
    .cfdp_name  = "key_type",
    .cfdp_min  = (uint64_t)0x0ULL,
    .cfdp_max  = (uint64_t)0xffffffffULL,
    .cfdp_type  = PFC_CFTYPE_UINT32,
    .cfdp_nelems  = PFC_CFPARAM_NELEMS_SCALAR,
    .cfdp_flags  = (0),
  },
  {
    .cfdp_name  = "attribute_name",
    .cfdp_min  = (uint64_t)0ULL,
    .cfdp_max  = (uint64_t)1023ULL,
    .cfdp_type  = PFC_CFTYPE_STRING,
    .cfdp_nelems  = PFC_CFPARAM_NELEMS_VARLEN,
    .cfdp_flags  = (0),
  },
  {
    .cfdp_name  = "version_supported",
    .cfdp_min  = (uint64_t)0ULL,
    .cfdp_max  = (uint64_t)1023ULL,
    .cfdp_type  = PFC_CFTYPE_STRING,
    .cfdp_nelems  = PFC_CFPARAM_NELEMS_VARLEN,
    .cfdp_flags  = (0),
  },
  {
    .cfdp_name  = "scalability_num",
    .cfdp_min  = (uint64_t)0x0ULL,
    .cfdp_max  = (uint64_t)0xffffffffULL,
    .cfdp_type  = PFC_CFTYPE_UINT32,
    .cfdp_nelems  = PFC_CFPARAM_NELEMS_SCALAR,
    .cfdp_flags  = (0),
  }
};

/*
 * Definitions for parameter blocks.
 */
static const pfc_cfdef_block_t  _cfdef_blocks[] = {
  {
    .cfdb_name  = "version_list",
    .cfdb_params  = _cfdef_params_block_version_list,
    .cfdb_nparams  = 1,
    .cfdb_flags  = (0),
  },
  {
    .cfdb_name  = "kt_cap_map_list",
    .cfdb_params  = _cfdef_params_block_kt_cap_map_list,
    .cfdb_nparams  = 1,
    .cfdb_flags  = (0),
  },
  {
    .cfdb_name  = "kt_cap",
    .cfdb_params  = _cfdef_params_map_kt_cap,
    .cfdb_nparams  = 4,
    .cfdb_flags  = (PFC_CFBF_MAP),
  }
};

const pfc_cfdef_t  ctr_cap_cfdef  = {
  .cfd_block  = _cfdef_blocks,
  .cfd_nblocks  = 3,
};
