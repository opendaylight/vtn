/*
 * Copyright (c) 2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef UPLL_CONVERT_VNODE_HH_
#define UPLL_CONVERT_VNODE_HH_

#include "pfc/ipc_struct.h"

typedef struct key_convert_vbr {
  key_vbr vbr_key;
  uint8_t conv_vbr_name[40];
} key_convert_vbr_t;

typedef struct val_convert_vbr {
  uint32_t label;
  uint8_t valid[1];
  uint8_t    cs_row_status;
  uint8_t    cs_attr[1];
} val_convert_vbr_t;

//  Converted vBridge Interface key structure
typedef struct key_convert_vbr_if {
  key_convert_vbr  convert_vbr_key;
  uint8_t          convert_if_name[32];
} key_convert_vbr_if_t;

//  Converted vBridge Interface value structure
typedef struct val_convert_vbr_if {
  uint8_t    cs_row_status;
} val_convert_vbr_if_t;

typedef struct val_db_rename_vlink {
  uint8_t valid[2];
  uint8_t ctrlr_vtn_name[32];
  uint8_t ctrlr_vlink_name[32];
} val_db_rename_vlink_t;

//  Converted vLink key structure
typedef struct key_convert_vlink {
  key_vbr vbr_key;                       //  vBridge key
  uint8_t convert_vlink_name[32];        //  converted vLink name
} key_convert_vlink_t;

//  Converted vLink value structure
typedef struct val_convert_vlink {
  uint8_t   valid[7];
  uint8_t   cs_row_status;
  uint8_t   cs_attr[7];
  uint8_t   vnode1_name[40];            //  leaf node name
  uint8_t   vnode1_ifname[32];          //  leaf node interface name
  uint8_t   vnode2_name[40];            //  spine node name
  uint8_t   vnode2_ifname[32];          //  spine node interface name
  uint8_t   boundary_name[32];          //  boundary name
  uint8_t   label_type;                 //  label type
  uint32_t  label;                      //  label value
} val_convert_vlink_t;

// Convert vTunnel key
typedef struct key_convert_vtunnel {
  key_vtn vtn_key;               // VTN key
  uint8_t convert_vtunnel_name[40];  // vTunnel name
} key_convert_vtunnel_t;

// Convert vTunnel Value
typedef struct val_convert_vtunnel {
  uint8_t   valid[2];                      // valid flag
  uint8_t   cs_row_status;                 // object configuration status
  uint8_t   cs_attr[2];                    // attributes configuration status
  uint32_t  ref_count;                     // reference count
  uint32_t  label;                         // gvtnid
} val_convert_vtunnel_t;

typedef struct key_convert_vtunnel_if {
       key_convert_vtunnel convert_vtunnel_key;
       uint8_t convert_if_name[32];
} key_convert_vtunnel_if_t;

typedef struct val_convert_vtunnel_if {
  uint8_t   valid[3];
  uint8_t   cs_row_status;
  uint8_t   cs_attr[3];
  uint8_t   rem_ctrlr_name[32];
  uint8_t   rem_domain_id[32];
  uint8_t   un_vbr_name[32];
} val_convert_vtunnel_if_t;

typedef struct val_vtn_gateway_port {
  uint8_t   valid[3];                      // valid flag
  uint8_t   cs_row_status;                 // object configuration status
  uint8_t   cs_attr[3];
  uint8_t   logical_port_id[320];
  uint32_t  label;
  uint32_t  ref_count;
} val_vtn_gateway_port_t;

/* index enumeration for val_convert_vtunnel structure */
enum val_convert_vtunnel_index {
  UPLL_IDX_REF_COUNT_CONV_VTNL = 0,
  UPLL_IDX_LABEL_CONV_VTNL
};

enum val_convert_vtunnel_if_index {
  UPLL_IDX_RCTRLR_NAME_CONV_VTNL_IF = 0,
  UPLL_IDX_RDOMAIN_ID_CONV_VTNL_IF,
  UPLL_IDX_UN_VBR_NAME_CONV_VTNL_IF
};

// index enumeration for val_convert_vLink structure
enum val_convert_vlink_index {
  UPLL_IDX_VNODE1_NAME_CVLINK = 0,
  UPLL_IDX_VNODE1_IF_NAME_CVLINK,
  UPLL_IDX_VNODE2_NAME_CVLINK,
  UPLL_IDX_VNODE2_IF_NAME_CVLINK,
  UPLL_IDX_BOUNDARY_NAME_CVLINK,
  UPLL_IDX_LABEL_TYPE_CVLINK,
  UPLL_IDX_LABEL_CVLINK,
};

enum val_vtn_gateway_port_index {
  UPLL_IDX_LOGICAL_PORT_ID_GWPORT  = 0,
  UPLL_IDX_LABEL_GWPORT,
  UPLL_IDX_REF_COUNT_GWPORT,
};

enum val_convert_vbr_index {
  UPLL_IDX_LABEL_CONV_VBR = 0
};

// TOOD(SOU)need to remove, once it is unw_spine_domain code is checked in
enum val_unw_spine_domain_ext_t {
  UPLL_IDX_SPINE_DOMAIN_VAL = 0,
  UPLL_IDX_SPINE_USED_LABEL_COUNT_UNWS,
    // UPLL_IDX_SPINE_ALARAM_RAISED_UNWS,
};

enum val_unw_spdom_st_index {
    UPLL_IDX_SPINE_ALARAM_RAISED_UNWS = 0
};

typedef struct val_unw_spdom_ext {
  uint8_t valid[2];
  val_unw_spine_domain val_unw_spine_dom;
  uint32_t used_label_count;  // Used label count
  // uint8_t alarm_status;    // flags to keep
} val_unw_spdom_ext_t;

typedef struct val_spdom_st {
  uint8_t valid[1];
  uint8_t alarm_status;    // flags to keep
} val_spdom_st_t;

typedef struct val_gateway_port {
  uint8_t valid[4];
  uint8_t    cs_row_status;
  uint8_t    cs_attr[4];
  uint8_t    controller_id[32];
  uint8_t    domain_id[32];
  uint8_t logical_port_id[320];
  uint32_t label;
} val_gateway_port_t;

typedef struct key_vbid_label {
  key_vtn vtn_key;
  uint8_t label_row;
} key_vbid_label_t;
typedef struct val_vbid_label {
  uint32_t label_id;
} val_vbid_label_t;

typedef struct key_gvtnid_label {
  uint8_t ctrlr_name[32];
  uint8_t domain_name[32];
  uint8_t label_row;
} key_gvtnid_label_t;
typedef struct val_gvtnid_label {
  uint32_t label_id;
} val_gvtnid_label_t;

#endif
