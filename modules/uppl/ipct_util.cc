/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/**
 * @brief   IPC Util implementation
 * @file    ipc_util.cc
 *
 */

#include "ipct_util.hh"

using unc::uppl::PhysicalLayer;
using unc::uppl::ODBCMUtils;


/*** KT_ROOT *****/
/** get_string
 * * @Description : This function returns the root key from
 * the key structure
 * * * @param[in] : key structure
 * * * @return    : root key
 * */
string IpctUtil::get_string(const key_root_t &k) {
  stringstream ss;
  ss << "KT_ROOT:[KEY: "
      << "root_key:" << PhyUtil::uint8tostr(k.root_key)
  << "]";
  return ss.str();
}

/*** KT_CONTROLLER *****/
/** get_string
 * * @Description : This function returns the controller name from
 * the key structure
 * * * @param[in] : key structure
 * * * @return    : controller name
 * */
string IpctUtil::get_string(const key_ctr_t &k) {
  stringstream ss;
  ss << "KT_CONTROLLER:[KEY: "
      << "name:" << k.controller_name
      << "]";
  return ss.str();
}

/** get_string
 * * @Description : This function returns the values from the value structure
 * * * @param[in] : value structure
 * * * @return    : values of each attribute in value structure
 * */
string IpctUtil::get_string(const val_ctr_t &v) {
  stringstream ss;
  stringstream valid;
  for (unsigned int i = 0; i < 7 ; ++i) {
    valid << PhyUtil::uint8tostr(v.valid[i]);
  }
  stringstream cs_attr;
  for (unsigned int i = 0; i < 7 ; ++i) {
    cs_attr << PhyUtil::uint8tostr(v.cs_attr[i]);
  }
  ss << "KT_CONTROLLER:[VAL: "
      << "type:" << PhyUtil::uint8tostr(v.type)
  << ", description:" << v.description
  << ", version:" << v.version
  << ", ip_address:" << ODBCMUtils::get_ip_string(v.ip_address.s_addr)
  << ", user:" << v.user
  << ", password:" << v.password
  << ", enable_audit:" << PhyUtil::uint8tostr(v.enable_audit)
  << ", valid:" << valid.str()
  << ", cs_row_status:" << PhyUtil::uint8tostr(v.cs_row_status)
  << ", cs_attr:" << cs_attr.str()
  << "]"
  << endl;
  return ss.str();
}

/** get_string
 * * @Description : This function returns the values from the st value structure
 * * * @param[in] : st value structure
 * * * @return    : values of each attribute in value structure
 * */
string IpctUtil::get_string(const val_ctr_st_t &val_st) {
  stringstream ss;
  val_ctr_t v = val_st.controller;
  stringstream valid;
  for (unsigned int i = 0; i < 7 ; ++i) {
    valid << PhyUtil::uint8tostr(v.valid[i]);
  }
  stringstream st_valid;
  for (unsigned int i = 0; i < 3 ; ++i) {
    st_valid << PhyUtil::uint8tostr(val_st.valid[i]);
  }
  stringstream cs_attr;
  for (unsigned int i = 0; i < 7 ; ++i) {
    cs_attr << PhyUtil::uint8tostr(v.cs_attr[i]);
  }
  ss << "KT_CONTROLLER:[VAL: "
      << "type:" << PhyUtil::uint8tostr(v.type)
  << ", version:" << v.version
  << ", description:" << v.description
  << ", ip_address:" << ODBCMUtils::get_ip_string(v.ip_address.s_addr)
  << ", user:" << v.user
  << ", password:" << v.password
  << ", enable_audit:" << PhyUtil::uint8tostr(v.enable_audit)
  << ", valid:" << valid.str()
  << ", cs_row_status:" << PhyUtil::uint8tostr(v.cs_row_status)
  << ", cs_attr:" << cs_attr.str()
  << ", actual_version:" << val_st.actual_version
  << ", oper_status:" << PhyUtil::uint8tostr(val_st.oper_status)
  << ", st valid:" << st_valid.str()
  << "]"
  << endl;
  return ss.str();
}

/***** KT_CTR_DOMAIN *****/
/** get_string
 * * @Description : This function returns the domain name from
 * the key structure
 * * * @param[in] : key structure
 * * * @return    : domain name
 * */
string IpctUtil::get_string(const key_ctr_domain &k) {
  stringstream ss;
  ss << "KT_CTR_DOMAIN:[KEY: "
      << "controller_name:" << k.ctr_key.controller_name
      << ", domain_name:" << k.domain_name
      << "]";
  return ss.str();
}

/** get_string
 * * @Description : This function returns the values from the value structure
 * * * @param[in] : value structure
 * * * @return    : values of each attribute in value structure
 * */
string IpctUtil::get_string(const val_ctr_domain &v) {
  stringstream ss;
  stringstream valid;
  for (unsigned int i = 0; i < 2; ++i) {
    valid << PhyUtil::uint8tostr(v.valid[i]);
  }
  stringstream cs_attr;
  for (unsigned int i = 0; i < 2; ++i) {
    cs_attr << PhyUtil::uint8tostr(v.cs_attr[i]);
  }
  ss << "KT_CTR_DOMAIN:[VAL: "
      << "type:" << PhyUtil::uint8tostr(v.type)
  << ", description:" << v.description
  << ", valid:" << valid.str()
  << ", cs_row_status:" << PhyUtil::uint8tostr(v.cs_row_status)
  << ", cs_attr:" << cs_attr.str()
  << "]"
  << endl;
  return ss.str();
}

/** get_string
 * * @Description : This function returns the values from the st value structure
 * * * @param[in] : st value structure
 * * * @return    : values of each attribute in value structure
 * */
string IpctUtil::get_string(const val_ctr_domain_st &val_st) {
  stringstream ss;
  val_ctr_domain v = val_st.domain;
  stringstream valid;
  for (unsigned int i = 0; i < 2; ++i) {
    valid << PhyUtil::uint8tostr(v.valid[i]);
  }
  stringstream valid_st;
  for (unsigned int i = 0; i < 2; ++i) {
    valid_st << PhyUtil::uint8tostr(val_st.valid[i]);
  }
  stringstream cs_attr;
  for (unsigned int i = 0; i < 2; ++i) {
    cs_attr << PhyUtil::uint8tostr(v.cs_attr[i]);
  }
  ss << "KT_CTR_DOMAIN:[VAL: "
      << "type:" << PhyUtil::uint8tostr(v.type)
  << ", description:" << v.description
  << ", valid:" << valid.str()
  << ", cs_row_status:" << PhyUtil::uint8tostr(v.cs_row_status)
  << ", cs_attr:" << cs_attr.str()
  << ", oper_status:" << PhyUtil::uint8tostr(val_st.oper_status)
  << ", st valid:" << valid_st.str()
  << "]"
  << endl;
  return ss.str();
}

/***** KT_LOGICALPORT ******/
/** get_string
 * * @Description : This function returns the logicalport name from
 * the key structure
 * * * @param[in] : key structure
 * * * @return    : logicalport name
 * */
string IpctUtil::get_string(const key_logical_port_t &k) {
  stringstream ss;
  ss << "KT_LOGICAL_PORT[KEY: "
      << "controller name:" << k.domain_key.ctr_key.controller_name
      << ", domain name:" << k.domain_key.domain_name
      << ", port name:" << k.port_id
      << "]";
  return ss.str();
}

/** get_string
 * * @Description : This function returns the values from the value structure
 * * * @param[in] : value structure
 * * * @return    : values of each attribute in value structure
 * */
string IpctUtil::get_string(const val_logical_port_st_t &v) {
  stringstream ss;
  stringstream st_valid;
  for (unsigned int i = 0; i < 2; ++i) {
    st_valid << PhyUtil::uint8tostr(v.valid[i]);
  }
  stringstream valid;
  for (unsigned int i = 0; i < 5; ++i) {
    valid << PhyUtil::uint8tostr(v.logical_port.valid[i]);
  }
  ss << "KT_LOGICAL_PORT:[VAL: "
      << "description:" << v.logical_port.description
      << ", port_type:" << PhyUtil::uint8tostr(v.logical_port.port_type)
  << ", switch_id:" << v.logical_port.switch_id
  << ", physical_port_id:" << v.logical_port.physical_port_id
  << ", oper_down_criteria:" << PhyUtil::uint8tostr
  (v.logical_port.oper_down_criteria)
  << ", valid:" << valid.str()
  << ", oper_status:" << PhyUtil::uint8tostr(v.oper_status)
  << ", st_valid:" << st_valid.str()
  << "]" << endl;
  return ss.str();
}

/** get_string
 * * @Description : This function returns the values from the value structure
 * * * @param[in] : value structure
 * * * @return    : values of each attribute in value structure
 * */
string IpctUtil::get_string(const val_logical_port_t &v) {
  stringstream ss;
  stringstream valid;
  for (unsigned int i = 0; i < 5; ++i) {
    valid << PhyUtil::uint8tostr(v.valid[i]);
  }
  ss << "KT_LOGICAL_PORT:[VAL: "
      << ", description:" << v.description
      << ", port_type:" << PhyUtil::uint8tostr(v.port_type)
  << ", switch_id:" << v.switch_id
  << ", physical_port_id:" << v.physical_port_id
  << ", oper_down_criteria:" << PhyUtil::uint8tostr
  (v.oper_down_criteria)
  << ", valid:" << valid.str()
  << "]"
  << endl;
  return ss.str();
}

/***** KT_LOGICAL_MEMBER_PORT *****/
/** get_string
 * * @Description :
 * * * @param[in] : key_logical_member_port
 * * * @return    : string
 * * */
string IpctUtil::get_string(const key_logical_member_port_t &k) {
  stringstream ss;
  ss << "KT_LOGICAL_MEMBER_PORT[KEY: "
      << "controller_name: " <<
      k.logical_port_key.domain_key.ctr_key.controller_name
      << ", domain_name: " << k.logical_port_key.domain_key.domain_name
      << ", port_id: " << k.logical_port_key.port_id
      << ", switch_id: " << k.switch_id
      << ", physical_port_id: " << k.physical_port_id
      << "]"
      << endl;
  return ss.str();
}

/****** KT_SWITCH ******/
/** get_string
 * * @Description : this functions returns the switch_id and controller name
 * from the key structure
 * * @param[in] : key_struct of kt switch
 * * @return    : string
 * */
string IpctUtil::get_string(const key_switch_t &k) {
  stringstream ss;
  ss << "KT_SWITCH:[KEY: "
      << "controller_name:" << k.ctr_key.controller_name
      << ", switch_id:" << k.switch_id
      << "]";
  return ss.str();
}

/** get_string
 * * @Description : this function returns the values contained in the value
 * structure
 * * @param[in] : val_switch_t - value structure of kt switch
 * * @return  : string
 * */
string IpctUtil::get_string(const val_switch_t &v) {
  stringstream ss;
  stringstream valid;
  for (unsigned int i = 0 ; i < 6 ; ++i) {
    valid << PhyUtil::uint8tostr(v.valid[i]);
  }
  char ip_value[INET6_ADDRSTRLEN];
  inet_ntop(AF_INET6, &v.ipv6_address.s6_addr, ip_value, INET6_ADDRSTRLEN);
  ss << "KT_SWITCH:[VAL: "
      << "description:" << v.description
      << ", model:" << v.model
      << ", ip_address:" << ODBCMUtils::get_ip_string(v.ip_address.s_addr)
  << ", ipv6_address:" << ip_value
  << ", admin_status:" << PhyUtil::uint8tostr(v.admin_status)
  << ", DomainName:" << v.domain_name
  << ", valid:" << valid.str()
  << "]"
  << endl;
  return ss.str();
}

/** get_string
 * * @Description : this function returns the values contained in the val_st
 * structure
 * * @param[in] : val_switch_st - val_st structure of kt switch
 * * @return  : string
 * */
string IpctUtil::get_string(const val_switch_st &val_st) {
  stringstream ss;
  val_switch_t v = val_st.switch_val;
  stringstream valid;
  for (unsigned int i = 0 ; i < 6 ; ++i) {
    valid << PhyUtil::uint8tostr(v.valid[i]);
  }
  stringstream st_valid;
  for (unsigned int i = 0 ; i < 6 ; ++i) {
    st_valid << PhyUtil::uint8tostr(val_st.valid[i]);
  }
  /* A string of contiguous zero fields in the 
   preferred form can be shown as "::" */
  char ip_value[INET6_ADDRSTRLEN];
  inet_ntop(AF_INET6, &v.ipv6_address.s6_addr, ip_value, INET6_ADDRSTRLEN);
  ss << "KT_SWITCH:[VAL: "
      << "description:" << v.description
      << ", model:" << v.model
      << ", ip_address:" << ODBCMUtils::get_ip_string(v.ip_address.s_addr)
  << ", ipv6_address:" << ip_value
  << ", admin_status:" << PhyUtil::uint8tostr(v.admin_status)
  << ", DomainName:" << v.domain_name
  << ", valid:" << valid.str()
  << ", oper_status:" << PhyUtil::uint8tostr(val_st.oper_status)
  << ", manufacturer:" << val_st.manufacturer
  << ", hardware:" << val_st.hardware
  << ", software:" << val_st.software
  << ", alarms_status:" << PhyUtil::uint64tostr(val_st.alarms_status)
  << ", st_valid:" << st_valid.str()
  << "]"
  << endl;
  return ss.str();
}

/*** KT_PORT **********/
/** get_string
 * * @Description : This function returns the key names from
 * the key structure
 * * * @param[in] : key structure
 * * * @return    : controller name
 * */
string IpctUtil::get_string(const key_port_t &k) {
  stringstream ss;
  ss << "KT_PORT:[KEY: "
      << "Controller_name:" << k.sw_key.ctr_key.controller_name
      << ", switch_id:" << k.sw_key.switch_id
      << ", port_id:" << k.port_id
      << "]";
  return ss.str();
}

/** get_string
 * * @Description : This function returns the values from the value structure
 * * * @param[in] : value structure
 * * * @return    : values of each attribute in value structure
 * */
string IpctUtil::get_string(const val_port_st_t &v) {
  stringstream ss;
  stringstream valid;
  for (unsigned int i = 0 ; i < 4; ++i) {
    valid << PhyUtil::uint8tostr(v.port.valid[i]);
  }
  stringstream st_valid;
  for (unsigned int i = 0; i < 8; ++i) {
    st_valid << PhyUtil::uint8tostr(v.valid[i]);
  }
  ss << "KT_PORT:[VAL: "
      << "port_number:" << v.port.port_number
  << ", description:" << v.port.description
  << ", admin_status:" << PhyUtil::uint8tostr(v.port.admin_status)
  << ", trunk_allowed_vlan:" << PhyUtil::uint8tostr
  (v.port.trunk_allowed_vlan)
  << ", oper_status:" << PhyUtil::uint8tostr(v.oper_status)
  << ", mac_address:" << v.mac_address
  << ", direction:" << PhyUtil::uint8tostr(v.direction)
  << ", duplex:" << PhyUtil::uint8tostr(v.duplex)
  << ", speed:" << PhyUtil::uint64tostr(v.speed)
  << ", alarms_status:" << PhyUtil::uint64tostr(v.alarms_status)
  << ", logical_port_id:" << v.logical_port_id
  << ", valid:" << valid.str()
  << ", st_valid:" << st_valid.str()
  << "]"
  << endl;
  return ss.str();
}

/** get_string
 * * @Description : This function returns the values from the value structure
 * * * @param[in] : value structure
 * * * @return    : values of each attribute in value structure
 * */
string IpctUtil::get_string(const val_port_t &v) {
  stringstream ss;
  stringstream valid;
  for (unsigned int i = 0 ; i < 4; ++i) {
    valid << PhyUtil::uint8tostr(v.valid[i]);
  }
  ss << "KT_PORT:[VAL: "
      << "port_number:" << v.port_number
  << ", description:" << v.description
  << ", admin_status:" << PhyUtil::uint8tostr(v.admin_status)
  << ", trunk_allowed_vlan:" << PhyUtil::uint8tostr
  (v.trunk_allowed_vlan)
  << ", valid:" << valid.str()
  << "]"
  << endl;
  return ss.str();
}

/** get_string
 * * @Description : This function returns the values from the value structure
 * * * @param[in] : value structure
 * * * @return    : values of each attribute in value structure
 * */
string IpctUtil::get_string(const val_port_st_neighbor &v) {
  stringstream ss;
  stringstream valid;
  for (unsigned int i = 0 ; i < 4; ++i) {
    valid << PhyUtil::uint8tostr(v.port.valid[i]);
  }
  stringstream neigh_valid;
  for (unsigned int i = 0; i < 3; ++i) {
    neigh_valid << PhyUtil::uint8tostr(v.valid[i]);
  }
  ss << "KT_PORT:[NEIGHBOUR VAL: "
      << "port_number:" << PhyUtil::uint8tostr(v.port.port_number)
  << ", description:" << v.port.description
  << ", admin_status:" << PhyUtil::uint8tostr(v.port.admin_status)
  << ", trunk_allowed_vlan:" << PhyUtil::uint8tostr
  (v.port.trunk_allowed_vlan)
  << ", connected_switch_id:" << v.connected_switch_id
  << ", connected_port_id:" << v.connected_port_id
  << ", valid:" << valid.str()
  << ", neigbour_valid:" << neigh_valid.str()
  << "]"
  << endl;
  return ss.str();
}

/***** KT_LINK ************/
/** get_string
 * * @Description : This function returns the link name from
 * the key structure
 * * * @param[in] : key structure
 * * * @return    : controller name
 * */
string IpctUtil::get_string(const key_link_t &k) {
  stringstream ss;
  ss << "KT_LINK:[KEY: "
      << "controller_name:" << k.ctr_key.controller_name
      << ", switch_id1:" << k.switch_id1
      << ", port_id1:" << k.port_id1
      << ", switch_id2:" << k.switch_id2
      << ", port_id2:" << k.port_id2
      << "]";
  return ss.str();
}
/** get_string
 * * @Description : This function returns the values from the value structure
 * * * @param[in] : value structure
 * * * @return    : values of each attribute in value structure
 * */
string IpctUtil::get_string(const val_link_t &v) {
  stringstream ss;
  ss << "KT_LINK:[VAL: "
      << "description:" << v.description
      << ", valid:" << PhyUtil::uint8tostr(v.valid[0])
  << "]"
  << endl;
  return ss.str();
}
/** get_string
 * * @Description : This function returns the values from the value structure
 * * * @param[in] : value structure
 * * * @return    : values of each attribute in value structure
 * */
string IpctUtil::get_string(const val_link_st_t &v) {
  stringstream ss;
  stringstream st_valid;
  for (unsigned int i = 0; i < 2; ++i) {
    st_valid << PhyUtil::uint8tostr(v.valid[i]);
  }
  ss << "KT_LINK:[VAL: " <<
      "description: " << v.link.description <<
      ", valid: " << PhyUtil::uint8tostr(v.link.valid[0]) <<
      ", oper_status: " << PhyUtil::uint8tostr(v.oper_status) <<
      ", st valid: " << st_valid.str()
      << "]"
      << endl;
  return ss.str();
}

/***** KT_BOUNDARY *****/
/** get_string
 * * @Description : This function returns the boundary_id from the key structure
 * * * @param[in] : key structure
 * * * @return    : boundary_id
 * */
string IpctUtil::get_string(const key_boundary_t &key_obj) {
  stringstream ss;
  ss << "KT_BOUNDARY:[KEY: "
      << "name:" << key_obj.boundary_id
      << "]";
  return ss.str();
}

/** get_string
 * * @Description : This function returns the values from the value structure
 * * * @param[in] : value structure
 * * * @return    : values of each attribute in value structure
 * */
string IpctUtil::get_string(const val_boundary_t &val_obj) {
  stringstream ss;
  stringstream valid;
  for (unsigned int i = 0; i < 7 ; ++i) {
    valid << PhyUtil::uint8tostr(val_obj.valid[i]);
  }
  stringstream cs_attr;
  for (unsigned int i = 0; i < 7 ; ++i) {
    cs_attr << PhyUtil::uint8tostr(val_obj.cs_attr[i]);
  }
  ss << "KT_BOUNDARY:[VAL: "
      << "description:" << val_obj.description
      << ", controller_name1:" << val_obj.controller_name1
      << ", domain_name1:" << val_obj.domain_name1
      << ", logical_port_id1:" << val_obj.logical_port_id1
      << ", controller_name2:" << val_obj.controller_name2
      << ", domain_name2:" << val_obj.domain_name2
      << ", logical_port_id2:" << val_obj.logical_port_id2
      << ", valid:" << valid.str()
      << ", cs_row_status:" << PhyUtil::uint8tostr(val_obj.cs_row_status)
  << ", cs_attr:" << cs_attr.str()
  << "]"
  << endl;
  return ss.str();
}

/** get_string
 * * @Description : This function returns the values from the value structure
 * * * @param[in] : value structure
 * * * @return    : values of each attribute in value structure
 * */
string IpctUtil::get_string(const val_boundary_st_t &val_obj) {
  stringstream ss;
  stringstream valid;
  for (unsigned int i = 0; i < 7 ; ++i) {
    valid << PhyUtil::uint8tostr(val_obj.boundary.valid[i]);
  }
  stringstream cs_attr;
  for (unsigned int i = 0; i < 7 ; ++i) {
    cs_attr << PhyUtil::uint8tostr(val_obj.boundary.cs_attr[i]);
  }
  stringstream valid_st;
  for (unsigned int i = 0; i < 2 ; ++i) {
    valid_st << PhyUtil::uint8tostr(val_obj.valid[i]);
  }
  ss << "KT_BOUNDARY:[VAL: "
      << "description:" << val_obj.boundary.description
      << ", controller_name1:" << val_obj.boundary.controller_name1
      << ", domain_name1:" << val_obj.boundary.domain_name1
      << ", logical_port_id1:" << val_obj.boundary.logical_port_id1
      << ", controller_name2:" << val_obj.boundary.controller_name2
      << ", domain_name2:" << val_obj.boundary.domain_name2
      << ", logical_port_id2:" << val_obj.boundary.logical_port_id2
      << ", oper_status: " << PhyUtil::uint8tostr(val_obj.oper_status)
  << ", valid:" << valid.str()
  << ", valid_st:" << valid_st.str()
  << ", cs_row_status:" <<
  PhyUtil::uint8tostr(val_obj.boundary.cs_row_status)
  << ", cs_attr:" << cs_attr.str()
  << "]"
  << endl;
  return ss.str();
}

/******** PATH_FAULT_ALARM ****/
/** get_string
 * * @Description : This function returns the values from the value structure
 * * * @param[in] : value structure
 * * * @return    : values of each attribute in value structure
 * */
string IpctUtil::get_string(const val_path_fault_alarm_t  &v) {
  stringstream ss;
  stringstream valid;
  for (unsigned int i = 0; i < 4 ; ++i) {
    valid << PhyUtil::uint8tostr(v.valid[i]);
  }
  ss << "PATH_FAULT_ALARM:[VAL: "
      << "ingress_logical_port:" << v.ingress_logical_port
      << ", egress_logical_port:" << v.egress_logical_port
      << ", ingress_num_of_ports:" << v.ingress_num_of_ports
      << ", egress_num_of_ports:" << v.egress_num_of_ports
      << ", valid:" << valid.str()
      << "]"
      << endl;
  return ss.str();
}

/********* PHY_PATH_FAULT_ALARM ********/
/** get_string
 * * @Description : This function returns the values from the value structure
 * * * @param[in] : value structure
 * * * @return    : values of each attribute in value structure
 * */
string IpctUtil::get_string(const val_phys_path_fault_alarm_t  &v) {
  stringstream ss;
  stringstream valid;
  for (unsigned int i = 0; i < 2 ; ++i) {
    valid << PhyUtil::uint8tostr(v.valid[i]);
  }
  ss << "PHYS_PATH_FAULT_ALARM:[VAL: "
      << "ingress_ofs_dpid:" << v.ingress_ofs_dpid
      << ", egress_ofs_dpid:" << v.egress_ofs_dpid
      << ", valid:" << valid.str()
      << "]"
      << endl;
  return ss.str();
}

/********* FLOW_ENTRY_FULL_ALARM ********/
/** get_string
 * * @Description : This function returns the values from the value structure
 * * * @param[in] : value structure
 * * * @return    : values of each attribute in value structure
 * */
string IpctUtil::get_string(const val_flow_entry_full_alarm_t  &v) {
  stringstream ss;
  ss << "FLOW_ENTRY_FULL_ALARM:[VAL: "
      << "ofs_dpid:" << v.ofs_dpid
      << "]"
      << endl;
  return ss.str();
}

/********* OFS_LACK_FEATURES_ALARM ********/
/** get_string
 * * @Description : This function returns the values from the value structure
 * * * @param[in] : value structure
 * * * @return    : values of each attribute in value structure
 * */
string IpctUtil::get_string(const val_ofs_lack_features_alarm_t  &v) {
  stringstream ss;
  ss << "OFS_LACK_FEATURES_ALARM:[VAL: "
      << "ofs_dpid:" << v.ofs_dpid
      << "]"
      << endl;
  return ss.str();
}

/********* VAL_PORT_ALARM ********/
/** get_string
 * * @Description : This function returns the values from the value structure
 * * * @param[in] : value structure
 * * * @return    : values of each attribute in value structure
 * */
string IpctUtil::get_string(const val_port_alarm_t  &v) {
  stringstream ss;
  stringstream valid;
  for (unsigned int i = 0; i < 2 ; ++i) {
    valid << PhyUtil::uint8tostr(v.valid[i]);
  }
  ss << "VAL_PORT_ALARM:[VAL: "
      << "ofs_dpid:" << v.ofs_dpid
      << ", port_id:" << v.port_id
      << ", valid:" << valid.str()
      << "]"
      << endl;
  return ss.str();
}
