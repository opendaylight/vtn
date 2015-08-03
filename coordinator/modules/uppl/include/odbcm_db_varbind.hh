/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/**
 * @brief   ODBC Manager
 * @file    odbcm_db_varbind.hh
 *
 */

#ifndef ODBCM_DB_VARBIND_HH_
#define ODBCM_DB_VARBIND_HH_

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <vector>
#include "odbcm_common.hh"
#include "odbcm_db_tableschema.hh"
#include "physicallayer.hh"

namespace unc {
namespace uppl {

/*
 * Macros to define the attribute size
 */
#define ODBCM_SIZE_1            1
#define ODBCM_SIZE_2            2
#define ODBCM_SIZE_3            3
#define ODBCM_SIZE_4            4
#define ODBCM_SIZE_5            5
#define ODBCM_SIZE_6            6
#define ODBCM_SIZE_7            7
#define ODBCM_SIZE_8            8
#define ODBCM_SIZE_10           10
#define ODBCM_SIZE_11           11
#define ODBCM_SIZE_14           14
#define ODBCM_SIZE_15           15
#define ODBCM_SIZE_16           16
#define ODBCM_SIZE_19           19
#define ODBCM_SIZE_20           20
#define ODBCM_SIZE_32           32
#define ODBCM_SIZE_33           33
#define ODBCM_SIZE_128          128
#define ODBCM_SIZE_256          256
#define ODBCM_SIZE_257          257
#define ODBCM_SIZE_320          320
/* 
 * uppl memcpy macro
 */
#define ODBCM_MEMCPY(dst, src, size)                      \
  if ((src) != NULL && (size) > 0)                                \
  memcpy((dst), (src), (size));
/* 
 * uppl memset macro
 */
#define ODBCM_MEMSET(dst, val, size)                      \
  memset((dst), (val), (size));
/* 
 * Allocate memory for ColumnAttrValue template, 
 * this will be called up in the fetch functions
 */
#define ODBCM_ALLOCATE_COLUMN_ATTRVALUE_T(dt, T_name)          \
  ColumnAttrValue <dt> *(T_name) = new ColumnAttrValue <dt>; \
  ODBCM_MEMSET((T_name), '\0', sizeof(dt));

/* 
 * To bind the input datatypes
 * for binding SQL_VARCHAR data type
 */
#define BindInputParameter_SQL_VARCHAR(stmt, param_no,        \
  col_size, decimal, buffer, buf_len, lenptr)   \
  SQLBindParameter((stmt), (param_no), SQL_PARAM_INPUT,           \
      SQL_C_CHAR, SQL_VARCHAR, (col_size), (decimal),             \
      (buffer), (buf_len), (lenptr))
/* 
 * For binding SQL_SMALLINT data type
 */
#define BindInputParameter_SQL_SMALLINT(stmt, param_no,       \
    col_size, decimal, buffer, buf_len, lenptr)  \
      SQLBindParameter((stmt), (param_no), SQL_PARAM_INPUT,       \
      SQL_C_SHORT, SQL_SMALLINT, (col_size), (decimal),           \
      (buffer), (buf_len), (lenptr))
/* 
 * For binding SQL_INTIGER data type
 */
#define BindInputParameter_SQL_INTEGER(stmt, param_no,        \
    col_size, decimal, buffer, buf_len, lenptr)  \
    SQLBindParameter((stmt), (param_no), SQL_PARAM_INPUT,         \
      SQL_C_LONG, SQL_INTEGER, (col_size), (decimal),             \
      (buffer), (buf_len), (lenptr))
/* 
 * For binding SQL_BIGINT( data type
 */
#define BindInputParameter_SQL_BIGINT(stmt, param_no,         \
    col_size, decimal, buffer, buf_len, lenptr)  \
    SQLBindParameter((stmt), (param_no), SQL_PARAM_INPUT,         \
      SQL_C_UBIGINT, SQL_BIGINT, (col_size), (decimal),           \
      (buffer), (buf_len), (lenptr))
/* 
 * For binding SQL_CHAR data type
 */
#define BindInputParameter_SQL_CHAR(stmt, param_no,           \
    col_size, decimal, buffer, buf_len, lenptr)  \
    SQLBindParameter((stmt), (param_no), SQL_PARAM_INPUT,         \
      SQL_C_CHAR, SQL_CHAR, (col_size), (decimal),                \
      (buffer), (buf_len), (lenptr))
/*
 * For binding SQL_BINARY data type 
 * calculate the length of buffer and assign to lenptr
 */
#define BindInputParameter_SQL_BINARY(stmt, param_no,         \
    col_size, decimal, buffer, buf_len, lenptr)  \
    SQLBindParameter((stmt), (param_no), SQL_PARAM_INPUT,         \
      SQL_C_BINARY, SQL_BINARY, (col_size), (decimal),            \
      (buffer), (buf_len), (lenptr))
/* 
 * To bind the output datatypes 
 * For binding SQL_VARCHAR data type
 */
#define BindCol_SQL_VARCHAR(stmt, param_no, buffer,   \
    col_size, lenptr)                                     \
    SQLBindCol((stmt), (param_no), SQL_C_CHAR, (buffer),    \
      (col_size), (lenptr))
/*
 * For binding SQL_SMALLINT data type
 */
#define BindCol_SQL_SMALLINT(stmt, param_no, buffer,  \
    col_size, lenptr)                                    \
    SQLBindCol((stmt), (param_no), SQL_C_SHORT, (buffer),   \
      (col_size), (lenptr))
/* 
 * For binding SQL_INTEGER data type
 */
#define BindCol_SQL_INTEGER(stmt, param_no, buffer,   \
    col_size, lenptr)                                    \
    SQLBindCol((stmt), (param_no), SQL_C_LONG, (buffer),    \
      (col_size), (lenptr))
/* 
 * For binding SQL_BIGINT data type
 */
#define BindCol_SQL_BIGINT(stmt, param_no, buffer,    \
    col_size, lenptr)                                     \
    SQLBindCol((stmt), (param_no), SQL_C_UBIGINT, (buffer), \
      (col_size), (lenptr))
/* 
 * For binding SQL_BINARY data type
 */
#define BindCol_SQL_BINARY(stmt, param_no, buffer,    \
    col_size, lenptr)                                    \
    SQLBindCol((stmt), (param_no), SQL_C_BINARY, (buffer), \
      (col_size), (lenptr))

/*
 * Binding types, Input binding or Output binding
 */
enum stream {
  BIND_IN,
  BIND_OUT
};
/* 
 * Table row exists possible return stat 
 */
enum is_exists {
  UNKNOWN = -1,
  NOT_EXISTS,
  EXISTS
};

/* 
 * This structure is having the column details in same order
 * created in DB tables.  Below table structure are common 
 * for all databases (CANDIDATE,RUNNING,STARTUP, STATE & IMPORT)
 */

/*
 * IsRowExists, cs_row_status - outbind structure
 */
typedef struct {
    SQLSMALLINT is_exists;
    SQLSMALLINT cs_row_status;
    SQLLEN        cbIsExistsNum;
    SQLLEN        cbRowStatusNum;
}is_row_exists_t;

/** 
 * Members of controller_table 
 * +1 is added to store the '\0'
 */
typedef struct {
    SQLVARCHAR    szcontroller_name[ODBCM_SIZE_32+1];
    SQLSMALLINT   stype;
    SQLVARCHAR    szversion[ODBCM_SIZE_32+1];
    SQLVARCHAR    szdescription[ODBCM_SIZE_128+1];
    SQLBIGINT     szip_address;
    SQLVARCHAR    szuser_name[ODBCM_SIZE_32+1];
    SQLVARCHAR    szpassword[ODBCM_SIZE_257+1];
    SQLSMALLINT   senable_audit;
    SQLINTEGER    sport;
    SQLVARCHAR    szactual_version[ODBCM_SIZE_32+1];
    SQLSMALLINT   soper_status;
    SQLVARCHAR    szactual_id[ODBCM_SIZE_32+1];
    SQLVARCHAR    svalid_actual_id[ODBCM_SIZE_1+1];
    SQLCHAR       svalid[ODBCM_SIZE_10+1];
    SQLSMALLINT   scs_row_status;
    SQLCHAR       scs_attr[ODBCM_SIZE_10+1];
    SQLUBIGINT    scommit_number;
    SQLUBIGINT    scommit_date;
    SQLVARCHAR    szcommit_application[ODBCM_SIZE_256+1];
    SQLVARCHAR    svalid_commit_version[ODBCM_SIZE_3+1];
    SQLLEN        cbname;
    SQLLEN        cbtype;
    SQLLEN        cbversion;
    SQLLEN        cbctrdesc;
    SQLLEN        cbipaddr;
    SQLLEN        cbuser;
    SQLLEN        cbpass;
    SQLLEN        cbenableaudit;
    SQLLEN        cbport;
    SQLLEN        cbaversion;
    SQLLEN        cboperstatus;
    SQLLEN        cbvalid;
    SQLLEN        cbrowstatus;
    SQLLEN        cbcsattr;
    SQLLEN        cbcommitnumber;
    SQLLEN        cbcommitdate;
    SQLLEN        cbcommitapplication;
    SQLLEN        cbacname;
    SQLLEN        cbvalid_cv;
    SQLLEN        cbvalid_ac;
}controller_table_t;

/** 
 * Members of ctr_domain_table
 * +1 is added to store the '\0'
 */
typedef struct {
    SQLVARCHAR    szcontroller_name[ODBCM_SIZE_32+1];
    SQLVARCHAR    szdomain_name[ODBCM_SIZE_32+1];
    SQLSMALLINT   stype;
    SQLVARCHAR    szdescription[ODBCM_SIZE_128+1];
    SQLSMALLINT   soper_status;
    SQLCHAR       svalid[ODBCM_SIZE_3+1];
    SQLSMALLINT   scs_row_status;
    SQLCHAR       scs_attr[ODBCM_SIZE_3+1];
    SQLLEN        cbname;
    SQLLEN        cbdomain;
    SQLLEN        cbtype;
    SQLLEN        cbdesc;
    SQLLEN        cboperstatus;
    SQLLEN        cbvalid;
    SQLLEN        cbrowstatus;
    SQLLEN        cbcsattr;
}ctr_domain_table_t;

/** 
 * Members of logicalport_table
 * +1 is added to store the '\0'
 */
typedef struct {
    SQLVARCHAR    szController_name[ODBCM_SIZE_32+1];
    SQLVARCHAR    szdomain_name[ODBCM_SIZE_32+1];
    SQLVARCHAR    szport_id[ODBCM_SIZE_320+1];
    SQLVARCHAR    szdescription[ODBCM_SIZE_128+1];
    SQLSMALLINT   sport_type;
    SQLVARCHAR    szswitch_id[ODBCM_SIZE_256+1];
    SQLVARCHAR    szphysical_port_id[ODBCM_SIZE_32+1];
    SQLSMALLINT   soper_down_criteria;
    SQLSMALLINT   soper_status;
    SQLCHAR       svalid[ODBCM_SIZE_6+1];
    SQLLEN        cbname;
    SQLLEN        cbdomain;
    SQLLEN        cbdesc;
    SQLLEN        cbporttype;
    SQLLEN        cbpportid;
    SQLLEN        cbopercriteria;
    SQLLEN        cboperstatus;
    SQLLEN        cbvalid;
}logicalport_table_t;

/** 
 * Members of logicalmemberport_table
 * +1 is added to store the '\0'
 */
typedef struct {
    SQLVARCHAR    szController_name[ODBCM_SIZE_32+1];
    SQLVARCHAR    szDomain_name[ODBCM_SIZE_32+1];
    SQLVARCHAR    szlogical_port_id[ODBCM_SIZE_320+1];
    SQLVARCHAR    szswitch_id[ODBCM_SIZE_256+1];
    SQLVARCHAR    szphysical_port_id[ODBCM_SIZE_32+1];
    SQLLEN        cbname;
    SQLLEN        cbdomain;
    SQLLEN        cbpportid;
}logical_memberport_table_t;

/** 
 * Members of switch_table
 * +1 is added to store the '\0'
 */
typedef struct {
    SQLVARCHAR    szController_name[ODBCM_SIZE_32+1];
    SQLVARCHAR    szswitch_id[ODBCM_SIZE_256+1];
    SQLVARCHAR    szdescription[ODBCM_SIZE_128+1];
    SQLVARCHAR    szmodel[ODBCM_SIZE_16+1];
    SQLUBIGINT    szip_address;
    SQLCHAR       szipv6_address[ODBCM_SIZE_16+1];
    SQLSMALLINT   sadmin_status;
    SQLVARCHAR    szdomain_name[ODBCM_SIZE_32+1];
    SQLSMALLINT   soper_status;
    SQLVARCHAR    szmanufacturer[ODBCM_SIZE_256+1];
    SQLVARCHAR    szhardware[ODBCM_SIZE_256+1];
    SQLVARCHAR    szsoftware[ODBCM_SIZE_256+1];
    SQLUBIGINT    salarms_status;
    SQLCHAR       svalid[ODBCM_SIZE_11+1];
    SQLLEN        cbname;
    SQLLEN        cbdesc;
    SQLLEN        cbmodel;
    SQLLEN        cbipaddr;
    SQLLEN        cbadminstatus;
    SQLLEN        cbdomainname;
    SQLLEN        cboperstatus;
    SQLLEN        cbmanufacturer;
    SQLLEN        cbhardware;
    SQLLEN        cbsoftware;
    SQLLEN        cbvalid;
}switch_table_t;

/** 
 * Members of port_table
 * +1 is added to store the '\0'
 */
typedef struct {
    SQLVARCHAR    szcontroller_name[ODBCM_SIZE_32+1];
    SQLCHAR       szswitch_id[ODBCM_SIZE_256+1];
    SQLVARCHAR    szport_id[ODBCM_SIZE_32+1];
    SQLBIGINT     sport_number;
    SQLVARCHAR    szdescription[ODBCM_SIZE_128+1];
    SQLSMALLINT   sadmins_status;
    SQLSMALLINT   sdirection;
    SQLINTEGER    strunk_allowed_vlan;
    SQLSMALLINT   soper_status;
    SQLVARCHAR    smac_address[ODBCM_SIZE_6+1];
    SQLSMALLINT   sduplex;
    SQLUBIGINT    sspeed;
    SQLUBIGINT    salarms_status;
    SQLCHAR       slogical_port_id[ODBCM_SIZE_320+1];
    SQLCHAR       svalid[ODBCM_SIZE_11+1];
    SQLCHAR       szconnected_switch_id[ODBCM_SIZE_256+1];
    SQLVARCHAR    szconnected_port_id[ODBCM_SIZE_32+1];
    SQLVARCHAR    szconnected_controller_id[ODBCM_SIZE_32+1];
    SQLCHAR       szconnectedneighbor_valid[ODBCM_SIZE_3+1];
    SQLLEN        cbname;
    SQLLEN        cbportid;
    SQLLEN        cbportnumber;
    SQLLEN        cbdesc;
    SQLLEN        cbadminstatus;
    SQLLEN        cbdirection;
    SQLLEN        cbtavlan;
    SQLLEN        cboperstatus;
    SQLLEN        cbduplex;
    SQLLEN        cbvalid;
    SQLLEN        cbconnswitchid;
    SQLLEN        cbconnportid;
    SQLLEN        cbconnctrid;
    SQLLEN        cbconnvalid;
}port_table_t;

/** 
 * Members of link_table
 * +1 is added to store the '\0'
 */
typedef struct {
    SQLVARCHAR    szcontroller_name[ODBCM_SIZE_32+1];
    SQLVARCHAR    szswitch_id1[ODBCM_SIZE_256+1];
    SQLVARCHAR    szport_id1[ODBCM_SIZE_32+1];
    SQLVARCHAR    szswitch_id2[ODBCM_SIZE_256+1];
    SQLVARCHAR    szport_id2[ODBCM_SIZE_32+1];
    SQLVARCHAR    szdescription[ODBCM_SIZE_128+1];
    SQLSMALLINT   soper_status;
    SQLCHAR       svalid[ODBCM_SIZE_2+1];
    SQLLEN        cbname;
    SQLLEN        cbport1;
    SQLLEN        cbport2;
    SQLLEN        cbdesc;
    SQLLEN        cboperstatus;
    SQLLEN        cbvalid;
}link_table_t;

/** 
 * Members of boundary_table
 * +1 is added to store the '\0'
 */
typedef struct {
    SQLVARCHAR    szboundary_id[ODBCM_SIZE_32+1];
    SQLVARCHAR    szdescription[ODBCM_SIZE_128+1];
    SQLVARCHAR    szcontroller_name1[ODBCM_SIZE_32+1];
    SQLVARCHAR    szdomain_name1[ODBCM_SIZE_32+1];
    SQLVARCHAR    szlogical_port_id1[ODBCM_SIZE_320+1];
    SQLVARCHAR    szcontroller_name2[ODBCM_SIZE_32+1];
    SQLVARCHAR    szdomain_name2[ODBCM_SIZE_32+1];
    SQLVARCHAR    szlogical_port_id2[ODBCM_SIZE_320+1];
    SQLSMALLINT   soper_status;
    SQLCHAR       svalid[ODBCM_SIZE_8+1];
    SQLSMALLINT   scs_row_status;
    SQLCHAR       scs_attr[ODBCM_SIZE_8+1];
    SQLLEN        cbbid;
    SQLLEN        cbdesc;
    SQLLEN        cbctrname1;
    SQLLEN        cbdomname1;
    SQLLEN        cbctrname2;
    SQLLEN        cbdomname2;
    SQLLEN        cboperstatus;
    SQLLEN        cbvalid;
    SQLLEN        cbrowstatus;
    SQLLEN        cbcsattr;
}boundary_table_t;

/**
 * This class implements the methods for 
 * 1. Database input binding,
 * 2. Database output binding,
 * 3. Filling input for datbase
 * 4. Fetching data from database 
 */
class DBVarbind {
  public:
    DBVarbind();
    ~DBVarbind();
    DBVarbind(const DBVarbind&);
    DBVarbind& operator=(const DBVarbind&);
    /* 
     * Function pointer setter, to decide which tables bind call to take
     * table name, in/out binding 
     */
    void SetBinding(int table_id, int stream);
    /*
     * To set the fptr with input binding function
     */
    void BindingInput(int table_id);
    /*
     * To set the fptr with output binding function
     */
    void BindingOutput(int table_id);
    /* 
     * Function pointer setter, to decide which tables value structure
     * fill/fetch call to take 
     * DBTableSchema->rowlist_ entry 
     */
    void SetValueStruct(int table_id, int stream);
    /* 
     * Function pointer method call for binding input parameter 
     * DBTableSchema->rowlist_ entry 
     * statement handler which carries the SQL Query
     */
    ODBCM_RC_STATUS
        (DBVarbind::*BindINParameter)(std::vector<TableAttrSchema>&, HSTMT&);
    /* 
     * Function pointer method call for binding output parameter
     * DBTableSchema->rowlist_ entry
     * Statement handler which carries the SQL Query 
     */
    ODBCM_RC_STATUS
        (DBVarbind::*BindOUTParameter)(std::vector<TableAttrSchema>&, HSTMT&);
    /* 
     * Function pointer method call for filling input value into structure
     * DBTableSchema->rowlist_ entry
     */
    ODBCM_RC_STATUS
        (DBVarbind::*FillINPUTValues)(std::vector<TableAttrSchema>&);
    /* 
     * Function pointer method call for fetch output value from structure
     * DBTableSchema->rowlist_ entry 
     */
    ODBCM_RC_STATUS
        (DBVarbind::*FetchOUTPUTValues)(std::vector<TableAttrSchema>&);
    /* 
     * Clear the allocated memory after usage of structure is over
     */
    void FreeingBindedStructure(const uint32_t);
    /* 
     * Pointer to db table structures
     */
    controller_table_t                  *p_ctr_table;
    ctr_domain_table_t                  *p_domain_table;
    logicalport_table_t                 *p_logicalport_table;
    logical_memberport_table_t          *p_logical_memberport_table;
    switch_table_t                      *p_switch_table;
    port_table_t                        *p_port_table;
    link_table_t                        *p_link_table;
    boundary_table_t                    *p_boundary_table;
    /** 
      * is_row_exists output structure
      */
    is_row_exists_t             *p_isrowexists;
    /** 
      * Binary data buffer length pointers
      */
    SQLLEN *p_switch_id1_len;
    SQLLEN *p_switch_id2_len;
    SQLLEN *p_logicalport_id1_len;
    SQLLEN *p_logicalport_id2_len;
    SQLLEN *p_ipv6_len;
    SQLLEN *p_alarms_status_len;
    SQLLEN *p_mac_len;
    SQLLEN *p_speed_len;
    SQLLEN *p_commit_number_len;
    SQLLEN *p_commit_date_len;
    SQLLEN *p_connected_switch_id_len;

  private:
    /** Binding methods for controller_table*/
    ODBCM_RC_STATUS bind_controller_table_input(
        std::vector<TableAttrSchema>&/*DBTableSchema->rowlist_ entry*/,
        HSTMT&/**statement handler which carries the SQL Query*/);
    ODBCM_RC_STATUS bind_controller_table_output(
        std::vector<TableAttrSchema>&/*DBTableSchema->rowlist_ entry*/,
        HSTMT&/**statement handler which carries the SQL Query*/);
    ODBCM_RC_STATUS fill_controller_table(
        std::vector<TableAttrSchema>&/*DBTableSchema->rowlist_ entry*/);
    ODBCM_RC_STATUS fetch_controller_table(
        std::vector<TableAttrSchema>&/*DBTableSchema->rowlist_ entry*/);

    /** Binding methods for domain_common_table */
    ODBCM_RC_STATUS bind_domain_table_input(
        std::vector<TableAttrSchema>&/*DBTableSchema->rowlist_ entry*/,
        HSTMT&);
    ODBCM_RC_STATUS bind_domain_table_output(
        std::vector<TableAttrSchema>&/*DBTableSchema->rowlist_ entry*/,
        HSTMT&/**statement handler which carries the SQL Query*/);
    ODBCM_RC_STATUS fill_domain_table(
        std::vector<TableAttrSchema>&/*DBTableSchema->rowlist_ entry*/);
    ODBCM_RC_STATUS fetch_domain_table(
        std::vector<TableAttrSchema>&/*DBTableSchema->rowlist_ entry*/);

    /** Binding methods for logical_table */
    ODBCM_RC_STATUS bind_logicalport_table_input(
        std::vector<TableAttrSchema>&/*DBTableSchema->rowlist_ entry*/,
        HSTMT&/**statement handler which carries the SQL Query*/);
    ODBCM_RC_STATUS bind_logicalport_table_output(
        std::vector<TableAttrSchema>&/*DBTableSchema->rowlist_ entry*/,
        HSTMT&/**statement handler which carries the SQL Query*/);
    ODBCM_RC_STATUS fill_logicalport_table(
        std::vector<TableAttrSchema>&/*DBTableSchema->rowlist_ entry*/);
    ODBCM_RC_STATUS fetch_logicalport_table(
        std::vector<TableAttrSchema>&/*DBTableSchema->rowlist_ entry*/);

    /** Binding methods for logical_member_port_table */
    ODBCM_RC_STATUS bind_logical_memberport_table_input(
        std::vector<TableAttrSchema>&/*DBTableSchema->rowlist_ entry*/,
        HSTMT&/**statement handler which carries the SQL Query*/);
    ODBCM_RC_STATUS bind_logical_memberport_table_output(
        std::vector<TableAttrSchema>&/*DBTableSchema->rowlist_ entry*/,
        HSTMT&/**statement handler which carries the SQL Query*/);
    ODBCM_RC_STATUS fill_logical_memberport_table(
        std::vector<TableAttrSchema>&/*DBTableSchema->rowlist_ entry*/);
    ODBCM_RC_STATUS fetch_logical_memberport_table(
        std::vector<TableAttrSchema>&/*DBTableSchema->rowlist_ entry*/);

    /** Binding methods for switch_table */
    ODBCM_RC_STATUS bind_switch_table_input(
        std::vector<TableAttrSchema>&/*DBTableSchema->rowlist_ entry*/,
        HSTMT&/**statement handler which carries the SQL Query*/);
    ODBCM_RC_STATUS bind_switch_table_output(
        std::vector<TableAttrSchema>&/*DBTableSchema->rowlist_ entry*/,
        HSTMT&/**statement handler which carries the SQL Query*/);
    ODBCM_RC_STATUS fill_switch_table(
        std::vector<TableAttrSchema>&/*DBTableSchema->rowlist_ entry*/);
    ODBCM_RC_STATUS fetch_switch_table(
        std::vector<TableAttrSchema>&/*DBTableSchema->rowlist_ entry*/);

    /** Binding methods for port_table */
    ODBCM_RC_STATUS bind_port_table_input(
        std::vector<TableAttrSchema>&/*DBTableSchema->rowlist_ entry*/,
        HSTMT&/**statement handler which carries the SQL Query*/);
    ODBCM_RC_STATUS bind_port_table_output(
        std::vector<TableAttrSchema>&/*DBTableSchema->rowlist_ entry*/,
        HSTMT&/**statement handler which carries the SQL Query*/);
    ODBCM_RC_STATUS fill_port_table(
        std::vector<TableAttrSchema>&/*DBTableSchema->rowlist_ entry*/);
    ODBCM_RC_STATUS fetch_port_table(
        std::vector<TableAttrSchema>&/*DBTableSchema->rowlist_ entry*/);

    /** Binding methods for link_table */
    ODBCM_RC_STATUS bind_link_table_input(
        std::vector<TableAttrSchema>&/*DBTableSchema->rowlist_ entry*/,
        HSTMT&/**statement handler which carries the SQL Query*/);
    ODBCM_RC_STATUS bind_link_table_output(
        std::vector<TableAttrSchema>&/*DBTableSchema->rowlist_ entry*/,
        HSTMT&/**statement handler which carries the SQL Query*/);
    ODBCM_RC_STATUS fill_link_table(
        std::vector<TableAttrSchema>&/*DBTableSchema->rowlist_ entry*/);
    ODBCM_RC_STATUS fetch_link_table(
        std::vector<TableAttrSchema>&/*DBTableSchema->rowlist_ entry*/);

    /** Binding methods for boundary_common_table*/
    ODBCM_RC_STATUS bind_boundary_table_input(
        std::vector<TableAttrSchema>&/*DBTableSchema->rowlist_ entry*/,
        HSTMT&/**statement handler which carries the SQL Query*/);
    ODBCM_RC_STATUS bind_boundary_table_output(
        std::vector<TableAttrSchema>&/*DBTableSchema->rowlist_ entry*/,
        HSTMT&/**statement handler which carries the SQL Query*/);
    ODBCM_RC_STATUS fill_boundary_table(
        std::vector<TableAttrSchema>&/*DBTableSchema->rowlist_ entry*/);
    ODBCM_RC_STATUS fetch_boundary_table(
        std::vector<TableAttrSchema>&/*DBTableSchema->rowlist_ entry*/);

    /** Special case - no table mapping, for Isrowexists */
    ODBCM_RC_STATUS fetch_is_row_exists_status(
        std::vector<TableAttrSchema>&
        /*DBTableSchema->rowlist_ entry*/);
    ODBCM_RC_STATUS bind_is_row_exists_output(
        std::vector<TableAttrSchema>&
        /*DBTableSchema->rowlist_ entry*/,
        HSTMT&/**statement handler which carries the SQL Query*/);
};
}  // namespace uppl
}  // namespace unc
#endif /* ODBCM_DB_VARBIND_HH_ */
