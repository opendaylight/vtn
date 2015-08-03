/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * dal_schema.cc - contians schema information of all tables
 *
 */

#include <stdint.h>
#include "dal_schema.hh"

namespace unc {
namespace upll {
namespace dal {

namespace schema {
namespace table {

// L2/L3 MoMgr Schemas - Start
namespace vtn {
const DalColumnSchema vtn_schema[] =  {
  { "vtn_name", SQL_VARCHAR, SQL_C_CHAR, 32, 0 },
  { "description", SQL_VARCHAR, SQL_C_CHAR, 128, 0 },
  { "oper_status", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "alarm_status", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "down_count", SQL_BIGINT, SQL_C_UBIGINT, 1, 0 },
  { "creation_time", SQL_BINARY, SQL_C_BINARY, 8, 0 },
  { "last_updated_time", SQL_BINARY, SQL_C_BINARY, 8, 0 },
  { "flags", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_description", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_oper_status", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_alarm_status", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_creation_time", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_last_updated_time", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_rowstatus", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_description", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "unknown_count", SQL_BIGINT, SQL_C_UBIGINT, 1, 0 },
  { "c_flag", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "u_flag", SQL_SMALLINT, SQL_C_SHORT, 1, 0 }
};
}  // namespace vtn

namespace vtn_controller {
const DalColumnSchema vtn_controller_schema[] =  {
  { "vtn_name", SQL_VARCHAR, SQL_C_CHAR, 32, 0 },
  { "ctrlr_name", SQL_VARCHAR, SQL_C_CHAR, 32, kDcsCtrlrName },
  { "domain_id", SQL_VARCHAR, SQL_C_CHAR, 32, kDcsDomainId },
  { "oper_status", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "alarm_status", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "down_count", SQL_BIGINT, SQL_C_UBIGINT, 1, 0 },
  { "vnode_ref_cnt", SQL_BIGINT, SQL_C_UBIGINT, 1, 0 },
  { "ref_cnt_2", SQL_BIGINT, SQL_C_UBIGINT, 1, 0 },
  { "flags", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_oper_status", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_alarm_status", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_rowstatus", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_description", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "unknown_count", SQL_BIGINT, SQL_C_UBIGINT, 1, 0 },
  { "c_flag", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "u_flag", SQL_SMALLINT, SQL_C_SHORT, 1, 0 }
};
}  // namespace vtn_controller

namespace vtn_rename {
const DalColumnSchema vtn_rename_schema[] =  {
  { "ctrlr_vtn_name", SQL_VARCHAR, SQL_C_CHAR, 32, 0 },
  { "ctrlr_name", SQL_VARCHAR, SQL_C_CHAR, 32, kDcsCtrlrName },
  { "domain_id", SQL_VARCHAR, SQL_C_CHAR, 32, kDcsDomainId },
  { "vtn_name", SQL_VARCHAR, SQL_C_CHAR, 32, 0 },
  { "c_flag", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "u_flag", SQL_SMALLINT, SQL_C_SHORT, 1, 0 }
};
}  // namespace vtn_rename

namespace vbridge {
const DalColumnSchema vbridge_schema[] =  {
  { "vtn_name", SQL_VARCHAR, SQL_C_CHAR, 32, 0 },
  { "vbridge_name", SQL_VARCHAR, SQL_C_CHAR, 32, 0 },
  { "ctrlr_name", SQL_VARCHAR, SQL_C_CHAR, 32, kDcsCtrlrName },
  { "domain_id", SQL_VARCHAR, SQL_C_CHAR, 32, kDcsDomainId },
  { "vbr_description", SQL_VARCHAR, SQL_C_CHAR, 128, 0 },
  { "host_addr", SQL_BINARY, SQL_C_BINARY, 4, 0 },
  { "host_addr_mask", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "oper_status", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "down_count", SQL_BIGINT, SQL_C_UBIGINT, 1, 0 },
  { "unknown_count", SQL_BIGINT, SQL_C_UBIGINT, 1, 0 },
  { "valid_ctrlr_name", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_domain_id", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_vbr_description", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_host_addr", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_host_addr_mask", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_oper_status", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_rowstatus", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_ctrlr_name", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_domain_id", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_vbr_description", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_host_addr", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_host_addr_mask", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "flags", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "c_flag", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "u_flag", SQL_SMALLINT, SQL_C_SHORT, 1, 0 }
};
}  // namespace vbridge

namespace vbridge_vlanmap {
const DalColumnSchema vbridge_vlanmap_schema[] =  {
  { "vtn_name", SQL_VARCHAR, SQL_C_CHAR, 32, 0 },
  { "vbridge_name", SQL_VARCHAR, SQL_C_CHAR, 32, 0 },
  { "logical_port_id", SQL_VARCHAR, SQL_C_CHAR, 320, 0 },
  { "logical_port_id_valid", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "vlanid", SQL_INTEGER, SQL_C_ULONG, 1, 0 },
  { "bdry_ref_count", SQL_BIGINT, SQL_C_UBIGINT, 1, 0 },
  { "ctrlr_name", SQL_VARCHAR, SQL_C_CHAR, 32, kDcsCtrlrName },
  { "domain_id", SQL_VARCHAR, SQL_C_CHAR, 32, kDcsDomainId },
  { "flags", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_vlanid", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_bdry_ref_count", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_rowstatus", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_vlanid", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "c_flag", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "u_flag", SQL_SMALLINT, SQL_C_SHORT, 1, 0 }
};
}  // namespace vbridge_vlanmap

namespace vbridge_interface {
const DalColumnSchema vbridge_interface_schema[] =  {
  { "vtn_name", SQL_VARCHAR, SQL_C_CHAR, 32, 0 },
  { "vbridge_name", SQL_VARCHAR, SQL_C_CHAR, 32, 0 },
  { "if_name", SQL_VARCHAR, SQL_C_CHAR, 32, 0 },
  { "admin_status", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "description", SQL_VARCHAR, SQL_C_CHAR, 128, 0 },
  { "logical_port_id", SQL_VARCHAR, SQL_C_CHAR, 320, 0 },
  { "vlanid", SQL_INTEGER, SQL_C_ULONG, 1, 0 },
  { "tagged", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "vex_name", SQL_VARCHAR, SQL_C_CHAR, 32, 0 },
  { "vex_if_name", SQL_VARCHAR, SQL_C_CHAR, 32, 0 },
  { "vex_link_name", SQL_VARCHAR, SQL_C_CHAR, 32, 0 },
  { "oper_status", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "down_count", SQL_BIGINT, SQL_C_UBIGINT, 1, 0 },
  { "ctrlr_name", SQL_VARCHAR, SQL_C_CHAR, 32, kDcsCtrlrName },
  { "domain_id", SQL_VARCHAR, SQL_C_CHAR, 32, kDcsDomainId },
  { "flags", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_admin_status", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_description", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_portmap", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_logical_port_id", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_vlanid", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_tagged", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_vex_name", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_vex_if_name", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_vex_link_name", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_oper_status", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_rowstatus", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_admin_status", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_description", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_portmap", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_logical_port_id", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_vlanid", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_tagged", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "c_flag", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "u_flag", SQL_SMALLINT, SQL_C_SHORT, 1, 0 }
};
}  // namespace vbridge_interface

namespace vrouter {
const DalColumnSchema vrouter_schema[] =  {
  { "vtn_name", SQL_VARCHAR, SQL_C_CHAR, 32, 0 },
  { "vrouter_name", SQL_VARCHAR, SQL_C_CHAR, 32, 0 },
  { "ctrlr_name", SQL_VARCHAR, SQL_C_CHAR, 32, kDcsCtrlrName },
  { "domain_id", SQL_VARCHAR, SQL_C_CHAR, 32, kDcsDomainId },
  { "vrt_description", SQL_VARCHAR, SQL_C_CHAR, 128, 0 },
  { "dhcprelay_admin_status", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "oper_status", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "down_count", SQL_BIGINT, SQL_C_UBIGINT, 1, 0 },
  { "flags", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_ctrlr_name", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_domain_id", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_vrt_description", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_dhcprelay_admin_status", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_oper_status", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_rowstatus", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_ctrlr_name", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_domain_id", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_vrt_description", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_dhcprelay_admin_status", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "c_flag", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "u_flag", SQL_SMALLINT, SQL_C_SHORT, 1, 0 }
};
}  // namespace vrouter

namespace vrouter_interface {
const DalColumnSchema vrouter_interface_schema[] =  {
  { "vtn_name", SQL_VARCHAR, SQL_C_CHAR, 32, 0 },
  { "vrouter_name", SQL_VARCHAR, SQL_C_CHAR, 32, 0 },
  { "if_name", SQL_VARCHAR, SQL_C_CHAR, 32, 0 },
  { "description", SQL_VARCHAR, SQL_C_CHAR, 128, 0 },
  { "ip_addr", SQL_BINARY, SQL_C_BINARY, 4, 0 },
  { "mask", SQL_SMALLINT, SQL_SMALLINT, 1, 0 },
  { "mac_addr", SQL_BINARY, SQL_C_BINARY, 6, 0 },
  { "admin_status", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "oper_status", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "down_count", SQL_BIGINT, SQL_C_UBIGINT, 1, 0 },
  { "ctrlr_name", SQL_VARCHAR, SQL_C_CHAR, 32, kDcsCtrlrName },
  { "domain_id", SQL_VARCHAR, SQL_C_CHAR, 32, kDcsDomainId },
  { "flags", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_description", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_ip_addr", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_mask", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_mac_addr", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_admin_status", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_oper_status", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_rowstatus", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_description", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_ip_addr", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_mask", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_mac_addr", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_admin_status", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "c_flag", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "u_flag", SQL_SMALLINT, SQL_C_SHORT, 1, 0 }
};
}  // namespace vrouter_interface

namespace vterminal {
const DalColumnSchema vterminal_schema[] =  {
  { "vtn_name", SQL_VARCHAR, SQL_C_CHAR, 32, 0 },
  { "vterminal_name", SQL_VARCHAR, SQL_C_CHAR, 32, 0 },
  { "ctrlr_name", SQL_VARCHAR, SQL_C_CHAR, 32, kDcsCtrlrName },
  { "domain_id", SQL_VARCHAR, SQL_C_CHAR, 32, kDcsDomainId },
  { "vterminal_description", SQL_VARCHAR, SQL_C_CHAR, 128, 0 },
  { "oper_status", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "down_count", SQL_BIGINT, SQL_C_UBIGINT, 1, 0 },
  { "unknown_count", SQL_BIGINT, SQL_C_UBIGINT, 1, 0 },
  { "valid_ctrlr_name", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_domain_id", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_vterminal_description", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_oper_status", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_rowstatus", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_ctrlr_name", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_domain_id", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_vterminal_description", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "flags", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "c_flag", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "u_flag", SQL_SMALLINT, SQL_C_SHORT, 1, 0 }
};
}  // namespace vterminal

namespace vterminal_interface {
const DalColumnSchema vterminal_interface_schema[] =  {
  { "vtn_name", SQL_VARCHAR, SQL_C_CHAR, 32, 0 },
  { "vterminal_name", SQL_VARCHAR, SQL_C_CHAR, 32, 0 },
  { "if_name", SQL_VARCHAR, SQL_C_CHAR, 32, 0 },
  { "admin_status", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "description", SQL_VARCHAR, SQL_C_CHAR, 128, 0 },
  { "logical_port_id", SQL_VARCHAR, SQL_C_CHAR, 320, 0 },
  { "vlanid", SQL_INTEGER, SQL_C_ULONG, 1, 0 },
  { "tagged", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "oper_status", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "down_count", SQL_BIGINT, SQL_C_UBIGINT, 1, 0 },
  { "ctrlr_name", SQL_VARCHAR, SQL_C_CHAR, 32, kDcsCtrlrName },
  { "domain_id", SQL_VARCHAR, SQL_C_CHAR, 32, kDcsDomainId },
  { "flags", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_admin_status", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_description", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_portmap", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_logical_port_id", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_vlanid", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_tagged", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_oper_status", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_rowstatus", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_admin_status", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_description", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_portmap", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_logical_port_id", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_vlanid", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_tagged", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "c_flag", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "u_flag", SQL_SMALLINT, SQL_C_SHORT, 1, 0 }
};
}  // namespace vterminal_interface


namespace vnode_rename {
const DalColumnSchema vnode_rename_schema[] =  {
  { "ctrlr_vtn_name", SQL_VARCHAR, SQL_C_CHAR, 32, 0 },
  { "ctrlr_vnode_name", SQL_VARCHAR, SQL_C_CHAR, 32, 0 },
  { "ctrlr_name", SQL_VARCHAR, SQL_C_CHAR, 32, kDcsCtrlrName },
  { "domain_id", SQL_VARCHAR, SQL_C_CHAR, 32, kDcsDomainId },
  { "vtn_name", SQL_VARCHAR, SQL_C_CHAR, 32, 0 },
  { "unc_vnode_name", SQL_VARCHAR, SQL_C_CHAR, 32, 0 },
  { "c_flag", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "u_flag", SQL_SMALLINT, SQL_C_SHORT, 1, 0 }
};
}  // namespace vnode_rename

namespace vlink {
const DalColumnSchema vlink_schema[] =  {
  { "vtn_name", SQL_VARCHAR, SQL_C_CHAR, 32, 0 },
  { "vlink_name", SQL_VARCHAR, SQL_C_CHAR, 32, 0 },
  { "admin_status", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "vnode1_name", SQL_VARCHAR, SQL_C_CHAR, 32, 0 },
  { "vnode1_ifname", SQL_VARCHAR, SQL_C_CHAR, 32, 0 },
  { "vnode2_name", SQL_VARCHAR, SQL_C_CHAR, 32, 0 },
  { "vnode2_ifname", SQL_VARCHAR, SQL_C_CHAR, 32, 0 },
  { "boundary_name", SQL_VARCHAR, SQL_C_CHAR, 32, 0 },
  { "label_type", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "label", SQL_BIGINT, SQL_C_UBIGINT, 1, 0 },
  { "description", SQL_VARCHAR, SQL_C_CHAR, 128, 0 },
  { "oper_status", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "down_count", SQL_BIGINT, SQL_C_UBIGINT, 1, 0 },
  { "controller1_name", SQL_VARCHAR, SQL_C_CHAR, 32, kDcsCtrlrName },
  { "controller2_name", SQL_VARCHAR, SQL_C_CHAR, 32, kDcsCtrlrName },
  { "domain1_id", SQL_VARCHAR, SQL_C_CHAR, 32, kDcsDomainId },
  { "domain2_id", SQL_VARCHAR, SQL_C_CHAR, 32, kDcsDomainId },
  { "key_flags", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "val_flags", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_admin_status", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_vnode1_name", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_vnode1_ifname", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_vnode2_name", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_vnode2_ifname", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_boundary_name", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_label_type", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_label", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_description", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_oper_status", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_rowstatus", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_admin_status", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_vnode1_name", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_vnode1_ifname", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_vnode2_name", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_vnode2_ifname", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_boundary_name", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_label_type", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_label", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_description", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "c_flag", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "u_flag", SQL_SMALLINT, SQL_C_SHORT, 1, 0 }
};
}  // namespace vlink

namespace vlink_rename {
const DalColumnSchema vlink_rename_schema[] =  {
  { "ctrlr_vtn_name", SQL_VARCHAR, SQL_C_CHAR, 32, 0 },
  { "ctrlr_vlink_name", SQL_VARCHAR, SQL_C_CHAR, 32, 0 },
  { "ctrlr_name", SQL_VARCHAR, SQL_C_CHAR, 32, kDcsCtrlrName },
  { "domain_id", SQL_VARCHAR, SQL_C_CHAR, 32, kDcsDomainId },
  { "vtn_name", SQL_VARCHAR, SQL_C_CHAR, 32, 0 },
  { "unc_vlink_name", SQL_VARCHAR, SQL_C_CHAR, 32, 0 },
  { "c_flag", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "u_flag", SQL_SMALLINT, SQL_C_SHORT, 1, 0 }
};
}  // namespace vlink_rename

namespace vterm_if_policingmap {
const DalColumnSchema vterm_if_policingmap_schema[] = {
  { "vtn_name", SQL_VARCHAR, SQL_C_CHAR, 32, 0 },
  { "vterm_name", SQL_VARCHAR, SQL_C_CHAR, 32, 0 },
  { "vterm_if_name", SQL_VARCHAR, SQL_C_CHAR, 32, 0 },
  { "ctrlr_name", SQL_VARCHAR, SQL_C_CHAR, 32, kDcsCtrlrName },
  { "domain_id", SQL_VARCHAR, SQL_C_CHAR, 32, kDcsDomainId },
  { "policername", SQL_VARCHAR, SQL_C_CHAR, 33, 0 },
  { "flags", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_policername", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_rowstatus", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_policername", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "c_flag", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "u_flag", SQL_SMALLINT, SQL_C_SHORT, 1, 0 }
};  // vterm_if_policingmap_schema
}  // namespace vterm_if_policingmap

// Note: Furthur new primary keys should be added above nwm_name
namespace static_ip_route {
const DalColumnSchema static_ip_route_schema[] =  {
  { "vtn_name", SQL_VARCHAR, SQL_C_CHAR, 32, 0 },
  { "vrouter_name", SQL_VARCHAR, SQL_C_CHAR, 32, 0 },
  { "dst_ip_addr", SQL_BINARY, SQL_C_BINARY, 4, 0 },
  { "mask", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "next_hop_addr", SQL_BINARY, SQL_C_BINARY, 4, 0 },
  { "nwm_name", SQL_VARCHAR, SQL_C_CHAR, 32, 0 },
  { "nwm_name2", SQL_VARCHAR, SQL_C_CHAR, 32, 0 },
  { "metric", SQL_INTEGER, SQL_C_ULONG, 1, 0 },
  { "ctrlr_name", SQL_VARCHAR, SQL_C_CHAR, 32, kDcsCtrlrName },
  { "domain_id", SQL_VARCHAR, SQL_C_CHAR, 32, kDcsDomainId },
  { "flags", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_nwm_name2", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_metric", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_rowstatus", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_nwm_name2", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_metric", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "c_flag", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "u_flag", SQL_SMALLINT, SQL_C_SHORT, 1, 0 }
};
}  // namespace static_ip_route

namespace dhcprelay_server {
const DalColumnSchema dhcprelay_server_schema[] =  {
  { "vtn_name", SQL_VARCHAR, SQL_C_CHAR, 32, 0 },
  { "vrouter_name", SQL_VARCHAR, SQL_C_CHAR, 32, 0 },
  { "server_ip_addr", SQL_BINARY, SQL_C_BINARY, 4, 0 },
  { "ctrlr_name", SQL_VARCHAR, SQL_C_CHAR, 32, kDcsCtrlrName },
  { "domain_id", SQL_VARCHAR, SQL_C_CHAR, 32, kDcsDomainId },
  { "flags", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_rowstatus", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "c_flag", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "u_flag", SQL_SMALLINT, SQL_C_SHORT, 1, 0 }
};
}  // namespace dhcprelay_server

namespace dhcprelay_interface {
const DalColumnSchema dhcprelay_interface_schema[] =  {
  { "vtn_name", SQL_VARCHAR, SQL_C_CHAR, 32, 0 },
  { "vrouter_name", SQL_VARCHAR, SQL_C_CHAR, 32, 0 },
  { "if_name", SQL_VARCHAR, SQL_C_CHAR, 32, 0 },
  { "ctrlr_name", SQL_VARCHAR, SQL_C_CHAR, 32, kDcsCtrlrName },
  { "domain_id", SQL_VARCHAR, SQL_C_CHAR, 32, kDcsDomainId },
  { "flags", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_rowstatus", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "c_flag", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "u_flag", SQL_SMALLINT, SQL_C_SHORT, 1, 0 }
};
}  // namespace dhcprelay_interface

namespace vbridge_networkmonitor_group {
const DalColumnSchema vbridge_networkmonitor_group_schema[] =  {
  { "vtn_name", SQL_VARCHAR, SQL_C_CHAR, 32, 0 },
  { "vbridge_name", SQL_VARCHAR, SQL_C_CHAR, 32, 0 },
  { "nwm_name", SQL_VARCHAR, SQL_C_CHAR, 32, 0 },
  { "admin_status", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "oper_status", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "ctrlr_name", SQL_VARCHAR, SQL_C_CHAR, 32, kDcsCtrlrName },
  { "domain_id", SQL_VARCHAR, SQL_C_CHAR, 32, kDcsDomainId },
  { "flags", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_admin_status", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_oper_status", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_rowstatus", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_admin_status", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "c_flag", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "u_flag", SQL_SMALLINT, SQL_C_SHORT, 1, 0 }
};
}  // namespace vbridge_networkmonitor_group

namespace vbridge_networkmonitor_host {
const DalColumnSchema vbridge_networkmonitor_host_schema[] =  {
  { "vtn_name", SQL_VARCHAR, SQL_C_CHAR, 32, 0 },
  { "vbridge_name", SQL_VARCHAR, SQL_C_CHAR, 32, 0 },
  { "nwm_name", SQL_VARCHAR, SQL_C_CHAR, 32, 0 },
  { "host_address", SQL_BINARY, SQL_C_BINARY, 4, 0 },
  { "health_interval", SQL_INTEGER, SQL_C_ULONG, 1, 0 },
  { "recovery_interval", SQL_INTEGER, SQL_C_ULONG, 1, 0 },
  { "failure_count", SQL_INTEGER, SQL_C_ULONG, 1, 0 },
  { "recovery_count", SQL_INTEGER, SQL_C_ULONG, 1, 0 },
  { "wait_time", SQL_INTEGER, SQL_C_ULONG, 1, 0 },
  { "ctrlr_name", SQL_VARCHAR, SQL_C_CHAR, 32, kDcsCtrlrName },
  { "domain_id", SQL_VARCHAR, SQL_C_CHAR, 32, kDcsDomainId },
  { "flags", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_health_interval", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_recovery_interval", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_failure_count", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_recovery_count", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_wait_time", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_rowstatus", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_health_interval", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_recovery_interval", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_failure_count", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_recovery_count", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_wait_time", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "c_flag", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "u_flag", SQL_SMALLINT, SQL_C_SHORT, 1, 0 }
};
}  // namespace vbridge_networkmonitor_host

namespace vunknown {
const DalColumnSchema vunknown_schema[] =  {
  { "vtn_name", SQL_VARCHAR, SQL_C_CHAR, 32, 0 },
  { "vunknown_name", SQL_VARCHAR, SQL_C_CHAR, 32, 0 },
  { "description", SQL_VARCHAR, SQL_C_CHAR, 128, 0 },
  { "type", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "domain_id", SQL_VARCHAR, SQL_C_CHAR, 32, kDcsDomainId },
  { "flags", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_description", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_type", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_domain_id", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_rowstatus", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_description", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_type", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_domain_id", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "c_flag", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "u_flag", SQL_SMALLINT, SQL_C_SHORT, 1, 0 }
};
}  // namespace vunknown

namespace vunknown_interface {
const DalColumnSchema vunknown_interface_schema[] =  {
  { "vtn_name", SQL_VARCHAR, SQL_C_CHAR, 32, 0 },
  { "vunknown_name", SQL_VARCHAR, SQL_C_CHAR, 32, 0 },
  { "if_name", SQL_VARCHAR, SQL_C_CHAR, 32, 0 },
  { "description", SQL_VARCHAR, SQL_C_CHAR, 128, 0 },
  { "admin_status", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "domain_id", SQL_VARCHAR, SQL_C_CHAR, 32, kDcsDomainId },
  { "flags", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_description", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_admin_status", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_rowstatus", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_description", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_admin_status", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "c_flag", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "u_flag", SQL_SMALLINT, SQL_C_SHORT, 1, 0 }
};
}  // namespace vunknown_interface

namespace vtep {
const DalColumnSchema vtep_schema[] =  {
  { "vtn_name", SQL_VARCHAR, SQL_C_CHAR, 32, 0 },
  { "vtep_name", SQL_VARCHAR, SQL_C_CHAR, 32, 0 },
  { "description", SQL_VARCHAR, SQL_C_CHAR, 128, 0 },
  { "ctrlr_name", SQL_VARCHAR, SQL_C_CHAR, 32, kDcsCtrlrName },
  { "domain_id", SQL_VARCHAR, SQL_C_CHAR, 32, kDcsDomainId },
  { "oper_status", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "down_count", SQL_BIGINT, SQL_C_UBIGINT, 1, 0 },
  { "flags", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_description", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_ctrlr_name", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_domain_id", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_oper_status", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_rowstatus", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_description", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_ctrlr_name", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_domain_id", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "c_flag", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "u_flag", SQL_SMALLINT, SQL_C_SHORT, 1, 0 }
};
}  // namespace vtep

namespace vtep_interface {
const DalColumnSchema vtep_interface_schema[] =  {
  { "vtn_name", SQL_VARCHAR, SQL_C_CHAR, 32, 0 },
  { "vtep_name", SQL_VARCHAR, SQL_C_CHAR, 32, 0 },
  { "if_name", SQL_VARCHAR, SQL_C_CHAR, 32, 0 },
  { "description", SQL_VARCHAR, SQL_C_CHAR, 128, 0 },
  { "admin_status", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "logical_port_id", SQL_VARCHAR, SQL_C_CHAR, 320, 0 },
  { "vlanid", SQL_INTEGER, SQL_C_ULONG, 1, 0 },
  { "tagged", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "oper_status", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "down_count", SQL_BIGINT, SQL_C_UBIGINT, 1, 0 },
  { "ctrlr_name", SQL_VARCHAR, SQL_C_CHAR, 32, kDcsCtrlrName },
  { "domain_id", SQL_VARCHAR, SQL_C_CHAR, 32, kDcsDomainId },
  { "flags", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_description", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_admin_status", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_portmap", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_logical_port_id", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_vlanid", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_tagged", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_oper_status", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_rowstatus", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_description", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_admin_status", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_portmap", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_logical_port_id", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_vlanid", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_tagged", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "c_flag", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "u_flag", SQL_SMALLINT, SQL_C_SHORT, 1, 0 }
};
}  // namespace vtep_interface

namespace vtep_group {
const DalColumnSchema vtep_group_schema[] =  {
  { "vtn_name", SQL_VARCHAR, SQL_C_CHAR, 32, 0 },
  { "vtepgrp_name", SQL_VARCHAR, SQL_C_CHAR, 32, 0 },
  { "ctrlr_name", SQL_VARCHAR, SQL_C_CHAR, 32, kDcsCtrlrName },
  { "description", SQL_VARCHAR, SQL_C_CHAR, 128, 0 },
  { "flags", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_ctrlr_name", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_description", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_rowstatus", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_ctrlr_name", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_description", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "c_flag", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "u_flag", SQL_SMALLINT, SQL_C_SHORT, 1, 0 }
};
}  // namespace vtep_group

namespace vtep_groupmember {
const DalColumnSchema vtep_groupmember_schema[] =  {
  { "vtn_name", SQL_VARCHAR, SQL_C_CHAR, 32, 0 },
  { "vtepgrp_name", SQL_VARCHAR, SQL_C_CHAR, 32, 0 },
  { "vtepgrp_member_name", SQL_VARCHAR, SQL_C_CHAR, 32, 0 },
  { "ctrlr_name", SQL_VARCHAR, SQL_C_CHAR, 32, kDcsCtrlrName },
  { "domain_id", SQL_VARCHAR, SQL_C_CHAR, 32, kDcsDomainId },
  { "flags", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_rowstatus", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "c_flag", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "u_flag", SQL_SMALLINT, SQL_C_SHORT, 1, 0 }
};
}  // namespace vtep_groupmember

namespace vtunnel {
const DalColumnSchema vtunnel_schema[] =  {
  { "vtn_name", SQL_VARCHAR, SQL_C_CHAR, 32, 0 },
  { "vtunnel_name", SQL_VARCHAR, SQL_C_CHAR, 32, 0 },
  { "description", SQL_VARCHAR, SQL_C_CHAR, 128, 0 },
  { "ctrlr_name", SQL_VARCHAR, SQL_C_CHAR, 32, kDcsCtrlrName },
  { "domain_id", SQL_VARCHAR, SQL_C_CHAR, 32, kDcsDomainId },
  { "underlay_vtn_name", SQL_VARCHAR, SQL_C_CHAR, 32, 0 },
  { "vtepgrp_name ", SQL_VARCHAR, SQL_C_CHAR, 32, 0 },
  { "label", SQL_BIGINT, SQL_C_UBIGINT, 1, 0 },
  { "oper_status", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "down_count", SQL_BIGINT, SQL_C_UBIGINT, 1, 0 },
  { "flags", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_description", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_ctrlr_name", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_domain_id", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_underlay_vtn_name", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_vtepgrp_name", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_label", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_oper_status", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_rowstatus", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_description", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_ctrlr_name", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_domain_id", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_underlay_vtn_name", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_vtepgrp_name", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_label", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "c_flag", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "u_flag", SQL_SMALLINT, SQL_C_SHORT, 1, 0 }
};
}  // namespace vtunnel

namespace vtunnel_interface {
const DalColumnSchema vtunnel_interface_schema[] =  {
  { "vtn_name", SQL_VARCHAR, SQL_C_CHAR, 32, 0 },
  { "vtunnel_name", SQL_VARCHAR, SQL_C_CHAR, 32, 0 },
  { "if_name", SQL_VARCHAR, SQL_C_CHAR, 32, 0 },
  { "description", SQL_VARCHAR, SQL_C_CHAR, 128, 0 },
  { "admin_status", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "logical_port_id", SQL_VARCHAR, SQL_C_CHAR, 320, 0 },
  { "vlanid", SQL_INTEGER, SQL_C_ULONG, 1, 0 },
  { "tagged", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "oper_status",  SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "down_count", SQL_BIGINT, SQL_C_UBIGINT, 1, 0 },
  { "ctrlr_name", SQL_VARCHAR, SQL_C_CHAR, 32, kDcsCtrlrName },
  { "domain_id", SQL_VARCHAR, SQL_C_CHAR, 32, kDcsDomainId },
  { "flags",  SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_description",  SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_admin_status",  SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_portmap", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_logical_port_id", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_vlanid", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_tagged", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_oper_status",  SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_rowstatus",  SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_description", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_admin_status",  SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_portmap", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_logical_port_id", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_vlanid", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_tagged", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "c_flag", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "u_flag", SQL_SMALLINT, SQL_C_SHORT, 1, 0 }
};
}  // namespace vtunnel_interface

namespace convert_vbridge {
const DalColumnSchema convert_vbridge_schema[] =  {
  { "vtn_name", SQL_VARCHAR, SQL_C_CHAR, 32, 0 },
  { "unified_vbridge_name", SQL_VARCHAR, SQL_C_CHAR, 32, 0 },
  { "converted_vbridge_name", SQL_VARCHAR, SQL_C_CHAR, 40, 0 },
  { "ctrlr_name", SQL_VARCHAR, SQL_C_CHAR, 32, kDcsCtrlrName },
  { "domain_id", SQL_VARCHAR, SQL_C_CHAR, 32, kDcsDomainId },
  { "label", SQL_BIGINT, SQL_C_UBIGINT, 1, 0 },
  { "oper_status", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "down_count", SQL_BIGINT, SQL_C_UBIGINT, 1, 0 },
  { "unknown_count", SQL_BIGINT, SQL_C_UBIGINT, 1, 0 },
  { "valid_label", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_oper_status", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_rowstatus", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_label", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "flags", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "c_flag", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "u_flag", SQL_SMALLINT, SQL_C_SHORT, 1, 0 }
};
}  // namespace convert_vbridge

namespace convert_vbridge_interface {
const DalColumnSchema convert_vbridge_interface_schema[] =  {
  { "vtn_name", SQL_VARCHAR, SQL_C_CHAR, 32, 0 },
  { "unified_vbridge_name", SQL_VARCHAR, SQL_C_CHAR, 32, 0 },
  { "converted_vbridge_name", SQL_VARCHAR, SQL_C_CHAR, 40, 0 },
  { "converted_vbridge_if_name", SQL_VARCHAR, SQL_C_CHAR, 32, 0 },
  { "ctrlr_name", SQL_VARCHAR, SQL_C_CHAR, 32, kDcsCtrlrName },
  { "domain_id", SQL_VARCHAR, SQL_C_CHAR, 32, kDcsDomainId },
  { "oper_status", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "down_count", SQL_BIGINT, SQL_C_UBIGINT, 1, 0 },
  { "valid_oper_status", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_rowstatus", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "flags", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "c_flag", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "u_flag", SQL_SMALLINT, SQL_C_SHORT, 1, 0 }
};
}  // namespace convert_vbridge interface

namespace convert_vlink {
const DalColumnSchema convert_vlink_schema[] =  {
  { "vtn_name", SQL_VARCHAR, SQL_C_CHAR, 32, 0 },
  { "unified_vbridge_name", SQL_VARCHAR, SQL_C_CHAR, 32, 0 },
  { "converted_vlink_name", SQL_VARCHAR, SQL_C_CHAR, 32, 0 },
  { "vnode1_name", SQL_VARCHAR, SQL_C_CHAR, 40, 0 },
  { "vnode1_ifname", SQL_VARCHAR, SQL_C_CHAR, 32, 0 },
  { "vnode2_name", SQL_VARCHAR, SQL_C_CHAR, 40, 0 },
  { "vnode2_ifname", SQL_VARCHAR, SQL_C_CHAR, 32, 0 },
  { "boundary_name", SQL_VARCHAR, SQL_C_CHAR, 32, 0 },
  { "label_type", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "label", SQL_BIGINT, SQL_C_UBIGINT, 1, 0 },
  { "controller1_name", SQL_VARCHAR, SQL_C_CHAR, 32, kDcsCtrlrName },
  { "controller2_name", SQL_VARCHAR, SQL_C_CHAR, 32, kDcsCtrlrName },
  { "domain1_id", SQL_VARCHAR, SQL_C_CHAR, 32, kDcsDomainId },
  { "domain2_id", SQL_VARCHAR, SQL_C_CHAR, 32, kDcsDomainId },
  { "oper_status", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "down_count", SQL_BIGINT, SQL_C_UBIGINT, 1, 0 },
  { "key_flags", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "val_flags", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_vnode1_name", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_vnode1_ifname", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_vnode2_name", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_vnode2_ifname", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_boundary_name", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_label_type", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_label", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_oper_status", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_rowstatus", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_vnode1_name", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_vnode1_ifname", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_vnode2_name", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_vnode2_ifname", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_boundary_name", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_label_type", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_label", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "c_flag", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "u_flag", SQL_SMALLINT, SQL_C_SHORT, 1, 0 }
};
}  // namespace vlink

namespace vbridge_portmap {
const DalColumnSchema vbridge_portmap_schema[] = {
  { "vtn_name", SQL_VARCHAR, SQL_C_CHAR, 32, 0 },
  { "vbridge_name", SQL_VARCHAR, SQL_C_CHAR, 32, 0 },
  { "portmap_id", SQL_VARCHAR, SQL_C_CHAR, 32, 0 },
  { "ctrlr_name", SQL_VARCHAR, SQL_C_CHAR, 32, kDcsCtrlrName },
  { "domain_id", SQL_VARCHAR, SQL_C_CHAR, 32, kDcsDomainId },
  { "logical_port_id", SQL_VARCHAR, SQL_C_CHAR, 320, 0},
  { "label_type", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "label", SQL_INTEGER, SQL_C_ULONG, 1, 0 },
  { "bdry_ref_count", SQL_BIGINT, SQL_C_UBIGINT, 1, 0 },
  { "oper_status", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "down_count", SQL_BIGINT, SQL_C_UBIGINT, 1, 0 },
  { "flags", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_ctrlr_name", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_domain_id", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_logical_port_id", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_label_type", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_label", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_bdry_ref_count", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_oper_status", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_rowstatus", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_ctrlr_name", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_domain_id", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_logical_port_id", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_label_type", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_label", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "c_flag", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "u_flag", SQL_SMALLINT, SQL_C_SHORT, 1, 0 }
};
}
namespace unified_nw {
const DalColumnSchema unified_nw_schema[] =  {
  { "unified_nw_name", SQL_VARCHAR, SQL_C_CHAR, 32, 0 },
  { "routing_type", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "is_default", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_routing_type", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_is_default", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_rowstatus", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_routing_type", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_is_default", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "c_flag", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "u_flag", SQL_SMALLINT, SQL_C_SHORT, 1, 0 }
};
}

namespace unw_label {
const DalColumnSchema unw_label_schema[] =  {
  { "unified_nw_name", SQL_VARCHAR, SQL_C_CHAR, 32, 0 },
  { "unw_label_name", SQL_VARCHAR, SQL_C_CHAR, 32, 0 },
  { "max_count", SQL_BIGINT, SQL_C_UBIGINT, 1, 0 },
  { "raising_threshold", SQL_BIGINT, SQL_C_UBIGINT, 1, 0 },
  { "falling_threshold", SQL_BIGINT, SQL_C_UBIGINT, 1, 0 },
  { "valid_max_count", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_raising_threshold", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_falling_threshold", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_rowstatus", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_max_count", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_raising_threshold", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_falling_threshold", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "c_flag", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "u_flag", SQL_SMALLINT, SQL_C_SHORT, 1, 0 }
};
}

namespace unw_label_range {
const DalColumnSchema unw_label_range_schema[] =  {
  { "unified_nw_name", SQL_VARCHAR, SQL_C_CHAR, 32, 0 },
  { "unw_label_name", SQL_VARCHAR, SQL_C_CHAR, 32, 0 },
  { "min_range", SQL_BIGINT, SQL_C_UBIGINT, 1, 0 },
  { "max_range", SQL_BIGINT, SQL_C_UBIGINT, 1, 0 },
  { "cs_rowstatus", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "c_flag", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "u_flag", SQL_SMALLINT, SQL_C_SHORT, 1, 0 }
};
}

namespace unw_spine_domain {
const DalColumnSchema unw_spine_domain_schema[] =  {
  { "unified_nw_name", SQL_VARCHAR, SQL_C_CHAR, 32, 0 },
  { "unw_spine_domain_name", SQL_VARCHAR, SQL_C_CHAR, 32, 0 },
  { "ctrlr_name", SQL_VARCHAR, SQL_C_CHAR, 32, kDcsCtrlrName },
  { "domain_id", SQL_VARCHAR, SQL_C_CHAR, 32, kDcsDomainId },
  { "unw_label_name", SQL_VARCHAR, SQL_C_CHAR, 32, 0 },
  { "used_label_count", SQL_BIGINT,  SQL_C_UBIGINT, 1, 0 },
  { "alarm_raised", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_ctrlr_name", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_domain_id", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_unw_label_name", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_used_label_count", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_alarm_raised", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_rowstatus", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_ctrlr_name", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_domain_id", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_unw_label_name", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "c_flag", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "u_flag", SQL_SMALLINT, SQL_C_SHORT, 1, 0 }
};
}

namespace vtn_unified {
const DalColumnSchema vtn_unified_schema[] = {
  { "vtn_name", SQL_VARCHAR, SQL_C_CHAR, 32, 0 },
  { "unified_nw_name", SQL_VARCHAR, SQL_C_CHAR, 32, 0 },
  { "unw_spine_domain_name", SQL_VARCHAR, SQL_C_CHAR, 32, 0 },
  { "valid_unw_spine_domain_name", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_rowstatus", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_unw_spine_domain_name", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "flags", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "c_flag", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "u_flag", SQL_SMALLINT, SQL_C_SHORT, 1, 0 }
};
}

namespace vbid_label {
const DalColumnSchema vbid_label_schema[] = {
  { "vtn_name", SQL_VARCHAR, SQL_C_CHAR, 32, 0 },
  { "label_row", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "label_id", SQL_BIGINT, SQL_C_UBIGINT, 1, 0 },
  { "flags", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "c_flag", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "u_flag", SQL_SMALLINT, SQL_C_SHORT, 1, 0 }
};
}  // namespace vbid_label

namespace gvtnid_label {
const DalColumnSchema gvtnid_label_schema[] = {
  { "ctrlr_name", SQL_VARCHAR, SQL_C_CHAR, 32, 0 },
  { "dom_id", SQL_VARCHAR, SQL_C_CHAR, 32, 0 },
  { "label_row", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "label_id", SQL_BIGINT, SQL_C_UBIGINT, 1, 0 },
  { "c_flag", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "u_flag", SQL_SMALLINT, SQL_C_SHORT, 1, 0 }
};
}  // namespace gvtnid_label

namespace convert_vtunnel {
const DalColumnSchema convert_vtunnel_schema[] =  {
  { "vtn_name", SQL_VARCHAR, SQL_C_CHAR, 32, 0 },
  { "vtunnel_name", SQL_VARCHAR, SQL_C_CHAR, 40, 0 },
  { "ctrlr_name", SQL_VARCHAR, SQL_C_CHAR, 32, kDcsCtrlrName },
  { "domain_id", SQL_VARCHAR, SQL_C_CHAR, 32, kDcsDomainId },
  { "ref_count", SQL_BIGINT, SQL_C_UBIGINT, 1, 0 },
  { "label", SQL_BIGINT, SQL_C_UBIGINT, 1, 0 },
  { "oper_status", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "down_count", SQL_BIGINT, SQL_C_UBIGINT, 1, 0 },
  { "flags", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_ref_count", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_label", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_oper_status", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_rowstatus", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_ref_count", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_label", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "c_flag", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "u_flag", SQL_SMALLINT, SQL_C_SHORT, 1, 0 }
};
}  // namespace convert vtunnel

namespace convert_vtunnel_interface {
const DalColumnSchema convert_vtunnel_interface_schema[] =  {
  { "vtn_name", SQL_VARCHAR, SQL_C_CHAR, 32, 0 },
  { "vtunnel_name", SQL_VARCHAR, SQL_C_CHAR, 40, 0 },
  { "if_name", SQL_VARCHAR, SQL_C_CHAR, 32, 0 },
  { "rem_ctrlr_name", SQL_VARCHAR, SQL_C_CHAR, 32, kDcsCtrlrName },
  { "rem_domain_id", SQL_VARCHAR, SQL_C_CHAR, 32, kDcsDomainId },
  { "un_vbr_name", SQL_VARCHAR, SQL_C_CHAR, 32, 0 },
  { "oper_status", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "down_count", SQL_BIGINT, SQL_C_UBIGINT, 1, 0 },
  { "flags", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_rem_ctrlr_name", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_rem_domain_id", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_un_vbr_name", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_oper_status", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_rowstatus", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_rem_ctrlr_name", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_rem_domain_id", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_un_vbr_name", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "c_flag", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "u_flag", SQL_SMALLINT, SQL_C_SHORT, 1, 0 }
};
}  // namespace convert_vtunnel_interface

namespace vtn_gateway_port {
const DalColumnSchema vtn_gateway_port_schema[] =  {
  { "vtn_name", SQL_VARCHAR, SQL_C_CHAR, 32, 0 },
  { "ctrlr_name", SQL_VARCHAR, SQL_C_CHAR, 32, kDcsCtrlrName },
  { "domain_id", SQL_VARCHAR, SQL_C_CHAR, 32, kDcsDomainId },
  { "logical_port_id", SQL_VARCHAR, SQL_C_CHAR, 320, 0 },
  { "label", SQL_BIGINT, SQL_C_UBIGINT, 1, 0 },
  { "ref_count", SQL_BIGINT, SQL_C_UBIGINT, 1, 0 },
  { "oper_status", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "down_count", SQL_BIGINT, SQL_C_UBIGINT, 1, 0 },
  { "flags", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_logical_port_id", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_label", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_ref_count", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_oper_status", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_rowstatus", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_logical_port_id", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_label", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_ref_count", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "c_flag", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "u_flag", SQL_SMALLINT, SQL_C_SHORT, 1, 0 }
};
}  // namespace vtn gateway port

// L2/L3 MoMgrs Schema End

// POM MoMgrs Schema Start

namespace flowlist {
const DalColumnSchema flowlist_schema[] = {
  { "flowlist_name", SQL_VARCHAR, SQL_C_CHAR, 33, 0 },
  { "ip_type", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "flags", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_ip_type", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_rowstatus", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_ip_type", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "c_flag", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "u_flag", SQL_SMALLINT, SQL_C_SHORT, 1, 0 }
};  // flowlist_schema
}  // namespace flowlist

namespace flowlist_ctrlr {
const DalColumnSchema flowlist_ctrlr_schema[] = {
  { "flowlist_name", SQL_VARCHAR, SQL_C_CHAR, 33, 0 },
  { "ctrlr_name", SQL_VARCHAR, SQL_C_CHAR, 32, kDcsCtrlrName },
  { "ref_count", SQL_BIGINT, SQL_C_UBIGINT, 1, 0 },
  { "flags", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_ip_type", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_rowstatus", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_ip_type", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "c_flag", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "u_flag", SQL_SMALLINT, SQL_C_SHORT, 1, 0 }
};  // flowlist_ctrlr_schema
}  // namespace flowlist_ctrlr

namespace flowlist_rename {
const DalColumnSchema flowlist_rename_schema[] = {
  { "ctrlr_flowlist_name", SQL_VARCHAR, SQL_C_CHAR, 33, 0 },
  { "ctrlr_name", SQL_VARCHAR, SQL_C_CHAR, 32, kDcsCtrlrName },
  { "unc_flowlist_name", SQL_VARCHAR, SQL_C_CHAR, 33, 0 },
  { "c_flag", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "u_flag", SQL_SMALLINT, SQL_C_SHORT, 1, 0 }
};  // flowlist_rename_schema
}  // namespace flowlist_rename

namespace flowlist_entry {
const DalColumnSchema flowlist_entry_schema[] = {
  { "flowlist_name", SQL_VARCHAR, SQL_C_CHAR, 33, 0 },
  { "sequence_num", SQL_INTEGER, SQL_C_ULONG, 1, 0 },
  { "mac_dst", SQL_BINARY, SQL_C_BINARY, 6, 0 },
  { "mac_src", SQL_BINARY, SQL_C_BINARY, 6, 0 },
  { "mac_eth_type", SQL_INTEGER, SQL_C_ULONG, 1, 0 },
  { "dst_ip", SQL_BIGINT, SQL_C_UBIGINT, 1, 0 },
  { "dst_ip_prefix", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "src_ip", SQL_BIGINT, SQL_C_UBIGINT, 1, 0 },
  { "src_ip_prefix", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "vlan_priority", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "dst_ipv6", SQL_BINARY, SQL_C_BINARY, 16, 0 },
  { "dst_ipv6_prefix", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "src_ipv6", SQL_BINARY, SQL_C_BINARY, 16, 0 },
  { "src_ipv6_prefix", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "ip_protocol", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "ip_dscp", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "l4_dst_port", SQL_INTEGER, SQL_C_ULONG, 1, 0 },
  { "l4_dst_port_endpt", SQL_INTEGER, SQL_C_ULONG, 1, 0 },
  { "l4_src_port", SQL_INTEGER, SQL_C_ULONG, 1, 0 },
  { "l4_src_port_endpt", SQL_INTEGER, SQL_C_ULONG, 1, 0 },
  { "icmp_type", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "icmp_code", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "icmpv6_type", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "icmpv6_code", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "flags", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_mac_dst", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_mac_src", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_mac_eth_type", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_dst_ip", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_dst_ip_prefix", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_src_ip", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_src_ip_prefix", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_vlan_priority", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_dst_ipv6", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_dst_ipv6_prefix", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_src_ipv6", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_src_ipv6_prefix", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_ip_protocol", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_ip_dscp", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_l4_dst_port", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_l4_dst_port_endpt", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_l4_src_port", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_l4_src_port_endpt", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_icmp_type", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_icmp_code", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_icmpv6_type", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_icmpv6_code", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_rowstatus", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_mac_dst", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_mac_src", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_mac_eth_type", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_dst_ip", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_dst_ip_prefix", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_src_ip", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_src_ip_prefix", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_vlan_priority", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_dst_ipv6", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_dst_ipv6_prefix", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_src_ipv6", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_src_ipv6_prefix", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_ip_protocol", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_ip_dscp", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_l4_dst_port", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_l4_dst_port_endpt", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_l4_src_port", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_l4_src_port_endpt", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_icmp_type", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_icmp_code", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_icmpv6_type", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_icmpv6_code", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "c_flag", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "u_flag", SQL_SMALLINT, SQL_C_SHORT, 1, 0 }
};  // flowlist_entry_schema
}  // namespace flowlist_entry


namespace flowlist_entry_ctrlr {
const DalColumnSchema flowlist_entry_ctrlr_schema[] = {
  { "flowlist_name", SQL_VARCHAR, SQL_C_CHAR, 33, 0 },
  { "sequence_num", SQL_INTEGER, SQL_C_ULONG, 1, 0 },
  { "ctrlr_name", SQL_VARCHAR, SQL_C_CHAR, 32, kDcsCtrlrName },
  { "flags", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_mac_dst", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_mac_src", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_mac_eth_type", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_dst_ip", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_dst_ip_prefix", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_src_ip", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_src_ip_prefix", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_vlan_priority", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_dst_ipv6", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_dst_ipv6_prefix", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_src_ipv6", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_src_ipv6_prefix", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_ip_protocol", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_ip_dscp", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_l4_dst_port", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_l4_dst_port_endpt", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_l4_src_port", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_l4_src_port_endpt", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_icmp_type", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_icmp_code", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_icmpv6_type", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_icmpv6_code", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_rowstatus", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_mac_dst", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_mac_src", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_mac_eth_type", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_dst_ip", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_dst_ip_prefix", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_src_ip", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_src_ip_prefix", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_vlan_priority", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_dst_ipv6", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_dst_ipv6_prefix", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_src_ipv6", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_src_ipv6_prefix", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_ip_protocol", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_ip_dscp", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_l4_dst_port", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_l4_dst_port_endpt", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_l4_src_port", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_l4_src_port_endpt", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_icmp_type", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_icmp_code", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_icmpv6_type", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_icmpv6_code", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "c_flag", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "u_flag", SQL_SMALLINT, SQL_C_SHORT, 1, 0 }
};  // flowlist_entry_ctrlr_schema
}  // namespace flowlist_entry_ctrlr

namespace policingprofile {
const DalColumnSchema policingprofile_schema[] = {
  { "policingprofile_name", SQL_VARCHAR, SQL_C_CHAR, 33, 0 },
  { "flags", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_rowstatus", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "c_flag", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "u_flag", SQL_SMALLINT, SQL_C_SHORT, 1, 0 }
};  // policingprofile_schema
}  // namespace policingprofile

namespace policingprofile_ctrlr {
const DalColumnSchema policingprofile_ctrlr_schema[] = {
  { "policingprofile_name", SQL_VARCHAR, SQL_C_CHAR, 33, 0 },
  { "ctrlr_name", SQL_VARCHAR, SQL_C_CHAR, 32, kDcsCtrlrName },
  { "ref_count", SQL_BIGINT, SQL_C_UBIGINT, 1, 0 },
  { "flags", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_rowstatus", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "c_flag", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "u_flag", SQL_SMALLINT, SQL_C_SHORT, 1, 0 }
};  // policingprofile_ctrlr_schema
}  // namespace policingprofile_ctrlr

namespace policingprofile_rename {
const DalColumnSchema policingprofile_rename_schema[] = {
  { "ctrlr_policingprofile_name", SQL_VARCHAR, SQL_C_CHAR, 33, 0 },
  { "ctrlr_name", SQL_VARCHAR, SQL_C_CHAR, 32, kDcsCtrlrName },
  { "unc_policingprofile_name", SQL_VARCHAR, SQL_C_CHAR, 33, 0 },
  { "c_flag", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "u_flag", SQL_SMALLINT, SQL_C_SHORT, 1, 0 }
};  // policingprofile_rename_schema
}  // namespace policingprofile_rename

namespace policingprofile_entry {
const DalColumnSchema policingprofile_entry_schema[] = {
  { "policingprofile_name", SQL_VARCHAR, SQL_C_CHAR, 33, 0 },
  { "sequence_num", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "flowlist", SQL_VARCHAR, SQL_C_CHAR, 33, 0 },
  { "rate", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cir", SQL_BIGINT, SQL_C_UBIGINT, 1, 0 },
  { "cbs", SQL_BIGINT, SQL_C_UBIGINT, 1, 0 },
  { "pir", SQL_BIGINT, SQL_C_UBIGINT, 1, 0 },
  { "pbs", SQL_BIGINT, SQL_C_UBIGINT, 1, 0 },
  { "green_action", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "green_priority", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "green_dscp", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "green_drop", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "yellow_action", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "yellow_priority", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "yellow_dscp", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "yellow_drop", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "red_action", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "red_priority", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "red_dscp", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "red_drop", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "flags", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_flowlist", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_rate", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_cir", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_cbs", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_pir", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_pbs", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_green_action", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_green_priority", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_green_dscp", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_green_drop", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_yellow_action", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_yellow_priority", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_yellow_dscp", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_yellow_drop", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_red_action", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_red_priority", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_red_dscp", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_red_drop", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_rowstatus", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_flowlist", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_rate", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_cir", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_cbs", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_pir", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_pbs", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_green_action", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_green_priority", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_green_dscp", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_green_drop", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_yellow_action", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_yellow_priority", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_yellow_dscp", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_yellow_drop", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_red_action", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_red_priority", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_red_dscp", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_red_drop", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "c_flag", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "u_flag", SQL_SMALLINT, SQL_C_SHORT, 1, 0 }
};  // polcingprofile_entry_schema
}  // namespace policingprofile_entry

namespace policingprofile_entry_ctrlr {
const DalColumnSchema policingprofile_entry_ctrlr_schema[] = {
  { "policingprofile_name", SQL_VARCHAR, SQL_C_CHAR, 33, 0 },
  { "sequence_num", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "ctrlr_name", SQL_VARCHAR, SQL_C_CHAR, 32, kDcsCtrlrName },
  { "flags", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_flowlist", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_rate", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_cir", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_cbs", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_pir", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_pbs", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_green_action", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_green_priority", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_green_dscp", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_green_drop", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_yellow_action", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_yellow_priority", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_yellow_dscp", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_yellow_drop", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_red_action", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_red_priority", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_red_dscp", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_red_drop", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_rowstatus", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_flowlist", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_rate", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_cir", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_cbs", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_pir", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_pbs", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_green_action", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_green_priority", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_green_dscp", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_green_drop", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_yellow_action", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_yellow_priority", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_yellow_dscp", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_yellow_drop", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_red_action", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_red_priority", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_red_dscp", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_red_drop", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "c_flag", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "u_flag", SQL_SMALLINT, SQL_C_SHORT, 1, 0 }
};  // polcingprofile_entry_ctrlr_schema
}  // namespace policingprofile_entry_ctrlr

namespace vtn_flowfilter {
const DalColumnSchema vtn_flowfilter_schema[] = {
  { "vtn_name", SQL_VARCHAR, SQL_C_CHAR, 32, 0 },
  { "direction", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "flags", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_rowstatus", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "c_flag", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "u_flag", SQL_SMALLINT, SQL_C_SHORT, 1, 0 }
};  // vtn_flowfilter_schema
}  // namespace vtn_flowfilter

namespace vtn_flowfilter_ctrlr {
const DalColumnSchema vtn_flowfilter_ctrlr_schema[] = {
  { "vtn_name", SQL_VARCHAR, SQL_C_CHAR, 32, 0 },
  { "direction", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "ctrlr_name", SQL_VARCHAR, SQL_C_CHAR, 32, kDcsCtrlrName },
  { "domain_id", SQL_VARCHAR, SQL_C_CHAR, 32, kDcsDomainId },
  { "flags", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_rowstatus", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "c_flag", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "u_flag", SQL_SMALLINT, SQL_C_SHORT, 1, 0 }
};  // vtn_flowfilter_ctrlr_schema
}  // namespace vtn_flowfilter_ctrlr

namespace vtn_flowfilter_entry {
const DalColumnSchema vtn_flowfilter_entry_schema[] = {
  { "vtn_name", SQL_VARCHAR, SQL_C_CHAR, 32, 0 },
  { "direction", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "sequence_num", SQL_INTEGER, SQL_C_ULONG, 1, 0 },
  { "flowlist_name", SQL_VARCHAR, SQL_C_CHAR, 33, 0 },
  { "action", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "nwm_name", SQL_VARCHAR, SQL_C_CHAR, 32, 0 },
  { "dscp", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "priority", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "flags", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_flowlist_name", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_action", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_nwn_name", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_dscp", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_priority", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_rowstatus", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_flowlist_name", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_action", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_nwn_name", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_dscp", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_priority", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "c_flag", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "u_flag", SQL_SMALLINT, SQL_C_SHORT, 1, 0 }
};  // vtn_flowfilter_entry_schema
}  // namespace vtn_flowfilter_entry

namespace vtn_flowfilter_entry_ctrlr {
const DalColumnSchema vtn_flowfilter_entry_ctrlr_schema[] = {
  { "vtn_name", SQL_VARCHAR, SQL_C_CHAR, 32, 0 },
  { "direction", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "sequence_num", SQL_INTEGER, SQL_C_ULONG, 1, 0 },
  { "ctrlr_name", SQL_VARCHAR, SQL_C_CHAR, 32, kDcsCtrlrName },
  { "domain_id", SQL_VARCHAR, SQL_C_CHAR, 32, kDcsDomainId },
  { "flags", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_flowlist_name", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_action", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_nwn_name", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_dscp", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_priority", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_rowstatus", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_flowlist_name", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_action", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_nwn_name", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_dscp", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_priority", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "c_flag", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "u_flag", SQL_SMALLINT, SQL_C_SHORT, 1, 0 }
};  // vtn_flowfilter_entry_ctrlr_schema
}  // namespace vtn_flowfilter_entry_ctrlr

namespace vbr_flowfilter {
const DalColumnSchema vbr_flowfilter_schema[] = {
  { "vtn_name", SQL_VARCHAR, SQL_C_CHAR, 32, 0 },
  { "vbr_name", SQL_VARCHAR, SQL_C_CHAR, 32, 0 },
  { "direction", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "ctrlr_name", SQL_VARCHAR, SQL_C_CHAR, 32, kDcsCtrlrName },
  { "domain_id", SQL_VARCHAR, SQL_C_CHAR, 32, kDcsDomainId },
  { "flags", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_rowstatus", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "c_flag", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "u_flag", SQL_SMALLINT, SQL_C_SHORT, 1, 0 }
};  // vbr_flowfilter_schema
}  // namespace vbr_flowfilter

namespace vbr_flowfilter_entry {
const DalColumnSchema vbr_flowfilter_entry_schema[] = {
  { "vtn_name", SQL_VARCHAR, SQL_C_CHAR, 32, 0 },
  { "vbr_name", SQL_VARCHAR, SQL_C_CHAR, 32, 0 },
  { "direction", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "sequence_num", SQL_INTEGER, SQL_C_ULONG, 1, 0 },
  { "ctrlr_name", SQL_VARCHAR, SQL_C_CHAR, 32, kDcsCtrlrName },
  { "domain_id", SQL_VARCHAR, SQL_C_CHAR, 32, kDcsDomainId },
  { "flowlist_name", SQL_VARCHAR, SQL_C_CHAR, 33, 0 },
  { "action", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "redirect_node", SQL_VARCHAR, SQL_C_CHAR, 32, 0 },
  { "redirect_port", SQL_VARCHAR, SQL_C_CHAR, 32, 0 },
  { "redirect_direction", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "modify_dst_mac", SQL_BINARY, SQL_C_BINARY, 6, 0 },
  { "modify_src_mac", SQL_BINARY, SQL_C_BINARY, 6, 0 },
  { "nwm_name", SQL_VARCHAR, SQL_C_CHAR, 32, 0 },
  { "dscp", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "priority", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "flags", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_flowlist_name", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_action", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_redirect_node", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_redirect_port", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_redirect_direction", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_modify_dst_mac", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_modify_src_mac", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_nwn_name", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_dscp", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_priority", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_rowstatus", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_flowlist_name", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_action", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_redirect_node", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_redirect_port", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_redirect_direction", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_modify_dst_mac", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_modify_src_mac", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_nwm_name", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_dscp", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_priority", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "c_flag", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "u_flag", SQL_SMALLINT, SQL_C_SHORT, 1, 0 }
};  // vbr_flowfilter_entry_schema
}  // namespace vbr_flowfilter_entry

namespace vbr_if_flowfilter {
const DalColumnSchema vbr_if_flowfilter_schema[] = {
  { "vtn_name", SQL_VARCHAR, SQL_C_CHAR, 32, 0 },
  { "vbr_name", SQL_VARCHAR, SQL_C_CHAR, 32, 0 },
  { "vbr_if_name", SQL_VARCHAR, SQL_C_CHAR, 32, 0 },
  { "direction", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "ctrlr_name", SQL_VARCHAR, SQL_C_CHAR, 32, kDcsCtrlrName },
  { "domain_id", SQL_VARCHAR, SQL_C_CHAR, 32, kDcsDomainId },
  { "flags", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_rowstatus", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "c_flag", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "u_flag", SQL_SMALLINT, SQL_C_SHORT, 1, 0 }
};  // vbr_if_flowfilter_schema
}  // namespace vbr_if_flowfilter

namespace vbr_if_flowfilter_entry {
const DalColumnSchema vbr_if_flowfilter_entry_schema[] = {
  { "vtn_name", SQL_VARCHAR, SQL_C_CHAR, 32, 0 },
  { "vbr_name", SQL_VARCHAR, SQL_C_CHAR, 32, 0 },
  { "vbr_if_name", SQL_VARCHAR, SQL_C_CHAR, 32, 0 },
  { "direction", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "sequence_num", SQL_INTEGER, SQL_C_ULONG, 1, 0 },
  { "ctrlr_name", SQL_VARCHAR, SQL_C_CHAR, 32, kDcsCtrlrName },
  { "domain_id", SQL_VARCHAR, SQL_C_CHAR, 32, kDcsDomainId },
  { "flowlist_name", SQL_VARCHAR, SQL_C_CHAR, 33, 0 },
  { "action", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "redirect_node", SQL_VARCHAR, SQL_C_CHAR, 32, 0 },
  { "redirect_port", SQL_VARCHAR, SQL_C_CHAR, 32, 0 },
  { "redirect_direction", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "modify_dst_mac", SQL_BINARY, SQL_C_BINARY, 6, 0 },
  { "modify_src_mac", SQL_BINARY, SQL_C_BINARY, 6, 0 },
  { "nwm_name", SQL_VARCHAR, SQL_C_CHAR, 32, 0 },
  { "dscp", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "priority", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "flags", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_flowlist_name", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_action", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_redirect_node", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_redirect_port", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_redirect_direction", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_modify_dst_mac", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_modify_src_mac", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_nwn_name", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_dscp", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_priority", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_rowstatus", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_flowlist_name", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_action", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_redirect_node", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_redirect_port", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_redirect_direction", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_modify_dst_mac", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_modify_src_mac", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_nwm_name", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_dscp", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_priority", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "c_flag", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "u_flag", SQL_SMALLINT, SQL_C_SHORT, 1, 0 }
};  // vbr_if_flowfilter_entry_schema
}  // namespace vbr_if_flowfilter_entry

namespace vrt_if_flowfilter {
const DalColumnSchema vrt_if_flowfilter_schema[] = {
  { "vtn_name", SQL_VARCHAR, SQL_C_CHAR, 32, 0 },
  { "vrt_name", SQL_VARCHAR, SQL_C_CHAR, 32, 0 },
  { "vrt_if_name", SQL_VARCHAR, SQL_C_CHAR, 32, 0 },
  { "direction", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "ctrlr_name", SQL_VARCHAR, SQL_C_CHAR, 32, kDcsCtrlrName },
  { "domain_id", SQL_VARCHAR, SQL_C_CHAR, 32, kDcsDomainId },
  { "flags", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_rowstatus", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "c_flag", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "u_flag", SQL_SMALLINT, SQL_C_SHORT, 1, 0 }
};  // vrt_if_flowfilter_schema
}  // namespace vrt_if_flowfilter

namespace vrt_if_flowfilter_entry {
const DalColumnSchema vrt_if_flowfilter_entry_schema[] = {
  { "vtn_name", SQL_VARCHAR, SQL_C_CHAR, 32, 0 },
  { "vrt_name", SQL_VARCHAR, SQL_C_CHAR, 32, 0 },
  { "vrt_if_name", SQL_VARCHAR, SQL_C_CHAR, 32, 0 },
  { "direction", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "sequence_num", SQL_INTEGER, SQL_C_ULONG, 1, 0 },
  { "ctrlr_name", SQL_VARCHAR, SQL_C_CHAR, 32, kDcsCtrlrName },
  { "domain_id", SQL_VARCHAR, SQL_C_CHAR, 32, kDcsDomainId },
  { "flowlist_name", SQL_VARCHAR, SQL_C_CHAR, 33, 0 },
  { "action", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "redirect_node", SQL_VARCHAR, SQL_C_CHAR, 32, 0 },
  { "redirect_port", SQL_VARCHAR, SQL_C_CHAR, 32, 0 },
  { "redirect_direction", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "modify_dst_mac", SQL_BINARY, SQL_C_BINARY, 6, 0 },
  { "modify_src_mac", SQL_BINARY, SQL_C_BINARY, 6, 0 },
  { "nwm_name", SQL_VARCHAR, SQL_C_CHAR, 32, 0 },
  { "dscp", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "priority", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "flags", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_flowlist_name", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_action", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_redirect_node", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_redirect_port", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_redirect_direction", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_modify_dst_mac", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_modify_src_mac", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_nwn_name", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_dscp", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_priority", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_rowstatus", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_flowlist_name", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_action", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_redirect_node", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_redirect_port", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_redirect_direction", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_modify_dst_mac", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_modify_src_mac", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_nwm_name", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_dscp", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_priority", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "c_flag", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "u_flag", SQL_SMALLINT, SQL_C_SHORT, 1, 0 }
};  // vrt_if_flowfilter_entry_schema
}  // namespace vrt_if_flowfilter_entry

namespace vterm_if_flowfilter {
const DalColumnSchema vterm_if_flowfilter_schema[] = {
  { "vtn_name", SQL_VARCHAR, SQL_C_CHAR, 32, 0 },
  { "vterm_name", SQL_VARCHAR, SQL_C_CHAR, 32, 0 },
  { "vterm_if_name", SQL_VARCHAR, SQL_C_CHAR, 32, 0 },
  { "direction", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "ctrlr_name", SQL_VARCHAR, SQL_C_CHAR, 32, kDcsCtrlrName },
  { "domain_id", SQL_VARCHAR, SQL_C_CHAR, 32, kDcsDomainId },
  { "flags", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_rowstatus", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "c_flag", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "u_flag", SQL_SMALLINT, SQL_C_SHORT, 1, 0 }
};  // vterm_if_flowfilter_schema
}  // namespace vterm_if_flowfilter

namespace vterm_if_flowfilter_entry {
const DalColumnSchema vterm_if_flowfilter_entry_schema[] = {
  { "vtn_name", SQL_VARCHAR, SQL_C_CHAR, 32, 0 },
  { "vterm_name", SQL_VARCHAR, SQL_C_CHAR, 32, 0 },
  { "vterm_if_name", SQL_VARCHAR, SQL_C_CHAR, 32, 0 },
  { "direction", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "sequence_num", SQL_INTEGER, SQL_C_ULONG, 1, 0 },
  { "ctrlr_name", SQL_VARCHAR, SQL_C_CHAR, 32, kDcsCtrlrName },
  { "domain_id", SQL_VARCHAR, SQL_C_CHAR, 32, kDcsDomainId },
  { "flowlist_name", SQL_VARCHAR, SQL_C_CHAR, 33, 0 },
  { "action", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "redirect_node", SQL_VARCHAR, SQL_C_CHAR, 32, 0 },
  { "redirect_port", SQL_VARCHAR, SQL_C_CHAR, 32, 0 },
  { "redirect_direction", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "modify_dst_mac", SQL_BINARY, SQL_C_BINARY, 6, 0 },
  { "modify_src_mac", SQL_BINARY, SQL_C_BINARY, 6, 0 },
  { "nwm_name", SQL_VARCHAR, SQL_C_CHAR, 32, 0 },
  { "dscp", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "priority", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "flags", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_flowlist_name", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_action", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_redirect_node", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_redirect_port", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_redirect_direction", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_modify_dst_mac", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_modify_src_mac", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_nwn_name", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_dscp", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_priority", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_rowstatus", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_flowlist_name", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_action", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_redirect_node", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_redirect_port", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_redirect_direction", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_modify_dst_mac", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_modify_src_mac", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_nwm_name", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_dscp", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_priority", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "c_flag", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "u_flag", SQL_SMALLINT, SQL_C_SHORT, 1, 0 }
};  // vterm_if_flowfilter_entry_schema
}  // namespace vterm_if_flowfilter_entry

namespace vtn_policingmap {
const DalColumnSchema vtn_policingmap_schema[] = {
  { "vtn_name", SQL_VARCHAR, SQL_C_CHAR, 32, 0 },
  { "policername", SQL_VARCHAR, SQL_C_CHAR, 33, 0 },
  { "flags", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_policername", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_rowstatus", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_policername", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "c_flag", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "u_flag", SQL_SMALLINT, SQL_C_SHORT, 1, 0 }
};  // vtn_policingmap_schema
}  // namespace vtn_policingmap


namespace vtn_policingmap_ctrlr {
const DalColumnSchema vtn_policingmap_ctrlr_schema[] = {
  { "vtn_name", SQL_VARCHAR, SQL_C_CHAR, 32, 0 },
  { "ctrlr_name", SQL_VARCHAR, SQL_C_CHAR, 32, kDcsCtrlrName },
  { "domain_id", SQL_VARCHAR, SQL_C_CHAR, 32, kDcsDomainId },
  { "policername", SQL_VARCHAR, SQL_C_CHAR, 33, 0 },
  { "flags", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_policername", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_rowstatus", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_policername", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "c_flag", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "u_flag", SQL_SMALLINT, SQL_C_SHORT, 1, 0 }
};  // vtn_policingmap_ctrlr_schema
}  // namespace vtn_policingmap_ctrlr

namespace vbr_policingmap {
const DalColumnSchema vbr_policingmap_schema[] = {
  { "vtn_name", SQL_VARCHAR, SQL_C_CHAR, 32, 0 },
  { "vbr_name", SQL_VARCHAR, SQL_C_CHAR, 32, 0 },
  { "ctrlr_name", SQL_VARCHAR, SQL_C_CHAR, 32, kDcsCtrlrName },
  { "domain_id", SQL_VARCHAR, SQL_C_CHAR, 32, kDcsDomainId },
  { "policername", SQL_VARCHAR, SQL_C_CHAR, 33, 0 },
  { "flags", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_policername", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_rowstatus", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_policername", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "c_flag", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "u_flag", SQL_SMALLINT, SQL_C_SHORT, 1, 0 }
};  // vbr_policingmap_schema
}  // namespace vbr_policingmap

namespace vbr_if_policingmap {
const DalColumnSchema vbr_if_policingmap_schema[] = {
  { "vtn_name", SQL_VARCHAR, SQL_C_CHAR, 32, 0 },
  { "vbr_name", SQL_VARCHAR, SQL_C_CHAR, 32, 0 },
  { "vbr_if_name", SQL_VARCHAR, SQL_C_CHAR, 32, 0 },
  { "ctrlr_name", SQL_VARCHAR, SQL_C_CHAR, 32, kDcsCtrlrName },
  { "domain_id", SQL_VARCHAR, SQL_C_CHAR, 32, kDcsDomainId },
  { "policername", SQL_VARCHAR, SQL_C_CHAR, 33, 0 },
  { "flags", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "valid_policername", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_rowstatus", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "cs_policername", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "c_flag", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "u_flag", SQL_SMALLINT, SQL_C_SHORT, 1, 0 }
};  // vbr_policingmap_schema
}  // namespace vbr_if_policingmap

// POM MoMgrs Schema End

namespace ctrlr {
const DalColumnSchema ctrlr_schema[] = {
  { "name", SQL_VARCHAR, SQL_C_CHAR, 32, 0 },
  { "type", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "version", SQL_VARCHAR, SQL_C_CHAR, 32, 0 },
  { "audit_done", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "config_done", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "invalid_config", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "state", SQL_SMALLINT, SQL_C_SHORT, 1, 0 }
};
}  // namespace ctrlr

namespace cfg_tbl_dirty {
const DalColumnSchema cfg_tbl_dirty_schema[] = {
  { "table_name", SQL_VARCHAR, SQL_C_CHAR, max_cfg_tbl_name_len, 0 },
  { "operation", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },  // CREATE/DELETE/UPDATE
  { "dirty", SQL_SMALLINT, SQL_C_SHORT, 1, 0 }
};
}  // namespace cfg_tbl_dirty

namespace upll_system_tbl {
const DalColumnSchema upll_system_tbl_schema[] = {
    { "property", SQL_VARCHAR, SQL_C_CHAR, 64, 0 },
    { "value", SQL_VARCHAR, SQL_C_CHAR, 128, 0 }
};
}  // namespace upll_system_tbl

namespace vtn_cfg_tbl_dirty {
const DalColumnSchema vtn_cfg_tbl_dirty_schema[] = {
  { "table_index", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "operation", SQL_SMALLINT, SQL_C_SHORT, 1, 0 },
  { "vtn_name", SQL_VARCHAR, SQL_C_CHAR, 32, 0 }
};
}  // namespace vtn_cfg_tbl_dirty

namespace pp_scratch {
const DalColumnSchema pp_scratch_tbl_schema[] = {
  { "policingprofile_name", SQL_VARCHAR, SQL_C_CHAR, 33, 0 },
  { "ctrlr_name", SQL_VARCHAR, SQL_C_CHAR, 32, 0 },
  { "vtn_name", SQL_VARCHAR, SQL_C_CHAR, 32, 0 },
  // TODO(pcm) - Support for -ve ref_count
  { "ref_count", SQL_BIGINT, SQL_C_UBIGINT, 1, 0 }
};
}  // namespace pp_scratch

namespace fl_scratch {
const DalColumnSchema fl_scratch_tbl_schema[] = {
  { "flowlist_name", SQL_VARCHAR, SQL_C_CHAR, 33, 0 },
  { "ctrlr_name", SQL_VARCHAR, SQL_C_CHAR, 32, 0 },
  { "vtn_name", SQL_VARCHAR, SQL_C_CHAR, 32, 0 },
  // TODO(pcm) - Support for -ve ref_count
  { "ref_count", SQL_BIGINT, SQL_C_UBIGINT, 1, 0 }
};
}  // namespace fl_scratch

namespace spd_scratch {
const DalColumnSchema spd_scratch_tbl_schema[] = {
  { "unw_name", SQL_VARCHAR, SQL_C_CHAR, 32, 0 },
  { "unw_spine_domain_name", SQL_VARCHAR, SQL_C_CHAR, 32, 0 },
  { "vtn_name", SQL_VARCHAR, SQL_C_CHAR, 32, 0 },
  // TODO(pcm) - Support for -ve ref_count
  { "used_count", SQL_BIGINT, SQL_C_UBIGINT, 1, 0 }
};
}  // namespace spd_scratch

// Relational Schema - Shows parent-child relation
const DalRelnSchema relational_schema[] = {
// VTN Schemas
  { kDalNumTables, 0 },  // VTN Schema
  { kDalNumTables, 0 },  // VTN Controller Schema
  { kDalNumTables, 0 },  // VTN Rename Schema
  { kDbiVtnTbl, 1 },  // Vbridge Schema
  { kDbiVbrTbl, 2 },  // Vbridge Vlanmap Schema
  { kDbiVbrTbl, 2 },  // Vbridge Interface Schema
  { kDbiVtnTbl, 1 },  // Vrouter Schema
  { kDbiVrtTbl, 2 },  // Vrouter Interface Schema
  { kDbiVtnTbl, 1 },  // Vterminal Schema
  { kDbiVterminalTbl, 2 },  // Vterminal Interface Schema
  { kDalNumTables, 0 },  // Vnode Rename Schema
  { kDbiVtnTbl, 1 },  // Vlink Schema
  { kDalNumTables, 0 },  // Vlink Rename Schema
  { kDbiVrtTbl, 2 },  // Static IP Route Schema
  { kDbiVrtTbl, 2 },  // Dhcp Relay Server Schema
  { kDbiVrtTbl, 2 },  // Dhcp Relay Interface Schema
  { kDbiVbrTbl, 2 },  // Vbridge Network Monitor Schema
  { kDbiVbrNwMonTbl, 3 },  // Vbridge Network Monitor Host Schema
  { kDbiVtnTbl, 1 },  // Vunknown Schema
  { kDbiVunknownTbl, 2 },  // Vunknown Interface Schema
  { kDbiVtnTbl, 1 },  // Vtep Schema
  { kDbiVtepTbl, 2 },  // Vtep Interface Schema
  { kDbiVtnTbl, 1 },  // Vtep Group Schema
  { kDbiVtepGrpTbl, 2 },  // Vtep Group Member Schema
  { kDbiVtnTbl, 1 },  // Vtunnel Schema
  { kDbiVtunnelTbl, 2 },  // Vtunnel Interface Schema
  { kDbiVbrTbl, 2 },  // Convert Vbridge Schema
  { kDbiConvertVbrTbl, 3 },  // Convert Vbridge interface Schema
  { kDbiVbrTbl, 2 },  // Convert Vlink Schema
  { kDbiVbrTbl, 2 },  // vbridge Portmap Schema
  // U17 added
  { kDalNumTables, 0 },     // Unified network Schema
  { kDbiUnifiedNwTbl, 1 },  // Unified Network Label Schema
  { kDbiUnwLabelTbl, 2 },   // Unified Network Label Range Schema
  { kDbiUnifiedNwTbl, 1 },  // Unified Network Spine Domain
  { kDbiVtnTbl, 1 },        // Vtn Unified Schema
  { kDalNumTables, 0 },     // Vbid schema
  { kDalNumTables, 0 },     // GvtnId schema
  { kDbiVtnTbl, 1 },        // Convert Vtunnel Schema
  { kDbiConvertVtunnelTbl, 2 },  // Convert Vtunnel Interface Schema
  { kDbiVtnTbl, 1 },        // vtn gateway port schema
// POM schemas
  { kDalNumTables, 0 },     // Flowlist Schema
  { kDalNumTables, 0 },     // Flowlist Controller Schema
  { kDalNumTables, 0 },     // Flowlist Rename Schema
  { kDbiFlowListTbl, 1 },   // Flowlist Entry Schema
  { kDalNumTables, 0 },     // Flowlist Entry Controller Schema
  { kDalNumTables, 0 },     // Policing profile Schema
  { kDalNumTables, 0 },     // Policing profile Controller Schema
  { kDalNumTables, 0 },     // Policing profile Rename Schema
  { kDbiPolicingProfileTbl, 1 },    // Policing profile Entry Schema
  { kDalNumTables, 0 },     // Policing profile Entry Controller Schema
  { kDbiVtnTbl, 1 },        // VTN Flowfilter Schema
  { kDalNumTables, 0 },     // VTN Flowfilter Controller Schema
  { kDbiVtnFlowFilterTbl, 2 },      // VTN Flowfilter Entry Schema
  { kDalNumTables, 0 },     // VTN Flowfilter Entry Controller Schema
  { kDbiVbrTbl, 2 },        // Vbridge Flowfilter Schema
  { kDbiVbrFlowFilterTbl, 3 },      // Vbridge Flowfilter Entry Schema
  { kDbiVbrIfTbl, 3 },      // Vbridge Interface Flowfilter Schema
  { kDbiVbrIfFlowFilterTbl, 4 },    // Vbridge Interface Flowfilter Entry Schema
  { kDbiVrtIfTbl, 3 },      // Vrouter Interface Flowfilter Schema
  { kDbiVrtIfFlowFilterTbl, 4 },    // Vrouter Interface Flowfilter Entry Schema
  { kDbiVtermIfTbl, 3 },    // Vterminal Interface Flowfilter Schema
  { kDbiVtermIfFlowFilterTbl, 4 },  // Vterm Interface Flowfilter Entry schema
  { kDbiVtnTbl, 1 },        // VTN Policing Map Schema
  { kDalNumTables, 0 },     // VTN Policing Map Controller Schema
  { kDbiVbrTbl, 2 },        // Vbridge Policing Map Schema
  { kDbiVbrIfTbl, 3 },      // Vbridge Interface Policing Map Schema
  { kDbiVtermIfTbl, 3 },    // Vterminal Interface Policing Map Schema

// Dirty_table Schema
  { kDalNumTables, 0 },     // Dirty Table Schema

// upll_system_table Schema
  { kDalNumTables, 0 },
// Ctrlr Schema
  { kDalNumTables, 0 },      // Controller Schema
// VTn dirty table schema
  { kDalNumTables, 0 },      // Vtn dirty Table  Schema
// Scratch Table schema
  { kDalNumTables, 0 },      // FlowList scratch Schema
  { kDalNumTables, 0 },      // Policingprofile scratch Schema
  { kDalNumTables, 0 }       // Spine domain scratch Schema
};  // Relational schema

const DalTableSchema table_schema[] = {
  // L2/L3 MoMgr table Schema Info Start
  { "vtn_tbl",
    vtn::kDbiVtnNumCols,
    vtn::kVtnNumPks,
    vtn::vtn_schema },
  { "vtn_ctrlr_tbl",
    vtn_controller::kDbiVtnCtrlrNumCols,
    vtn_controller::kVtnCtrlrNumPks,
    vtn_controller::vtn_controller_schema },
  { "vtn_rename_tbl",
    vtn_rename::kDbiVtnRenameNumCols,
    vtn_rename::kVtnRenameNumPks,
    vtn_rename::vtn_rename_schema },
  { "vbr_tbl",
    vbridge::kDbiVbrNumCols,
    vbridge::kVbrNumPks,
    vbridge::vbridge_schema },
  { "vbr_vlanmap_tbl",
    vbridge_vlanmap::kDbiVbrVlanMapNumCols,
    vbridge_vlanmap::kVbrVlanMapNumPks,
    vbridge_vlanmap::vbridge_vlanmap_schema },
  { "vbr_if_tbl",
    vbridge_interface::kDbiVbrIfNumCols,
    vbridge_interface::kVbrIfNumPks,
    vbridge_interface::vbridge_interface_schema },
  { "vrt_tbl",
    vrouter::kDbiVrtNumCols,
    vrouter::kVrtNumPks,
    vrouter::vrouter_schema },
  { "vrt_if_tbl",
    vrouter_interface::kDbiVrtIfNumCols,
    vrouter_interface::kVrtIfNumPks,
    vrouter_interface::vrouter_interface_schema },
  { "vterminal_tbl",
    vterminal::kDbiVterminalNumCols,
    vterminal::kVterminalNumPks,
    vterminal::vterminal_schema },
  { "vterm_if_tbl",
    vterminal_interface::kDbiVtermIfNumCols,
    vterminal_interface::kVtermIfNumPks,
    vterminal_interface::vterminal_interface_schema },
  { "vnode_rename_tbl",
    vnode_rename::kDbiVnodeRenameNumCols,
    vnode_rename::kVnodeRenameNumPks,
    vnode_rename::vnode_rename_schema },
  { "vlink_tbl",
    vlink::kDbiVlinkNumCols,
    vlink::kVlinkNumPks,
    vlink::vlink_schema },
  { "vlink_rename_tbl",
    vlink_rename::kDbiVlinkRenameNumCols,
    vlink_rename::kVlinkRenameNumPks,
    vlink_rename::vlink_rename_schema },
  { "static_ip_route_tbl",
    static_ip_route::kDbiStaticIpRouteNumCols,
    static_ip_route::kStaticIpRouteNumPks,
    static_ip_route::static_ip_route_schema },
  { "dhcp_relay_server_tbl",
    dhcprelay_server::kDbiDhcpRelayServerNumCols,
    dhcprelay_server::kDhcpRelayServerNumPks,
    dhcprelay_server::dhcprelay_server_schema },
  { "dhcp_relay_if_tbl",
    dhcprelay_interface::kDbiDhcpRelayIfNumCols,
    dhcprelay_interface::kDhcpRelayIfNumPks,
    dhcprelay_interface::dhcprelay_interface_schema },
  { "vbr_nwmon_grp_tbl",
    vbridge_networkmonitor_group::kDbiVbrNwMonGrpNumCols,
    vbridge_networkmonitor_group::kVbrNwMonGrpNumPks,
    vbridge_networkmonitor_group::vbridge_networkmonitor_group_schema },
  { "vbr_nwmon_host_tbl",
    vbridge_networkmonitor_host::kDbiVbrNwMonHostNumCols,
    vbridge_networkmonitor_host::kVbrNwMonHostNumPks,
    vbridge_networkmonitor_host::vbridge_networkmonitor_host_schema },
  { "vunknown_tbl",
    vunknown::kDbiVunknownNumCols,
    vunknown::kVunknownNumPks,
    vunknown::vunknown_schema },
  { "vunknown_if_tbl",
    vunknown_interface::kDbiVunknownIfNumCols,
    vunknown_interface::kVunknownIfNumPks,
    vunknown_interface::vunknown_interface_schema },
  { "vtep_tbl",
    vtep::kDbiVtepNumCols,
    vtep::kVtepNumPks,
    vtep::vtep_schema },
  { "vtep_if_tbl",
    vtep_interface::kDbiVtepIfNumCols,
    vtep_interface::kVtepIfNumPks,
    vtep_interface::vtep_interface_schema },
  { "vtep_grp_tbl",
    vtep_group::kDbiVtepGrpNumCols,
    vtep_group::kVtepGrpNumPks,
    vtep_group::vtep_group_schema },
  { "vtep_grp_mem_tbl",
    vtep_groupmember::kDbiVtepGrpMemNumCols,
    vtep_groupmember::kVtepGrpMemNumPks,
    vtep_groupmember::vtep_groupmember_schema },
  { "vtunnel_tbl",
    vtunnel::kDbiVtunnelNumCols,
    vtunnel::kVtunnelNumPks,
    vtunnel::vtunnel_schema },
  { "vtunnel_if_tbl",
    vtunnel_interface::kDbiVtunnelIfNumCols,
    vtunnel_interface::kVtunnelIfNumPks,
    vtunnel_interface::vtunnel_interface_schema },
  { "convert_vbr_tbl",
    convert_vbridge::kDbiConvertVbrNumCols,
    convert_vbridge::kConvertVbrNumPks,
    convert_vbridge::convert_vbridge_schema },
  { "convert_vbr_if_tbl",
    convert_vbridge_interface::kDbiConvertVbrIfNumCols,
    convert_vbridge_interface::kConvertVbrIfNumPks,
    convert_vbridge_interface::convert_vbridge_interface_schema },
  { "convert_vlink_tbl",
    convert_vlink::kDbiConvertVlinkNumCols,
    convert_vlink::kConvertVlinkNumPks,
    convert_vlink::convert_vlink_schema },
  { "vbr_portmap_tbl",
    vbridge_portmap::kDbiVbrPortmapNumCols,
    vbridge_portmap::kVbrPortmapNumPks,
    vbridge_portmap::vbridge_portmap_schema },
  { "unified_nw_tbl",
    unified_nw::kDbiUnifiedNwNumCols,
    unified_nw::kUnifiedNwNumPks,
    unified_nw::unified_nw_schema },
  { "unw_label_tbl",
    unw_label::kDbiUnwLabelNumCols,
    unw_label::kUnwLabelNumPks,
    unw_label::unw_label_schema },
  { "unw_label_range_tbl",
    unw_label_range::kDbiUnwLabelRangeNumCols,
    unw_label_range::kUnwLabelRangeNumPks,
    unw_label_range::unw_label_range_schema },
  { "unw_spine_domain_tbl",
    unw_spine_domain::kDbiUnwSpineNumCols,
    unw_spine_domain::kUnwSpineDomainNumPks,
    unw_spine_domain::unw_spine_domain_schema },
  { "vtn_unified_tbl",
    vtn_unified::kDbiVtnUnifiedNumCols,
    vtn_unified::kVtnUnifiedNumPks,
    vtn_unified::vtn_unified_schema },
  { "vbid_label_tbl",
    vbid_label::kDbiVBIdNumCols,
    vbid_label::kVBIdTblNumPkCols,
    vbid_label::vbid_label_schema },
  { "gvtnid_label_tbl",
    gvtnid_label::kDbiGVtnIdNumCols,
    gvtnid_label::kGVtnIdTblNumPkCols,
    gvtnid_label::gvtnid_label_schema },
  { "convert_vtunnel_tbl",
    convert_vtunnel::kDbiConvertVtunnelNumCols,
    convert_vtunnel::kConvertVtunnelNumPks,
    convert_vtunnel::convert_vtunnel_schema },
  { "convert_vtunnel_if_tbl",
    convert_vtunnel_interface::kDbiConvertVtunnelIfNumCols,
    convert_vtunnel_interface::kConvertVtunnelIfNumPks,
    convert_vtunnel_interface::convert_vtunnel_interface_schema },
  { "vtn_gateway_port_tbl",
    vtn_gateway_port::kDbiVtnGatewayPortNumCols,
    vtn_gateway_port::kVtnGatewayPortNumPks,
    vtn_gateway_port::vtn_gateway_port_schema },

  // L2/L3 MoMgr table Schema Info End

  // POM MoMgr table Schema Info Start

  { "flowlist_tbl",
    flowlist::kDbiFlowListNumCols,
    flowlist::kFlowListNumPks,
    flowlist::flowlist_schema },
  { "flowlist_ctrlr_tbl",
    flowlist_ctrlr::kDbiFlowListCtrlrNumCols,
    flowlist_ctrlr::kFlowListCtrlrNumPks,
    flowlist_ctrlr::flowlist_ctrlr_schema },
  { "flowlist_rename_tbl",
    flowlist_rename::kDbiFlowListRenameNumCols,
    flowlist_rename::kFlowListRenameNumPks,
    flowlist_rename::flowlist_rename_schema },
  { "flowlist_entry_tbl",
    flowlist_entry::kDbiFlowListEntryNumCols,
    flowlist_entry::kFlowListEntryNumPks,
    flowlist_entry::flowlist_entry_schema },
  { "flowlist_entry_ctrlr_tbl",
    flowlist_entry_ctrlr::kDbiFlowListEntryCtrlrNumCols,
    flowlist_entry_ctrlr::kFlowListEntryCtrlrNumPks,
    flowlist_entry_ctrlr::flowlist_entry_ctrlr_schema },
  { "policingprofile_tbl",
    policingprofile::kDbiPolicingProfileNumCols,
    policingprofile::kPolicingProfileNumPks,
    policingprofile::policingprofile_schema },
  { "policingprofile_ctrlr_tbl",
    policingprofile_ctrlr::kDbiPolicingProfileCtrlrNumCols,
    policingprofile_ctrlr::kPolicingProfileCtrlrNumPks,
    policingprofile_ctrlr::policingprofile_ctrlr_schema },
  { "policingprofile_rename_tbl",
    policingprofile_rename::kDbiPolicingProfileRenameNumCols,
    policingprofile_rename::kPolicingProfileRenameNumPks,
    policingprofile_rename::policingprofile_rename_schema },
  { "policingprofile_entry_tbl",
    policingprofile_entry::kDbiPolicingProfileEntryNumCols,
    policingprofile_entry::kPolicingProfileEntryNumPks,
    policingprofile_entry::policingprofile_entry_schema },
  { "policingprofile_entry_ctrlr_tbl",
    policingprofile_entry_ctrlr::kDbiPolicingProfileEntryCtrlrNumCols,
    policingprofile_entry_ctrlr::kPolicingProfileEntryCtrlrNumPks,
    policingprofile_entry_ctrlr::policingprofile_entry_ctrlr_schema },
  { "vtn_flowfilter_tbl",
    vtn_flowfilter::kDbiVtnFlowFilterNumCols,
    vtn_flowfilter::kVtnFlowFilterNumPks,
    vtn_flowfilter::vtn_flowfilter_schema },
  { "vtn_flowfilter_ctrlr_tbl",
    vtn_flowfilter_ctrlr::kDbiVtnFlowFilterCtrlrNumCols,
    vtn_flowfilter_ctrlr::kVtnFlowFilterCtrlrNumPks,
    vtn_flowfilter_ctrlr::vtn_flowfilter_ctrlr_schema },
  { "vtn_flowfilter_entry_tbl",
    vtn_flowfilter_entry::kDbiVtnFlowFilterEntryNumCols,
    vtn_flowfilter_entry::kVtnFlowFilterEntryNumPks,
    vtn_flowfilter_entry::vtn_flowfilter_entry_schema },
  { "vtn_flowfilter_entry_ctrlr_tbl",
    vtn_flowfilter_entry_ctrlr::kDbiVtnFlowFilterEntryCtrlrNumCols,
    vtn_flowfilter_entry_ctrlr::kVtnFlowFilterEntryCtrlrNumPks,
    vtn_flowfilter_entry_ctrlr::vtn_flowfilter_entry_ctrlr_schema },
  { "vbr_flowfilter_tbl",
    vbr_flowfilter::kDbiVbrFlowFilterNumCols,
    vbr_flowfilter::kVbrFlowFilterNumPks,
    vbr_flowfilter::vbr_flowfilter_schema },
  { "vbr_flowfilter_entry_tbl",
    vbr_flowfilter_entry::kDbiVbrFlowFilterEntryNumCols,
    vbr_flowfilter_entry::kVbrFlowFilterEntryNumPks,
    vbr_flowfilter_entry::vbr_flowfilter_entry_schema },
  { "vbr_if_flowfilter_tbl",
    vbr_if_flowfilter::kDbiVbrIfFlowFilterNumCols,
    vbr_if_flowfilter::kVbrIfFlowFilterNumPks,
    vbr_if_flowfilter::vbr_if_flowfilter_schema },
  { "vbr_if_flowfilter_entry_tbl",
    vbr_if_flowfilter_entry::kDbiVbrIfFlowFilterEntryNumCols,
    vbr_if_flowfilter_entry::kVbrIfFlowFilterEntryNumPks,
    vbr_if_flowfilter_entry::vbr_if_flowfilter_entry_schema },
  { "vrt_if_flowfilter_tbl",
    vrt_if_flowfilter::kDbiVrtIfFlowFilterNumCols,
    vrt_if_flowfilter::kVrtIfFlowFilterNumPks,
    vrt_if_flowfilter::vrt_if_flowfilter_schema },
  { "vrt_if_flowfilter_entry_tbl",
    vrt_if_flowfilter_entry::kDbiVrtIfFlowFilterEntryNumCols,
    vrt_if_flowfilter_entry::kVrtIfFlowFilterEntryNumPks,
    vrt_if_flowfilter_entry::vrt_if_flowfilter_entry_schema },
  { "vterm_if_flowfilter_tbl",
    vterm_if_flowfilter::kDbiVtermIfFlowFilterNumCols,
    vterm_if_flowfilter::kVtermIfFlowFilterNumPks,
    vterm_if_flowfilter::vterm_if_flowfilter_schema },
  { "vterm_if_flowfilter_entry_tbl",
    vterm_if_flowfilter_entry::kDbiVtermIfFlowFilterEntryNumCols,
    vterm_if_flowfilter_entry::kVtermIfFlowFilterEntryNumPks,
    vterm_if_flowfilter_entry::vterm_if_flowfilter_entry_schema },
  { "vtn_policingmap_tbl",
    vtn_policingmap::kDbiVtnPolicingMapNumCols,
    vtn_policingmap::kVtnPolicingMapNumPks,
    vtn_policingmap::vtn_policingmap_schema },
  { "vtn_policingmap_ctrlr_tbl",
    vtn_policingmap_ctrlr::kDbiVtnPolicingMapCtrlrNumCols,
    vtn_policingmap_ctrlr::kVtnPolicingMapCtrlrNumPks,
    vtn_policingmap_ctrlr::vtn_policingmap_ctrlr_schema },
  { "vbr_policingmap_tbl",
    vbr_policingmap::kDbiVbrPolicingMapNumCols,
    vbr_policingmap::kVbrPolicingMapNumPks,
    vbr_policingmap::vbr_policingmap_schema },
  { "vbr_if_policingmap_tbl",
    vbr_if_policingmap::kDbiVbrIfPolicingMapNumCols,
    vbr_if_policingmap::kVbrIfPolicingMapNumPks,
    vbr_if_policingmap::vbr_if_policingmap_schema },
  { "vterm_if_policingmap_tbl",
    vterm_if_policingmap::kDbiVtermIfPolicingMapNumCols,
    vterm_if_policingmap::kVtermIfPolicingMapNumPks,
    vterm_if_policingmap::vterm_if_policingmap_schema },
  // POM MoMgr table Schema Info End

  { "upll_cfg_dirty_tbl",
    cfg_tbl_dirty::kCfgTblDirtyNumCols,
    cfg_tbl_dirty::kCfgTblDirtyNumPkCols,
    cfg_tbl_dirty::cfg_tbl_dirty_schema },

  { "upll_system_tbl",
    upll_system_tbl::kUpllSystemTblNumCols,
    upll_system_tbl::kUpllSystemTblNumPkCols,
    upll_system_tbl::upll_system_tbl_schema },

  { "ctrlr_tbl",
    ctrlr::kCtrlrNumCols,
    ctrlr::kCtrlrNumPkCols,
    ctrlr::ctrlr_schema },
  { "upll_vtn_cfg_dirty_tbl",
    vtn_cfg_tbl_dirty::kVtnCfgTblDirtyNumCols,
    vtn_cfg_tbl_dirty::kVtnCfgTblDirtyNumPkCols,
    vtn_cfg_tbl_dirty::vtn_cfg_tbl_dirty_schema },
  { "pp_scratch_tbl",
    pp_scratch::kPpScratchTblNumCols,
    pp_scratch::kPpScratchNumPkCols,
    pp_scratch::pp_scratch_tbl_schema },
  { "fl_scratch_tbl",
    fl_scratch::kFlScratchTblNumCols,
    fl_scratch::kFlScratchNumPkCols,
    fl_scratch::fl_scratch_tbl_schema },
  { "spd_scratch_tbl",
    spd_scratch::kSpdScratchTblNumCols,
    spd_scratch::kSpdScratchNumPkCols,
    spd_scratch::spd_scratch_tbl_schema }
};  // table_schema

};  // namespace table

};  // namespace schema

}  // namespace dal
}  // namespace upll
}  // namespace unc
