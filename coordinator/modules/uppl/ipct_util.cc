/*
 * Copyright (c) 2012-2015 NEC Corporation
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


/** KT_ROOT 
 * @Description : This function is used to return the root key from the
 *                key structure
 * @param[in]   : k - structure variable of type key_root_t
 * @return      : key structure elements of kt_root
 **/
string IpctUtil::get_string(const key_root_t &k) {
  stringstream ss;
  ss << "KT_ROOT:[KEY: "
      << "root_key:" << PhyUtil::uint8tostr(k.root_key)
  << "]";
  return ss.str();
}

/*** KT_CONTROLLER *****/
/**
 * @Description : This function returns the controller name from
 *                the key structure
 * @param[in]   : k - structure variable of type key_ctr_t
 * @return      : key structure elements of kt_controller
 **/
string IpctUtil::get_string(const key_ctr_t &k) {
  stringstream ss;
  ss << "KT_CONTROLLER:[KEY: "
      << "name:" << k.controller_name
      << "]";
  return ss.str();
}

/**
 * @Description : This function returns the values from the value structure
 * of kt_controller
 * @param[in]   : v - structure variable of type val_ctr_t
 * @return      : value structure elements of kt_controller
 **/
string IpctUtil::get_string(const val_ctr_t &v) {
  stringstream ss;
  stringstream valid;
  for (unsigned int i = 0; i < 8 ; ++i) {
    valid << PhyUtil::uint8tostr(v.valid[i]);
  }
  stringstream cs_attr;
  for (unsigned int i = 0; i < 8 ; ++i) {
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
  << ", port:" << v.port
  << ", valid:" << valid.str()
  << ", cs_row_status:" << PhyUtil::uint8tostr(v.cs_row_status)
  << ", cs_attr:" << cs_attr.str()
  << "]"
  << endl;
  return ss.str();
}

/** 
 * @Description : This function returns the values from the kt_controller
 * value structure
 * @param[in]   : val_st - structure variable of type val_ctr_st_t 
 * @return      : value structure elements of val_ctr_st_t structure of
 * kt_controller
 **/
string IpctUtil::get_string(const val_ctr_st_t &val_st) {
  stringstream ss;
  val_ctr_t v = val_st.controller;
  stringstream valid;
  for (unsigned int i = 0; i < 8 ; ++i) {
    valid << PhyUtil::uint8tostr(v.valid[i]);
  }
  stringstream st_valid;
  for (unsigned int i = 0; i < 4 ; ++i) {
    st_valid << PhyUtil::uint8tostr(val_st.valid[i]);
  }
  stringstream cs_attr;
  for (unsigned int i = 0; i < 8 ; ++i) {
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
  << ", port:" << v.port
  << ", valid:" << valid.str()
  << ", cs_row_status:" << PhyUtil::uint8tostr(v.cs_row_status)
  << ", cs_attr:" << cs_attr.str()
  << ", actual_version:" << val_st.actual_version
  << ", oper_status:" << PhyUtil::uint8tostr(val_st.oper_status)
  << ", actual_controllerid:" << val_st.actual_id
  << ", st valid:" << st_valid.str()
  << "]"
  << endl;
  return ss.str();
}

/** 
 * @Description : This function returns the values from the kt_controller
 * commmit versionvalue structure
 * @param[in]   : val_ctr_commit_ver - structure variable of type val_ctr_st_t 
 * @return      : value structure elements of val_ctr_st_t structure of
 * kt_controller
 **/
string IpctUtil::get_string(const val_ctr_commit_ver_t &val_commit_ver) {
  stringstream ss;
  val_ctr_t v = val_commit_ver.controller;
  stringstream valid;
  for (unsigned int i = 0; i < 8 ; ++i) {
    valid << PhyUtil::uint8tostr(v.valid[i]);
  }
  stringstream commit_ver_valid;
  for (unsigned int i = 0; i < 4 ; ++i) {
    commit_ver_valid << PhyUtil::uint8tostr(val_commit_ver.valid[i]);
  }
  stringstream cs_attr;
  for (unsigned int i = 0; i < 8 ; ++i) {
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
  << ", port:" << v.port
  << ", valid:" << valid.str()
  << ", cs_row_status:" << PhyUtil::uint8tostr(v.cs_row_status)
  << ", cs_attr:" << cs_attr.str()
  << ", commit_ver:" << PhyUtil::uint64tostr(val_commit_ver.commit_number)
  << ", commitdate:" << PhyUtil::uint64tostr(val_commit_ver.commit_date)
  << ", commit_appl:" << val_commit_ver.commit_application
  << ", commit_ver valid:" << commit_ver_valid.str()
  << "]"
  << endl;
  return ss.str();
}

/***** KT_CTR_DOMAIN *****/
/** 
 * @Description : This function returns the domain name from
 *                the key structure
 * @param[in]   : k - structure variable of type key_ctr_domain
 * @return      : key structure elements of key_ctr_domain of kt_domain are
 * returned
 **/
string IpctUtil::get_string(const key_ctr_domain &k) {
  stringstream ss;
  ss << "KT_CTR_DOMAIN:[KEY: "
      << "controller_name:" << k.ctr_key.controller_name
      << ", domain_name:" << k.domain_name
      << "]";
  return ss.str();
}

/** 
 * @Description : This function returns the values from the value structure of
 * type kt_ctr_domain
 * @param[in]   : v - structure variable of type val_ctr_domain
 * @return      : value structure elements of controller domain are returned
 **/
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

/** 
 * @Description : This function returns the values from the st value structure
 * @param[in]   : val_st - structure variable of type val_ctr_domain_st
 * @return      : value structure elements of val_ctr_domain_st are returned
 **/
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
/** 
 * @Description : This function returns the logicalport name from
 *                the key structure
 * @param[in]   : k - structure variable of type key_logical_port_t
 * @return      : structure elements of key_logicalport_t are returned
 **/
string IpctUtil::get_string(const key_logical_port_t &k) {
  stringstream ss;
  ss << "KT_LOGICAL_PORT[KEY: "
      << "controller name:" << k.domain_key.ctr_key.controller_name
      << ", domain name:" << k.domain_key.domain_name
      << ", port name:" << k.port_id
      << "]";
  return ss.str();
}

/** 
 * @Description : This function returns the values from the value structure
 * @param[in]   : v - structure variable of type val_logical_port_st_t
 * @return      : value structure elements of val_logical_port_st_t are returned
 **/
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
  ss << "KT_LOGICAL_PORT:(st)[VAL: "
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

/** 
 * @Description : This function returns the values from the value structure
 * @param[in]   : v - structure variable of type val_logical_port_boundary_t
 * @return      : string - value structure elements of val_logical_port_boundary_t
 **/
string IpctUtil::get_string(const val_logical_port_boundary_t &v) {
  stringstream ss;
  stringstream st_valid;
  stringstream valid;
  for (unsigned int i = 0; i < 4; ++i) {
    valid << PhyUtil::uint8tostr(v.valid[i]);
  }
  for (unsigned int i = 0; i < 2; ++i) {
    st_valid << PhyUtil::uint8tostr(v.logical_port_st_val.valid[i]);
  }
  ss << "KT_LOGICAL_PORT(boundary):[VAL: "
  << " logical_port:"<< get_string(v.logical_port_st_val)
  << ", st_valid:" << st_valid.str()
  << ", boundary_candidate:" << PhyUtil::uint8tostr(v.boundary_candidate)
  << ", connected_controller:" << v.connected_controller
  << ", connected_domain:" << v.connected_domain
  << ", valid:" << valid.str()
  << "]" << endl;
  return ss.str();
}


/** 
 * @Description : This function returns the values from the value structure
 * @param[in]   : v - structure variable of type val_logical_port_t
 * @return      : value structure elements of val_logical_port_t are returned
 **/
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
/** 
 * @Description : This function returns the values from the value structure
 * @param[in]   : k - structure element of type key_logical_member_port_t
 * @return      : structure elements of key_logical_member_port_t are returned
 **/
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

/** 
 * @Description : This function returns the values from the value structure
 * @param[in]   : v - structure variable of type val_lm_port_st_neighbor
 * @return      : string - attributes in value structure of  val_lm_port_st_neighbor
 *
 **/
string IpctUtil::get_string(const val_lm_port_st_neighbor &v) {
  stringstream ss;
  stringstream valid;
  for (unsigned int i = 0 ; i < 4; ++i) {
    valid << PhyUtil::uint8tostr(v.port.valid[i]);
  }
  stringstream neigh_valid;
  for (unsigned int i = 0; i < 4; ++i) {
    neigh_valid << PhyUtil::uint8tostr(v.valid[i]);
  }
  ss << "KT_LOGICAL_MEMBER_PORT neighbor VAL: "
  << "port_number:" << PhyUtil::uint8tostr(v.port.port_number)
  << ", description:" << v.port.description
  << ", admin_st:" << PhyUtil::uint8tostr(v.port.admin_status)
  << ", tav:" << PhyUtil::uint8tostr
           (v.port.trunk_allowed_vlan)
  << ", valid:" << valid.str()
  << ", cn_ctr_id:" << v.connected_controller_id
  << ", cn_sw_id:" << v.connected_switch_id
  << ", cn_port_id:" << v.connected_port_id
  << ", neigbour_valid:" << neigh_valid.str()
  << "]";
  return ss.str();
}

/****** KT_SWITCH ******/
/** 
 * @Description : this functions returns the switch_id and controller name
 *                from the key structure
 * @param[in]   : k - structure variable of type key_switch_t
 * @return      : attributes in structure key_switch_t are returned
 **/
string IpctUtil::get_string(const key_switch_t &k) {
  stringstream ss;
  ss << "KT_SWITCH:[KEY: "
      << "controller_name:" << k.ctr_key.controller_name
      << ", switch_id:" << k.switch_id
      << "]";
  return ss.str();
}

/** 
 * @Description : this function returns the values contained in the value
 *                structure
 * @param[in]   : v - structure variable of type val_switch_t 
 * @return      : attributes in structure val_switch_t are returned 
 **/
string IpctUtil::get_string(const val_switch_t &v) {
  stringstream ss;
  stringstream valid;
  for (unsigned int i = 0 ; i < 6 ; ++i) {
    valid << PhyUtil::uint8tostr(v.valid[i]);
  }
  char ip_value[INET6_ADDRSTRLEN];
  memset(&ip_value, '\0', INET6_ADDRSTRLEN);
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
 * @Description : this function returns the values contained in the val_st
 *                structure
 * @param[in]   : val_st - structure variable of type val_switch_st 
 * @return      : attributes in value structure of switch are returned 
 **/
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
  memset(&ip_value, '\0', INET6_ADDRSTRLEN);
  inet_ntop(AF_INET6, &v.ipv6_address.s6_addr, ip_value, INET6_ADDRSTRLEN);
  ss << "KT_SWITCH:[VAL_SW_ST: "
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

/** get_string
 * @Description : this function returns the values contained in the val_st
 *                structure
 * @param[in]   : val_st - structure variable of type val_switch_st 
 * @return      : attributes in value structure of switch are returned 
 **/
string IpctUtil::get_string(const val_switch_st_detail_t &val_stats) {
  stringstream ss;
  val_switch_st_t val_st = val_stats.switch_st_val;
  val_switch_t v = val_st.switch_val;
  stringstream valid;
  for (unsigned int i = 0 ; i < 6 ; ++i) {
    valid << PhyUtil::uint8tostr(v.valid[i]);
  }
  stringstream st_valid;
  for (unsigned int i = 0 ; i < 6 ; ++i) {
    st_valid << PhyUtil::uint8tostr(val_st.valid[i]);
  }
  stringstream stdet_valid;
  for (unsigned int i = 0 ; i < 2 ; ++i) {
    stdet_valid << PhyUtil::uint8tostr(val_stats.valid[i]);
  }
  /* A string of contiguous zero fields in the 
   preferred form can be shown as "::" */
  char ip_value[INET6_ADDRSTRLEN];
  memset(&ip_value, '\0', INET6_ADDRSTRLEN);
  inet_ntop(AF_INET6, &v.ipv6_address.s6_addr, ip_value, INET6_ADDRSTRLEN);
  ss << "KT_SWITCH:[VAL_SW_ST: "
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
  << ", st_valid:" << st_valid.str() << endl
  << ", flow_count:" << val_stats.flow_count
  << ", stdet_valid:" << stdet_valid.str()
  << "]"
  << endl;
  return ss.str();
}

/*** KT_PORT **********/
/** 
 * @Description : This function returns the key names from
 *                the key structure
 * @param[in]   : k - structure variable of type key_port_t 
 * @return      : attributes in key structure of port are returned 
 **/
string IpctUtil::get_string(const key_port_t &k) {
  stringstream ss;
  ss << "KT_PORT:[KEY: "
      << "Controller_name:" << k.sw_key.ctr_key.controller_name
      << ", switch_id:" << k.sw_key.switch_id
      << ", port_id:" << k.port_id
      << "]";
  return ss.str();
}

/** 
 * @Description : This function returns the values from the value structure
 * @param[in]   : v - structure variable of type val_port_st_t 
 * @return      : attributes in value structure of val_port_st_t are returned
 **/
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
  char macaddr[18];
  memset(&macaddr, '\0', 18);
  snprintf(macaddr, sizeof(macaddr), "%02x%02x.%02x%02x.%02x%02x",
           v.mac_address[0], v.mac_address[1], v.mac_address[2],
           v.mac_address[3], v.mac_address[4], v.mac_address[5]);

  ss << "KT_PORT:[VAL_PORT_ST: "
      << "port_number:" << v.port.port_number
      << ", description:" << v.port.description
      << ", admin_status:" << PhyUtil::uint8tostr(v.port.admin_status)
  << ", tav:" << PhyUtil::uint8tostr
  (v.port.trunk_allowed_vlan)
  << ", valid:" << valid.str()
  << ", oper_st:" << PhyUtil::uint8tostr(v.oper_status)
  << ", mac_addr:" << macaddr
  << ", direction:" << PhyUtil::uint8tostr(v.direction)
  << ", duplex:" << PhyUtil::uint8tostr(v.duplex)
  << ", speed:" << PhyUtil::uint64tostr(v.speed)
  << ", alarms_st:" << PhyUtil::uint64tostr(v.alarms_status)
  << ", lp_id:" << v.logical_port_id
  << ", st_valid:" << st_valid.str()
  << "]";
  return ss.str();
}

/** 
 * @Description : This function returns the values from the value structure
 * @param[in]   : v - structure variable of type val_port_t  
 * @return      : attributes in value structure of val_port_t are returned
 **/
string IpctUtil::get_string(const val_port_stats_t &v) {
  stringstream ss;
  stringstream valid;
  for (unsigned int i = 0 ; i < 4; ++i) {
    valid << PhyUtil::uint8tostr(v.port_st_val.port.valid[i]);
  }
  stringstream st_valid;
  for (unsigned int i = 0; i < 8; ++i) {
    st_valid << PhyUtil::uint8tostr(v.port_st_val.valid[i]);
  }
  stringstream stats_valid;
  for (unsigned int i = 0; i < 13; ++i) {
    stats_valid << PhyUtil::uint8tostr(v.valid[i]);
  }
  char macaddr[18];
  memset(&macaddr, '\0', 18);
  snprintf(macaddr, sizeof(macaddr), "%02x%02x.%02x%02x.%02x%02x",
           v.port_st_val.mac_address[0], v.port_st_val.mac_address[1],
           v.port_st_val.mac_address[2], v.port_st_val.mac_address[3],
           v.port_st_val.mac_address[4], v.port_st_val.mac_address[5]);

  ss << "KT_PORT:[VAL_PORT_STATS: "
      << "port_number:" << v.port_st_val.port.port_number
      << ", description:" << v.port_st_val.port.description
  << ", admin_status:" << PhyUtil::uint8tostr
  (v.port_st_val.port.admin_status)
  << ", tav:" << PhyUtil::uint8tostr
  (v.port_st_val.port.trunk_allowed_vlan)
  << ", valid:" << valid.str()
  << ", oper_st:" << PhyUtil::uint8tostr(v.port_st_val.oper_status)
  << ", mac_addr:" << macaddr
  << ", direction:" << PhyUtil::uint8tostr(v.port_st_val.direction)
  << ", duplex:" << PhyUtil::uint8tostr(v.port_st_val.duplex)
  << ", speed:" << PhyUtil::uint64tostr(v.port_st_val.speed)
  << ", alarms_st:" << PhyUtil::uint64tostr(v.port_st_val.alarms_status)
  << ", lp_id:" << v.port_st_val.logical_port_id
  << ", st_valid:" << st_valid.str() << endl
  << ", rx_packets:" << v.rx_packets << ", tx_packets:" << v.tx_packets
  << ", rx_bytes:" << v.rx_bytes << ", tx_bytes:" << v.tx_bytes
  << ", rx_dropped:" << v.rx_dropped << ", tx_dropped:" << v.tx_dropped
  << ", rx_errors:" << v.rx_errors << ", tx_errors:" << v.tx_errors
  << ", rx_frame_err:" << v.rx_frame_err << ", rx_over_err:" << v.rx_over_err
  << ", rx_crc_err:" << v.rx_crc_err << ", collisions:" << v.collisions
  << ", stats_valid:" << stats_valid.str()
  << "]";
  return ss.str();
}

/** 
 * @Description : This function returns the values from the value structure
 * @param[in]   : v - structure variable of type val_port_t  
 * @return      : attributes in value structure of val_port_t are returned
 **/
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
  << ", tav:" << PhyUtil::uint8tostr
  (v.trunk_allowed_vlan)
  << ", valid:" << valid.str()
  << "]";
  return ss.str();
}

/** 
 * @Description : This function returns the values from the value structure
 * @param[in]   : v - structure variable of type val_port_st_neighbor
 * @return      : attributes in value structure of  val_port_st_neighbor are
 * returned
 *
 **/
string IpctUtil::get_string(const val_port_st_neighbor &v) {
  stringstream ss;
  stringstream valid;
  for (unsigned int i = 0 ; i < 4; ++i) {
    valid << PhyUtil::uint8tostr(v.port.valid[i]);
  }
  stringstream neigh_valid;
  for (unsigned int i = 0; i < 4; ++i) {
    neigh_valid << PhyUtil::uint8tostr(v.valid[i]);
  }
  ss << "KT_PORT_NEIGHBOUR VAL: "
  << "port_number:" << PhyUtil::uint8tostr(v.port.port_number)
  << ", description:" << v.port.description
  << ", admin_st:" << PhyUtil::uint8tostr(v.port.admin_status)
  << ", tav:" << PhyUtil::uint8tostr
           (v.port.trunk_allowed_vlan)
  << ", valid:" << valid.str()
  << ", cn_ctr_id:" << v.connected_controller_id
  << ", cn_sw_id:" << v.connected_switch_id
  << ", cn_port_id:" << v.connected_port_id
  << ", neigbour_valid:" << neigh_valid.str()
  << "]";
  return ss.str();
}

/***** KT_LINK ************/
/** 
 * @Description : This function returns the link name from
 *                the key structure
 * @param[in]   : k - structure variable of type key_link_t
 * @return      : attributes in key structure of link are returned 
 **/
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
/** 
 * @Description : This function returns the values from the value structure
 * @param[in]   : v - structure variable of type val_link_t 
 * @return      : attributes in value structure of val_link_t are returned
 **/
string IpctUtil::get_string(const val_link_t &v) {
  stringstream ss;
  ss << "KT_LINK:[VAL: "
      << "description:" << v.description
      << ", valid:" << PhyUtil::uint8tostr(v.valid[0])
  << "]"
  << endl;
  return ss.str();
}
/** 
 * @Description : This function returns the values from the value structure
 * @param[in]   : v - structure variable of type val_link_st_t 
 * @return      : attributes in value structure of val_link_st_t are returned
 **/
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
/** 
 * @Description : This function returns the boundary_id from the key structure
 * @param[in]   : key_obj - structure variable of type key_boundary_t
 * @return      : attributes in key structure of key_boundary_t are returned 
 **/
string IpctUtil::get_string(const key_boundary_t &key_obj) {
  stringstream ss;
  ss << "KT_BOUNDARY:[KEY: "
      << "name:" << key_obj.boundary_id
      << "]";
  return ss.str();
}

/** 
 * @Description : This function returns the values from the value structure
 * @param[in]   : val_obj - structure variable of type val_boundary_t
 * @return      : attributes in value structure of val_boundary_t are returned
 **/
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

/** 
 * @Description : This function returns the values from the value structure
 * @param[in]   : val_obj - structure variable of type val_boundary_st_t
 * @return      : attributes in value structure of val_boundary_st_t are
 *                returned
 **/
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
      << ", valid:" << valid.str()
      << ", cs_row_status:" <<
      PhyUtil::uint8tostr(val_obj.boundary.cs_row_status)
  << ", cs_attr:" << cs_attr.str()
  << ", oper_status: " << PhyUtil::uint8tostr(val_obj.oper_status)
  << ", valid_st:" << valid_st.str()
  << "]"
  << endl;
  return ss.str();
}

/******** PATH_FAULT_ALARM ****/
/** 
 * @Description : This function returns the values from the value structure of
 * alarm type PATH_FAULT_ALARM
 * @param[in]   : v - structure variable of type val_path_fault_alarm_t 
 * @return      : attributes in value structure of val_path_fault_alarm_t are
 * returned
 **/
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
/** 
 * @Description : This function returns the values from the value structure of
 * type PHY_PATH_FAULT_ALARM
 * @param[in]   : v - structure variable of type val_phys_path_fault_alarm_t
 * @return      : attributes in value structure of val_phys_path_fault_alarm_t
 * are returned
 **/
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
/** 
 * @Description : This function returns the values from the value structure of
 * type FLOW_ENTRY_FULL_ALARM
 * @param[in]   : v - structure variable of type val_flow_entry_full_alarm_t
 * @return      : attributes in the value structure of FLOW_ENTRY_FULL_ALARM
 * are returned
 **/
string IpctUtil::get_string(const val_flow_entry_full_alarm_t  &v) {
  stringstream ss;
  ss << "FLOW_ENTRY_FULL_ALARM:[VAL: "
      << "ofs_dpid:" << v.ofs_dpid
      << "]"
      << endl;
  return ss.str();
}

/********* OFS_LACK_FEATURES_ALARM ********/
/** 
 * @Description : This function returns the values from the value structure of
 * type OFS_LACK_FEATURES_ALARM
 * @param[in]   : v - structure variable of type val_ofs_lack_features_alarm_t
 * @return      : attributes in the value structure of
 * val_ofs_lack_features_alarm_t are returned
 **/
string IpctUtil::get_string(const val_ofs_lack_features_alarm_t  &v) {
  stringstream ss;
  ss << "OFS_LACK_FEATURES_ALARM:[VAL: "
      << "ofs_dpid:" << v.ofs_dpid
      << "]"
      << endl;
  return ss.str();
}

/********* VAL_PORT_ALARM ********/
/** 
 * @Description : This function returns the values from the value structure of
 * type VAL_PORT_ALARM
 * @param[in]   : v - structure variable of type val_port_alarm_t
 * @return      : attributes in the value structure of val_port_alarm_t are
 * returned
 **/
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

/** 
 * @Description : This function returns the values from the value structure
 * @param[in]   : val_obj - structure variable of type val_df_data_flow_t
 * @return      : attributes in value structure of val_df_data_flow_t are
 * returned
 **/
string IpctUtil::get_string(const val_df_data_flow_t &val_obj) {
  stringstream ss;
  stringstream valid;
  for (unsigned int i = 0; i < 2 ; ++i) {
    valid << PhyUtil::uint8tostr(val_obj.valid[i]);
  }
  ss << "KT_CTR_DATAFLOW:[VAL: \n"
      << " reason: " << val_obj.reason
      << " controller_count: " << val_obj.controller_count
      << "\n"
      << " valid: " << valid.str()
      << " ]"
      << endl;
  return ss.str();
}

/** 
 * @Description : This function returns the values from the value structure
 * @param[in]   : val_obj - structure variable of type val_df_data_flow_t
 * @return      : attributes in value structure of val_df_data_flow_t are
 * returned
 **/
string IpctUtil::get_string(const val_df_data_flow_st_t &val_obj) {
  stringstream ss;
  stringstream valid_st;
  for (unsigned int i = 0; i < 3 ; ++i) {
    valid_st << PhyUtil::uint8tostr(val_obj.valid[i]);
  }
  ss << "KT_CTR_DATAFLOW:[VAL: \n"
      << " packets: " << PhyUtil::uint64tostr(val_obj.packets)
      << ", octets: " << PhyUtil::uint64tostr(val_obj.octets)
      << ", duration: " << val_obj.duration
      << "\n"
      << " valid_st: " << valid_st.str()
      << " ]"
      << endl;
  return ss.str();
}

