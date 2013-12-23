/*
 * Copyright (c) 2012-2013 NEC Corporation
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
  {"vtn_name", SQL_VARCHAR, SQL_C_CHAR, 32},
  {"description", SQL_VARCHAR, SQL_C_CHAR, 128},
  {"oper_status", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"alarm_status", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"down_count", SQL_BIGINT, SQL_C_UBIGINT, 1},
  {"creation_time", SQL_BINARY, SQL_C_BINARY, 8},
  {"last_updated_time", SQL_BINARY, SQL_C_BINARY, 8},
  {"flags", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"valid_description", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"valid_oper_status", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"valid_alarm_status", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"valid_creation_time", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"valid_last_updated_time", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"cs_rowstatus", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"cs_description", SQL_SMALLINT, SQL_C_SHORT, 1}
};
}  //  namespace vtn

namespace vtn_controller {
const DalColumnSchema vtn_controller_schema[] =  {
  {"vtn_name", SQL_VARCHAR, SQL_C_CHAR, 32},
  {"controller_name", SQL_VARCHAR, SQL_C_CHAR, 32},
  {"domain_id", SQL_VARCHAR, SQL_C_CHAR, 32},
  {"oper_status", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"alarm_status", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"down_count", SQL_BIGINT, SQL_C_UBIGINT, 1},
  {"ref_count", SQL_BIGINT, SQL_C_UBIGINT, 1},
  {"flags", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"valid_oper_status", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"valid_alarm_status", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"cs_rowstatus", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"cs_description", SQL_SMALLINT, SQL_C_SHORT, 1}
};
}  //  namespace vtn_controller

namespace vtn_rename {
const DalColumnSchema vtn_rename_schema[] =  {
  {"ctrlr_vtn_name", SQL_VARCHAR, SQL_C_CHAR, 32},
  {"controller_name", SQL_VARCHAR, SQL_C_CHAR, 32},
  {"domain_id", SQL_VARCHAR, SQL_C_CHAR, 32},
  {"unc_vtn_name", SQL_VARCHAR, SQL_C_CHAR, 32}
};
}  //  namespcae vtn_rename

namespace vbridge {
const DalColumnSchema vbridge_schema[] =  {
  {"vtn_name", SQL_VARCHAR, SQL_C_CHAR, 32},
  {"vbridge_name", SQL_VARCHAR, SQL_C_CHAR, 32},
  {"controller_name", SQL_VARCHAR, SQL_C_CHAR, 32},
  {"domain_id", SQL_VARCHAR, SQL_C_CHAR, 32},
  {"vbr_description", SQL_VARCHAR, SQL_C_CHAR, 128},
  {"host_addr", SQL_BINARY, SQL_C_BINARY, 4},
  {"host_addr_mask", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"oper_status", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"down_count", SQL_BIGINT, SQL_C_UBIGINT, 1},
  {"fault_count", SQL_BIGINT, SQL_C_UBIGINT, 1},
  {"valid_controller_name", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"valid_domain_id", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"valid_vbr_description", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"valid_host_addr", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"valid_host_addr_mask", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"valid_oper_status", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"cs_rowstatus", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"cs_controller_name", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"cs_domain_id", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"cs_vbr_description", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"cs_host_addr", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"cs_host_addr_mask", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"flags", SQL_SMALLINT, SQL_C_SHORT, 1}
};
}  //  namespace vbridge

namespace vbridge_vlanmap {
const DalColumnSchema vbridge_vlanmap_schema[] =  {
  {"vtn_name", SQL_VARCHAR, SQL_C_CHAR, 32},
  {"vbridge_name", SQL_VARCHAR, SQL_C_CHAR, 32},
  {"logical_port_id", SQL_VARCHAR, SQL_C_CHAR, 320},
  {"logical_port_id_valid", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"vlanid", SQL_INTEGER, SQL_C_ULONG, 1},
  {"controller_name", SQL_VARCHAR, SQL_C_CHAR, 32},
  {"domain_id", SQL_VARCHAR, SQL_C_CHAR, 32},
  {"flags", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"valid_vlanid", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"cs_rowstatus", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"cs_vlanid", SQL_SMALLINT, SQL_C_SHORT, 1}
};
}  //  namespace vbridge_vlanmap

namespace vbridge_interface {
const DalColumnSchema vbridge_interface_schema[] =  {
  {"vtn_name", SQL_VARCHAR, SQL_C_CHAR, 32},
  {"vbridge_name", SQL_VARCHAR, SQL_C_CHAR, 32},
  {"if_name", SQL_VARCHAR, SQL_C_CHAR, 32},
  {"admin_status", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"description", SQL_VARCHAR, SQL_C_CHAR, 128},
  {"logical_port_id", SQL_VARCHAR, SQL_C_CHAR, 320},
  {"vlanid", SQL_INTEGER, SQL_C_ULONG, 1},
  {"tagged", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"vex_name", SQL_VARCHAR, SQL_C_CHAR, 32},
  {"vex_if_name", SQL_VARCHAR, SQL_C_CHAR, 32},
  {"vex_link_name", SQL_VARCHAR, SQL_C_CHAR, 32},
  {"oper_status", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"down_count", SQL_BIGINT, SQL_C_UBIGINT, 1},
  {"controller_name", SQL_VARCHAR, SQL_C_CHAR, 32},
  {"domain_id", SQL_VARCHAR, SQL_C_CHAR, 32},
  {"flags", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"valid_admin_status", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"valid_description", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"valid_portmap", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"valid_logical_port_id", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"valid_vlanid", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"valid_tagged", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"valid_vex_name", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"valid_vex_if_name", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"valid_vex_link_name", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"valid_oper_status", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"cs_rowstatus", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"cs_admin_status", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"cs_description", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"cs_portmap", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"cs_logical_port_id", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"cs_vlanid", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"cs_tagged", SQL_SMALLINT, SQL_C_SHORT, 1}
};
}  //  namespace vbridge_interface

namespace vrouter {
const DalColumnSchema vrouter_schema[] =  {
  {"vtn_name", SQL_VARCHAR, SQL_C_CHAR, 32},
  {"vrouter_name", SQL_VARCHAR, SQL_C_CHAR, 32},
  {"controller_name", SQL_VARCHAR, SQL_C_CHAR, 32},
  {"domain_id", SQL_VARCHAR, SQL_C_CHAR, 32},
  {"vrt_description", SQL_VARCHAR, SQL_C_CHAR, 128},
  {"dhcprelay_admin_status", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"oper_status", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"down_count", SQL_BIGINT, SQL_C_UBIGINT, 1},
  {"flags", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"valid_controller_name", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"valid_domain_id", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"valid_vrt_description", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"valid_dhcprelay_admin_status", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"valid_oper_status", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"cs_rowstatus", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"cs_controller_name", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"cs_domain_id", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"cs_vrt_description", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"cs_dhcprelay_admin_status", SQL_SMALLINT, SQL_C_SHORT, 1}
};
}  //  namespace vrouter

namespace vrouter_interface {
const DalColumnSchema vrouter_interface_schema[] =  {
  {"vtn_name", SQL_VARCHAR, SQL_C_CHAR, 32},
  {"vrouter_name", SQL_VARCHAR, SQL_C_CHAR, 32},
  {"if_name", SQL_VARCHAR, SQL_C_CHAR, 32},
  {"description", SQL_VARCHAR, SQL_C_CHAR, 128},
  {"ip_addr", SQL_BINARY, SQL_C_BINARY, 4},
  {"mask", SQL_SMALLINT, SQL_SMALLINT, 1},
  {"mac_addr", SQL_BINARY, SQL_C_BINARY, 6},
  {"admin_status", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"oper_status", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"down_count", SQL_BIGINT, SQL_C_UBIGINT, 1},
  {"controller_name", SQL_VARCHAR, SQL_C_CHAR, 32},
  {"domain_id", SQL_VARCHAR, SQL_C_CHAR, 32},
  {"flags", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"valid_description", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"valid_ip_addr", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"valid_mask", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"valid_mac_addr", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"valid_admin_status", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"valid_oper_status", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"cs_rowstatus", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"cs_description", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"cs_ip_addr", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"cs_mask", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"cs_mac_addr", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"cs_admin_status", SQL_SMALLINT, SQL_C_SHORT, 1}
};
}  //  namespace vrouter_interface

namespace vnode_rename {
const DalColumnSchema vnode_rename_schema[] =  {
  {"ctrlr_vtn_name", SQL_VARCHAR, SQL_C_CHAR, 32},
  {"ctrlr_vnode_name", SQL_VARCHAR, SQL_C_CHAR, 32},
  {"controller_name", SQL_VARCHAR, SQL_C_CHAR, 32},
  {"domain_id", SQL_VARCHAR, SQL_C_CHAR, 32},
  {"unc_vtn_name", SQL_VARCHAR, SQL_C_CHAR, 32},
  {"unc_vnode_name", SQL_VARCHAR, SQL_C_CHAR, 32}
};
}  //  namespace vnode_rename

namespace vlink {
const DalColumnSchema vlink_schema[] =  {
  {"vtn_name", SQL_VARCHAR, SQL_C_CHAR, 32},
  {"vlink_name", SQL_VARCHAR, SQL_C_CHAR, 32},
  {"admin_status", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"vnode1_name", SQL_VARCHAR, SQL_C_CHAR, 32},
  {"vnode1_ifname", SQL_VARCHAR, SQL_C_CHAR, 32},
  {"vnode2_name", SQL_VARCHAR, SQL_C_CHAR, 32},
  {"vnode2_ifname", SQL_VARCHAR, SQL_C_CHAR, 32},
  {"boundary_name", SQL_VARCHAR, SQL_C_CHAR, 32},
  {"vlanid", SQL_INTEGER, SQL_C_ULONG, 1},
  {"description", SQL_VARCHAR, SQL_C_CHAR, 128},
  {"oper_status", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"down_count", SQL_BIGINT, SQL_C_UBIGINT, 1},
  {"controller1_name", SQL_VARCHAR, SQL_C_CHAR, 32},
  {"controller2_name", SQL_VARCHAR, SQL_C_CHAR, 32},
  {"domain1_id", SQL_VARCHAR, SQL_C_CHAR, 32},
  {"domain2_id", SQL_VARCHAR, SQL_C_CHAR, 32},
  {"key_flags", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"val_flags", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"valid_admin_status", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"valid_vnode1_name", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"valid_vnode1_ifname", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"valid_vnode2_name", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"valid_vnode2_ifname", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"valid_boundary_name", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"valid_vlanid", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"valid_description", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"valid_oper_status", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"cs_rowstatus", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"cs_admin_status", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"cs_vnode1_name", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"cs_vnode1_ifname", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"cs_vnode2_name", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"cs_vnode2_ifname", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"cs_boundary_name", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"cs_vlanid", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"cs_description", SQL_SMALLINT, SQL_C_SHORT, 1}
};
}  //  namespace vlink

namespace vlink_rename {
const DalColumnSchema vlink_rename_schema[] =  {
  {"ctrlr_vtn_name", SQL_VARCHAR, SQL_C_CHAR, 32},
  {"ctrlr_vlink_name", SQL_VARCHAR, SQL_C_CHAR, 32},
  {"controller_name", SQL_VARCHAR, SQL_C_CHAR, 32},
  {"domain_id", SQL_VARCHAR, SQL_C_CHAR, 32},
  {"unc_vtn_name", SQL_VARCHAR, SQL_C_CHAR, 32},
  {"unc_vlink_name", SQL_VARCHAR, SQL_C_CHAR, 32}
};
}  //  namespace vlink_rename

namespace static_ip_route {
const DalColumnSchema static_ip_route_schema[] =  {
  {"vtn_name", SQL_VARCHAR, SQL_C_CHAR, 32},
  {"vrouter_name", SQL_VARCHAR, SQL_C_CHAR, 32},
  {"dst_ip_addr", SQL_BINARY, SQL_C_BINARY, 4},
  {"mask", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"next_hop_addr", SQL_BINARY, SQL_C_BINARY, 4},
  {"nwm_name", SQL_VARCHAR, SQL_C_CHAR, 32},
  {"metric", SQL_INTEGER, SQL_C_ULONG, 1},
  {"controller_name", SQL_VARCHAR, SQL_C_CHAR, 32},
  {"domain_id", SQL_VARCHAR, SQL_C_CHAR, 32},
  {"flags", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"valid_metric", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"cs_rowstatus", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"cs_metric", SQL_SMALLINT, SQL_C_SHORT, 1}
};
}  //  namespace static_ip_route

namespace dhcprelay_server {
const DalColumnSchema dhcprelay_server_schema[] =  {
  {"vtn_name", SQL_VARCHAR, SQL_C_CHAR, 32},
  {"vrouter_name", SQL_VARCHAR, SQL_C_CHAR, 32},
  {"server_ip_addr", SQL_BINARY, SQL_C_BINARY, 4},
  {"controller_name", SQL_VARCHAR, SQL_C_CHAR, 32},
  {"domain_id", SQL_VARCHAR, SQL_C_CHAR, 32},
  {"flags", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"cs_rowstatus", SQL_SMALLINT, SQL_C_SHORT, 1}
};
}  //  namespace dhcprelay_server

namespace dhcprelay_interface {
const DalColumnSchema dhcprelay_interface_schema[] =  {
  {"vtn_name", SQL_VARCHAR, SQL_C_CHAR, 32},
  {"vrouter_name", SQL_VARCHAR, SQL_C_CHAR, 32},
  {"if_name", SQL_VARCHAR, SQL_C_CHAR, 32},
  {"controller_name", SQL_VARCHAR, SQL_C_CHAR, 32},
  {"domain_id", SQL_VARCHAR, SQL_C_CHAR, 32},
  {"flags", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"cs_rowstatus", SQL_SMALLINT, SQL_C_SHORT, 1}
};
}  //  namespace dhcprelay_interface

namespace vbridge_networkmonitor_group {
const DalColumnSchema vbridge_networkmonitor_group_schema[] =  {
  {"vtn_name", SQL_VARCHAR, SQL_C_CHAR, 32},
  {"vbridge_name", SQL_VARCHAR, SQL_C_CHAR, 32},
  {"nwm_name", SQL_VARCHAR, SQL_C_CHAR, 32},
  {"admin_status", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"oper_status", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"controller_name", SQL_VARCHAR, SQL_C_CHAR, 32},
  {"domain_id", SQL_VARCHAR, SQL_C_CHAR, 32},
  {"flags", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"valid_admin_status", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"valid_oper_status", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"cs_rowstatus", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"cs_admin_status", SQL_SMALLINT, SQL_C_SHORT, 1}
};
}  //  namespace vbridge_networkmonitor_group

namespace vbridge_networkmonitor_host {
const DalColumnSchema vbridge_networkmonitor_host_schema[] =  {
  {"vtn_name", SQL_VARCHAR, SQL_C_CHAR, 32},
  {"vbridge_name", SQL_VARCHAR, SQL_C_CHAR, 32},
  {"nwm_name", SQL_VARCHAR, SQL_C_CHAR, 32},
  {"host_address", SQL_BINARY, SQL_C_BINARY, 4},
  {"health_interval", SQL_INTEGER, SQL_C_ULONG, 1},
  {"recovery_interval", SQL_INTEGER, SQL_C_ULONG, 1},
  {"failure_count", SQL_INTEGER, SQL_C_ULONG, 1},
  {"recovery_count", SQL_INTEGER, SQL_C_ULONG, 1},
  {"wait_time", SQL_INTEGER, SQL_C_ULONG, 1},
  {"controller_name", SQL_VARCHAR, SQL_C_CHAR, 32},
  {"domain_id", SQL_VARCHAR, SQL_C_CHAR, 32},
  {"flags", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"valid_health_interval", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"valid_recovery_interval", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"valid_failure_count", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"valid_recovery_count", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"valid_wait_time", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"cs_rowstatus", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"cs_health_interval", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"cs_recovery_interval", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"cs_failure_count", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"cs_recovery_count", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"cs_wait_time", SQL_SMALLINT, SQL_C_SHORT, 1}
};
}  //  namespace vbridge_networkmonitor_host

namespace vunknown {
const DalColumnSchema vunknown_schema[] =  {
  {"vtn_name", SQL_VARCHAR, SQL_C_CHAR, 32},
  {"vunknown_name", SQL_VARCHAR, SQL_C_CHAR, 32},
  {"description", SQL_VARCHAR, SQL_C_CHAR, 128},
  {"type", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"domain_id", SQL_VARCHAR, SQL_C_CHAR, 32},
  {"flags", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"valid_description", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"valid_type", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"valid_domain_id", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"cs_rowstatus", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"cs_description", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"cs_type", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"cs_domain_id", SQL_SMALLINT, SQL_C_SHORT, 1}
};
}  //  namespace vunknown

namespace vunknown_interface {
const DalColumnSchema vunknown_interface_schema[] =  {
  {"vtn_name", SQL_VARCHAR, SQL_C_CHAR, 32},
  {"vunknown_name", SQL_VARCHAR, SQL_C_CHAR, 32},
  {"if_name", SQL_VARCHAR, SQL_C_CHAR, 32},
  {"description", SQL_VARCHAR, SQL_C_CHAR, 128},
  {"admin_status", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"domain_id", SQL_VARCHAR, SQL_C_CHAR, 32},
  {"flags", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"valid_description", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"valid_admin_status", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"cs_rowstatus", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"cs_description", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"cs_admin_status", SQL_SMALLINT, SQL_C_SHORT, 1},
};
}  //  namespace vunknown_interface

namespace vtep {
const DalColumnSchema vtep_schema[] =  {
  {"vtn_name", SQL_VARCHAR, SQL_C_CHAR, 32},
  {"vtep_name", SQL_VARCHAR, SQL_C_CHAR, 32},
  {"description", SQL_VARCHAR, SQL_C_CHAR, 128},
  {"controller_name", SQL_VARCHAR, SQL_C_CHAR, 32},
  {"domain_id", SQL_VARCHAR, SQL_C_CHAR, 32},
  {"oper_status", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"down_count", SQL_BIGINT, SQL_C_UBIGINT, 1},
  {"flags", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"valid_description", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"valid_controller_name", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"valid_domain_id", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"valid_oper_status", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"cs_rowstatus", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"cs_description", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"cs_controller_name", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"cs_domain_id", SQL_SMALLINT, SQL_C_SHORT, 1}
};
}  //  namespace vtep

namespace vtep_interface {
const DalColumnSchema vtep_interface_schema[] =  {
  {"vtn_name", SQL_VARCHAR, SQL_C_CHAR, 32},
  {"vtep_name", SQL_VARCHAR, SQL_C_CHAR, 32},
  {"if_name", SQL_VARCHAR, SQL_C_CHAR, 32},
  {"description", SQL_VARCHAR, SQL_C_CHAR, 128},
  {"admin_status", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"logical_port_id", SQL_VARCHAR, SQL_C_CHAR, 320},
  {"vlanid", SQL_INTEGER, SQL_C_ULONG, 1},
  {"tagged", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"oper_status", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"down_count", SQL_BIGINT, SQL_C_UBIGINT, 1},
  {"controller_name", SQL_VARCHAR, SQL_C_CHAR, 32},
  {"domain_id", SQL_VARCHAR, SQL_C_CHAR, 32},
  {"flags", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"valid_description", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"valid_admin_status", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"valid_portmap", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"valid_logical_port_id", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"valid_vlanid", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"valid_tagged", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"valid_oper_status", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"cs_rowstatus", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"cs_description", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"cs_admin_status", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"cs_portmap", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"cs_logical_port_id", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"cs_vlanid", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"cs_tagged", SQL_SMALLINT, SQL_C_SHORT, 1}
};
}  //  namespace vtep_interface

namespace vtep_group {
const DalColumnSchema vtep_group_schema[] =  {
  {"vtn_name", SQL_VARCHAR, SQL_C_CHAR, 32},
  {"vtepgrp_name", SQL_VARCHAR, SQL_C_CHAR, 32},
  {"controller_name", SQL_VARCHAR, SQL_C_CHAR, 32},
  {"description", SQL_VARCHAR, SQL_C_CHAR, 128},
  {"flags", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"valid_controller_name", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"valid_description", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"cs_rowstatus", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"cs_controller_name", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"cs_description", SQL_SMALLINT, SQL_C_SHORT, 1},
};
}  //  namespace vtep_group

namespace vtep_groupmember {
const DalColumnSchema vtep_groupmember_schema[] =  {
  {"vtn_name", SQL_VARCHAR, SQL_C_CHAR, 32},
  {"vtepgrp_name", SQL_VARCHAR, SQL_C_CHAR, 32},
  {"vtepgrp_member_name", SQL_VARCHAR, SQL_C_CHAR, 32},
  {"controller_name", SQL_VARCHAR, SQL_C_CHAR, 32},
  {"domain_id", SQL_VARCHAR, SQL_C_CHAR, 32},
  {"flags", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"cs_rowstatus", SQL_SMALLINT, SQL_C_SHORT, 1},
};
}  //  namespace vtep_groupmember

namespace vtunnel {
const DalColumnSchema vtunnel_schema[] =  {
  {"vtn_name", SQL_VARCHAR, SQL_C_CHAR, 32},
  {"vtunnel_name", SQL_VARCHAR, SQL_C_CHAR, 32},
  {"description", SQL_VARCHAR, SQL_C_CHAR, 128},
  {"controller_name", SQL_VARCHAR, SQL_C_CHAR, 32},
  {"domain_id", SQL_VARCHAR, SQL_C_CHAR, 32},
  {"underlay_vtn_name", SQL_VARCHAR, SQL_C_CHAR, 32},
  {"vtepgrp_name ", SQL_VARCHAR, SQL_C_CHAR, 32},
  {"label", SQL_BIGINT, SQL_C_UBIGINT, 1},
  {"oper_status", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"down_count", SQL_BIGINT, SQL_C_UBIGINT, 1},
  {"flags", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"valid_description", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"valid_controller_name", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"valid_domain_id", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"valid_underlay_vtn_name", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"valid_vtepgrp_name", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"valid_label", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"valid_oper_status", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"cs_rowstatus", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"cs_description", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"cs_controller_name", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"cs_domain_id", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"cs_underlay_vtn_name", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"cs_vtepgrp_name", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"cs_label", SQL_SMALLINT, SQL_C_SHORT, 1}
};
}  //  namespace vtunnel

namespace vtunnel_interface {
const DalColumnSchema vtunnel_interface_schema[] =  {
  {"vtn_name", SQL_VARCHAR, SQL_C_CHAR, 32},
  {"vtunnel_name", SQL_VARCHAR, SQL_C_CHAR, 32},
  {"if_name", SQL_VARCHAR, SQL_C_CHAR, 32},
  {"description", SQL_VARCHAR, SQL_C_CHAR, 128},
  {"admin_status", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"logical_port_id", SQL_VARCHAR, SQL_C_CHAR, 320},
  {"vlanid", SQL_INTEGER, SQL_C_ULONG, 1},
  {"tagged", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"oper_status",  SQL_SMALLINT, SQL_C_SHORT, 1},
  {"down_count", SQL_BIGINT, SQL_C_UBIGINT, 1},
  {"controller_name", SQL_VARCHAR, SQL_C_CHAR, 32},
  {"domain_id", SQL_VARCHAR, SQL_C_CHAR, 32},
  {"flags",  SQL_SMALLINT, SQL_C_SHORT, 1},
  {"valid_description",  SQL_SMALLINT, SQL_C_SHORT, 1},
  {"valid_admin_status",  SQL_SMALLINT, SQL_C_SHORT, 1},
  {"valid_portmap", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"valid_logical_port_id", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"valid_vlanid", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"valid_tagged", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"valid_oper_status",  SQL_SMALLINT, SQL_C_SHORT, 1},
  {"cs_rowstatus",  SQL_SMALLINT, SQL_C_SHORT, 1},
  {"cs_description", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"cs_admin_status",  SQL_SMALLINT, SQL_C_SHORT, 1},
  {"cs_portmap", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"cs_logical_port_id", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"cs_vlanid", SQL_SMALLINT, SQL_C_SHORT, 1},
  {"cs_tagged", SQL_SMALLINT, SQL_C_SHORT, 1}
};
}  //  namespace vtunnel_interface

// L2/L3 MoMgrs Schema End

// POM MoMgrs Schema Start

namespace flowlist {
const DalColumnSchema flowlist_schema[] = {
  { "flowlist_name", SQL_VARCHAR, SQL_C_CHAR, 33 },
  { "ip_type", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "flags", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "valid_ip_type", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "cs_rowstatus", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "cs_ip_type", SQL_SMALLINT, SQL_C_SHORT, 1 }
};  // flowlist_schema
}  // namespace flowlist

namespace flowlist_ctrlr {
const DalColumnSchema flowlist_ctrlr_schema[] = {
  { "flowlist_name", SQL_VARCHAR, SQL_C_CHAR, 33 },
  { "ctrlr_name", SQL_VARCHAR, SQL_C_CHAR, 32 },
  { "ref_count", SQL_BIGINT, SQL_C_UBIGINT, 1 },
  { "flags", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "valid_ip_type", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "cs_rowstatus", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "cs_ip_type", SQL_SMALLINT, SQL_C_SHORT, 1 }
};
}  //  namespace flowlist_ctrlr

namespace flowlist_rename {
const DalColumnSchema flowlist_rename_schema[] = {
  { "ctrlr_flowlist_name", SQL_VARCHAR, SQL_C_CHAR, 33 },
  { "ctrlr_name", SQL_VARCHAR, SQL_C_CHAR, 32 },
  { "unc_flowlist_name", SQL_VARCHAR, SQL_C_CHAR, 33 }
};  // flowlist_rename_schema
}  // namespace flowlist_rename

namespace flowlist_entry {
const DalColumnSchema flowlist_entry_schema[] = {
  { "flowlist_name", SQL_VARCHAR, SQL_C_CHAR, 33 },
  { "sequence_num", SQL_INTEGER, SQL_C_ULONG, 1 },
  { "mac_dst", SQL_BINARY, SQL_C_BINARY, 6 },
  { "mac_src", SQL_BINARY, SQL_C_BINARY, 6 },
  { "mac_eth_type", SQL_INTEGER, SQL_C_ULONG, 1 },
  { "dst_ip", SQL_BIGINT, SQL_C_UBIGINT, 1 },
  { "dst_ip_prefix", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "src_ip", SQL_BIGINT, SQL_C_UBIGINT, 1 },
  { "src_ip_prefix", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "vlan_priority", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "dst_ipv6", SQL_BINARY, SQL_C_BINARY, 16 },
  { "dst_ipv6_prefix", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "src_ipv6", SQL_BINARY, SQL_C_BINARY, 16 },
  { "src_ipv6_prefix", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "ip_protocol", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "ip_dscp", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "l4_dst_port", SQL_INTEGER, SQL_C_ULONG, 1 },
  { "l4_dst_port_endpt", SQL_INTEGER, SQL_C_ULONG, 1 },
  { "l4_src_port", SQL_INTEGER, SQL_C_ULONG, 1 },
  { "l4_src_port_endpt", SQL_INTEGER, SQL_C_ULONG, 1 },
  { "icmp_type", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "icmp_code", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "icmpv6_type", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "icmpv6_code", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "flags", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "valid_mac_dst", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "valid_mac_src", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "valid_mac_eth_type", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "valid_dst_ip", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "valid_dst_ip_prefix", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "valid_src_ip", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "valid_src_ip_prefix", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "valid_vlan_priority", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "valid_dst_ipv6", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "valid_dst_ipv6_prefix", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "valid_src_ipv6", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "valid_src_ipv6_prefix", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "valid_ip_protocol", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "valid_ip_dscp", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "valid_l4_dst_port", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "valid_l4_dst_port_endpt", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "valid_l4_src_port", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "valid_l4_src_port_endpt", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "valid_icmp_type", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "valid_icmp_code", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "valid_icmpv6_type", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "valid_icmpv6_code", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "cs_rowstatus", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "cs_mac_dst", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "cs_mac_src", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "cs_mac_eth_type", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "cs_dst_ip", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "cs_dst_ip_prefix", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "cs_src_ip", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "cs_src_ip_prefix", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "cs_vlan_priority", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "cs_dst_ipv6", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "cs_dst_ipv6_prefix", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "cs_src_ipv6", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "cs_src_ipv6_prefix", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "cs_ip_protocol", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "cs_ip_dscp", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "cs_l4_dst_port", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "cs_l4_dst_port_endpt", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "cs_l4_src_port", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "cs_l4_src_port_endpt", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "cs_icmp_type", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "cs_icmp_code", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "cs_icmpv6_type", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "cs_icmpv6_code", SQL_SMALLINT, SQL_C_SHORT, 1 }
};  // flowlist_entry_schema
}  // namespace flowlist_entry


namespace flowlist_entry_ctrlr {
const DalColumnSchema flowlist_entry_ctrlr_schema[] = {
  { "flowlist_name", SQL_VARCHAR, SQL_C_CHAR, 33 },
  { "sequence_num", SQL_INTEGER, SQL_C_ULONG, 1 },
  { "ctrlr_name", SQL_VARCHAR, SQL_C_CHAR, 32 },
  { "flags", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "valid_mac_dst", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "valid_mac_src", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "valid_mac_eth_type", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "valid_dst_ip", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "valid_dst_ip_prefix", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "valid_src_ip", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "valid_src_ip_prefix", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "valid_vlan_priority", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "valid_dst_ipv6", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "valid_dst_ipv6_prefix", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "valid_src_ipv6", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "valid_src_ipv6_prefix", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "valid_ip_protocol", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "valid_ip_dscp", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "valid_l4_dst_port", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "valid_l4_dst_port_endpt", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "valid_l4_src_port", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "valid_l4_src_port_endpt", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "valid_icmp_type", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "valid_icmp_code", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "valid_icmpv6_type", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "valid_icmpv6_code", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "cs_rowstatus", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "cs_mac_dst", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "cs_mac_src", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "cs_mac_eth_type", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "cs_dst_ip", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "cs_dst_ip_prefix", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "cs_src_ip", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "cs_src_ip_prefix", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "cs_vlan_priority", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "cs_dst_ipv6", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "cs_dst_ipv6_prefix", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "cs_src_ipv6", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "cs_src_ipv6_prefix", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "cs_ip_protocol", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "cs_ip_dscp", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "cs_l4_dst_port", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "cs_l4_dst_port_endpt", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "cs_l4_src_port", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "cs_l4_src_port_endpt", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "cs_icmp_type", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "cs_icmp_code", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "cs_icmpv6_type", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "cs_icmpv6_code", SQL_SMALLINT, SQL_C_SHORT, 1 }
};  // flowlist_entry_ctrlr_schema
}  // namespace flowlist_entry_ctrlr

namespace policingprofile {
const DalColumnSchema policingprofile_schema[] = {
  { "policingprofile_name", SQL_VARCHAR, SQL_C_CHAR, 33 },
  { "flags", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "cs_rowstatus", SQL_SMALLINT, SQL_C_SHORT, 1 }
};  // policingprofile_schema
}  // namespace policingprofile

namespace policingprofile_ctrlr {
const DalColumnSchema policingprofile_ctrlr_schema[] = {
  { "policingprofile_name", SQL_VARCHAR, SQL_C_CHAR, 33 },
  { "ctrlr_name", SQL_VARCHAR, SQL_C_CHAR, 32 },
  { "ref_count", SQL_BIGINT, SQL_C_UBIGINT, 1 },
  { "flags", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "cs_rowstatus", SQL_SMALLINT, SQL_C_SHORT, 1 }
};  // policingprofile_ctrlr_schema
}  // namespace policingprofile_ctrlr

namespace policingprofile_rename {
const DalColumnSchema policingprofile_rename_schema[] = {
  { "ctrlr_policingprofile_name", SQL_VARCHAR, SQL_C_CHAR, 33 },
  { "ctrlr_name", SQL_VARCHAR, SQL_C_CHAR, 32 },
  { "unc_policingprofile_name", SQL_VARCHAR, SQL_C_CHAR, 33 }
};  // policingprofile_rename_schema
}  // namespace policingprofile_rename

namespace policingprofile_entry {
const DalColumnSchema policingprofile_entry_schema[] = {
  { "policingprofile_name", SQL_VARCHAR, SQL_C_CHAR, 33 },
  { "sequence_num", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "flowlist", SQL_VARCHAR, SQL_C_CHAR, 33 },
  { "rate", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "cir", SQL_BIGINT, SQL_C_UBIGINT, 1 },
  { "cbs", SQL_BIGINT, SQL_C_UBIGINT, 1 },
  { "pir", SQL_BIGINT, SQL_C_UBIGINT, 1 },
  { "pbs", SQL_BIGINT, SQL_C_UBIGINT, 1 },
  { "green_action", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "green_priority", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "green_dscp", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "green_drop", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "yellow_action", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "yellow_priority", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "yellow_dscp", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "yellow_drop", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "red_action", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "red_priority", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "red_dscp", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "red_drop", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "flags", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "valid_flowlist", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "valid_rate", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "valid_cir", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "valid_cbs", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "valid_pir", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "valid_pbs", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "valid_green_action", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "valid_green_priority", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "valid_green_dscp", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "valid_green_drop", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "valid_yellow_action", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "valid_yellow_priority", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "valid_yellow_dscp", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "valid_yellow_drop", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "valid_red_action", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "valid_red_priority", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "valid_red_dscp", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "valid_red_drop", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "cs_rowstatus", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "cs_flowlist", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "cs_rate", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "cs_cir", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "cs_cbs", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "cs_pir", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "cs_pbs", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "cs_green_action", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "cs_green_priority", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "cs_green_dscp", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "cs_green_drop", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "cs_yellow_action", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "cs_yellow_priority", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "cs_yellow_dscp", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "cs_yellow_drop", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "cs_red_action", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "cs_red_priority", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "cs_red_dscp", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "cs_red_drop", SQL_SMALLINT, SQL_C_SHORT, 1 }
};  // polcingprofile_entry_schema
}  // namespace policingprofile_entry

namespace policingprofile_entry_ctrlr {
const DalColumnSchema policingprofile_entry_ctrlr_schema[] = {
  { "policingprofile_name", SQL_VARCHAR, SQL_C_CHAR, 33 },
  { "sequence_num", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "ctrlr_name", SQL_VARCHAR, SQL_C_CHAR, 32 },
  { "flags", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "valid_flowlist", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "valid_rate", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "valid_cir", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "valid_cbs", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "valid_pir", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "valid_pbs", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "valid_green_action", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "valid_green_priority", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "valid_green_dscp", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "valid_green_drop", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "valid_yellow_action", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "valid_yellow_priority", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "valid_yellow_dscp", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "valid_yellow_drop", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "valid_red_action", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "valid_red_priority", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "valid_red_dscp", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "valid_red_drop", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "cs_rowstatus", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "cs_flowlist", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "cs_rate", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "cs_cir", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "cs_cbs", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "cs_pir", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "cs_pbs", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "cs_green_action", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "cs_green_priority", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "cs_green_dscp", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "cs_green_drop", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "cs_yellow_action", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "cs_yellow_priority", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "cs_yellow_dscp", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "cs_yellow_drop", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "cs_red_action", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "cs_red_priority", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "cs_red_dscp", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "cs_red_drop", SQL_SMALLINT, SQL_C_SHORT, 1 }
};  // polcingprofile_entry_ctrlr_schema
}  // namespace policingprofile_entry_ctrlr

namespace vtn_flowfilter {
const DalColumnSchema vtn_flowfilter_schema[] = {
  { "vtn_name", SQL_VARCHAR, SQL_C_CHAR, 32 },
  { "direction", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "flags", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "cs_rowstatus", SQL_SMALLINT, SQL_C_SHORT, 1 }
};  // vtn_flowfilter_schema
}  // namespace vtn_flowfilter

namespace vtn_flowfilter_ctrlr {
const DalColumnSchema vtn_flowfilter_ctrlr_schema[] = {
  { "vtn_name", SQL_VARCHAR, SQL_C_CHAR, 32 },
  { "direction", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "ctrlr_name", SQL_VARCHAR, SQL_C_CHAR, 32 },
  { "domain_id", SQL_VARCHAR, SQL_C_CHAR, 32 },
  { "flags", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "cs_rowstatus", SQL_SMALLINT, SQL_C_SHORT, 1 }
};  // vtn_flowfilter_ctrlr_schema
}  // namespace vtn_flowfilter_ctrlr

namespace vtn_flowfilter_entry {
const DalColumnSchema vtn_flowfilter_entry_schema[] = {
  { "vtn_name", SQL_VARCHAR, SQL_C_CHAR, 32 },
  { "direction", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "sequence_num", SQL_INTEGER, SQL_C_ULONG, 1 },
  { "flowlist_name", SQL_VARCHAR, SQL_C_CHAR, 33 },
  { "action", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "nwm_name", SQL_VARCHAR, SQL_C_CHAR, 32 },
  { "dscp", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "priority", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "flags", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "valid_flowlist_name", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "valid_action", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "valid_nwn_name", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "valid_dscp", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "valid_priority", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "cs_rowstatus", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "cs_flowlist_name", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "cs_action", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "cs_nwn_name", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "cs_dscp", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "cs_priority", SQL_SMALLINT, SQL_C_SHORT, 1 }
};  // vtn_flowfilter_entry_schema
}  // namespace vtn_flowfilter_entry

namespace vtn_flowfilter_entry_ctrlr {
const DalColumnSchema vtn_flowfilter_entry_ctrlr_schema[] = {
  { "vtn_name", SQL_VARCHAR, SQL_C_CHAR, 32 },
  { "direction", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "sequence_num", SQL_INTEGER, SQL_C_ULONG, 1 },
  { "ctrlr_name", SQL_VARCHAR, SQL_C_CHAR, 32 },
  { "domain_id", SQL_VARCHAR, SQL_C_CHAR, 32 },
  { "flags", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "valid_flowlist_name", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "valid_action", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "valid_nwn_name", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "valid_dscp", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "valid_priority", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "cs_rowstatus", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "cs_flowlist_name", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "cs_action", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "cs_nwn_name", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "cs_dscp", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "cs_priority", SQL_SMALLINT, SQL_C_SHORT, 1 }
};  // vtn_flowfilter_entry_ctrlr_schema
}  // namespace vtn_flowfilter_entry_ctrlr

namespace vbr_flowfilter {
const DalColumnSchema vbr_flowfilter_schema[] = {
  { "vtn_name", SQL_VARCHAR, SQL_C_CHAR, 32 },
  { "vbr_name", SQL_VARCHAR, SQL_C_CHAR, 32 },
  { "direction", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "ctrlr_name", SQL_VARCHAR, SQL_C_CHAR, 32 },
  { "domain_id", SQL_VARCHAR, SQL_C_CHAR, 32 },
  { "flags", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "cs_rowstatus", SQL_SMALLINT, SQL_C_SHORT, 1 }
};  // vbr_flowfilter_schema
}  // namespace vbr_flowfilter

namespace vbr_flowfilter_entry {
const DalColumnSchema vbr_flowfilter_entry_schema[] = {
  { "vtn_name", SQL_VARCHAR, SQL_C_CHAR, 32 },
  { "vbr_name", SQL_VARCHAR, SQL_C_CHAR, 32 },
  { "direction", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "sequence_num", SQL_INTEGER, SQL_C_ULONG, 1 },
  { "ctrlr_name", SQL_VARCHAR, SQL_C_CHAR, 32 },
  { "domain_id", SQL_VARCHAR, SQL_C_CHAR, 32 },
  { "flowlist_name", SQL_VARCHAR, SQL_C_CHAR, 33 },
  { "action", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "redirect_node", SQL_VARCHAR, SQL_C_CHAR, 32 },
  { "redirect_port", SQL_VARCHAR, SQL_C_CHAR, 32 },
  { "modify_dst_mac", SQL_BINARY, SQL_C_BINARY, 6 },
  { "modify_src_mac", SQL_BINARY, SQL_C_BINARY, 6 },
  { "nwm_name", SQL_VARCHAR, SQL_C_CHAR, 32 },
  { "dscp", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "priority", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "flags", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "valid_flowlist_name", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "valid_action", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "valid_redirect_node", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "valid_redirect_port", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "valid_modify_dst_mac", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "valid_modify_src_mac", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "valid_nwn_name", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "valid_dscp", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "valid_priority", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "cs_rowstatus", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "cs_flowlist_name", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "cs_action", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "cs_redirect_node", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "cs_redirect_port", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "cs_modify_dst_mac", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "cs_modify_src_mac", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "cs_nwm_name", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "cs_dscp", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "cs_priority", SQL_SMALLINT, SQL_C_SHORT, 1 }
};  // vbr_flowfilter_entry_schema
}  // namespace vbr_flowfilter_entry

namespace vbr_if_flowfilter {
const DalColumnSchema vbr_if_flowfilter_schema[] = {
  { "vtn_name", SQL_VARCHAR, SQL_C_CHAR, 32 },
  { "vbr_name", SQL_VARCHAR, SQL_C_CHAR, 32 },
  { "vbr_if_name", SQL_VARCHAR, SQL_C_CHAR, 32 },
  { "direction", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "ctrlr_name", SQL_VARCHAR, SQL_C_CHAR, 32 },
  { "domain_id", SQL_VARCHAR, SQL_C_CHAR, 32 },
  { "flags", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "cs_rowstatus", SQL_SMALLINT, SQL_C_SHORT, 1 },
};  // vbr_if_flowfilter_schema
}  // namespace vbr_if_flowfilter

namespace vbr_if_flowfilter_entry {
const DalColumnSchema vbr_if_flowfilter_entry_schema[] = {
  { "vtn_name", SQL_VARCHAR, SQL_C_CHAR, 32 },
  { "vbr_name", SQL_VARCHAR, SQL_C_CHAR, 32 },
  { "vbr_if_name", SQL_VARCHAR, SQL_C_CHAR, 32 },
  { "direction", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "sequence_num", SQL_INTEGER, SQL_C_ULONG, 1 },
  { "ctrlr_name", SQL_VARCHAR, SQL_C_CHAR, 32 },
  { "domain_id", SQL_VARCHAR, SQL_C_CHAR, 32 },
  { "flowlist_name", SQL_VARCHAR, SQL_C_CHAR, 33 },
  { "action", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "redirect_node", SQL_VARCHAR, SQL_C_CHAR, 32 },
  { "redirect_port", SQL_VARCHAR, SQL_C_CHAR, 32 },
  { "modify_dst_mac", SQL_BINARY, SQL_C_BINARY, 6 },
  { "modify_src_mac", SQL_BINARY, SQL_C_BINARY, 6 },
  { "nwm_name", SQL_VARCHAR, SQL_C_CHAR, 32 },
  { "dscp", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "priority", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "flags", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "valid_flowlist_name", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "valid_action", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "valid_redirect_node", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "valid_redirect_port", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "valid_modify_dst_mac", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "valid_modify_src_mac", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "valid_nwn_name", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "valid_dscp", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "valid_priority", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "cs_rowstatus", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "cs_flowlist_name", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "cs_action", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "cs_redirect_node", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "cs_redirect_port", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "cs_modify_dst_mac", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "cs_modify_src_mac", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "cs_nwm_name", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "cs_dscp", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "cs_priority", SQL_SMALLINT, SQL_C_SHORT, 1 }
};  // vbr_if_flowfilter_entry_schema
}  // namespace vbr_if_flowfilter_entry

namespace vrt_if_flowfilter {
const DalColumnSchema vrt_if_flowfilter_schema[] = {
  { "vtn_name", SQL_VARCHAR, SQL_C_CHAR, 32 },
  { "vrt_name", SQL_VARCHAR, SQL_C_CHAR, 32 },
  { "vrt_if_name", SQL_VARCHAR, SQL_C_CHAR, 32 },
  { "direction", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "ctrlr_name", SQL_VARCHAR, SQL_C_CHAR, 32 },
  { "domain_id", SQL_VARCHAR, SQL_C_CHAR, 32 },
  { "flags", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "cs_rowstatus", SQL_SMALLINT, SQL_C_SHORT, 1 }
};  // vrt_if_flowfilter_schema
}  // namespace vrt_if_flowfilter

namespace vrt_if_flowfilter_entry {
const DalColumnSchema vrt_if_flowfilter_entry_schema[] = {
  { "vtn_name", SQL_VARCHAR, SQL_C_CHAR, 32 },
  { "vrt_name", SQL_VARCHAR, SQL_C_CHAR, 32 },
  { "vrt_if_name", SQL_VARCHAR, SQL_C_CHAR, 32 },
  { "direction", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "sequence_num", SQL_INTEGER, SQL_C_ULONG, 1 },
  { "ctrlr_name", SQL_VARCHAR, SQL_C_CHAR, 32 },
  { "domain_id", SQL_VARCHAR, SQL_C_CHAR, 32 },
  { "flowlist_name", SQL_VARCHAR, SQL_C_CHAR, 33 },
  { "action", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "redirect_node", SQL_VARCHAR, SQL_C_CHAR, 32 },
  { "redirect_port", SQL_VARCHAR, SQL_C_CHAR, 32 },
  { "modify_dst_mac", SQL_BINARY, SQL_C_BINARY, 6 },
  { "modify_src_mac", SQL_BINARY, SQL_C_BINARY, 6 },
  { "nwm_name", SQL_VARCHAR, SQL_C_CHAR, 32 },
  { "dscp", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "priority", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "flags", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "valid_flowlist_name", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "valid_action", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "valid_redirect_node", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "valid_redirect_port", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "valid_modify_dst_mac", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "valid_modify_src_mac", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "valid_nwn_name", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "valid_dscp", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "valid_priority", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "cs_rowstatus", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "cs_flowlist_name", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "cs_action", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "cs_redirect_node", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "cs_redirect_port", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "cs_modify_dst_mac", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "cs_modify_src_mac", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "cs_nwm_name", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "cs_dscp", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "cs_priority", SQL_SMALLINT, SQL_C_SHORT, 1 }
};  // vrt_if_flowfilter_entry_schema
}  // namespace vrt_if_flowfilter_entry

namespace vtn_policingmap {
const DalColumnSchema vtn_policingmap_schema[] = {
  { "vtn_name", SQL_VARCHAR, SQL_C_CHAR, 32 },
  { "policername", SQL_VARCHAR, SQL_C_CHAR, 33 },
  { "flags", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "valid_policername", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "cs_rowstatus", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "cs_policername", SQL_SMALLINT, SQL_C_SHORT, 1 }
};  // vtn_policingmap_schema
}  // namespace vtn_policingmap


namespace vtn_policingmap_ctrlr {
const DalColumnSchema vtn_policingmap_ctrlr_schema[] = {
  { "vtn_name", SQL_VARCHAR, SQL_C_CHAR, 32 },
  { "ctrlr_name", SQL_VARCHAR, SQL_C_CHAR, 32 },
  { "domain_id", SQL_VARCHAR, SQL_C_CHAR, 32 },
  { "policername", SQL_VARCHAR, SQL_C_CHAR, 33 },
  { "flags", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "valid_policername", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "cs_rowstatus", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "cs_policername", SQL_SMALLINT, SQL_C_SHORT, 1 }
};  // vtn_policingmap_ctrlr_schema
}  // namespace vtn_policingmap_ctrlr

namespace vbr_policingmap {
const DalColumnSchema vbr_policingmap_schema[] = {
  { "vtn_name", SQL_VARCHAR, SQL_C_CHAR, 32 },
  { "vbr_name", SQL_VARCHAR, SQL_C_CHAR, 32 },
  { "ctrlr_name", SQL_VARCHAR, SQL_C_CHAR, 32 },
  { "domain_id", SQL_VARCHAR, SQL_C_CHAR, 32 },
  { "policername", SQL_VARCHAR, SQL_C_CHAR, 33 },
  { "flags", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "valid_policername", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "cs_rowstatus", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "cs_policername", SQL_SMALLINT, SQL_C_SHORT, 1 }
};  // vbr_policingmap_schema
}  // namespace vbr_policingmap

namespace vbr_if_policingmap {
const DalColumnSchema vbr_if_policingmap_schema[] = {
  { "vtn_name", SQL_VARCHAR, SQL_C_CHAR, 32 },
  { "vbr_name", SQL_VARCHAR, SQL_C_CHAR, 32 },
  { "vbr_if_name", SQL_VARCHAR, SQL_C_CHAR, 32 },
  { "ctrlr_name", SQL_VARCHAR, SQL_C_CHAR, 32 },
  { "domain_id", SQL_VARCHAR, SQL_C_CHAR, 32 },
  { "policername", SQL_VARCHAR, SQL_C_CHAR, 33 },
  { "flags", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "valid_policername", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "cs_rowstatus", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "cs_policername", SQL_SMALLINT, SQL_C_SHORT, 1 }
};  // vbr_policingmap_schema
}  // namespace vbr_if_policingmap

// POM MoMgrs Schema End

namespace ctrlr {
const DalColumnSchema ctrlr_schema[] = {
  { "name", SQL_VARCHAR, SQL_C_CHAR, 32 },
  { "type", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "version", SQL_VARCHAR, SQL_C_CHAR, 32 },
  { "audit_done", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "config_done", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "invalid_config", SQL_SMALLINT, SQL_C_SHORT, 1 },
  { "state", SQL_SMALLINT, SQL_C_SHORT, 1 }
};
}  // namespace ctrlr

const DalTableSchema table_schema[] = {
  // L2/L3 MoMgr table Schema Info Start
  {"vtn_tbl",
    vtn::kDbiVtnNumCols,
    vtn::kVtnNumPks,
    vtn::vtn_schema},
  {"vtn_ctrlr_tbl",
    vtn_controller::kDbiVtnCtrlrNumCols,
    vtn_controller::kVtnCtrlrNumPks,
    vtn_controller::vtn_controller_schema},
  {"vtn_rename_tbl",
    vtn_rename::kDbiVtnRenameNumCols,
    vtn_rename::kVtnRenameNumPks,
    vtn_rename::vtn_rename_schema},
  {"vbr_tbl",
    vbridge::kDbiVbrNumCols,
    vbridge::kVbrNumPks,
    vbridge::vbridge_schema},
  {"vbr_vlanmap_tbl",
    vbridge_vlanmap::kDbiVbrVlanMapNumCols,
    vbridge_vlanmap::kVbrVlanMapNumPks,
    vbridge_vlanmap::vbridge_vlanmap_schema},
  {"vbr_if_tbl",
    vbridge_interface::kDbiVbrIfNumCols,
    vbridge_interface::kVbrIfNumPks,
    vbridge_interface::vbridge_interface_schema},
  {"vrt_tbl",
    vrouter::kDbiVrtNumCols,
    vrouter::kVrtNumPks,
    vrouter::vrouter_schema},
  {"vrt_if_tbl",
    vrouter_interface::kDbiVrtIfNumCols,
    vrouter_interface::kVrtIfNumPks,
    vrouter_interface::vrouter_interface_schema},
  {"vnode_rename_tbl",
    vnode_rename::kDbiVnodeRenameNumCols,
    vnode_rename::kVnodeRenameNumPks,
    vnode_rename::vnode_rename_schema},
  {"vlink_tbl",
    vlink::kDbiVlinkNumCols,
    vlink::kVlinkNumPks,
    vlink::vlink_schema},
  {"vlink_rename_tbl",
    vlink_rename::kDbiVlinkRenameNumCols,
    vlink_rename::kVlinkRenameNumPks,
    vlink_rename::vlink_rename_schema},
  {"static_ip_route_tbl",
    static_ip_route::kDbiStaticIpRouteNumCols,
    static_ip_route::kStaticIpRouteNumPks,
    static_ip_route::static_ip_route_schema},
  {"dhcp_relay_server_tbl",
    dhcprelay_server::kDbiDhcpRelayServerNumCols,
    dhcprelay_server::kDhcpRelayServerNumPks,
    dhcprelay_server::dhcprelay_server_schema},
  {"dhcp_relay_if_tbl",
    dhcprelay_interface::kDbiDhcpRelayIfNumCols,
    dhcprelay_interface::kDhcpRelayIfNumPks,
    dhcprelay_interface::dhcprelay_interface_schema},
  {"vbr_nwmon_grp_tbl",
    vbridge_networkmonitor_group::kDbiVbrNwMonGrpNumCols,
    vbridge_networkmonitor_group::kVbrNwMonGrpNumPks,
    vbridge_networkmonitor_group::vbridge_networkmonitor_group_schema},
  {"vbr_nwmon_host_tbl",
    vbridge_networkmonitor_host::kDbiVbrNwMonHostNumCols,
    vbridge_networkmonitor_host::kVbrNwMonHostNumPks,
    vbridge_networkmonitor_host::vbridge_networkmonitor_host_schema},
  {"vunknown_tbl",
    vunknown::kDbiVunknownNumCols,
    vunknown::kVunknownNumPks,
    vunknown::vunknown_schema},
  {"vunknown_if_tbl",
    vunknown_interface::kDbiVunknownIfNumCols,
    vunknown_interface::kVunknownIfNumPks,
    vunknown_interface::vunknown_interface_schema},
  {"vtep_tbl",
    vtep::kDbiVtepNumCols,
    vtep::kVtepNumPks,
    vtep::vtep_schema},
  {"vtep_if_tbl",
    vtep_interface::kDbiVtepIfNumCols,
    vtep_interface::kVtepIfNumPks,
    vtep_interface::vtep_interface_schema},
  {"vtep_grp_tbl",
    vtep_group::kDbiVtepGrpNumCols,
    vtep_group::kVtepGrpNumPks,
    vtep_group::vtep_group_schema},
  {"vtep_grp_mem_tbl",
    vtep_groupmember::kDbiVtepGrpMemNumCols,
    vtep_groupmember::kVtepGrpMemNumPks,
    vtep_groupmember::vtep_groupmember_schema},
  {"vtunnel_tbl",
    vtunnel::kDbiVtunnelNumCols,
    vtunnel::kVtunnelNumPks,
    vtunnel::vtunnel_schema},
  {"vtunnel_if_tbl",
    vtunnel_interface::kDbiVtunnelIfNumCols,
    vtunnel_interface::kVtunnelIfNumPks,
    vtunnel_interface::vtunnel_interface_schema},

  // L2/L3 MoMgr table Schema Info End

  // POM MoMgr table Schema Info Start

  {"flowlist_tbl",
    flowlist::kDbiFlowListNumCols,
    flowlist::kFlowListNumPks,
    flowlist::flowlist_schema},
  {"flowlist_ctrlr_tbl",
    flowlist_ctrlr::kDbiFlowListCtrlrNumCols,
    flowlist_ctrlr::kFlowListCtrlrNumPks,
    flowlist_ctrlr::flowlist_ctrlr_schema},
  {"flowlist_rename_tbl",
    flowlist_rename::kDbiFlowListRenameNumCols,
    flowlist_rename::kFlowListRenameNumPks,
    flowlist_rename::flowlist_rename_schema},
  {"flowlist_entry_tbl",
    flowlist_entry::kDbiFlowListEntryNumCols,
    flowlist_entry::kFlowListEntryNumPks,
    flowlist_entry::flowlist_entry_schema},
  {"flowlist_entry_ctrlr_tbl",
    flowlist_entry_ctrlr::kDbiFlowListEntryCtrlrNumCols,
    flowlist_entry_ctrlr::kFlowListEntryCtrlrNumPks,
    flowlist_entry_ctrlr::flowlist_entry_ctrlr_schema},
  {"policingprofile_tbl",
    policingprofile::kDbiPolicingProfileNumCols,
    policingprofile::kPolicingProfileNumPks,
    policingprofile::policingprofile_schema},
  {"policingprofile_ctrlr_tbl",
    policingprofile_ctrlr::kDbiPolicingProfileCtrlrNumCols,
    policingprofile_ctrlr::kPolicingProfileCtrlrNumPks,
    policingprofile_ctrlr::policingprofile_ctrlr_schema},
  {"policingprofile_rename_tbl",
    policingprofile_rename::kDbiPolicingProfileRenameNumCols,
    policingprofile_rename::kPolicingProfileRenameNumPks,
    policingprofile_rename::policingprofile_rename_schema},
  {"policingprofile_entry_tbl",
    policingprofile_entry::kDbiPolicingProfileEntryNumCols,
    policingprofile_entry::kPolicingProfileEntryNumPks,
    policingprofile_entry::policingprofile_entry_schema},
  {"policingprofile_entry_ctrlr_tbl",
    policingprofile_entry_ctrlr::kDbiPolicingProfileEntryCtrlrNumCols,
    policingprofile_entry_ctrlr::kPolicingProfileEntryCtrlrNumPks,
    policingprofile_entry_ctrlr::policingprofile_entry_ctrlr_schema},
  {"vtn_flowfilter_tbl",
    vtn_flowfilter::kDbiVtnFlowFilterNumCols,
    vtn_flowfilter::kVtnFlowFilterNumPks,
    vtn_flowfilter::vtn_flowfilter_schema},
  {"vtn_flowfilter_ctrlr_tbl",
    vtn_flowfilter_ctrlr::kDbiVtnFlowFilterCtrlrNumCols,
    vtn_flowfilter_ctrlr::kVtnFlowFilterCtrlrNumPks,
    vtn_flowfilter_ctrlr::vtn_flowfilter_ctrlr_schema},
  {"vtn_flowfilter_entry_tbl",
    vtn_flowfilter_entry::kDbiVtnFlowFilterEntryNumCols,
    vtn_flowfilter_entry::kVtnFlowFilterEntryNumPks,
    vtn_flowfilter_entry::vtn_flowfilter_entry_schema},
  {"vtn_flowfilter_entry_ctrlr_tbl",
    vtn_flowfilter_entry_ctrlr::kDbiVtnFlowFilterEntryCtrlrNumCols,
    vtn_flowfilter_entry_ctrlr::kVtnFlowFilterEntryCtrlrNumPks,
    vtn_flowfilter_entry_ctrlr::vtn_flowfilter_entry_ctrlr_schema},
  {"vbr_flowfilter_tbl",
    vbr_flowfilter::kDbiVbrFlowFilterNumCols,
    vbr_flowfilter::kVbrFlowFilterNumPks,
    vbr_flowfilter::vbr_flowfilter_schema},
  {"vbr_flowfilter_entry_tbl",
    vbr_flowfilter_entry::kDbiVbrFlowFilterEntryNumCols,
    vbr_flowfilter_entry::kVbrFlowFilterEntryNumPks,
    vbr_flowfilter_entry::vbr_flowfilter_entry_schema},
  {"vbr_if_flowfilter_tbl",
    vbr_if_flowfilter::kDbiVbrIfFlowFilterNumCols,
    vbr_if_flowfilter::kVbrIfFlowFilterNumPks,
    vbr_if_flowfilter::vbr_if_flowfilter_schema},
  {"vbr_if_flowfilter_entry_tbl",
    vbr_if_flowfilter_entry::kDbiVbrIfFlowFilterEntryNumCols,
    vbr_if_flowfilter_entry::kVbrIfFlowFilterEntryNumPks,
    vbr_if_flowfilter_entry::vbr_if_flowfilter_entry_schema},
  {"vrt_if_flowfilter_tbl",
    vrt_if_flowfilter::kDbiVrtIfFlowFilterNumCols,
    vrt_if_flowfilter::kVrtIfFlowFilterNumPks,
    vrt_if_flowfilter::vrt_if_flowfilter_schema},
  {"vrt_if_flowfilter_entry_tbl",
    vrt_if_flowfilter_entry::kDbiVrtIfFlowFilterEntryNumCols,
    vrt_if_flowfilter_entry::kVrtIfFlowFilterEntryNumPks,
    vrt_if_flowfilter_entry::vrt_if_flowfilter_entry_schema},
  {"vtn_policingmap_tbl",
    vtn_policingmap::kDbiVtnPolicingMapNumCols,
    vtn_policingmap::kVtnPolicingMapNumPks,
    vtn_policingmap::vtn_policingmap_schema},
  {"vtn_policingmap_ctrlr_tbl",
    vtn_policingmap_ctrlr::kDbiVtnPolicingMapCtrlrNumCols,
    vtn_policingmap_ctrlr::kVtnPolicingMapCtrlrNumPks,
    vtn_policingmap_ctrlr::vtn_policingmap_ctrlr_schema},
  {"vbr_policingmap_tbl",
    vbr_policingmap::kDbiVbrPolicingMapNumCols,
    vbr_policingmap::kVbrPolicingMapNumPks,
    vbr_policingmap::vbr_policingmap_schema},
  {"vbr_if_policingmap_tbl",
    vbr_if_policingmap::kDbiVbrIfPolicingMapNumCols,
    vbr_if_policingmap::kVbrIfPolicingMapNumPks,
    vbr_if_policingmap::vbr_if_policingmap_schema},
  // POM MoMgr table Schema Info End

  {"ctrlr_tbl",
    ctrlr::kCtrlrNumCols,
    ctrlr::kCtrlrNumPkCols,
    ctrlr::ctrlr_schema}
};  // table_schema

};  // namespace table
};  // namespace schema

}  // namespace dal
}  // namespace upll
}  // namespace unc
