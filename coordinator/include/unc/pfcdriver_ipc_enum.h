/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef IPC_PFCDRV_IPC_ENUM_H_
#define IPC_PFCDRV_IPC_ENUM_H_

#include "unc/base.h"

/* enum for pfcdrv val vbrif structure */
typedef enum {
  PFCDRV_IDX_VAL_VBRIF = 0,
  PFCDRV_IDX_VEXT_NAME_VBRIF,
  PFCDRV_IDX_VEXTIF_NAME_VBRIF,
  PFCDRV_IDX_VLINK_NAME_VBRIF,
} pfcdrv_val_vbr_if_index_t;

/*index enumeration for pfcdrv_val_vbrif structure */
typedef enum {
  PFCDRV_IDX_INTERFACE_TYPE = 0,
  PFCDRV_IDX_VEXTERNAL_NAME_VBRIF,
  PFCDRV_IDX_VEXT_IF_NAME_VBRIF,
} pfcdrv_val_vbrif_vextif_index_t;

/* enum for val vbrif type */
typedef enum {
  PFCDRV_IF_TYPE_VBRIF = 0,
  PFCDRV_IF_TYPE_VEXTIF
} pfcdrv_val_vbrif_type_t;

/* index for pfcdriver val_flowfilter entry structure */
typedef enum {
  PFCDRV_IDX_FLOWFILTER_ENTRY_FFE = 0,
  PFCDRV_IDX_VAL_VBRIF_VEXTIF_FFE,
} pfcdrv_val_flowfilter_entry_index_t;

/* index for pfcdriver val_policingamp structure */
typedef enum {
  PFCDRV_IDX_VAL_POLICINGMAP_PM = 0,
  PFCDRV_IDX_VAL_VBRIF_VEXTIF_PM,
} pfcdrv_val_vbrif_policingmap_index_t;

/* enum for pfcdrv val vbrif structure */
typedef enum {
  PFCDRV_IDX_VAL_VLAN_MAP = 0,
  PFCDRV_IDX_BDRY_REF_COUNT,
} pfcdrv_val_vlan_map_index_t;

/* enum for controller versions */
typedef enum {
  PFCDRV_IDX_CTR_NONE = 0,
  PFCDRV_IDX_CTR_FIVE_ZERO,
  PFCDRV_IDX_CTR_FIVE_ONE,
  PFCDRV_IDX_CTR_SIX_ZERO,
  PFCDRV_IDX_CTR_SIX_ONE,
  PFCDRV_IDX_CTR_SIX_TWO
} pfcdrv_ctr_versions_index_t;

typedef enum {
  PFCDRV_IDX_VAL_VBR_PORTMAP = 0,
  PFCDRV_IDX_VBRPM_BDRY_REF_COUNT,
} pfcdrv_val_vbr_portmap_index_t;

/* enum for pfcdrv_val_vtn_controller structure */
 typedef enum {
  PFCDRV_IDX_VTNCTRL_LABEL_TYPE = 0,
  PFCDRV_IDX_VTNCTRL_LABEL,
} pfcdrv_val_vtn_controller_index_t;

/* enum for pfcdrv_val_vtn_flooding_path structure */
 typedef enum {
   PFCDRV_IDX_VAL_GVTNID = 0
 } pfcdrv_val_vtn_flooding_path_index_t;

/* enum for import mode */
typedef enum {
  UNC_IMPORT_ERROR_MODE = 0,
  UNC_IMPORT_IGNORE_MODE
} UncImportMode;
#endif
