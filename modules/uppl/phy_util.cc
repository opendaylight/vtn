/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/**
 * @brief   Physical utility
 * @file    phy_util.cc
 *
 */

#include "phy_util.hh"
#include "physicallayer.hh"
#include "odbcm_utils.hh"

using unc::uppl::PhysicalLayer;
using unc::uppl::ODBCMUtils;

// For giving response to logical using serversession object
void PhyUtil::getRespHeaderFromReqHeader(physical_request_header rqh,
                                         physical_response_header& rsh) {
  rsh.client_sess_id = rqh.client_sess_id;
  rsh.config_id = rqh.config_id;
  rsh.operation = rqh.operation;
  rsh.max_rep_count = rqh.max_rep_count;
  rsh.option1 = rqh.option1;
  rsh.option2 = rqh.option2;
  rsh.data_type = rqh.data_type;
  rsh.result_code = 0;
}

// For giving response to logical using serversession object
int PhyUtil::sessOutRespHeader(ServerSession& sess,
                               physical_response_header& rsh) {
  int err = 0;
  err = sess.addOutput(rsh.client_sess_id);
  if (err != UPPL_RC_SUCCESS) return err;
  err = sess.addOutput(rsh.config_id);
  if (err != UPPL_RC_SUCCESS) return err;
  err = sess.addOutput(rsh.operation);
  if (err != UPPL_RC_SUCCESS) return err;
  err = sess.addOutput(rsh.max_rep_count);
  if (err != UPPL_RC_SUCCESS) return err;
  err = sess.addOutput(rsh.option1);
  if (err != UPPL_RC_SUCCESS) return err;
  err = sess.addOutput(rsh.option2);
  if (err != UPPL_RC_SUCCESS) return err;
  err = sess.addOutput(rsh.data_type);
  if (err != UPPL_RC_SUCCESS) return err;
  err = sess.addOutput(rsh.result_code);
  printRespHeader(rsh);
  return err;
}

void PhyUtil::printReqHeader(physical_request_header rqh) {
  stringstream ss;
  ss  << "Request to UPPL  : " << endl
      << "    session_id   : " << rqh.client_sess_id << endl
      << "    config_id    : " << rqh.config_id << endl
      << "    operation    : " << rqh.operation << endl
      << "    max_rep_count: " << rqh.max_rep_count << endl
      << "    option1      : " << rqh.option1 << endl
      << "    option2      : " << rqh.option2 << endl
      << "    data_type    : " << rqh.data_type << endl
      << "    key_type     : " << rqh.key_type << endl;
  pfc_log_info((const char*)(ss.str().c_str()));
}

void PhyUtil::printRespHeader(physical_response_header rsh) {
  stringstream ss;
  ss << "Response from UPPL: "<< endl
      << "    session_id   : " << rsh.client_sess_id << endl
      << "    config_id    : " << rsh.config_id << endl
      << "    operation    : " << rsh.operation << endl
      << "    max_rep_count: " << rsh.max_rep_count << endl
      << "    option1      : " << rsh.option1 << endl
      << "    option2      : " << rsh.option2 << endl
      << "    data_type    : " << rsh.data_type << endl
      << "    result_code  : " << rsh.result_code << endl;
  pfc_log_info((const char*)(ss.str().c_str()));
}

// For getting the request from logical using serversession object.
int PhyUtil::sessGetReqHeader(ServerSession& sess,
                              physical_request_header& rqh) {
  int err = 0;
  err = sess.getArgument(0, rqh.client_sess_id);
  if (err != UPPL_RC_SUCCESS) return err;
  err = sess.getArgument(1, rqh.config_id);
  if (err != UPPL_RC_SUCCESS) return err;
  err = sess.getArgument(2, rqh.operation);
  if (err != UPPL_RC_SUCCESS) return err;
  err = sess.getArgument(3, rqh.max_rep_count);
  if (err != UPPL_RC_SUCCESS) return err;
  err = sess.getArgument(4, rqh.option1);
  if (err != UPPL_RC_SUCCESS) return err;
  err = sess.getArgument(5, rqh.option2);
  if (err != UPPL_RC_SUCCESS) return err;
  err = sess.getArgument(6, rqh.data_type);
  if (err != UPPL_RC_SUCCESS) return err;
  err = sess.getArgument(7, rqh.key_type);
  return err;
}

// For sending request to driver using clientsessioin object
int PhyUtil::sessOutReqHeader(ClientSession& cli_sess,
                              physical_request_header rqh) {
  int err = 0;
  err = cli_sess.addOutput(rqh.client_sess_id);
  if (err != UPPL_RC_SUCCESS) return err;
  err = cli_sess.addOutput(rqh.config_id);
  if (err != UPPL_RC_SUCCESS) return err;
  err = cli_sess.addOutput(rqh.operation);
  if (err != UPPL_RC_SUCCESS) return err;
  err = cli_sess.addOutput(rqh.max_rep_count);
  if (err != UPPL_RC_SUCCESS) return err;
  err = cli_sess.addOutput(rqh.option1);
  if (err != UPPL_RC_SUCCESS) return err;
  err = cli_sess.addOutput(rqh.option2);
  if (err != UPPL_RC_SUCCESS) return err;
  err = cli_sess.addOutput(rqh.data_type);
  if (err != UPPL_RC_SUCCESS) return err;
  err = cli_sess.addOutput(rqh.key_type);
  return err;
}

// For getting the repsonse from driver
int PhyUtil::sessGetRespHeader(ClientSession& cli_sess,
                               physical_response_header& rsh) {
  int err = 0;
  err = cli_sess.getResponse(0, rsh.client_sess_id);
  if (err != UPPL_RC_SUCCESS) return err;
  err = cli_sess.getResponse(1, rsh.config_id);
  if (err != UPPL_RC_SUCCESS) return err;
  err = cli_sess.getResponse(2, rsh.operation);
  if (err != UPPL_RC_SUCCESS) return err;
  err = cli_sess.getResponse(3, rsh.max_rep_count);
  if (err != UPPL_RC_SUCCESS) return err;
  err = cli_sess.getResponse(4, rsh.option1);
  if (err != UPPL_RC_SUCCESS) return err;
  err = cli_sess.getResponse(5, rsh.option2);
  if (err != UPPL_RC_SUCCESS) return err;
  err = cli_sess.getResponse(6, rsh.data_type);
  if (err != UPPL_RC_SUCCESS) return err;
  err = cli_sess.getResponse(7, rsh.result_code);
  return err;
}

void PhyUtil::printDriverReqHeader(driver_request_header rqh) {
  stringstream ss;
  ss  << "Request to Driver : " << endl
      << "    client_sess_id: " << rqh.client_sess_id << endl
      << "    config_id     : " << rqh.config_id << endl
      << "    controller_id : " << rqh.controller_id << endl
      << "    domain_id     : " << rqh.domain_id << endl
      << "    operation     : " << rqh.operation << endl
      << "    max_rep_count : " << rqh.max_rep_count << endl
      << "    option1       : " << rqh.option1 << endl
      << "    option2       : " << rqh.option2 << endl
      << "    data_type     : " << rqh.data_type << endl
      << "    key_type      : " << rqh.key_type << endl;
  pfc_log_info((const char*)(ss.str().c_str()));
}

void PhyUtil::printDriverRespHeader(driver_response_header rsh) {
  stringstream ss;
  ss  << "Response from Driver: "<< endl
      << "    client_sess_id: " << rsh.client_sess_id << endl
      << "    config_id     : " << rsh.config_id << endl
      << "    controller_id : " << rsh.controller_id << endl
      << "    domain_id     : " << rsh.domain_id << endl
      << "    operation     : " << rsh.operation << endl
      << "    max_rep_count : " << rsh.max_rep_count << endl
      << "    option1       : " << rsh.option1 << endl
      << "    option2       : " << rsh.option2 << endl
      << "    data_type     : " << rsh.data_type << endl
      << "    result_code   : " << rsh.result_code << endl;
  pfc_log_info((const char*)(ss.str().c_str()));
}

// For sending request to driver using clientsessioin object
int PhyUtil::sessOutDriverReqHeader(ClientSession& cli_sess,
                                    driver_request_header rqh) {
  int err = 0;
  err = cli_sess.addOutput(rqh.client_sess_id);
  if (err != UPPL_RC_SUCCESS) return err;
  err = cli_sess.addOutput(rqh.config_id);
  if (err != UPPL_RC_SUCCESS) return err;
  err = cli_sess.addOutput(rqh.controller_id);
  if (err != UPPL_RC_SUCCESS) return err;
  err = cli_sess.addOutput(rqh.domain_id);
  if (err != UPPL_RC_SUCCESS) return err;
  err = cli_sess.addOutput(rqh.operation);
  if (err != UPPL_RC_SUCCESS) return err;
  err = cli_sess.addOutput(rqh.max_rep_count);
  if (err != UPPL_RC_SUCCESS) return err;
  err = cli_sess.addOutput(rqh.option1);
  if (err != UPPL_RC_SUCCESS) return err;
  err = cli_sess.addOutput(rqh.option2);
  if (err != UPPL_RC_SUCCESS) return err;
  err = cli_sess.addOutput(rqh.data_type);
  if (err != UPPL_RC_SUCCESS) return err;
  err = cli_sess.addOutput(rqh.key_type);
  if (err == 0) {
    printDriverReqHeader(rqh);
  }
  return err;
}

// For getting the repsonse from driver
int PhyUtil::sessGetDriverRespHeader(ClientSession& cli_sess,
                                     driver_response_header& rsh) {
  int err = 0;
  err = cli_sess.getResponse(0, rsh.client_sess_id);
  if (err != UPPL_RC_SUCCESS) return err;
  err = cli_sess.getResponse(1, rsh.config_id);
  if (err != UPPL_RC_SUCCESS) return err;
  const char* arr;
  err = cli_sess.getResponse(2, arr);
  if (err != UPPL_RC_SUCCESS) return err;
  rsh.controller_id = arr;
  err = cli_sess.getResponse(3, arr);
  if (err != UPPL_RC_SUCCESS) return err;
  rsh.domain_id = arr;
  err = cli_sess.getResponse(4, rsh.operation);
  if (err != UPPL_RC_SUCCESS) return err;
  err = cli_sess.getResponse(5, rsh.max_rep_count);
  if (err != UPPL_RC_SUCCESS) return err;
  err = cli_sess.getResponse(6, rsh.option1);
  if (err != UPPL_RC_SUCCESS) return err;
  err = cli_sess.getResponse(7, rsh.option2);
  if (err != UPPL_RC_SUCCESS) return err;
  err = cli_sess.getResponse(8, rsh.data_type);
  if (err != UPPL_RC_SUCCESS) return err;
  err = cli_sess.getResponse(9, rsh.result_code);
  if (err == 0) {
    printDriverRespHeader(rsh);
  }
  return err;
}

// For getting the event header from driver
int PhyUtil::sessGetDriverEventHeader(ClientSession& cli_sess,
                                      driver_event_header& rsh) {
  int err = 0;
  const char* val;
  err = cli_sess.getResponse(0, val);
  if (err != UPPL_RC_SUCCESS) {
    pfc_log_debug("error reading controller");
    return err;
  }
  rsh.controller_id = val;
  err = cli_sess.getResponse(1, val);
  if (err != UPPL_RC_SUCCESS) {
    pfc_log_debug("error reading domain");
    return err;
  }
  rsh.domain_id = val;
  err = cli_sess.getResponse(2, rsh.operation);
  if (err != UPPL_RC_SUCCESS) {
    pfc_log_debug("error reading operation");
    return err;
  }
  err = cli_sess.getResponse(3, rsh.data_type);
  if (err != UPPL_RC_SUCCESS) {
    pfc_log_debug("error reading data_type");
    return err;
  }
  err = cli_sess.getResponse(4, rsh.key_type);
  if (err != UPPL_RC_SUCCESS) {
    pfc_log_debug("error reading key_type");
    return err;
  }
  stringstream ss;
  ss  << "Event to UPPL    :" << endl
      << "    controller_id: " << rsh.controller_id << endl
      << "    domain_id    : " << rsh.domain_id << endl
      << "    operation    : " << rsh.operation << endl
      << "    data_type    : " << rsh.data_type << endl
      << "    key_type     : " << rsh.key_type;
  pfc_log_info((const char*)(ss.str().c_str()));
  return err;
}

// For getting the alarm header from driver
int PhyUtil::sessGetDriverAlarmHeader(ClientSession& cli_sess,
                                      driver_alarm_header& rsh) {
  int err = 0;
  const char* val;
  err = cli_sess.getResponse(0, val);
  if (err != UPPL_RC_SUCCESS) {
    pfc_log_debug("error reading controller");
    return err;
  }
  rsh.controller_id = val;
  err = cli_sess.getResponse(1, val);
  if (err != UPPL_RC_SUCCESS) {
    pfc_log_debug("error reading domain");
    return err;
  }
  rsh.domain_id = val;
  err = cli_sess.getResponse(2, rsh.operation);
  if (err != UPPL_RC_SUCCESS) {
    pfc_log_debug("error reading operation");
    return err;
  }
  err = cli_sess.getResponse(3, rsh.data_type);
  if (err != UPPL_RC_SUCCESS) {
    pfc_log_debug("error reading data_type");
    return err;
  }
  err = cli_sess.getResponse(4, rsh.key_type);
  if (err != UPPL_RC_SUCCESS) {
    pfc_log_debug("error reading key_type");
    return err;
  }
  err = cli_sess.getResponse(5, rsh.alarm_type);
  if (err != UPPL_RC_SUCCESS) {
    pfc_log_debug("error reading alarm_type");
    return err;
  }
  stringstream ss;
  ss  << "Alarm to UPPL    :" << endl
      << "    controller_id: " << rsh.controller_id << endl
      << "    domain_id    : " << rsh.domain_id << endl
      << "    operation    : " << rsh.operation << endl
      << "    data_type    : " << rsh.data_type << endl
      << "    key_type     : " << rsh.key_type << endl
      << "    alarm_type   : " << rsh.alarm_type;
  pfc_log_debug((const char*)(ss.str().c_str()));
  return err;
}

// For sending the event header to northbound
int PhyUtil::sessOutNBEventHeader(ServerEvent& cli_sess,
                                  northbound_event_header& rqh) {
  int err = 0;
  err = cli_sess.addOutput(rqh.operation);
  if (err != UPPL_RC_SUCCESS) return err;
  err = cli_sess.addOutput(rqh.data_type);
  if (err != UPPL_RC_SUCCESS) return err;
  err = cli_sess.addOutput(rqh.key_type);
  stringstream ss;
  ss  << "Event from UPPL    :" << endl
      << "    operation    : " << rqh.operation << endl
      << "    data_type    : " << rqh.data_type << endl
      << "    key_type     : " << rqh.key_type;
  pfc_log_debug((const char*)(ss.str().c_str()));
  return err;
}

// For sending the alarm header to northbound
int PhyUtil::sessOutNBAlarmHeader(ServerEvent& cli_sess,
                                  northbound_alarm_header& rqh) {
  int err = 0;
  err = cli_sess.addOutput(rqh.operation);
  if (err != UPPL_RC_SUCCESS) return err;
  err = cli_sess.addOutput(rqh.data_type);
  if (err != UPPL_RC_SUCCESS) return err;
  err = cli_sess.addOutput(rqh.key_type);
  if (err != UPPL_RC_SUCCESS) return err;
  err = cli_sess.addOutput(rqh.alarm_type);
  stringstream ss;
  ss  << "Alarm from UPPL    :" << endl
      << "    operation    : " << rqh.operation << endl
      << "    data_type    : " << rqh.data_type << endl
      << "    key_type     : " << rqh.key_type << endl
      << "    alarm_type   : " << rqh.alarm_type;
  pfc_log_debug((const char*)(ss.str().c_str()));
  return err;
}

/* Following function checks the controller status and gives the oper status
 * , audit status and audit_import state  */

UpplReturnCode PhyUtil::GetControllerStatus(unc_keytype_datatype_t dt_type,
                                            string controller_id,
                                            uint8_t& oper_status) {
  // Structure used to send request to ODBC
  DBTableSchema dbtableschema_obj;
  // Construct Primary key list
  vector<string> vect_prim_keys;
  vect_prim_keys.push_back("controller_name");

  dbtableschema_obj.set_primary_keys(vect_prim_keys);
  dbtableschema_obj.set_table_name("controller_common_table");

  // TableAttrSchema holds table_name, primary key, attr_name
  vector<TableAttrSchema> vect_table_attr_schema;
  TableAttrSchema  table_attr_schema_obj;


  // controller_name
  PhyUtil::FillDbSchema("controller_name", controller_id,
                        controller_id.length(), DATATYPE_UINT8_ARRAY_32,
                        vect_table_attr_schema);

  // oper_status
  string oper_status_value = PhyUtil::uint8tostr(oper_status);
  PhyUtil::FillDbSchema("oper_status", oper_status_value,
                        oper_status_value.length(), DATATYPE_UINT16,
                        vect_table_attr_schema);

  dbtableschema_obj.PushBackToRowList(vect_table_attr_schema);

  ODBCM_RC_STATUS read_db_status =
      ODBCManager::get_ODBCManager()->GetOneRow(dt_type, dbtableschema_obj);

  if (read_db_status == ODBCM_RC_SUCCESS) {
    list< vector<TableAttrSchema> > lvTas = dbtableschema_obj.get_row_list();
    list< vector<TableAttrSchema> >::iterator vIter = lvTas.begin();
    vector<TableAttrSchema> ::iterator tIter = (*vIter).begin();
    for ( ; tIter != (*vIter).end(); ++tIter) {
      TableAttrSchema tab_schema = (*tIter);
      string attr_name = tab_schema.table_attribute_name;
      string attr_value;
      if (attr_name == "oper_status") {
        PhyUtil::GetValueFromDbSchema(tab_schema, attr_value,
                                      DATATYPE_UINT16);
        oper_status = attr_value[0];
      }
    }
  } else {
    pfc_log_error("Unable to read from db");
  }
  return UPPL_RC_SUCCESS;
}

UpplReturnCode PhyUtil::GetOperStatus(unc_keytype_datatype_t dt_type,
                                      string controller_name,
                                      uint16_t& oper_status) {
  // Structure used to send request to ODBC
  DBTableSchema dbtableschema_obj;
  // Construct Primary key list
  vector<string> vect_prim_keys;
  vect_prim_keys.push_back("controller_name");

  dbtableschema_obj.set_primary_keys(vect_prim_keys);
  dbtableschema_obj.set_table_name("controller_table");

  // TableAttrSchema holds table_name, primary key, attr_name
  vector<TableAttrSchema> vect_table_attr_schema;
  TableAttrSchema  table_attr_schema_obj;


  // controller_name
  PhyUtil::FillDbSchema("controller_name", controller_name,
                        controller_name.length(), DATATYPE_UINT8_ARRAY_32,
                        vect_table_attr_schema);

  // oper_status
  string oper_status_value = PhyUtil::uint16tostr(oper_status);
  PhyUtil::FillDbSchema("oper_status", oper_status_value,
                        oper_status_value.length(), DATATYPE_UINT16,
                        vect_table_attr_schema);

  dbtableschema_obj.PushBackToRowList(vect_table_attr_schema);

  ODBCM_RC_STATUS read_db_status =
      ODBCManager::get_ODBCManager()->GetOneRow(dt_type, dbtableschema_obj);
  if (read_db_status == ODBCM_RC_SUCCESS) {
    list< vector<TableAttrSchema> > lvTas = dbtableschema_obj.get_row_list();
    list< vector<TableAttrSchema> >::iterator vIter = lvTas.begin();
    vector<TableAttrSchema> ::iterator tIter = (*vIter).begin();
    for ( ; tIter != (*vIter).end(); ++tIter) {
      TableAttrSchema tab_schema = (*tIter);
      string attr_name = tab_schema.table_attribute_name;
      string attr_value;
      pfc_log_info("PhyUtil: attr name is %s\n", attr_name.c_str());

      if (attr_name == "oper_status") {
        PhyUtil::GetValueFromDbSchema(tab_schema, attr_value,
                                      DATATYPE_UINT16);
        pfc_log_info("PhyUtil:Attribute value is %c\n", attr_value[0]);
        oper_status = attr_value[0];

        pfc_log_info("PhyUtil: oper_status is %d\n", oper_status);
      }
      pfc_log_info("PhyUtil: Iterating the loop\n");
    }
  } else {
    pfc_log_error("Unable to read from db");
  }
  return UPPL_RC_SUCCESS;
}
string PhyUtil::uint8tostr(uint8_t c) {
  char str[20];
  snprintf(str, sizeof(str), "%d", c);
  string str1 = str;
  return str1;
}

string PhyUtil::uint16tostr(uint16_t c) {
  char str[20];
  snprintf(str, sizeof(str), "%d", c);
  string str1 = str;
  return str1;
}

string PhyUtil::uint64tostr(uint64_t c) {
  char str[20];
  snprintf(str, sizeof(str), "%"PFC_PFMT_u64, c);
  string str1 = str;
  return str1;
}


int PhyUtil::uint8touint(uint8_t c) {
  stringstream stream;
  stream << c;
  return stream.get();
}

unsigned int PhyUtil::strtouint(string str) {
  unsigned int value;
  value = atoi((const char*)(str.c_str()));
  return value;
}

uint64_t PhyUtil::strtouint64(string str) {
  uint64_t value;
  value = atol((const char*)(str.c_str()));
  return value;
}

void PhyUtil::FillDbSchema(string attr_name, string attr_value,
                           unsigned int attr_length,
                           AttributeDataType attr_type,
                           vector<TableAttrSchema> &vect_attr) {
  TableAttrSchema table_attr_schema;
  table_attr_schema.table_attribute_name = attr_name;
  switch (attr_type) {
    case DATATYPE_UINT16:
    {
      ColumnAttrValue <uint16_t> *value = new ColumnAttrValue <uint16_t>;
      value->value = 0;
      uint16_t uint_val = strtouint(attr_value);
      value->value = uint_val;
      table_attr_schema.p_table_attribute_value = value;
      break;
    }
    case DATATYPE_UINT64:
    {
      ColumnAttrValue <uint64_t> *value = new ColumnAttrValue <uint64_t>;
      value->value = 0;
      uint64_t uint_val = strtouint64(attr_value);
      value->value = uint_val;
      table_attr_schema.p_table_attribute_value = value;
      break;
    }
    case DATATYPE_UINT32:
    {
      ColumnAttrValue <uint32_t> *value = new ColumnAttrValue <uint32_t>;
      value->value = 0;
      uint32_t uint_val = strtouint(attr_value);
      value->value = uint_val;
      table_attr_schema.p_table_attribute_value = value;
      break;
    }
    case DATATYPE_IPV4:
    {
      ColumnAttrValue <uint32_t> *value=
          new ColumnAttrValue <uint32_t>;
      uint32_t uint_val = 0;
      if (!attr_value.empty()) {
        uint_val = inet_addr(attr_value.c_str());
      }
      pfc_log_debug("ip address to db: %d", uint_val);
      value->value = uint_val;
      table_attr_schema.p_table_attribute_value = value;
      break;
    }
    case DATATYPE_IPV6:
    {
      ColumnAttrValue <unsigned char[16+1]> *value=
          new ColumnAttrValue <unsigned char[16+1]>;
      memset(&value->value, '\0', sizeof(value->value));
      memcpy(value->value,
             attr_value.c_str(), attr_length+1);
      table_attr_schema.p_table_attribute_value = value;
      table_attr_schema.table_attribute_length = attr_length;
      break;
    }
    case DATATYPE_UINT8_ARRAY_2:
    {
      ColumnAttrValue <unsigned char[2+1]> *value=
          new ColumnAttrValue <unsigned char[2+1]>;
      memset(&value->value, '\0', sizeof(value->value));
      memcpy(value->value,
             attr_value.c_str(), attr_length+1);
      table_attr_schema.p_table_attribute_value = value;
      table_attr_schema.table_attribute_length = attr_length;
      break;
    }
    case DATATYPE_UINT8_ARRAY_3:
    {
      ColumnAttrValue <unsigned char[3+1]> *value=
          new ColumnAttrValue <unsigned char[3+1]>;
      memset(&value->value, '\0', sizeof(value->value));
      memcpy(value->value,
             attr_value.c_str(), attr_length+1);
      table_attr_schema.p_table_attribute_value = value;
      table_attr_schema.table_attribute_length = attr_length;
      break;
    }
    case DATATYPE_UINT8_ARRAY_6:
    {
      ColumnAttrValue <unsigned char[6+1]> *value=
          new ColumnAttrValue <unsigned char[6+1]>;
      memset(&value->value, '\0', sizeof(value->value));
      memcpy(value->value,
             attr_value.c_str(), attr_length+1);
      table_attr_schema.p_table_attribute_value = value;
      table_attr_schema.table_attribute_length = attr_length;
      break;
    }
    case DATATYPE_UINT8_ARRAY_8:
    {
      ColumnAttrValue <unsigned char[8+1]> *value=
          new ColumnAttrValue <unsigned char[8+1]>;
      memset(&value->value, '\0', sizeof(value->value));
      memcpy(value->value,
             attr_value.c_str(), attr_length+1);
      table_attr_schema.p_table_attribute_value = value;
      table_attr_schema.table_attribute_length = attr_length;
      break;
    }
    case DATATYPE_UINT8_ARRAY_9:
    {
      ColumnAttrValue <unsigned char[9+1]> *value=
          new ColumnAttrValue <unsigned char[9+1]>;
      memset(&value->value, '\0', sizeof(value->value));
      memcpy(value->value,
             attr_value.c_str(), attr_length+1);
      table_attr_schema.p_table_attribute_value = value;
      table_attr_schema.table_attribute_length = attr_length;
      break;
    }
    case DATATYPE_UINT8_ARRAY_11:
    {
      ColumnAttrValue <unsigned char[11+1]> *value=
          new ColumnAttrValue <unsigned char[11+1]>;
      memset(&value->value, '\0', sizeof(value->value));
      memcpy(value->value,
             attr_value.c_str(), attr_length+1);
      table_attr_schema.p_table_attribute_value = value;
      table_attr_schema.table_attribute_length = attr_length;
      break;
    }
    case DATATYPE_UINT8_ARRAY_12:
    {
      ColumnAttrValue <unsigned char[12+1]> *value=
          new ColumnAttrValue <unsigned char[12+1]>;
      memset(&value->value, '\0', sizeof(value->value));
      memcpy(value->value,
             attr_value.c_str(), attr_length+1);
      table_attr_schema.p_table_attribute_value = value;
      table_attr_schema.table_attribute_length = attr_length;
      break;
    }
    case DATATYPE_UINT8_ARRAY_16:
    {
      ColumnAttrValue <unsigned char[16+1]> *value=
          new ColumnAttrValue <unsigned char[16+1]>;
      memset(&value->value, '\0', sizeof(value->value));
      memcpy(value->value,
             attr_value.c_str(), attr_length+1);
      table_attr_schema.p_table_attribute_value = value;
      table_attr_schema.table_attribute_length = attr_length;
      break;
    }
    case DATATYPE_UINT8_ARRAY_32:
    {
      ColumnAttrValue <unsigned char[32+1]> *value=
          new ColumnAttrValue <unsigned char[32+1]>;
      memset(value->value, '\0', sizeof(value->value));
      memcpy(value->value,
             attr_value.c_str(), attr_length+1);
      table_attr_schema.p_table_attribute_value = value;
      table_attr_schema.table_attribute_length = attr_length;
      break;
    }
    case DATATYPE_UINT8_ARRAY_128:
    {
      ColumnAttrValue <unsigned char[128+1]> *value=
          new ColumnAttrValue <unsigned char[128+1]>;
      memset(value->value, '\0', sizeof(value->value));
      memcpy(value->value,
             attr_value.c_str(), attr_length+1);
      table_attr_schema.p_table_attribute_value = value;
      table_attr_schema.table_attribute_length = attr_length;
      break;
    }
    case DATATYPE_UINT8_ARRAY_256:
    {
      ColumnAttrValue <unsigned char[256+1]> *value=
          new ColumnAttrValue <unsigned char[256+1]>;
      memset(&value->value, '\0', sizeof(value->value));
      memcpy(value->value,
             attr_value.c_str(), attr_length+1);
      table_attr_schema.p_table_attribute_value = value;
      table_attr_schema.table_attribute_length = attr_length;
      break;
    }
    case DATATYPE_UINT8_ARRAY_257:
    {
      ColumnAttrValue <unsigned char[257+1]> *value=
          new ColumnAttrValue <unsigned char[257+1]>;
      memset(value->value, '\0', sizeof(value->value));
      memcpy(value->value,
             attr_value.c_str(), attr_length+1);
      table_attr_schema.p_table_attribute_value = value;
      table_attr_schema.table_attribute_length = attr_length;
      break;
    }
    case DATATYPE_UINT8_ARRAY_320:
    {
      ColumnAttrValue <unsigned char[320+1]> *value=
          new ColumnAttrValue <unsigned char[320+1]>;
      memset(&value->value, '\0', sizeof(value->value));
      memcpy(value->value,
             attr_value.c_str(), attr_length);
      table_attr_schema.p_table_attribute_value = value;
      table_attr_schema.table_attribute_length = attr_length;
      break;
    }
  }
  table_attr_schema.request_attribute_type = attr_type;
  vect_attr.push_back(table_attr_schema);
}

void PhyUtil::FillDbSchema(string attr_name,
                           string attr_value,
                           unsigned int attr_length,
                           AttributeDataType attr_type,
                           uint32_t operation_type,
                           uint16_t in_valid_val,
                           uint16_t prev_db_valid_val,
                           vector<TableAttrSchema> &vect_attr,
                           vector<string> &vect_prim_keys,
                           stringstream &out_valid_value) {
  if (PhyUtil::IsValidValue(operation_type, in_valid_val) == true) {
    PhyUtil::FillDbSchema(attr_name, attr_value,
                          attr_value.length(), attr_type,
                          vect_attr);
    out_valid_value << UNC_VF_VALID;
    if (PhyUtil::IsFilteringOperation(operation_type, in_valid_val) == true) {
      // To be used as filter and add in vect_primary_key
      vect_prim_keys.push_back(attr_name);
    }
  } else if (operation_type == UNC_OP_CREATE &&
      in_valid_val == UNC_VF_INVALID) {
    pfc_log_debug("Attribute '%s' is not given in create request",
                  attr_name.c_str());
    out_valid_value << UNC_VF_INVALID;
  } else if ((operation_type == UNC_OP_CREATE ||
      operation_type == UNC_OP_UPDATE) &&
      in_valid_val == UPPL_NO_VAL_STRUCT) {
    pfc_log_debug("Attribute '%s' is not given in create/update request",
                  attr_name.c_str());
    out_valid_value << UNC_VF_VALID;
    PhyUtil::FillDbSchema(attr_name, attr_value,
                          attr_value.length(), attr_type,
                          vect_attr);
  } else if (operation_type == UNC_OP_UPDATE &&
      in_valid_val == UNC_VF_VALID_NO_VALUE) {
    // empty value - value to be deleted
    pfc_log_debug("Attribute '%s' value is to be deleted in update request",
                  attr_name.c_str());
    string empty;
    PhyUtil::FillDbSchema(attr_name, empty,
                          empty.length(), attr_type,
                          vect_attr);
    out_valid_value << UNC_VF_VALID_NO_VALUE;
  } else if (operation_type == UNC_OP_CREATE) {
    out_valid_value << in_valid_val;
  } else if (operation_type == UNC_OP_UPDATE &&
      in_valid_val == UNC_VF_INVALID) {
    pfc_log_debug(
        "Attribute '%s' not given in Update Request. Retain db valid",
        attr_name.c_str());
    out_valid_value << prev_db_valid_val;
  }  else if (operation_type >= UNC_OP_READ) {
    string empty;
    PhyUtil::FillDbSchema(attr_name, empty,
                          empty.length(), attr_type,
                          vect_attr);
  }
}

void PhyUtil::GetValueFromDbSchema(TableAttrSchema table_attr_schema,
                                   string &attr_value,
                                   AttributeDataType attr_type) {
  stringstream ss;
  switch (attr_type) {
    case DATATYPE_UINT16:
    {
      ColumnAttrValue <uint16_t> *value =
          (ColumnAttrValue <uint16_t>*)
          table_attr_schema.p_table_attribute_value;
      ss << value->value;
      break;
    }
    case DATATYPE_UINT64:
    {
      ColumnAttrValue <uint64_t> *value =
          (ColumnAttrValue <uint64_t>*)
          table_attr_schema.p_table_attribute_value;
      ss << value->value;
      break;
    }
    case DATATYPE_UINT32:
    {
      ColumnAttrValue <uint32_t> *value =
          (ColumnAttrValue <uint32_t>*)
          table_attr_schema.p_table_attribute_value;
      ss << value->value;
      break;
    }
    case DATATYPE_IPV4:
    {
      ColumnAttrValue <uint32_t> *value =
          (ColumnAttrValue <uint32_t>*)
          table_attr_schema.p_table_attribute_value;
      pfc_log_debug("Received ip value from DB: %d", value->value);
      if (value->value > 0) {
        struct sockaddr_in ipv4_addr;
        ipv4_addr.sin_addr.s_addr = value->value;
        ss << inet_ntoa(ipv4_addr.sin_addr);
        pfc_log_debug("ip address from db: %d", ipv4_addr.sin_addr.s_addr);
      } else {
        string empty;
        ss << empty;
      }
      break;
    }
    case DATATYPE_IPV6:
    {
      ColumnAttrValue <unsigned char[16+1]> *value =
          (ColumnAttrValue <unsigned char[16+1]>*)
          table_attr_schema.p_table_attribute_value;
      ss << value->value;
      break;
    }
    case DATATYPE_UINT8_ARRAY_2:
    {
      ColumnAttrValue <unsigned char[2+1]> *value =
          (ColumnAttrValue <unsigned char[2+1]>*)
          table_attr_schema.p_table_attribute_value;
      ss << value->value;
      break;
    }
    case DATATYPE_UINT8_ARRAY_3:
    {
      ColumnAttrValue <unsigned char[3+1]> *value =
          (ColumnAttrValue <unsigned char[3+1]>*)
          table_attr_schema.p_table_attribute_value;
      ss << value->value;
      break;
    }
    case DATATYPE_UINT8_ARRAY_6:
    {
      ColumnAttrValue <unsigned char[6+1]> *value =
          (ColumnAttrValue <unsigned char[6+1]>*)
          table_attr_schema.p_table_attribute_value;
      ss << value->value;
      break;
    }
    case DATATYPE_UINT8_ARRAY_8:
    {
      ColumnAttrValue <unsigned char[8+1]> *value =
          (ColumnAttrValue <unsigned char[8+1]>*)
          table_attr_schema.p_table_attribute_value;
      ss << value->value;
      break;
    }
    case DATATYPE_UINT8_ARRAY_9:
    {
      ColumnAttrValue <unsigned char[9+1]> *value =
          (ColumnAttrValue <unsigned char[9+1]>*)
          table_attr_schema.p_table_attribute_value;
      ss << value->value;
      break;
    }
    case DATATYPE_UINT8_ARRAY_11:
    {
      ColumnAttrValue <unsigned char[11+1]> *value =
          (ColumnAttrValue <unsigned char[11+1]>*)
          table_attr_schema.p_table_attribute_value;
      ss << value->value;
      break;
    }
    case DATATYPE_UINT8_ARRAY_12:
    {
      ColumnAttrValue <unsigned char[12+1]> *value =
          (ColumnAttrValue <unsigned char[12+1]>*)
          table_attr_schema.p_table_attribute_value;
      ss << value->value;
      break;
    }
    case DATATYPE_UINT8_ARRAY_16:
    {
      ColumnAttrValue <unsigned char[16+1]> *value =
          (ColumnAttrValue <unsigned char[16+1]>*)
          table_attr_schema.p_table_attribute_value;
      ss << value->value;
      break;
    }
    case DATATYPE_UINT8_ARRAY_32:
    {
      ColumnAttrValue <unsigned char[32+1]> *value =
          (ColumnAttrValue <unsigned char[32+1]>*)
          table_attr_schema.p_table_attribute_value;
      ss << value->value;
      break;
    }
    case DATATYPE_UINT8_ARRAY_128:
    {
      ColumnAttrValue <unsigned char[128+1]> *value =
          (ColumnAttrValue <unsigned char[128+1]>*)
          table_attr_schema.p_table_attribute_value;
      ss << value->value;
      break;
    }
    case DATATYPE_UINT8_ARRAY_256:
    {
      ColumnAttrValue <unsigned char[256+1]> *value =
          (ColumnAttrValue <unsigned char[256+1]>*)
          table_attr_schema.p_table_attribute_value;
      ss << value->value;
      break;
    }
    case DATATYPE_UINT8_ARRAY_257:
    {
      ColumnAttrValue <unsigned char[257+1]> *value =
          (ColumnAttrValue <unsigned char[257+1]>*)
          table_attr_schema.p_table_attribute_value;
      ss << value->value;
      break;
    }
    case DATATYPE_UINT8_ARRAY_320:
    {
      ColumnAttrValue <unsigned char[320+1]> *value =
          (ColumnAttrValue <unsigned char[320+1]>*)
          table_attr_schema.p_table_attribute_value;
      ss << value->value;
      break;
    }
  }
  attr_value = ss.str();
  ss.str("");
}

UpplReturnCode PhyUtil::get_controller_type(
    string controller_name,
    unc_keytype_ctrtype_t& controller_type,
    unc_keytype_datatype_t datatype) {
  UpplReturnCode ret_code = UPPL_RC_SUCCESS;
  string type;
  // Creating the Physical Layer instance
  PhysicalLayer *physical_layer = PhysicalLayer::get_instance();

  // Creating the object that consists of vectors of Table Atrr Schema
  vector<TableAttrSchema> vect_table_attr_schema;
  vector<string> vect_prim_keys;  // Construct Primary key list
  vect_prim_keys.push_back("controller_name");
  pfc_log_info("PhyUtil::controller_name is %s\n", controller_name.c_str());
  PhyUtil::FillDbSchema("controller_name", controller_name,
                        controller_name.length(), DATATYPE_UINT8_ARRAY_32,
                        vect_table_attr_schema);
  PhyUtil::FillDbSchema("type", type,
                        type.length(), DATATYPE_UINT16,
                        vect_table_attr_schema);
  // Structure used to send request to ODBC
  DBTableSchema dbtableschema_obj;
  dbtableschema_obj.set_primary_keys(vect_prim_keys);
  dbtableschema_obj.set_table_name("controller_table");
  dbtableschema_obj.PushBackToRowList(vect_table_attr_schema);
  ODBCM_RC_STATUS db_status = physical_layer->get_odbc_manager()-> \
      GetOneRow(datatype, dbtableschema_obj);
  if (db_status == ODBCM_RC_CONNECTION_ERROR) {
    // log fatal error to log daemon
    pfc_log_fatal("DB connection not available or cannot access DB");
    ret_code = UPPL_RC_ERR_DB_ACCESS;
    return ret_code;
  } else if (db_status != ODBCM_RC_SUCCESS) {
    string log_msg = "Unable to get controller type from the database";
    pfc_log_error((const char *)log_msg.c_str());
    ret_code = UPPL_RC_ERR_DB_GET;
    return ret_code;
  }
  list< vector<TableAttrSchema> > row_list_iter =
      dbtableschema_obj.get_row_list();
  list< vector<TableAttrSchema> >::iterator vect_iter =
      row_list_iter.begin();
  vector<TableAttrSchema> ::iterator tab_iter = (*vect_iter).begin();
  for ( ; tab_iter != (*vect_iter).end(); ++tab_iter) {
    TableAttrSchema tab_schema = (*tab_iter);
    string attr_name = tab_schema.table_attribute_name;
    string attr_value;
    if (attr_name == "type") {
      PhyUtil::GetValueFromDbSchema(tab_schema, attr_value,
                                    DATATYPE_UINT16);
      controller_type = (unc_keytype_ctrtype_t)(atoi(attr_value.c_str()));
    }
  }
  pfc_log_debug("Controller Type return: %d, type value %d",
                ret_code, controller_type);
  return ret_code;
}

void PhyUtil::reorder_col_attrs(
    vector<string> vect_prim_keys,
    vector<TableAttrSchema> &vect_table_attr_schema) {
  if (vect_prim_keys.empty()) {
    return;
  }
  for (int i = vect_prim_keys.size()-1; i >= 0; i--) {
    string key_attr_name = vect_prim_keys[i];
    vector<TableAttrSchema> ::iterator tab_iter =
        vect_table_attr_schema.begin();
    for ( ; tab_iter != vect_table_attr_schema.end(); ++tab_iter) {
      TableAttrSchema col_attr = (*tab_iter);
      if (key_attr_name == col_attr.table_attribute_name) {
        vect_table_attr_schema.erase(tab_iter);
        vect_table_attr_schema.insert(vect_table_attr_schema.begin(),
                                      col_attr);
        break;
      }
    }
  }
}

/** IsValidValue
 * * @Description : This function checks whether the value in value
 * structure can be considered
 * * * @param[in] : operation type and valid flag
 * * * @return    : True of false
 * */
bool PhyUtil::IsValidValue(uint32_t operation_type,
                           unsigned int valid) {
  if (valid == UPPL_NO_VAL_STRUCT) {
    return false;
  }
  if ((operation_type == UNC_OP_CREATE && valid == UNC_VF_VALID) ||
      (operation_type == UNC_OP_UPDATE && valid == UNC_VF_VALID) ||
      (operation_type == UNC_OP_READ) ||
      (operation_type == UNC_OP_READ_SIBLING_BEGIN) ||
      (operation_type == UNC_OP_READ_SIBLING) ||
      (operation_type == UNC_OP_READ_SIBLING_COUNT)) {
    return true;
  } else {
    return false;
  }
}

/** IsFilteringOperation
 * * @Description : This function checks whether the value in value
 * structure has to be used for filertering
 * * * @param[in] : operation type and valid flag
 * * * @return    : True of false
 * */
bool PhyUtil::IsFilteringOperation(uint32_t operation_type,
                                   unsigned int valid) {
  if ((operation_type == UNC_OP_READ ||
      operation_type == UNC_OP_READ_SIBLING_BEGIN ||
      operation_type == UNC_OP_READ_SIBLING ||
      operation_type == UNC_OP_READ_SIBLING_COUNT) &&
      valid == UNC_VF_VALID) {
    return true;
  } else {
    return false;
  }
}
