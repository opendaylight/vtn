/*
 * Copyright (c) 2012-2015 NEC Corporation
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
#include "odbcm_db_varbind.hh"

using unc::uppl::PhysicalLayer;
using unc::uppl::ODBCMUtils;
using unc::uppl::ODBCMTableColumns;

/*
 * Set UINT8 value to the specified TableAttrSchema.
 */
#define TABLE_ATTR_SCHEMA_UINT8_SET(schema, len, str, bufsize)   \
  do {                                                           \
    ColumnAttrValue <unsigned char[(bufsize)]> *__v=             \
          new ColumnAttrValue <unsigned char[(bufsize)]>;        \
    memset(__v->value, 0, bufsize);                              \
    strncpy(reinterpret_cast<char *>(__v->value), (str).c_str(), \
                                      (len)+1);                  \
    (schema).p_table_attribute_value = __v;                      \
    (schema).table_attribute_length = (len);                     \
  } while (0)

/**getRespHeaderFromReqHeader
 * @Description : This function is for giving response to logical using
 *                serversession object
 * @param[in]   : rqh - object of the physical request header structure
 * @param[out]  : &rsh - object of the physical response header structure
 * @return      : void
 * */
void PhyUtil::getRespHeaderFromReqHeader(const physical_request_header& rqh,
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

/**sessOutRespHeader
 * @Description : This function is for giving response to logical using
 *                serversession object
 * @param[in]   : sess - Object of ServerSession where the request
 *                argument present
 * @param[out]  : &rsh - object of the physical response header structure
 * @return      : void
 * */
int PhyUtil::sessOutRespHeader(ServerSession& sess,
                               const physical_response_header& rsh) {
  int err = 0;
  err = sess.addOutput(rsh.client_sess_id);
  if (err != UNC_RC_SUCCESS) return err;
  err = sess.addOutput(rsh.config_id);
  if (err != UNC_RC_SUCCESS) return err;
  err = sess.addOutput(rsh.operation);
  if (err != UNC_RC_SUCCESS) return err;
  err = sess.addOutput(rsh.max_rep_count);
  if (err != UNC_RC_SUCCESS) return err;
  err = sess.addOutput(rsh.option1);
  if (err != UNC_RC_SUCCESS) return err;
  err = sess.addOutput(rsh.option2);
  if (err != UNC_RC_SUCCESS) return err;
  err = sess.addOutput(rsh.data_type);
  if (err != UNC_RC_SUCCESS) return err;
  err = sess.addOutput(rsh.result_code);
  printRespHeader(rsh);
  return err;
}

/**printReqHeader
 * @Description : This function prints the attributes that are present in
 *                physical request header structure
 * @param[in]  :  rqh - object of the physical request header structure
 * @return      : void
 * */
void PhyUtil::printReqHeader(const physical_request_header& rqh) {
  stringstream ss;
  ss  << "Request to UPPL:["
      << "sess_id:" << rqh.client_sess_id
      << ",conf_id:" << rqh.config_id
      << ",oper:" << rqh.operation
      << ",max_rp_cnt:" << rqh.max_rep_count
      << ",opt1:" << rqh.option1
      << ",opt2:" << rqh.option2
      << ",dt:" << rqh.data_type
      << ",kt: " << rqh.key_type
      << "]";
  pfc_log_info((const char*)(ss.str().c_str()));
}

/**printRespHeader
 * @Description : This function prints the attributes that are present in
 *                physical response header structure
 * @param[in]  :  rsh - object of the physical response header structure
 * @return      : void
 * */
void PhyUtil::printRespHeader(const physical_response_header& rsh) {
  stringstream ss;
  ss << "Resp from UPPL:["
      << "sess_id:" << rsh.client_sess_id
      << ",conf_id: " << rsh.config_id
      << ",oper:" << rsh.operation
      << ",max_rp_ct:" << rsh.max_rep_count
      << ",opt1:" << rsh.option1
      << ",opt2:" << rsh.option2
      << ",dt:" << rsh.data_type
      << ",rc:" << rsh.result_code
      << "]";
  pfc_log_info((const char*)(ss.str().c_str()));
}

/**sessGetReqHeader
 * @Description : This function is for getting the request from logical using
 *                serversession object.
 * @param[in]   : sess - Object of ServerSession where the request
 *                argument present
 * @param[out]  : rqh - object of the physical request header structure
 * @return      : void
 * */
int PhyUtil::sessGetReqHeader(ServerSession& sess,
                              physical_request_header& rqh) {
  int err = 0;
  err = sess.getArgument(0, rqh.client_sess_id);
  if (err != UNC_RC_SUCCESS) return err;
  err = sess.getArgument(1, rqh.config_id);
  if (err != UNC_RC_SUCCESS) return err;
  err = sess.getArgument(2, rqh.operation);
  if (err != UNC_RC_SUCCESS) return err;
  err = sess.getArgument(3, rqh.max_rep_count);
  if (err != UNC_RC_SUCCESS) return err;
  err = sess.getArgument(4, rqh.option1);
  if (err != UNC_RC_SUCCESS) return err;
  err = sess.getArgument(5, rqh.option2);
  if (err != UNC_RC_SUCCESS) return err;
  err = sess.getArgument(6, rqh.data_type);
  if (err != UNC_RC_SUCCESS) return err;
  err = sess.getArgument(7, rqh.key_type);
  return err;
}

/**sessOutReqHeader
 * @Description : This function is for sending request to driver using
 *                clientsessioin object
 * @param[in]   : cli_sess - Object of ClientSession where the request
 *                argument present
 * @param[out]  : rqh - object of the physical request header structure
 * @return      : void
 * */
int PhyUtil::sessOutReqHeader(ClientSession& cli_sess,
                              const physical_request_header& rqh) {
  int err = 0;
  err = cli_sess.addOutput(rqh.client_sess_id);
  if (err != UNC_RC_SUCCESS) return err;
  err = cli_sess.addOutput(rqh.config_id);
  if (err != UNC_RC_SUCCESS) return err;
  err = cli_sess.addOutput(rqh.operation);
  if (err != UNC_RC_SUCCESS) return err;
  err = cli_sess.addOutput(rqh.max_rep_count);
  if (err != UNC_RC_SUCCESS) return err;
  err = cli_sess.addOutput(rqh.option1);
  if (err != UNC_RC_SUCCESS) return err;
  err = cli_sess.addOutput(rqh.option2);
  if (err != UNC_RC_SUCCESS) return err;
  err = cli_sess.addOutput(rqh.data_type);
  if (err != UNC_RC_SUCCESS) return err;
  err = cli_sess.addOutput(rqh.key_type);
  return err;
}

/**sessGetRespHeader
 * @Description : This function is for getting the repsonse from driver using
 *                clientsessioin object
 * @param[in]   : cli_sess - Object of ClientSession where the request
 *                argument present
 * @param[out]  : rsh - object of the physical response header structure
 * @return      : void
 * */
int PhyUtil::sessGetRespHeader(ClientSession& cli_sess,
                               physical_response_header& rsh) {
  int err = 0;
  err = cli_sess.getResponse(0, rsh.client_sess_id);
  if (err != UNC_RC_SUCCESS) return err;
  err = cli_sess.getResponse(1, rsh.config_id);
  if (err != UNC_RC_SUCCESS) return err;
  err = cli_sess.getResponse(2, rsh.operation);
  if (err != UNC_RC_SUCCESS) return err;
  err = cli_sess.getResponse(3, rsh.max_rep_count);
  if (err != UNC_RC_SUCCESS) return err;
  err = cli_sess.getResponse(4, rsh.option1);
  if (err != UNC_RC_SUCCESS) return err;
  err = cli_sess.getResponse(5, rsh.option2);
  if (err != UNC_RC_SUCCESS) return err;
  err = cli_sess.getResponse(6, rsh.data_type);
  if (err != UNC_RC_SUCCESS) return err;
  err = cli_sess.getResponse(7, rsh.result_code);
  return err;
}

/**printDriverReqHeader
 * @Description : This function prints the attributes that are present in
 *                driver request header structure
 * @param[in]   : rqh - object of the driver request header structure
 * @return      : void
 * */
void PhyUtil::printDriverReqHeader(const driver_request_header& rqh) {
  stringstream ss;
  ss  << "Request to Driver["
      << "sess_id:" << rqh.client_sess_id
      << ",conf_id:" << rqh.config_id
      << ",ctr_id: " << rqh.controller_id
      << ",dom_id: " << rqh.domain_id
      << ",oper: " << rqh.operation
      << ",max_rp_cnt: " << rqh.max_rep_count
      << ",opt1: " << rqh.option1
      << ",opt2: " << rqh.option2
      << ",dt: " << rqh.data_type
      << ",kt: " << rqh.key_type
      << "]";
  pfc_log_info((const char*)(ss.str().c_str()));
}

/**printDriverRespHeader
 * @Description : This function prints the attributes that are present in
 *                driver response header structure
 * @param[in]   : rsh - object of the driver response header structure
 * @return      : void
 * */
void PhyUtil::printDriverRespHeader(const driver_response_header& rsh) {
  stringstream ss;
  ss  << "Resp from Driver["
      << "sess_id:" << rsh.client_sess_id
      << ",conf_id:" << rsh.config_id
      << ",ctr_id:" << rsh.controller_id
      << ",dom_id:" << rsh.domain_id
      << ",oper:" << rsh.operation
      << ",max_rp_cnt:" << rsh.max_rep_count
      << ",opt1:" << rsh.option1
      << ",opt2:" << rsh.option2
      << ",dt:" << rsh.data_type
      << ",rc:" << rsh.result_code
      << "]";
  pfc_log_info((const char*)(ss.str().c_str()));
}

/**sessOutDriverReqHeader
 * @Description : This function is for sending request to driver using
 *                clientsessioin object
 * @param[in]   : cli_sess - Object of ClientSession where the request
 *                argument present
 * @param[out]  : rqh - object of the driver request header structure
 * @return      : void
 * */
int PhyUtil::sessOutDriverReqHeader(ClientSession& cli_sess,
                                    const driver_request_header& rqh) {
  int err = 0;
  err = cli_sess.addOutput(rqh.client_sess_id);
  if (err != UNC_RC_SUCCESS) return err;
  err = cli_sess.addOutput(rqh.config_id);
  if (err != UNC_RC_SUCCESS) return err;
  err = cli_sess.addOutput(rqh.controller_id);
  if (err != UNC_RC_SUCCESS) return err;
  err = cli_sess.addOutput(rqh.domain_id);
  if (err != UNC_RC_SUCCESS) return err;
  err = cli_sess.addOutput(rqh.operation);
  if (err != UNC_RC_SUCCESS) return err;
  err = cli_sess.addOutput(rqh.max_rep_count);
  if (err != UNC_RC_SUCCESS) return err;
  err = cli_sess.addOutput(rqh.option1);
  if (err != UNC_RC_SUCCESS) return err;
  err = cli_sess.addOutput(rqh.option2);
  if (err != UNC_RC_SUCCESS) return err;
  err = cli_sess.addOutput(rqh.data_type);
  if (err != UNC_RC_SUCCESS) return err;
  err = cli_sess.addOutput(rqh.key_type);
  if (err == 0) {
    printDriverReqHeader(rqh);
  }
  return err;
}

/**sessGetDriverRespHeader
 * @Description : This function is for getting the repsonse from driver using
 *                clientsessioin object
 * @param[in]   : cli_sess - Object of ClientSession where the request
 *                argument present
 * @param[out]  : rsh - object of the driver response header structure
 * @return      : void
 * */
int PhyUtil::sessGetDriverRespHeader(ClientSession& cli_sess,
                                     driver_response_header& rsh) {
  int err = 0;
  err = cli_sess.getResponse(0, rsh.client_sess_id);
  if (err != UNC_RC_SUCCESS) return err;
  err = cli_sess.getResponse(1, rsh.config_id);
  if (err != UNC_RC_SUCCESS) return err;
  const char* arr;
  err = cli_sess.getResponse(2, arr);
  if (err != UNC_RC_SUCCESS) return err;
  rsh.controller_id = arr;
  err = cli_sess.getResponse(3, arr);
  if (err != UNC_RC_SUCCESS) return err;
  rsh.domain_id = arr;
  err = cli_sess.getResponse(4, rsh.operation);
  if (err != UNC_RC_SUCCESS) return err;
  err = cli_sess.getResponse(5, rsh.max_rep_count);
  if (err != UNC_RC_SUCCESS) return err;
  err = cli_sess.getResponse(6, rsh.option1);
  if (err != UNC_RC_SUCCESS) return err;
  err = cli_sess.getResponse(7, rsh.option2);
  if (err != UNC_RC_SUCCESS) return err;
  err = cli_sess.getResponse(8, rsh.data_type);
  if (err != UNC_RC_SUCCESS) return err;
  err = cli_sess.getResponse(9, rsh.result_code);
  if (err == 0) {
    printDriverRespHeader(rsh);
  }
  return err;
}

/**sessGetDriverEventHeader
 * @Description : This function is for getting the event header from driver
 *                using clientsessioin object
 * @param[in]   : cli_sess - Object of ClientSession where the request
 *                argument present
 * @param[out]  : &rsh - object of the driver event header structure
 * @return      : void
 * */
int PhyUtil::sessGetDriverEventHeader(ClientSession& cli_sess,
                                      driver_event_header& rsh) {
  int err = 0;
  const char* val;
  err = cli_sess.getResponse(0, val);
  if (err != UNC_RC_SUCCESS) {
    pfc_log_debug("error reading controller");
    return err;
  }
  rsh.controller_id = val;
  err = cli_sess.getResponse(1, val);
  if (err != UNC_RC_SUCCESS) {
    pfc_log_debug("error reading domain");
    return err;
  }
  rsh.domain_id = val;
  err = cli_sess.getResponse(2, rsh.operation);
  if (err != UNC_RC_SUCCESS) {
    pfc_log_debug("error reading operation");
    return err;
  }
  err = cli_sess.getResponse(3, rsh.data_type);
  if (err != UNC_RC_SUCCESS) {
    pfc_log_debug("error reading data_type");
    return err;
  }
  err = cli_sess.getResponse(4, rsh.key_type);
  if (err != UNC_RC_SUCCESS) {
    pfc_log_debug("error reading key_type");
    return err;
  }
  stringstream ss;
  ss  << "Event to UPPL["
      << "ctr:" << rsh.controller_id
      << ",dom_id:" << rsh.domain_id
      << ",oper:" << rsh.operation
      << ",dt:" << rsh.data_type
      << ",kt:" << rsh.key_type
      << "]";
  pfc_log_info((const char*)(ss.str().c_str()));
  return err;
}

/**sessGetDriverAlarmHeader
 * @Description : This function is for getting the alarm header from driver
 *                using clientsessioin object
 * @param[in]   : cli_sess - Object of ClientSession where the request
 *                argument present
 * @param[out]  : &rsh - object of the driver alarm header structure
 * @return      : void
 * */
int PhyUtil::sessGetDriverAlarmHeader(ClientSession& cli_sess,
                                      driver_alarm_header& rsh) {
  int err = 0;
  const char* val;
  err = cli_sess.getResponse(0, val);
  if (err != UNC_RC_SUCCESS) {
    pfc_log_debug("error reading controller");
    return err;
  }
  rsh.controller_id = val;
  err = cli_sess.getResponse(1, val);
  if (err != UNC_RC_SUCCESS) {
    pfc_log_debug("error reading domain");
    return err;
  }
  rsh.domain_id = val;
  err = cli_sess.getResponse(2, rsh.operation);
  if (err != UNC_RC_SUCCESS) {
    pfc_log_debug("error reading operation");
    return err;
  }
  err = cli_sess.getResponse(3, rsh.data_type);
  if (err != UNC_RC_SUCCESS) {
    pfc_log_debug("error reading data_type");
    return err;
  }
  err = cli_sess.getResponse(4, rsh.key_type);
  if (err != UNC_RC_SUCCESS) {
    pfc_log_debug("error reading key_type");
    return err;
  }
  err = cli_sess.getResponse(5, rsh.alarm_type);
  if (err != UNC_RC_SUCCESS) {
    pfc_log_debug("error reading alarm_type");
    return err;
  }
  stringstream ss;
  ss  << "Alarm to UPPL["
      << "ctr:" << rsh.controller_id
      << ",dom_id:" << rsh.domain_id
      << ",oper:" << rsh.operation
      << ",dt:" << rsh.data_type
      << ",kt:" << rsh.key_type
      << ",alarm_tp:" << rsh.alarm_type
      << "]";
  pfc_log_info((const char*)(ss.str().c_str()));
  return err;
}

/**sessOutNBEventHeader
 * @Description : This function is for sending the event header to northbound
 *                using ServerEvent object
 * @param[in]   : cli_sess - Object of ServerEvent where the request
 *                argument present
 * @param[out]  : &rqh - object of the northbound event header structure
 * @return      : void
 * */
int PhyUtil::sessOutNBEventHeader(ServerEvent& cli_sess,
                                  const northbound_event_header& rqh) {
  int err = 0;
  err = cli_sess.addOutput(rqh.operation);
  if (err != UNC_RC_SUCCESS) return err;
  err = cli_sess.addOutput(rqh.data_type);
  if (err != UNC_RC_SUCCESS) return err;
  err = cli_sess.addOutput(rqh.key_type);
  stringstream ss;
  ss  << "Event from UPPL["
      << "oper:" << rqh.operation
      << ",dt:" << rqh.data_type
      << ",kt:" << rqh.key_type
      << "]";
  pfc_log_info((const char*)(ss.str().c_str()));
  return err;
}

/**sessOutNBAlarmHeader
 * @Description : This function is for sending the alarm header to northbound
 *                using ServerEvent object
 * @param[in]   : cli_sess - Object of ServerEvent where the request
 *                argument present
 * @param[out]  : &rqh - object of the northbound alarm header structure
 * @return      : void
 * */
int PhyUtil::sessOutNBAlarmHeader(ServerEvent& cli_sess,
                                  const northbound_alarm_header& rqh) {
  int err = 0;
  err = cli_sess.addOutput(rqh.operation);
  if (err != UNC_RC_SUCCESS) return err;
  err = cli_sess.addOutput(rqh.data_type);
  if (err != UNC_RC_SUCCESS) return err;
  err = cli_sess.addOutput(rqh.key_type);
  if (err != UNC_RC_SUCCESS) return err;
  err = cli_sess.addOutput(rqh.alarm_type);
  stringstream ss;
  ss  << "Alarm from UPPL["
      << "oper:" << rqh.operation
      << ",dt:" << rqh.data_type
      << ",kt:" << rqh.key_type
      << ",alarm_tp: " << rqh.alarm_type
      << "]";
  pfc_log_info((const char*)(ss.str().c_str()));
  return err;
}

/**uint8tostr
 * @Description : This function does the conversion of type uint8 to string
 * @param[in]   : c - varaiable of type uint8 
 * @return      : string
 * */
string PhyUtil::uint8tostr(const uint8_t& c) {
  char str[20];
  memset(&str, '\0', 20);
  snprintf(str, sizeof(str), "%d", c);
  string str1 = str;
  return str1;
}

/**uint16tostr
 * @Description : This function does the conversion of type uint16 to string
 * @param[in]   : c - varaiable of type uint16 
 * @return      : string
 * */
string PhyUtil::uint16tostr(const uint16_t& c) {
  char str[20];
  snprintf(str, sizeof(str), "%d", c);
  string str1 = str;
  return str1;
}

/**uint32tostr
 * @Description : This function does the conversion of type uint32 to string
 * @param[in]   : c - varaiable of type uint32 
 * @return      : string
 * */
string PhyUtil::uint32tostr(const uint32_t& c) {
  char str[20];
  snprintf(str, sizeof(str), "%u", c);
  string str1 = str;
  return str1;
}

/**uint64tostr
 * @Description : This function does the conversion of type uint64 to string
 * @param[in]   : c - varaiable of type uint64 
 * @return      : string
 * */
string PhyUtil::uint64tostr(const uint64_t& c) {
  char str[20];
  snprintf(str, sizeof(str), "%"PFC_PFMT_u64, c);
  string str1 = str;
  return str1;
}

/**uint8touint
 * @Description : This function does the conversion of type uint8 to uint
 * @param[in]   : c - varaiable of type uint8 
 * @return      : integer
 * */
int PhyUtil::uint8touint(const uint8_t& c) {
  stringstream stream;
  stream << c;
  return stream.get();
}

/**strtouint
 * @Description : This function does the conversion of type string to uint
 * @param[in]   : str - varaiable of type string
 * @return      : unsigned integer
 * */
unsigned int PhyUtil::strtouint(const string& str) {
  return atoi((const char*)(str.c_str()));
}

/**strtouint64
 * @Description : This function does the conversion of type string to uint64
 * @param[in]   : str - varaiable of type string
 * @return      : uint64
 * */
uint64_t PhyUtil::strtouint64(const string& str) {
  return atol((const char*)(str.c_str()));
}

/**FillDbSchema
 * @Description : This function fills the database schema with the attribute
 *                names ,attribute values ,attribute length and attribute type
 * @param[in]   : attr_name - Name of the attribute
 *                attr_value - Value of the attribute
 *                attr_length - length of the attribute
 *                attr_type - DATATYPE_* datatype of the attribute
 * @param[out]  : vect_attr - vector that contains all details of attributes
 * @return      : void
 * */
void PhyUtil::FillDbSchema(ODBCMTableColumns attr_name, string attr_value,
                           unsigned int attr_length,
                           AttributeDataType attr_type,
                           vector<TableAttrSchema> &vect_attr) {
  TableAttrSchema table_attr_schema;
  table_attr_schema.table_attribute_name = attr_name;
  switch (attr_type) {
    case DATATYPE_UINT16:
    {
      ColumnAttrValue <uint16_t> *value = new ColumnAttrValue <uint16_t>;
      value->value = strtouint(attr_value);
      table_attr_schema.p_table_attribute_value = value;
      break;
    }
    case DATATYPE_UINT64:
    {
      ColumnAttrValue <uint64_t> *value = new ColumnAttrValue <uint64_t>;
      value->value = strtouint64(attr_value);
      table_attr_schema.p_table_attribute_value = value;
      break;
    }
    case DATATYPE_UINT32:
    {
      ColumnAttrValue <uint32_t> *value = new ColumnAttrValue <uint32_t>;
      value->value = strtouint(attr_value);
      table_attr_schema.p_table_attribute_value = value;
      break;
    }
    case DATATYPE_IPV4:
    {
      ColumnAttrValue <uint32_t> *value=
          new ColumnAttrValue <uint32_t>;
      value->value = 0;
      if (!attr_value.empty()) {
        value->value = inet_addr(attr_value.c_str());
      }
      pfc_log_debug("ip address to db: %d", value->value);
      table_attr_schema.p_table_attribute_value = value;
      break;
    }

    case DATATYPE_UINT8_ARRAY_1:
      TABLE_ATTR_SCHEMA_UINT8_SET(table_attr_schema, attr_length, attr_value,
                                  1 + 1);
      break;

    case DATATYPE_UINT8_ARRAY_2:
      TABLE_ATTR_SCHEMA_UINT8_SET(table_attr_schema, attr_length, attr_value,
                                  2 + 1);
      break;

    case DATATYPE_UINT8_ARRAY_3:
      TABLE_ATTR_SCHEMA_UINT8_SET(table_attr_schema, attr_length, attr_value,
                                  3 + 1);
      break;

    case DATATYPE_UINT8_ARRAY_6:
      TABLE_ATTR_SCHEMA_UINT8_SET(table_attr_schema, attr_length, attr_value,
                                  6 + 1);
      break;

    case DATATYPE_UINT8_ARRAY_8:
      TABLE_ATTR_SCHEMA_UINT8_SET(table_attr_schema, attr_length, attr_value,
                                  8 + 1);
      break;

    case DATATYPE_UINT8_ARRAY_10:
      TABLE_ATTR_SCHEMA_UINT8_SET(table_attr_schema, attr_length, attr_value,
                                  10 + 1);
      break;

    case DATATYPE_UINT8_ARRAY_11:
      TABLE_ATTR_SCHEMA_UINT8_SET(table_attr_schema, attr_length, attr_value,
                                  11 + 1);
      break;

    case DATATYPE_IPV6:
    case DATATYPE_UINT8_ARRAY_16:
      TABLE_ATTR_SCHEMA_UINT8_SET(table_attr_schema, attr_length, attr_value,
                                  16 + 1);
      break;

    case DATATYPE_UINT8_ARRAY_32:
      TABLE_ATTR_SCHEMA_UINT8_SET(table_attr_schema, attr_length, attr_value,
                                  32 + 1);
      break;

    case DATATYPE_UINT8_ARRAY_128:
      TABLE_ATTR_SCHEMA_UINT8_SET(table_attr_schema, attr_length, attr_value,
                                  128 + 1);
      break;

    case DATATYPE_UINT8_ARRAY_256:
      TABLE_ATTR_SCHEMA_UINT8_SET(table_attr_schema, attr_length, attr_value,
                                  256 + 1);
      break;

    case DATATYPE_UINT8_ARRAY_257:
      TABLE_ATTR_SCHEMA_UINT8_SET(table_attr_schema, attr_length, attr_value,
                                  257 + 1);
      break;

    case DATATYPE_UINT8_ARRAY_320:
      TABLE_ATTR_SCHEMA_UINT8_SET(table_attr_schema, attr_length, attr_value,
                                  320 + 1);
      break;
  }
  table_attr_schema.request_attribute_type = attr_type;
  vect_attr.push_back(table_attr_schema);
}

/**FillDbSchema
 * @Description : This function fills the database schema with the attribute
 *                names ,attribute values ,attribute length and attribute type
 * @param[in]   : attr_name - Name of the attribute
 *                attr_value - Value of the attribute
 *                attr_length - length of the attribute
 *                attr_type - DATATYPE_* datatype of the attribute
 * @param[out]  : vect_attr - vector that contains all details of attributes
 * @return      : void
 * */
void PhyUtil::FillDbSchema(ODBCMTableColumns attr_name, uint8_t* attr_value,
                           unsigned int attr_length,
                           AttributeDataType attr_type,
                           vector<TableAttrSchema> &vect_attr) {
  TableAttrSchema table_attr_schema;
  table_attr_schema.table_attribute_name = attr_name;
  switch (attr_type) {
    case DATATYPE_UINT8_ARRAY_6: {
      ColumnAttrValue <unsigned char[6+1]> *value=
          new ColumnAttrValue <unsigned char[6+1]>;
      memset(&value->value, '\0', sizeof(value->value));
      memcpy(value->value,
             attr_value, attr_length+1);
      table_attr_schema.p_table_attribute_value = value;
      table_attr_schema.table_attribute_length = attr_length;
    }
    break;
    default:
      break;
  }
  table_attr_schema.request_attribute_type = attr_type;
  vect_attr.push_back(table_attr_schema);
}

/**FillDbSchema
 * @Description : This function fills the database schema with the attribute
 *                names ,attribute values ,attribute length and attribute type
 *                Validates the attributes with respect to operation type and
 *                valid_value
 * @param[in]   : attr_name - Name of the attribute
 *                attr_value - Value of the attribute
 *                attr_length - length of the attribute 
 *                attr_type - DATATYPE_* datatype of the attribute
 *                operation_type - UNC_OP_* specifies the type of operation
 *                in_valid_val - specifies the valid flag.Its a enum value
 *                prev_db_valid_val - specifies the db valid flag.
 *                Its a enum value
 * @param[out]  : vect_attr - vector that contains all details of attributes
 *                vect_prim_keys -vector that contains the primary keys
 *                out_valid_value - string that has the valid value
 * @return      : void
 * */
void PhyUtil::FillDbSchema(ODBCMTableColumns attr_name,
                           string attr_name_str,
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
      vect_prim_keys.push_back(attr_name_str);
    }
  } else if (operation_type >= UNC_OP_READ) {
    string empty = "";
    PhyUtil::FillDbSchema(attr_name, empty,
                          empty.length(), attr_type,
                          vect_attr);
  } else if (operation_type == UNC_OP_CREATE &&
      in_valid_val == UNC_VF_INVALID) {
    pfc_log_debug("Attribute '%s' is not given in create request",
                  attr_name_str.c_str());
    out_valid_value << UNC_VF_INVALID;
  } else if ((operation_type == UNC_OP_CREATE ||
      operation_type == UNC_OP_UPDATE) &&
      in_valid_val == UPPL_NO_VAL_STRUCT) {
    out_valid_value << UNC_VF_VALID;
    PhyUtil::FillDbSchema(attr_name, attr_value,
                          attr_value.length(), attr_type,
                          vect_attr);
  } else if (operation_type == UNC_OP_UPDATE &&
      in_valid_val == UNC_VF_VALID_NO_VALUE) {
    // empty value - value to be deleted
    pfc_log_debug("Attribute '%s' value is to be deleted in update request",
                  attr_name_str.c_str());
    string empty = "";
    PhyUtil::FillDbSchema(attr_name, empty,
                          empty.length(), attr_type,
                          vect_attr);
    if (attr_name == unc::uppl::CTR_ENABLE_AUDIT) {
      out_valid_value << UNC_VF_VALID;
    } else {
      out_valid_value << UNC_VF_INVALID;
    }
  } else if (operation_type == UNC_OP_CREATE) {
    out_valid_value << in_valid_val;
  } else if (operation_type == UNC_OP_UPDATE &&
      in_valid_val == UNC_VF_INVALID) {
    pfc_log_debug(
        "Attribute '%s' not given in Update Request. Retain db valid",
        attr_name_str.c_str());
    out_valid_value << prev_db_valid_val;
  }
}

/**FillDbSchema
 * @Description : This function fills the database schema with the attribute
 *                names ,attribute values ,attribute length and attribute type
 *                Validates the attributes with respect to operation type and
 *                valid_value
 * @param[in]   : attr_name - Name of the attribute
 *                attr_value - Value of the attribute
 *                attr_length - length of the attribute
 *                attr_type - DATATYPE_* datatype of the attribute
 *                operation_type - UNC_OP_* specifies the type of operation
 *                in_valid_val - specifies the valid flag.Its a enum value
 *                prev_db_valid_val - specifies the db valid flag.
 *                Its a enum value
 * @param[out]  : vect_attr - vector that contains all details of attributes
 *                vect_prim_keys -vector that contains the primary keys
 *                out_valid_value - string that has the valid value
 * @return      : void
 * */
void PhyUtil::FillDbSchema(ODBCMTableColumns attr_name,
                           string attr_name_str,
                           uint8_t* attr_value,
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
                          attr_length, attr_type,
                          vect_attr);
    out_valid_value << UNC_VF_VALID;
    if (PhyUtil::IsFilteringOperation(operation_type, in_valid_val) == true) {
      // To be used as filter and add in vect_primary_key
      vect_prim_keys.push_back(attr_name_str);
    }
  } else if (operation_type >= UNC_OP_READ) {
    string empty = "";
    PhyUtil::FillDbSchema(attr_name, empty,
                          empty.length(), attr_type,
                          vect_attr);
  } else if (operation_type == UNC_OP_CREATE &&
      in_valid_val == UNC_VF_INVALID) {
    pfc_log_debug("Attribute '%s' is not given in create request",
                  attr_name_str.c_str());
    out_valid_value << UNC_VF_INVALID;
  } else if ((operation_type == UNC_OP_CREATE ||
      operation_type == UNC_OP_UPDATE) &&
      in_valid_val == UPPL_NO_VAL_STRUCT) {
    pfc_log_debug("Attribute '%s' is not given in create/update request",
                  attr_name_str.c_str());
    out_valid_value << UNC_VF_VALID;
    PhyUtil::FillDbSchema(attr_name, attr_value,
                          attr_length, attr_type,
                          vect_attr);
  } else if (operation_type == UNC_OP_UPDATE &&
      in_valid_val == UNC_VF_VALID_NO_VALUE) {
    // empty value - value to be deleted
    pfc_log_debug("Attribute '%s' value is to be deleted in update request",
                  attr_name_str.c_str());
    string empty = "";
    PhyUtil::FillDbSchema(attr_name, empty,
                          empty.length(), attr_type,
                          vect_attr);
    out_valid_value << UNC_VF_INVALID;
  } else if (operation_type == UNC_OP_CREATE) {
    out_valid_value << in_valid_val;
  } else if (operation_type == UNC_OP_UPDATE &&
      in_valid_val == UNC_VF_INVALID) {
    pfc_log_debug(
        "Attribute '%s' not given in Update Request. Retain db valid",
        attr_name_str.c_str());
    out_valid_value << prev_db_valid_val;
  }
}

/**GetValueFromDbSchema
 * @Description : This function gets the attribute values thats stored in
 *                database schema
 * @param[in]   : table_attr_schema - Structure that contains the
 *                attribute values
 *                attr_type - DATATYPE_* datatype of the attribute
 * @param[out]  : attr_value - string that has Value of the attribute
 * @return      : void
 * */
void PhyUtil::GetValueFromDbSchema(const TableAttrSchema& table_attr_schema,
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
        memset(&ipv4_addr, 0, sizeof(sockaddr_in));
        ipv4_addr.sin_addr.s_addr = value->value;
        ss << inet_ntoa(ipv4_addr.sin_addr);
        pfc_log_debug("ip address from db: %d", ipv4_addr.sin_addr.s_addr);
      } else {
        string empty = "";
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
    default:
      break;
  }
  attr_value = ss.str();
}

/**GetValueFromDbSchemaStr
 * @Description : This function gets the attribute values thats stored in
 *                database schema
 * @param[in]   : table_attr_schema - Structure that contains the
 *                attribute values
 *                attr_type - DATATYPE_* datatype of the attribute
 * @param[out]  : attr_value - string that has Value of the attribute
 * @return      : void
 * */
void PhyUtil::GetValueFromDbSchemaStr(const TableAttrSchema& table_attr_schema,
                                      uint8_t *attr_value,
                                      AttributeDataType attr_type) {
  switch (attr_type) {
    case DATATYPE_UINT8_ARRAY_1:
    {
      ColumnAttrValue <unsigned char[1+1]> *value =
          (ColumnAttrValue <unsigned char[1+1]>*)
          table_attr_schema.p_table_attribute_value;
      memcpy(attr_value, value->value, strlen((const char*)value->value)+1);
      break;
    }
    case DATATYPE_UINT8_ARRAY_2:
    {
      ColumnAttrValue <unsigned char[2+1]> *value =
          (ColumnAttrValue <unsigned char[2+1]>*)
          table_attr_schema.p_table_attribute_value;
      memcpy(attr_value, value->value, strlen((const char*)value->value)+1);
      break;
    }
    case DATATYPE_UINT8_ARRAY_3:
    {
      ColumnAttrValue <unsigned char[3+1]> *value =
          (ColumnAttrValue <unsigned char[3+1]>*)
          table_attr_schema.p_table_attribute_value;
      memcpy(attr_value, value->value, strlen((const char*)value->value)+1);
      break;
    }
    case DATATYPE_UINT8_ARRAY_6:
    {
      ColumnAttrValue <unsigned char[6+1]> *value =
          (ColumnAttrValue <unsigned char[6+1]>*)
          table_attr_schema.p_table_attribute_value;
      memcpy(attr_value, value->value, strlen((const char*)value->value)+1);
      memcpy(attr_value, value->value, 6);
      break;
    }
    case DATATYPE_UINT8_ARRAY_8:
    {
      ColumnAttrValue <unsigned char[8+1]> *value =
          (ColumnAttrValue <unsigned char[8+1]>*)
          table_attr_schema.p_table_attribute_value;
      memcpy(attr_value, value->value, strlen((const char*)value->value)+1);
      break;
    }
     case DATATYPE_UINT8_ARRAY_10:
    {
      ColumnAttrValue <unsigned char[10+1]> *value =
          (ColumnAttrValue <unsigned char[10+1]>*)
          table_attr_schema.p_table_attribute_value;
      memcpy(attr_value, value->value, strlen((const char*)value->value)+1);
      break;
    }
    case DATATYPE_UINT8_ARRAY_11:
    {
      ColumnAttrValue <unsigned char[11+1]> *value =
          (ColumnAttrValue <unsigned char[11+1]>*)
          table_attr_schema.p_table_attribute_value;
      memcpy(attr_value, value->value, strlen((const char*)value->value)+1);
      break;
    }
    case DATATYPE_UINT8_ARRAY_16:
    {
      ColumnAttrValue <unsigned char[16+1]> *value =
          (ColumnAttrValue <unsigned char[16+1]>*)
          table_attr_schema.p_table_attribute_value;
      memcpy(attr_value, value->value, strlen((const char*)value->value)+1);
      break;
    }
    case DATATYPE_UINT8_ARRAY_32:
    {
      ColumnAttrValue <unsigned char[32+1]> *value =
          (ColumnAttrValue <unsigned char[32+1]>*)
          table_attr_schema.p_table_attribute_value;
      memcpy(attr_value, value->value, strlen((const char*)value->value)+1);
      break;
    }
    case DATATYPE_UINT8_ARRAY_128:
    {
      ColumnAttrValue <unsigned char[128+1]> *value =
          (ColumnAttrValue <unsigned char[128+1]>*)
          table_attr_schema.p_table_attribute_value;
      memcpy(attr_value, value->value, strlen((const char*)value->value)+1);
      break;
    }
    case DATATYPE_UINT8_ARRAY_256:
    {
      ColumnAttrValue <unsigned char[256+1]> *value =
          (ColumnAttrValue <unsigned char[256+1]>*)
          table_attr_schema.p_table_attribute_value;
      memcpy(attr_value, value->value, strlen((const char*)value->value)+1);
      break;
    }
    case DATATYPE_UINT8_ARRAY_257:
    {
      ColumnAttrValue <unsigned char[257+1]> *value =
          (ColumnAttrValue <unsigned char[257+1]>*)
          table_attr_schema.p_table_attribute_value;
      memcpy(attr_value, value->value, strlen((const char*)value->value)+1);
      break;
    }
    case DATATYPE_UINT8_ARRAY_320:
    {
      ColumnAttrValue <unsigned char[320+1]> *value =
          (ColumnAttrValue <unsigned char[320+1]>*)
          table_attr_schema.p_table_attribute_value;
      memcpy(attr_value, value->value, strlen((const char*)value->value)+1);
      break;
    }
    default:
      break;
  }
}

/**get_controller_type
 * @Description : This function gets the controller type of the specified 
 *                controller by sending the request to odbc manager
 * @param[in]   : controller_name - name of the controller
 *                datatype - UNC_DT_* specifies the datatype
 * @param[out]  : controller_type - UNC_CT_* Type of the controller(UNKNOWN,
 *                PFC,VNP)
 * @return      : UNC_RC_SUCCESS is returned when the response
 *                is added to ipc session successfully.
 *                UNC_UPPL_RC_ERR_* is returned when ipc response could not
 *                be added to session.
 * */
UncRespCode PhyUtil::get_controller_type(
    OdbcmConnectionHandler *db_conn,
    string controller_name,
    unc_keytype_ctrtype_t& controller_type,
    unc_keytype_datatype_t datatype) {
  UncRespCode ret_code = UNC_RC_SUCCESS;
  string type = "";
  // Creating the Physical Layer instance
  PhysicalLayer *physical_layer = PhysicalLayer::get_instance();

  // Creating the object that consists of vectors of Table Atrr Schema
  vector<TableAttrSchema> vect_table_attr_schema;
  vector<string> vect_prim_keys;  // Construct Primary key list
  vect_prim_keys.push_back(CTR_NAME_STR);
  pfc_log_debug("PhyUtil::controller_name is %s", controller_name.c_str());
  PhyUtil::FillDbSchema(unc::uppl::CTR_NAME, controller_name,
                        controller_name.length(), DATATYPE_UINT8_ARRAY_32,
                        vect_table_attr_schema);
  PhyUtil::FillDbSchema(unc::uppl::CTR_TYPE, type,
                        type.length(), DATATYPE_UINT16,
                        vect_table_attr_schema);
  // Structure used to send request to ODBC
  DBTableSchema dbtableschema_obj;
  dbtableschema_obj.set_primary_keys(vect_prim_keys);
  dbtableschema_obj.set_table_name(unc::uppl::CTR_TABLE);
  dbtableschema_obj.PushBackToRowList(vect_table_attr_schema);
  ODBCM_RC_STATUS db_status = physical_layer->get_odbc_manager()-> \
      GetOneRow(datatype, dbtableschema_obj, db_conn);
  if (db_status == ODBCM_RC_CONNECTION_ERROR) {
    // log fatal error to log daemon
    pfc_log_error("DB connection not available or cannot access DB");
    ret_code = UNC_UPPL_RC_ERR_DB_ACCESS;
    return ret_code;
  } else if (db_status != ODBCM_RC_SUCCESS) {
    string log_msg = "Unable to get controller type from the database";
    pfc_log_error((const char *)log_msg.c_str());
    ret_code = UNC_UPPL_RC_ERR_DB_GET;
    return ret_code;
  }
  list< vector<TableAttrSchema> >& row_list_iter =
      dbtableschema_obj.get_row_list();
  list< vector<TableAttrSchema> >::iterator vect_iter =
      row_list_iter.begin();
  vector<TableAttrSchema> ::iterator tab_iter = (*vect_iter).begin();
  for ( ; tab_iter != (*vect_iter).end(); ++tab_iter) {
    TableAttrSchema tab_schema = (*tab_iter);
    ODBCMTableColumns attr_name = tab_schema.table_attribute_name;
    string attr_value = "";
    if (attr_name == unc::uppl::CTR_TYPE) {
      PhyUtil::GetValueFromDbSchema(tab_schema, attr_value,
                                    DATATYPE_UINT16);
      controller_type = (unc_keytype_ctrtype_t)(atoi(attr_value.c_str()));
    }
  }
  pfc_log_debug("Controller Type return: %d, type value %d",
                ret_code, controller_type);
  return ret_code;
}

/**ConvertToControllerName
 * @Description : This function gets the controller name given actual
 *                controller id by sending the request to odbc manager
 * @param[in]   : actual_controller_id - actual controller id
 * @param[out]  : ctr_name
 * @return    : success/failure is returned
 **/
UncRespCode PhyUtil::ConvertToControllerName(OdbcmConnectionHandler *db_conn,
                                       string actual_controller_id,
                                       string &ctr_name) {
  //  if driver receives empty pfc name, driver sends
  //  connectedcontroller as "N/A" to uppl in kt_port_neighbor event
  //  if uppl found this string, conversion should fail, so set ctr_name=""
  std::string nastring("N/A");
  if (actual_controller_id.compare(nastring) == 0) {
    pfc_log_debug("actual_controller_id is N/A");
    ctr_name = "";
    return UNC_RC_SUCCESS;
  }
  // Creating the Physical Layer instance
  PhysicalLayer *physical_layer = PhysicalLayer::get_instance();

  // Creating the object that consists of vectors of Table Atrr Schema
  vector<TableAttrSchema> vect_table_attr_schema;
  vector<string> vect_prim_keys;  // Construct Primary key list
  vector<ODBCMOperator> vect_operators;
  vect_prim_keys.push_back(CTR_ACTUAL_CONTROLLERID_STR);
  vect_operators.push_back(unc::uppl::EQUAL);
  pfc_log_debug("PhyUtil::actual_controller_id is %s",
      actual_controller_id.c_str());
  PhyUtil::FillDbSchema(unc::uppl::CTR_ACTUAL_CONTROLLERID,
                        actual_controller_id,
                        actual_controller_id.length(), DATATYPE_UINT8_ARRAY_32,
                        vect_table_attr_schema);

  PhyUtil::FillDbSchema(unc::uppl::CTR_NAME, ctr_name,
                        ctr_name.length(), DATATYPE_UINT8_ARRAY_32,
                        vect_table_attr_schema);
  string ac_valid = "";
  // valid_actual_Id
  PhyUtil::FillDbSchema(unc::uppl::CTR_VALID_ACTUAL_CONTROLLERID, ac_valid,
                        ac_valid.length(), DATATYPE_UINT8_ARRAY_1,
                        vect_table_attr_schema);

  // Structure used to send request to ODBC
  DBTableSchema dbtableschema_obj;
  dbtableschema_obj.set_primary_keys(vect_prim_keys);
  dbtableschema_obj.set_table_name(unc::uppl::CTR_TABLE);
  dbtableschema_obj.PushBackToRowList(vect_table_attr_schema);
  ODBCM_RC_STATUS db_status = physical_layer->get_odbc_manager()-> \
      GetSiblingRows(UNC_DT_RUNNING, UPPL_MAX_REP_CT, dbtableschema_obj,
                  vect_operators, UNC_OP_READ_SIBLING, db_conn);
  if (db_status == ODBCM_RC_CONNECTION_ERROR) {
    pfc_log_error("DB connection not available or cannot access DB");
    return UNC_UPPL_RC_ERR_DB_ACCESS;
  } else if (db_status != ODBCM_RC_SUCCESS) {
    string log_msg = "Unable to get controller name from the database";
    pfc_log_error((const char *)log_msg.c_str());
    return UNC_UPPL_RC_ERR_DB_GET;
  }
  uint8_t actual_id_valid[1];
  memset(actual_id_valid, '\0', ODBCM_SIZE_1);
  list< vector<TableAttrSchema> >& row_list =
      dbtableschema_obj.get_row_list();
  list< vector<TableAttrSchema> >::iterator list_iter =
      row_list.begin();
  for ( ; list_iter != row_list.end(); list_iter++) {
    vector<TableAttrSchema> ::iterator tab_iter = (*list_iter).begin();
    for ( ; tab_iter != (*list_iter).end(); ++tab_iter) {
      TableAttrSchema tab_schema = (*tab_iter);
      ODBCMTableColumns attr_name = tab_schema.table_attribute_name;
      if (attr_name == unc::uppl::CTR_NAME) {
        uint8_t db_ctr_name[ODBCM_SIZE_32];
        memset(db_ctr_name, '\0', ODBCM_SIZE_32);
        PhyUtil::GetValueFromDbSchemaStr(tab_schema, db_ctr_name,
                                  DATATYPE_UINT8_ARRAY_32);
        ctr_name = reinterpret_cast<const char*>(db_ctr_name);
      } else if (attr_name == unc::uppl::CTR_VALID_ACTUAL_CONTROLLERID) {
        PhyUtil::GetValueFromDbSchemaStr(tab_schema,
                                       actual_id_valid,
                                       DATATYPE_UINT8_ARRAY_1);
      }
    }
    if (static_cast<int>(actual_id_valid[0] - 48) == UNC_VF_VALID) {
      break;
    } else {
      pfc_log_debug("Actual Id is invalid, hence fetching the next record");
      ctr_name = "";
    }
  }
  pfc_log_info("Controller Name:actualID %s:%s", ctr_name.c_str(),
                                            actual_controller_id.c_str());
  return UNC_RC_SUCCESS;
}

/**reorder_col_attrs
 * @Description : This function reorders the attributes present in the column
 *                attribute table in table attribute schema
 * @param[in]   : vect_prim_keys - vector that contains the primary keys
 *                vect_table_attr_schema - vector that contains the table
 *                attribute schema structure
 * @return      : void
 * */
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
      if (key_attr_name == ODBCManager::get_ODBCManager()->GetColumnName(
          col_attr.table_attribute_name)) {
        vect_table_attr_schema.erase(tab_iter);
        vect_table_attr_schema.insert(vect_table_attr_schema.begin(),
                                      col_attr);
        break;
      }
    }
  }
}

/**IsValidValue
 * @Description : This function checks whether the value in value
 *                structure can be considered
 * @param[in]   : operation type - UNC_OP_* specifies the type of operation
 *                valid - speicifies the valid flag
 * @return      : PFC_TRUE is returned if value is considered otherwise
 *                PFC_FALSE is returned to denote error
 * */
bool PhyUtil::IsValidValue(uint32_t operation_type,
                           unsigned int valid) {
  if (valid == UPPL_NO_VAL_STRUCT) {
    return false;
  }
  if ((operation_type == UNC_OP_READ) ||
      (operation_type == UNC_OP_READ_SIBLING_BEGIN) ||
      (operation_type == UNC_OP_READ_SIBLING) ||
      (operation_type == UNC_OP_READ_SIBLING_COUNT) ||
      (operation_type == UNC_OP_CREATE && valid == UNC_VF_VALID) ||
      (operation_type == UNC_OP_UPDATE && valid == UNC_VF_VALID)) {
    return true;
  } else {
    return false;
  }
}

/**IsFilteringOperation
 * @Description : This function checks whether the value in value
 *                structure has to be used for filertering
 * @param[in]   : operation type - UNC_OP_* specifies the type of operation
 *                valid - speicifies the valid flag
 * @return      : PFC_TRUE is returned if value in value structure is used for
 *                filertering otherwise PFC_FALSE is returned to denote error
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
/**getEventDetailsString
 * @Description : This function to covert the pfc_ipcevtype_t event type
 *                to string
 * @param[in]   : pfc_ipcevtype_t 
 *                
 * @return      : string of pfc_ipcevtype_t event type 
 *                
 * */
std::string PhyUtil::getEventDetailsString(pfc_ipcevtype_t event_type) {
  switch (event_type) {
    case UPPL_EVENTS_KT_LOGICAL_PORT:
      return "UPPL_EVENTS_KT_LOGICAL_PORT";
    case UPPL_EVENTS_KT_PORT:
      return "UPPL_EVENTS_KT_PORT";
    case UPPL_EVENTS_KT_LINK:
      return "UPPL_EVENTS_KT_LINK";
    case UPPL_EVENTS_KT_SWITCH:
      return "UPPL_EVENTS_KT_SWITCH";
    case UPPL_EVENTS_KT_CTR_DOMAIN:
      return "UPPL_EVENTS_KT_CTR_DOMAIN";
    case UPPL_EVENTS_KT_BOUNDARY:
      return "UPPL_EVENTS_KT_BOUNDARY";
    case UPPL_ALARMS_PHYS_PATH_FAULT:
      return "UPPL_ALARMS_PHYS_PATH_FAULT";
    case UPPL_EVENTS_KT_CONTROLLER:
      return "UPPL_EVENTS_KT_CONTROLLER";
    default:
      pfc_log_info("no matching event type found return """);
  }
  return "";
}

