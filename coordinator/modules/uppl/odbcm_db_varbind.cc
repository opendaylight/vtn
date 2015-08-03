/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 *  @brief   ODBC Manager
 *  @file    odbcm_db_varbind.cc
 */

#include <stdio.h>
#include <sstream>
#include "odbcm_db_varbind.hh"
#include "odbcm_common.hh"
#include "odbcm_utils.hh"
#include "odbcm_db_tableschema.hh"

using unc::uppl::DBVarbind;

/**
 * @Description : Constructor of DBVarbind
 * @param[in]   : None
 * @return      : None
 **/
DBVarbind::DBVarbind()
: /** Initialize the pointers in DBVarbind */
  BindINParameter(NULL),
  BindOUTParameter(NULL),
  FillINPUTValues(NULL),
  FetchOUTPUTValues(NULL),
  p_ctr_table(NULL),
  p_domain_table(NULL),
  p_logicalport_table(NULL),
  p_logical_memberport_table(NULL),
  p_switch_table(NULL),
  p_port_table(NULL),
  p_link_table(NULL),
  p_boundary_table(NULL),
  p_isrowexists(NULL) {
  /** SQL Binary buffer length ptr to use in 
    * database binding apis */
  p_switch_id1_len = new SQLLEN;
  if (p_switch_id1_len == NULL )
    UPPL_LOG_FATAL("ODBCM::DBVarbind:: Error in new: p_switch_id1_len");
  *p_switch_id1_len = 0;

  p_switch_id2_len = new SQLLEN;
  if (p_switch_id2_len == NULL )
    UPPL_LOG_FATAL("ODBCM::DBVarbind:: Error in new: p_switch_id2_len");
  *p_switch_id2_len = 0;

  p_connected_switch_id_len = new SQLLEN;
  if (p_connected_switch_id_len == NULL )
    UPPL_LOG_FATAL("ODBCM::DBVarbind:: Error in new: "
                    "p_connected_switch_id_len");
  *p_connected_switch_id_len = 0;

  p_logicalport_id1_len = new SQLLEN;
  if (p_logicalport_id1_len == NULL )
    UPPL_LOG_FATAL("ODBCM::DBVarbind:: Error in new: p_logicalport_id1_len");
  *p_logicalport_id1_len = 0;

  p_logicalport_id2_len = new SQLLEN;
  if (p_logicalport_id2_len == NULL )
    UPPL_LOG_FATAL("ODBCM::DBVarbind:: Error in new: p_logicalport_id2_len");
  *p_logicalport_id2_len = 0;

  p_ipv6_len = new SQLLEN;
  if (p_ipv6_len == NULL )
    UPPL_LOG_FATAL("ODBCM::DBVarbind:: Error in new: p_ipv6_len");
  *p_ipv6_len = 0;

  p_alarms_status_len = new SQLLEN;
  if (p_alarms_status_len == NULL )
    UPPL_LOG_FATAL("ODBCM::DBVarbind:: Error in new: p_alarms_status_len");
  *p_alarms_status_len = 0;

  p_mac_len = new SQLLEN;
  if (p_mac_len == NULL )
    UPPL_LOG_FATAL("ODBCM::DBVarbind:: Error in new: p_mac_len");
  *p_mac_len = 0;

  p_speed_len = new SQLLEN;
  if (p_speed_len == NULL )
    UPPL_LOG_FATAL("ODBCM::DBVarbind:: Error in new: p_speed_len");
  *p_speed_len = 0;
  p_commit_number_len = new SQLLEN;
  if (p_commit_number_len == NULL)
    UPPL_LOG_FATAL("ODBCM::DBVarbind:: Error in new: p_commit_number_len");
  *p_commit_number_len = 0;
  p_commit_date_len = new SQLLEN;
  if (p_commit_date_len == NULL)
    UPPL_LOG_FATAL("ODBCM::DBVarbind:: Error in new: p_commit_date_len");
  *p_commit_date_len = 0;
}

/**
 * @Description : Destructor of DBVarbind
 * @param[in]   : None
 * @return      : None
 **/
DBVarbind::~DBVarbind() {
  //  binary buffer length ptr
  if (NULL != p_switch_id1_len) {
    delete p_switch_id1_len;
    p_switch_id1_len = NULL;
  }
  if (NULL != p_connected_switch_id_len) {
    delete p_connected_switch_id_len;
    p_connected_switch_id_len = NULL;
  }
  if (NULL != p_switch_id2_len) {
    delete p_switch_id2_len;
    p_switch_id2_len = NULL;
  }
  if (NULL != p_logicalport_id1_len) {
    delete p_logicalport_id1_len;
    p_logicalport_id1_len = NULL;
  }
  if (NULL != p_logicalport_id2_len) {
    delete p_logicalport_id2_len;
    p_logicalport_id2_len = NULL;
  }
  if (NULL != p_ipv6_len) {
    delete p_ipv6_len;
    p_ipv6_len = NULL;
  }
  if (NULL != p_alarms_status_len) {
    delete p_alarms_status_len;
    p_alarms_status_len = NULL;
  }
  if (NULL != p_mac_len) {
    delete p_mac_len;
    p_mac_len = NULL;
  }
  if (NULL != p_speed_len) {
    delete p_speed_len;
    p_speed_len = NULL;
  }
  if (NULL != p_commit_number_len) {
    delete p_commit_number_len;
    p_commit_number_len = NULL;
  }
  if (NULL != p_commit_date_len) {
    delete p_commit_date_len;
    p_commit_date_len = NULL;
  }
}

/**
 * @Description : To free the allocated memory in bind struct pointers
 * @param[in]   : table_id - enum of the tables
 * @return      : void
 **/
void DBVarbind::FreeingBindedStructure(
    const uint32_t table_id) {
  switch (table_id) {
    case UNKNOWN_TABLE:
      pfc_log_debug("ODBCM::DBVarbind::FreeingBindedStructure:  "
                   "No table structure found id:%d", table_id);
      break;
    case CTR_TABLE:
      if (NULL != p_ctr_table) {
        delete p_ctr_table;
        p_ctr_table = NULL;
      }
      break;
    case CTR_DOMAIN_TABLE:
      if (NULL != p_domain_table) {
        delete p_domain_table;
        p_domain_table = NULL;
      }
      break;
    case LOGICALPORT_TABLE:
      if (NULL != p_logicalport_table) {
        delete p_logicalport_table;
        p_logicalport_table = NULL;
      }
      break;
    case LOGICAL_MEMBERPORT_TABLE:
      if (NULL != p_logical_memberport_table) {
        delete p_logical_memberport_table;
        p_logical_memberport_table = NULL;
      }
      break;
    case SWITCH_TABLE:
      if (NULL != p_switch_table) {
        delete p_switch_table;
        p_switch_table = NULL;
      }
      break;
    case PORT_TABLE:
      if (NULL != p_port_table) {
        delete p_port_table;
        p_port_table = NULL;
      }
      break;
    case LINK_TABLE:
      if (NULL != p_link_table) {
        delete p_link_table;
        p_link_table = NULL;
      }
      break;
    case BOUNDARY_TABLE:
      if (NULL != p_boundary_table) {
        delete p_boundary_table;
        p_boundary_table = NULL;
      }
      break;
    case IS_ROW_EXISTS:
      break;
    default:
      pfc_log_debug("ODBCM::DBVarbind::FreeingBindedStructure: "
                   "Invalid table_id: %d", table_id);
  }
}
/**
 * @Description : To set the fptr with i/p or o/p binding functions
 * @param[in]   : table_id - enum of the tables
 *                stream   - Binding input/output
 * @return      : void
 **/
void DBVarbind::SetBinding(int table_id, int stream) {
  switch (stream) {
    case BIND_IN:
      BindingInput(table_id);
      break;
    case BIND_OUT:
      BindingOutput(table_id);
      break;
    default:
      pfc_log_info("ODBCM::DBVarbind:SetBinding: Invalid: %d", stream);
      break;
  }
}

/**
 * @Description : To set the fptr with i/p binding functions
 * @param[in]   : table_id - enum of the tables
 * @return      : void
 **/
void DBVarbind::BindingInput(int table_id) {
  switch (table_id) {
    case CTR_TABLE:
      /** Allocate memory if binding struct pointer is NULL */
      if (p_ctr_table == NULL) {
        p_ctr_table =  new controller_table_t;
        /** If no memory allocated return */
        if (p_ctr_table == NULL) {
          pfc_log_debug("ODBCM::DBVarbind::BindingInput: "
              "Error in allocating memory: table_id:%d ",
              table_id);
          break;
        }
      }
      *p_commit_number_len = sizeof(SQLLEN);
      *p_commit_date_len = sizeof(SQLLEN);
      BindINParameter = &DBVarbind::bind_controller_table_input;
      break;
    case CTR_DOMAIN_TABLE:
      /** Allocate memory if binding struct pointer is NULL */
      if (p_domain_table == NULL) {
        p_domain_table =  new ctr_domain_table_t;
        /** If no memory allocated return */
        if (p_domain_table == NULL) {
          pfc_log_debug("ODBCM::DBVarbind::BindingInput: "
              "Error in allocating memory: table_id:%d ",
              table_id);
          break;
        }
      }
      BindINParameter = &DBVarbind::bind_domain_table_input;
      break;
    case LOGICALPORT_TABLE:
      /** Allocate memory if binding struct pointer is NULL */
      if (p_logicalport_table == NULL) {
      p_logicalport_table =  new logicalport_table_t;
        /** If no memory allocated return */
        if (p_logicalport_table == NULL) {
          pfc_log_debug("ODBCM::DBVarbind::BindingInput: "
              "Error in allocating memory: table_id:%d ",
              table_id);
          break;
        }
      }
      //  binary buffer length
      *p_logicalport_id1_len = ODBCM_SIZE_320;
      *p_switch_id1_len = ODBCM_SIZE_256;
      BindINParameter= &DBVarbind::bind_logicalport_table_input;
      break;
    case LOGICAL_MEMBERPORT_TABLE:
      /** Allocate memory if binding struct pointer is NULL */
      if (p_logical_memberport_table == NULL) {
        p_logical_memberport_table =  new logical_memberport_table_t;
        /** If no memory allocated return */
        if (p_logical_memberport_table == NULL) {
          pfc_log_debug("ODBCM::DBVarbind::BindingInput: "
              "Error in allocating memory: table_id:%d ",
              table_id);
          break;
        }
      }
      *p_logicalport_id1_len = ODBCM_SIZE_320;
      *p_switch_id1_len = ODBCM_SIZE_256;
      BindINParameter= &DBVarbind::bind_logical_memberport_table_input;
      break;
    case SWITCH_TABLE:
      /** Allocate memory if binding struct pointer is NULL */
      if (p_switch_table == NULL) {
        p_switch_table =  new switch_table_t;
        /** If no memory allocated return */
        if (p_switch_table == NULL) {
          pfc_log_debug("ODBCM::DBVarbind::BindingInput: "
              "Error in allocating memory: table_id:%d ",
              table_id);
          break;
        }
      }
      *p_switch_id1_len = ODBCM_SIZE_256;
      *p_ipv6_len = sizeof(SQLLEN);
      *p_alarms_status_len = sizeof(SQLLEN);
      BindINParameter = &DBVarbind::bind_switch_table_input;
      break;
    case PORT_TABLE:
      /** Allocate memory if binding struct pointer is NULL */
      if (p_port_table == NULL) {
        p_port_table = new port_table_t;
        /** If no memory allocated return */
        if (p_port_table == NULL) {
          pfc_log_debug("ODBCM::DBVarbind::BindingInput: "
              "Error in allocating memory: table_id:%d ",
              table_id);
          break;
        }
      }
      *p_switch_id1_len = ODBCM_SIZE_256;
      *p_connected_switch_id_len = ODBCM_SIZE_256;
      *p_logicalport_id1_len = ODBCM_SIZE_320;
      *p_alarms_status_len = sizeof(SQLLEN);
      *p_mac_len = ODBCM_SIZE_6;
      *p_speed_len = sizeof(SQLLEN);
      BindINParameter = &DBVarbind::bind_port_table_input;
      break;
    case LINK_TABLE:
      /** Allocate memory if binding struct pointer is NULL */
      if (p_link_table == NULL) {
        p_link_table =  new link_table_t;
        /** If no memory allocated return */
        if (p_link_table == NULL) {
          pfc_log_debug("ODBCM::DBVarbind::BindingInput: "
              "Error in allocating memory: table_id:%d ",
              table_id);
          break;
        }
      }
      *p_switch_id1_len = ODBCM_SIZE_256;
      *p_switch_id2_len = ODBCM_SIZE_256;
      BindINParameter = &DBVarbind::bind_link_table_input;
      break;
    case BOUNDARY_TABLE:
      /** Allocate memory if binding struct pointer is NULL */
      if (p_boundary_table == NULL) {
        p_boundary_table = new boundary_table_t;
        /** If no memory allocated return */
        if (p_boundary_table == NULL) {
          pfc_log_debug("ODBCM::DBVarbind::BindingInput: "
              "Error in allocating memory: table_id:%d ",
              table_id);
          break;
        }
      }
      *p_logicalport_id1_len = ODBCM_SIZE_320;
      *p_logicalport_id2_len = ODBCM_SIZE_320;
      BindINParameter = &DBVarbind::bind_boundary_table_input;
      break;
    case IS_ROW_EXISTS:  //  Special case - no table is available
      /** Allocate memory if binding struct pointer is NULL */
      if (p_isrowexists == NULL) {
        p_isrowexists =  new is_row_exists_t;
        /** If no memory allocated return */
        if (p_isrowexists == NULL) {
          pfc_log_debug("ODBCM::DBVarbind::BindingInput: "
              "Error in allocating memory: table_id:%d ",
              table_id);
          break;
        }
      }
      pfc_log_info("ODBCM::DBVarbind:SetBinding:"
                     "IS_ROW_EXISTS: stream:IN is not valid for ");
      break;
    default:
      pfc_log_info("ODBCM::DBVarbind:BindingInput: Invalid: %d",
        table_id);
      break;
  }
}

/**
 * @Description : To set the fptr with o/p binding functions
 * @param[in]   : table_id - enum of the tables 
 * @return      : void
 **/
void DBVarbind::BindingOutput(int table_id) {
  switch (table_id) {
    case CTR_TABLE:
      /** Allocate memory if binding struct pointer is NULL */
      if (p_ctr_table == NULL) {
        p_ctr_table =  new controller_table_t;
        /** If no memory allocated return */
        if (p_ctr_table == NULL) {
          pfc_log_debug("ODBCM::DBVarbind::BindingOutput: "
              "Error in allocating memory: table_id:%d ",
              table_id);
          break;
        }
      }
      *p_commit_number_len = sizeof(SQLLEN);
      *p_commit_date_len = sizeof(SQLLEN);
      ODBCM_MEMSET(p_ctr_table, 0, sizeof(controller_table_t));
      BindOUTParameter = &DBVarbind::bind_controller_table_output;
      break;
    case CTR_DOMAIN_TABLE:
      /** Allocate memory if binding struct pointer is NULL */
      if (p_domain_table == NULL) {
        p_domain_table =  new ctr_domain_table_t;
        /** If no memory allocated return */
        if (p_domain_table == NULL) {
          pfc_log_debug("ODBCM::DBVarbind::BindingOutput: "
              "Error in allocating memory: table_id:%d ",
              table_id);
          break;
        }
      }
      ODBCM_MEMSET(p_domain_table, 0, sizeof(ctr_domain_table_t));
      BindOUTParameter = &DBVarbind::bind_domain_table_output;
      break;
    case LOGICALPORT_TABLE:
      /** Allocate memory if binding struct pointer is NULL */
      if (p_logicalport_table == NULL) {
      p_logicalport_table =  new logicalport_table_t;
        /** If no memory allocated return */
        if (p_logicalport_table == NULL) {
          pfc_log_debug("ODBCM::DBVarbind::BindingOutput: "
              "Error in allocating memory: table_id:%d ",
              table_id);
          break;
        }
      }
      //  binary buffer length
      *p_logicalport_id1_len = ODBCM_SIZE_320;
      *p_switch_id1_len = ODBCM_SIZE_256;
      ODBCM_MEMSET(p_logicalport_table, 0, sizeof(logicalport_table_t));
      BindOUTParameter= &DBVarbind::bind_logicalport_table_output;
      break;
    case LOGICAL_MEMBERPORT_TABLE:
      /** Allocate memory if binding struct pointer is NULL */
      if (p_logical_memberport_table == NULL) {
        p_logical_memberport_table =  new logical_memberport_table_t;
        /** If no memory allocated return */
        if (p_logical_memberport_table == NULL) {
          pfc_log_debug("ODBCM::DBVarbind::BindingOutput: "
              "Error in allocating memory: table_id:%d ",
              table_id);
          break;
        }
      }
      *p_logicalport_id1_len = ODBCM_SIZE_320;
      *p_switch_id1_len = ODBCM_SIZE_256;
      ODBCM_MEMSET(p_logical_memberport_table, 0,
         sizeof(logical_memberport_table_t));
      BindOUTParameter= &DBVarbind::bind_logical_memberport_table_output;
      break;
    case SWITCH_TABLE:
      /** Allocate memory if binding struct pointer is NULL */
      if (p_switch_table == NULL) {
        p_switch_table =  new switch_table_t;
        /** If no memory allocated return */
        if (p_switch_table == NULL) {
          pfc_log_debug("ODBCM::DBVarbind::BindingOutput: "
              "Error in allocating memory: table_id:%d ",
              table_id);
          break;
        }
      }
      *p_switch_id1_len = ODBCM_SIZE_256;
      *p_ipv6_len = sizeof(SQLLEN);
      *p_alarms_status_len = sizeof(SQLLEN);
      ODBCM_MEMSET(p_switch_table, 0, sizeof(switch_table_t));
      BindOUTParameter = &DBVarbind::bind_switch_table_output;
      break;
    case PORT_TABLE:
      /** Allocate memory if binding struct pointer is NULL */
      if (p_port_table == NULL) {
        p_port_table = new port_table_t;
        /** If no memory allocated return */
        if (p_port_table == NULL) {
          pfc_log_debug("ODBCM::DBVarbind::BindingOutput: "
              "Error in allocating memory: table_id:%d ",
              table_id);
          break;
        }
      }
      *p_switch_id1_len = ODBCM_SIZE_256;
      *p_connected_switch_id_len = ODBCM_SIZE_256;
      *p_logicalport_id1_len = ODBCM_SIZE_320;
      *p_alarms_status_len = sizeof(SQLLEN);
      *p_mac_len = ODBCM_SIZE_6;
      *p_speed_len = sizeof(SQLLEN);
      ODBCM_MEMSET(p_port_table, 0, sizeof(port_table_t));
      BindOUTParameter = &DBVarbind::bind_port_table_output;
      break;
    case LINK_TABLE:
      /** Allocate memory if binding struct pointer is NULL */
      if (p_link_table == NULL) {
        p_link_table =  new link_table_t;
        /** If no memory allocated return */
        if (p_link_table == NULL) {
          pfc_log_debug("ODBCM::DBVarbind::BindingOutput: "
              "Error in allocating memory: table_id:%d ",
              table_id);
          break;
        }
      }
      *p_switch_id1_len = ODBCM_SIZE_256;
      *p_switch_id2_len = ODBCM_SIZE_256;
      ODBCM_MEMSET(p_link_table, 0, sizeof(link_table_t));
      BindOUTParameter = &DBVarbind::bind_link_table_output;
      break;
    case BOUNDARY_TABLE:
      /** Allocate memory if binding struct pointer is NULL */
      if (p_boundary_table == NULL) {
        p_boundary_table = new boundary_table_t;
        /** If no memory allocated return */
        if (p_boundary_table == NULL) {
          pfc_log_debug("ODBCM::DBVarbind::BindingOutput: "
              "Error in allocating memory: table_id:%d ",
              table_id);
          break;
        }
      }
      *p_logicalport_id1_len = ODBCM_SIZE_320;
      *p_logicalport_id2_len = ODBCM_SIZE_320;
      ODBCM_MEMSET(p_boundary_table, 0, sizeof(boundary_table_t));
      BindOUTParameter = &DBVarbind::bind_boundary_table_output;
      break;
    case IS_ROW_EXISTS:  //  Special case - no table is available
      /** Allocate memory if binding struct pointer is NULL */
      if (p_isrowexists == NULL) {
        p_isrowexists =  new is_row_exists_t;
        /** If no memory allocated return */
        if (p_isrowexists == NULL) {
          pfc_log_debug("ODBCM::DBVarbind::BindingOutput: "
              "Error in allocating memory: table_id:%d ",
              table_id);
          break;
        }
      }
      ODBCM_MEMSET(p_isrowexists, 0, sizeof(is_row_exists_t));
      BindOUTParameter = &DBVarbind::bind_is_row_exists_output;
      break;
    default:
      pfc_log_info("ODBCM::DBVarbind:BindingOutput: Invalid: %d",
        table_id);
      break;
  }
}
/**
 * @Description : To set the value struct for filling and fetching
 * @param[in]   : table_id - enum of the tables 
 *                stream   - Binding input/output
 * @return      : void
 **/
void DBVarbind::SetValueStruct(int table_id, int stream) {
  switch (table_id) {
    case CTR_TABLE:
      ODBCM_MEMSET(p_ctr_table, 0, sizeof(controller_table_t));
      if (stream == BIND_IN) {
        FillINPUTValues = &DBVarbind::fill_controller_table;
      } else if (stream == BIND_OUT)  {
        FetchOUTPUTValues = &DBVarbind::fetch_controller_table;
      }
      break;
    case CTR_DOMAIN_TABLE:
      ODBCM_MEMSET(p_domain_table, 0, sizeof(ctr_domain_table_t));
      if (stream == BIND_IN) {
        FillINPUTValues = &DBVarbind::fill_domain_table;
      } else if (stream == BIND_OUT) {
        FetchOUTPUTValues = &DBVarbind::fetch_domain_table;
      } else {
        pfc_log_info("ODBCM::DBVarbind:SetValueStruct: Invalid input "
                     "for ctr_domain_table: %d", stream);
      }
      break;
    case LOGICALPORT_TABLE:
      ODBCM_MEMSET(p_logicalport_table, 0, sizeof(logicalport_table_t));
      if (stream == BIND_IN) {
        FillINPUTValues = &DBVarbind::fill_logicalport_table;
      } else if (stream == BIND_OUT) {
        FetchOUTPUTValues = &DBVarbind::fetch_logicalport_table;
      } else {
        pfc_log_info("ODBCM::DBVarbind:SetValueStruct: Invalid input "
                     "for logicalport_table: %d", stream);
      }
      break;
    case LOGICAL_MEMBERPORT_TABLE:
      ODBCM_MEMSET(p_logical_memberport_table, 0,
                   sizeof(logical_memberport_table_t));
      if (stream == BIND_IN) {
        FillINPUTValues = &DBVarbind::fill_logical_memberport_table;
      } else if (stream == BIND_OUT) {
        FetchOUTPUTValues = &DBVarbind::fetch_logical_memberport_table;
      } else {
        pfc_log_info("ODBCM::DBVarbind:SetValueStruct: Invalid input "
                     "for logical_memberport_table: %d", stream);
      }
      break;
    case SWITCH_TABLE:
      ODBCM_MEMSET(p_switch_table, 0, sizeof(switch_table_t));
      if (stream == BIND_IN) {
        FillINPUTValues = &DBVarbind::fill_switch_table;
      } else if (stream == BIND_OUT) {
        FetchOUTPUTValues = &DBVarbind::fetch_switch_table;
      } else {
        pfc_log_info("ODBCM::DBVarbind:SetValueStruct: Invalid input "
                     "for switch_table: %d", stream);
      }
      break;
    case PORT_TABLE:
      ODBCM_MEMSET(p_port_table, 0, sizeof(port_table_t));
      if (stream == BIND_IN) {
        FillINPUTValues = &DBVarbind::fill_port_table;
      } else if (stream == BIND_OUT) {
        FetchOUTPUTValues = &DBVarbind::fetch_port_table;
      } else {
        pfc_log_info("ODBCM::DBVarbind:SetValueStruct: Invalid input "
                     "for port_table: %d", stream);
      }
      break;
    case LINK_TABLE:
      ODBCM_MEMSET(p_link_table, 0, sizeof(link_table_t));
      if (stream == BIND_IN) {
        FillINPUTValues = &DBVarbind::fill_link_table;
      } else if (stream == BIND_OUT) {
        FetchOUTPUTValues = &DBVarbind::fetch_link_table;
      } else {
        pfc_log_info("ODBCM::DBVarbind:SetValueStruct: Invalid input "
                     "for link_table %d", stream);
      }
      break;
    case BOUNDARY_TABLE:
      ODBCM_MEMSET(p_boundary_table, 0, sizeof(boundary_table_t));
      if (stream == BIND_IN) {
        FillINPUTValues = &DBVarbind::fill_boundary_table;
      } else if (stream == BIND_OUT) {
        FetchOUTPUTValues = &DBVarbind::fetch_boundary_table;
      } else {
        pfc_log_info("ODBCM::DBVarbind:SetValueStruct: Invalid input "
                     "for boundary_table: %d", stream);
      }
      break;
    case IS_ROW_EXISTS:  //  Special case - no table is available
      ODBCM_MEMSET(p_isrowexists, 0, sizeof(is_row_exists_t));
      p_isrowexists->cs_row_status = UNKNOWN;  // Initialize with UNKNOWN value
      if (stream == BIND_IN) {
        pfc_log_info("ODBCM::DBVarbind:SetValueStruct:"
                     " IS_ROW_EXISTS **IN**NOT**VALID**");
      } else {
        FetchOUTPUTValues = &DBVarbind::fetch_is_row_exists_status;
      }
      break;
    default:
      pfc_log_info("ODBCM::DBVarbind:SetValueStruct: Invalid table_id: %d",
                   table_id);
      break;
  }
}

/**
 * @Description : This is special case - No table mapping 
 * @param[in]   : column_attr - DBTableSchema->rowlist_entry
 * @return      : ODBCM_RC_ROW_EXISTS     - if the row appears in the table 
 *                ODBCM_RC_ROW_NOT_EXISTS - if there is no particular row 
 *                in the table
 **/
ODBCM_RC_STATUS DBVarbind::fetch_is_row_exists_status(
    std::vector<TableAttrSchema> &column_attr) {
  odbcm_debug_info("ODBCM::DBVarbind:fetch: "
    "IsRowExists: %d, cs_row_status:%d,",
    p_isrowexists->is_exists,
    p_isrowexists->cs_row_status);
  if (p_isrowexists->is_exists == EXISTS &&
      p_isrowexists->cs_row_status != UNKNOWN) {
    return ODBCM_RC_ROW_EXISTS;
  }
  return ODBCM_RC_ROW_NOT_EXISTS;
}

/**
 * @Description : This is special case - No table mapping 
 * @param[in]   : column_attr - DBTableSchema->rowlist_entry
 *                r_hstmt     - statement handler which carries the SQL Query
 * @return      : ODBCM_RC_SUCCESS          - if all bind opertions success
 *                ODBCM_RC_PARAM_BIND_ERROR - if any one of the bind got failed
 **/
ODBCM_RC_STATUS DBVarbind::bind_is_row_exists_output(
    std::vector<TableAttrSchema> &column_attr, HSTMT &r_hstmt) {
  SQLRETURN odbc_rc = ODBCM_RC_SUCCESS;
  odbc_rc = SQLBindCol(
    r_hstmt, 1, SQL_C_SHORT, reinterpret_cast<SQLSMALLINT*>(
        &p_isrowexists->is_exists), 0,
      (&p_isrowexists->cbIsExistsNum));
  if (odbc_rc == SQL_ERROR || odbc_rc == SQL_INVALID_HANDLE) {
    pfc_log_error("ODBCM::DBVarbind::bind o/p: "
        "IsRowExists bind is_exists error:"
        " SQL_ERROR or SQL_INVALID_HANDLE %s",
        ODBCMUtils::get_RC_Details(odbc_rc).c_str());
    return ODBCM_RC_PARAM_BIND_ERROR;
  }
// odbcm_debug_info("ODBCM::DBVarbind::bind o/p: IsRowExists col1-"
//      "is_exists: done");

  odbc_rc = SQLBindCol(r_hstmt, 2, SQL_C_SHORT, reinterpret_cast<SQLSMALLINT*>(
                &p_isrowexists->cs_row_status), 0,
      (&p_isrowexists->cbRowStatusNum));
  if (odbc_rc == SQL_ERROR||odbc_rc == SQL_INVALID_HANDLE) {
    pfc_log_error("ODBCM::DBVarbind::bind o/p: IsRowExists"
        "bind cs_row_status error::SQL_ERROR or SQL_INVALID_HANDLE");
    return ODBCM_RC_PARAM_BIND_ERROR;
  }
//  odbcm_debug_info("ODBCM::DBVarbind::bind o/p: IsRowExists col2-"
//      "cs_row_status: done");

  return ODBCM_RC_SUCCESS;
}
/**EOF*/
