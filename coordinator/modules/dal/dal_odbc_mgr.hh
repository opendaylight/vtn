/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/**
 * dal_odbc_mgr.hh
 *   contians DalOdbcMgr class definition
 */

#ifndef __DAL_ODBC_MGR_HH__
#define __DAL_ODBC_MGR_HH__

#include <sql.h>
#include <string>
#include <set>
#include <map>
#include <utility>
#include "pfcxx/module.hh"
#include "unc/config.h"
#include "dal_defines.hh"
#include "dal_conn_intf.hh"
#include "dal_dml_intf.hh"
#include "uncxx/upll_log.hh"
#include "cxx/pfcxx/synch.hh"

extern pfc_cfdef_t dal_cfdef;
namespace unc {
namespace upll {
namespace dal {

#define DAL_CONF_FILE  UNC_MODULEDIR "/dal.conf"

/**
 * DalOdbcMgr
 *   contains implementation of DalConnIntf and DalDmlIntf Apis
 *
 * Inherits from DalConnIntf and DalDmlIntf
 */
class DalOdbcMgr:public DalConnIntf, public DalDmlIntf {
  public:
    /**
     * DalOdbcMgr - Constructor
     *   Initilizes the handle variables to NULL.
     *
     * @return void             - None
     */
    DalOdbcMgr(void);

    /**
     * DalOdbcMgr - Constructor
     *   Destroys the allocated connection and environment handles.
     *
     * @return void             - None
     */
    ~DalOdbcMgr(void);

    /**
     * Init
     *   Initializes the environment and connection, sets the configuration
     *   parameters and keep it ready for communicating with the database
     *
     * @return DalResultCode      - kDalRcSuccess in case of success
     *                            - Valid errorcode otherwise
     */
    DalResultCode Init(void);

    /**
     * ConnectToDb
     *   Connects to the DB with the connection parameters from conf file.
     *
     * @param[in] conn_type       - Type of the connection
     *                              (Read-Only or Read/Write)
     *
     * @return DalResultCode      - kDalRcSuccess in case of success
     *                            - Valid errorcode otherwise
     */
    DalResultCode ConnectToDb(const DalConnType conn_type) const;

    /**
     * DisconnectFromDb
     * Disconnects from the database for the corresponding connection
     *
     * @return DalResultCode      - kDalRcSuccess in case of success
     *                            - Valid errorcode otherwise
     */
    DalResultCode DisconnectFromDb() const;

    inline DalConnType get_conn_type() { return conn_type_; }
    inline DalConnState get_conn_state() { return conn_state_; }
    inline uint32_t get_write_count() { return write_count_; }
    inline void reset_write_count() { write_count_ = 0; }

    /**
     * CommitTransaction
     * Commits all the pending changes in the database for the correpsonding
     * connection.
     *
     * @return DalResultCode      - kDalRcSuccess in case of success
     *                            - Valid errorcode otherwise
     */
    DalResultCode CommitTransaction() const;

    /**
     * RollbackTransaction
     * Discards all the pending changes in the database for the correpsonding
     * connection.
     *
     * @return DalResultCode      - kDalRcSuccess in case of success
     *                            - Valid errorcode otherwise
     */
    DalResultCode RollbackTransaction() const;

    /**
     * GetSingleRecords
     *   Gets single record of the given table that matches the given values
     *
     * @param[in] cfg_type        - Configuration Type from which the matching
     *                              records are required
     * @param[in] table_index     - Valid Index of the table
     * @param[in] output_and_matching_attr_info
     *                            - Bind Information for matching and output
     *                              attributes
     *                            - Output values are populated in the dal
     *                              output buffer bound, on successful execution
     *
     * @return DalResultCode      - kDalRcSuccess in case of success
     *                            - Valid errorcode otherwise
     *
     * Note:
     * Information on usage of DalBindInfo
     *  1. Valid instance of DalBindInfo with same table_index used in this API
     *  2. BindInput if used for any attributes, ignored.
     *  3. BindMatch is Mandatory.
     *     It is recommended to bind all the primary keys to get a unique ouput.
     *  4. BindOutput is mandatory for the interested attributes.
     */
    DalResultCode GetSingleRecord(
                    const UpllCfgType cfg_type,
                    const DalTableIndex table_index,
                    const DalBindInfo *ouput_and_matching_attr_info) const;

    /**
     * GetMultipleRecords
     *   Gets the records of the given table that matches the given values
     *
     * @param[in] cfg_type        - Configuration Type from which the matching
     *                              records are required
     * @param[in] table_index     - Valid Index of the table
     * @param[in] max_record_count- Will be filled later. Under discussion
     * @param[in] output_and_matching_attr_info
     *                            - Bind Information for matching and output
     *                              attributes
     * @param[in/out] cursor      - reference to the unallocated DalCursor
     *                              pointer
     *                            - Output - cursor pointer with valid instance
     *                              of DalCursor
     *
     * @return DalResultCode      - kDalRcSuccess in case of success
     *                            - Valid errorcode otherwise
     *
     * Note:
     * Information on usage of DalBindInfo
     *  1. Valid instance of DalBindInfo with same table_index used in this API
     *  2. BindInput if used for any attributes, ignored.
     *  3. BindMatch is Optional.
     *     If not used, return all the records of the table.
     *  4. BindOutput is mandatory for the interested attributes.
     *
     * Information on usage of cursor
     *  1. Input - Reference to the unallocated DalCursor pointer.
     *  2. Output - Allocated cursor pointer with valid instance of DalCursor
     *  3. GetNextRecord with this cursor as input populates the output
     *     records to the output_attr_info
     *  4. Only the attributes bound for output are populated in
     *     output_attr_info
     *  5. CloseCursor with this cursor as input will destroy the cursor object
     */
    DalResultCode GetMultipleRecords(
                    const UpllCfgType cfg_type,
                    const DalTableIndex table_index,
                    const size_t max_record_count,
                    const DalBindInfo *ouput_and_matching_attr_info,
                    DalCursor **cursor) const;

    /**
     * GetNextRecord
     *   Gets the next record pointer by the cursor.
     *   aavailable.
     *
     * @param[in] cursor          - Pointer to the valid cursor instance
     *
     * @return DalResultCode      - kDalRcSuccess in case of success
     *                            - Valid errorcode otherwise
     *
     * Note:
     * On successful execution, the values of the attributes that are bound for
     * output are populated in the dal user output buffer
     */
    DalResultCode GetNextRecord(const DalCursor *cursor) const;

    /**
     * CloseCursor
     *   Close and destroy the cursor. After this call, cursor object is not
     *   aavailable.
     *
     * @param[in] cursor          - Pointer to the valid cursor instance
     * @param[in] delete_bind     - If true, deletes the associated bind info.
     *                            - If false, does not delete bind info.
     *                            - It is false by default.
     * @return DalResultCode      - kDalRcSuccess in case of success
     *                            - Valid errorcode otherwise
     */
    virtual DalResultCode CloseCursor(DalCursor *cursor,
                                      bool delete_bind = false) const;

    /**
     * RecordExists
     *   Checks the existence of the record that matches the given values from
     *   the table
     *
     * @param[in] cfg_type        - Configuration Type from which the count is
     *                              required
     * @param[in] table_index     - Valid Index of the table
     * @param[in] matching_attr_info
     *                            - Bind Information for matching attributes
     * @param[in/out] existence   - Input - Reference to the existence variable
     *                            - Output - contains valid existence value on
     *                              successful execution
     *                              true - if record exists
     *                              false - if record not exists
     *
     * @return DalResultCode      - kDalRcSuccess in case of success
     *                            - Valid errorcode otherwise
     *
     * Note:
     * Information on usage of DalBindInfo
     *  1. Valid instance of DalBindInfo with same table_index used in this API
     *  2. BindInput if used for any attributes, ignored.
     *  3. BindMatch is Mandatory.
     *  4. BindOutput if used for any attributes, ignored.
     */
    DalResultCode RecordExists(const UpllCfgType cfg_type,
                               const DalTableIndex table_index,
                               const DalBindInfo *matching_attr_info,
                               bool *existence) const;

    /**
     * GetSiblingBegin
     *   Gets the sibling records from the begin to the given matching record
     *   of the table from the given cfg_type
     *
     * @param[in] cfg_type        - Configuration Type from which the
     *                              sibling records are required
     * @param[in] table_index     - Valid Index of the table
     * @param[in] max_record_count- Will be filled later. Under discussion
     * @param[in] output_and_matching_attr_info
     *                            - Bind Information for matching and output
     *                              attributes
     * @param[in/out] cursor      - reference to the unallocated DalCursor
     *                              pointer
     *                            - Output - cursor pointer with valid instance
     *                              of DalCursor
     *
     * @return DalResultCode      - kDalRcSuccess in case of success
     *                            - Valid errorcode otherwise
     *
     * Note:
     * Definition of Sibling Begin
     *   Records that match the last but one bound attributes
     *
     * Note:
     * Information on usage of DalBindInfo
     *  1. Valid instance of DalBindInfo with same table_index used in this API
     *  2. BindInput if used for any attributes, ignored.
     *  3. BindMatch is Optional.
     *     It is recommended to bind only the primary keys.
     *  4. BindOutput is mandatory for the interested attributes.
     *
     * Information on usage of cursor
     *  1. Input - Reference to the unallocated DalCursor pointer.
     *  2. Output - Allocated cursor pointer with valid instance of DalCursor
     *  3. GetNextRecord with this cursor as input populates the output
     *     records to the output_attr_info
     *  4. Only the attributes bound for output are populated in
     *     output_attr_info
     *  5. CloseCursor with this cursor as input will destroy the cursor object
     */
    DalResultCode GetSiblingBegin(
                    const UpllCfgType cfg_type,
                    const DalTableIndex table_index,
                    const size_t max_record_count,
                    const DalBindInfo *ouput_and_matching_attr_info,
                    DalCursor **cursor) const;

    /**
     * GetSiblingRecords
     *   Gets the sibling records to the given matching record of the
     *   table from the given cfg_type
     *
     * @param[in] cfg_type        - Configuration Type from which the
     *                              sibling records are required
     * @param[in] table_index     - Valid Index of the table
     * @param[in] max_record_count- Will be filled later. Under discussion
     * @param[in] output_and_matching_attr_info
     *                            - Bind Information for matching and output
     *                              attributes
     * @param[in/out] cursor      - reference to the unallocated DalCursor
     *                              pointer
     *                            - Output - cursor pointer with valid instance
     *                              of DalCursor
     *
     * @return DalResultCode      - kDalRcSuccess in case of success
     *                            - Valid errorcode otherwise
     *
     * Note:
     * Definition of Sibling
     *   Records that match the last but one bound attributes and the last
     *   bound attribute with greater values
     *
     * Note:
     * Information on usage of DalBindInfo
     *  1. Valid instance of DalBindInfo with same table_index used in this API
     *  2. BindInput if used for any attributes, ignored.
     *  3. BindMatch is mandatory.
     *     It is recommended to bind only the primary keys.
     *  4. BindOutput is mandatory for the interested attributes.
     *
     * Information on usage of cursor
     *  1. Input - Reference to the unallocated DalCursor pointer.
     *  2. Output - Allocated cursor pointer with valid instance of DalCursor
     *  3. GetNextRecord with this cursor as input populates the output
     *     records to the output_attr_info
     *  4. Only the attributes bound for output are populated in
     *     output_attr_info
     *  5. CloseCursor with this cursor as input will destroy the cursor object
     */
    DalResultCode GetSiblingRecords(
                    const UpllCfgType cfg_type,
                    const DalTableIndex table_index,
                    const size_t max_record_count,
                    const DalBindInfo *ouput_and_matching_attr_info,
                    DalCursor **cursor) const;

    /**
     * GetSiblingCount
     *   Gets the count of sibling records to the given matching record of the
     *   table from the given cfg_type
     *
     * @param[in] cfg_type        - Configuration Type from which the count of
     *                              sibling records is required
     * @param[in] table_index     - Valid Index of the table
     * @param[in] matching_attr_info
     *                            - Bind Information for matching attributes
     * @param[in/out] count       - Input - Reference to the count variable
     *                            - Output - contains valid count value on
     *                              successful execution
     *
     * @return DalResultCode      - kDalRcSuccess in case of success
     *                            - Valid errorcode otherwise
     *
     * Note:
     * Definition of Sibling
     *   Records that match the last but one bound attributes and the last
     *   bound attribute with greater values
     *
     * Note:
     * Information on usage of DalBindInfo
     *  1. Valid instance of DalBindInfo with same table_index used in this API
     *  2. BindInput if used for any attributes, ignored.
     *  3. BindMatch is Mandatory.
     *     It is recommended to bind only the primary keys.
     *  4. BindOutput if used for any attributes, ignored.
     */
    DalResultCode GetSiblingCount(const UpllCfgType cfg_type,
                                  const DalTableIndex table_index,
                                  const DalBindInfo *matching_attr_info,
                                  uint32_t *count) const;

    /**
     * GetRecordCount
     *   Gets the count of the records of the table from the given cfg_type
     *   that match the given values.
     *
     * @param[in] cfg_type        - Configuration Type from which the count is
     *                              required
     * @param[in] table_index     - Valid Index of the table
     * @param[in] matching_attr_info
     *                            - Bind Information for matching attributes
     * @param[in/out] count       - Input - Reference to the count variable
     *                            - Output - contains valid count value on
     *                              successful execution
     *
     * @return DalResultCode      - kDalRcSuccess in case of success
     *                            - Valid errorcode otherwise
     *
     * Note:
     * Information on usage of DalBindInfo
     *  1. Valid instance of DalBindInfo with same table_index used in this API
     *  2. BindInput if used for any attributes, ignored.
     *  3. BindMatch is optional.
     *     BindMatch if used, count has the number of records that match the
     *     given values.
     *     BindMatch if not used, count has the number of records in the table.
     *  4. BindOutput if used for any attributes, ignored.
     */
    DalResultCode GetRecordCount(const UpllCfgType cfg_type,
                                 const DalTableIndex table_index,
                                 const DalBindInfo *matching_attr_info,
                                 uint32_t *count) const;

    /**
     * DeleteRecords
     *   Deletes the records of table from the given cfg_type
     *   Increments the write count if operation is successful.
     *
     * @param[in] cfg_type        - Configuration Type for which the records
     *                              have to be updated
     * @param[in] table_index     - Valid Index of the table
     * @param[in] matching_attr_info
     *                            - Bind Information for deleting records
     * @param[in] cfg_mode        - Configuration mode
     * @param[in] vtn_name        - VTN name of corresponding VTN config mode
     *
     * @return DalResultCode      - kDalRcSuccess in case of success
     *                            - Valid errorcode otherwise
     *
     * Note:
     * Information on usage of DalBindInfo
     *  1. Valid instance of DalBindInfo with same table_index used in this API
     *  2. BindInput if used for any attributes, ignored.
     *  3. BindMatch is optional.
     *     But it is recommended to provide all primary keys for match to
     *     delete unique record.
     *     BindMatch if used, deletes all the records that match the given
     *     values.
     *     BindMatch if not used, deletes all the records from the table.
     *  4. BindOutput if used for any attributes, ignored.
     *
     *  NOTE: truncate cannot be used with bindinfo.
     *  NOTE: truncate cannot be used for candidate operations which require
     *        abort/commit
     */
    DalResultCode DeleteRecords(const UpllCfgType cfg_type,
                                const DalTableIndex table_index,
                                const DalBindInfo *matching_attr_info,
                                const bool truncate,
                                const CfgModeType cfg_mode,
                                const uint8_t  *vtn_name) const;

    /**
     * CreateRecord
     *   Creates the record in table with the given input data for
     *   the given cfg_type
     *   Performs parent existence check if cfg_type is UPLL_DT_IMPORT
     *   Performs existence check and parent existence check if
     *   kDalRcGeneralError is returned. This is done to get proper error
     *   codes when Database cannot.
     *   Increments the write count if operation is successful.
     *
     * @param[in] cfg_type        - Configuration Type for which the record
     *                              has to be created
     * @param[in] table_index     - Valid Index of the table
     * @param[in] input_attr_info - Bind Information for creating record
     * @param[in] cfg_mode        - Configuration mode
     * @param[in] vtn_name        - VTN name of corresponding VTN config mode
     *
     * @return DalResultCode      - kDalRcSuccess in case of success
     *                            - Valid errorcode otherwise
     *
     * Note:
     * Information on usage of DalBindInfo
     *  1. Valid instance of DalBindInfo with same table_index used in this API
     *  2. BindInput is mandatory for the interested attributes.
     *     It is mandatory to bind input for all primary keys.
     *  3. BindMatch if used for any attributes, ignored.
     *  4. BindOutput if used for any attributes, ignored.
     */
    DalResultCode CreateRecord(const UpllCfgType cfg_type,
                               const DalTableIndex table_index,
                               const DalBindInfo *input_attr_info,
                               const CfgModeType cfg_mode,
                               const uint8_t* vtn_name) const;

    /**
     * UpdateRecords
     *   Updates the records of table with the given input data for
     *   the given cfg_type
     *   Increments the write count if operation is successful.
     *
     * @param[in] cfg_type        - Configuration Type for which the records
     *                              have to be updated
     * @param[in] table_index     - Valid Index of the table
     * @param[in] input_and_matching_attr_info
     *                            - Bind Information for updating records
     * @param[in] cfg_mode        - Configuration mode
     * @param[in] vtn_name        - VTN name of corresponding VTN config mode
     *
     * @return DalResultCode      - kDalRcSuccess in case of success
     *                            - Valid errorcode otherwise
     *
     * Note:
     * Information on usage of DalBindInfo
     *  1. Valid instance of DalBindInfo with same table_index used in this API
     *  2. BindInput is mandatory for the interested attributes.
     *  3. BindMatch is optional.
     *     But it is recommended to provide all primary keys for match to
     *     update unique record.
     *     BindMatch if used, updates multiple records that match the given
     *     values.
     *     BindMatch if not used, updates all the records from the table.
     *  4. BindOutput if used for any attributes, ignored.
     */
    DalResultCode UpdateRecords(
                    const UpllCfgType cfg_type,
                    const DalTableIndex table_index,
                    const DalBindInfo *input_and_matching_attr_info,
                    const CfgModeType cfg_mode,
                    const uint8_t* vtn_name) const;

    /**
     * ExecuteAppQuery
     *   Updates(create/update/delete) the records of table with the given
     *   sql query.
     *   Increments the write count if operation is successful.
     *
     * @param[in] query_stmt      - User supplied executable query statement
     * @param[in] cfg_type        - Configuration Type for which the records
     *                              have to be updated
     * @param[in] table_index     - Valid Index of the table
     * @param[in] input_and_matching_attr_info
     *                            - Bind Information for updating records
     * @param[in] dirty_op        - How to treat the modification -
     *                              create/update/delete
     * @param[in] cfg_mode        - Configuration mode
     * @param[in] vtn_name        - VTN name of corresponding VTN config mode
     *
     * @return DalResultCode      - kDalRcSuccess in case of success
     *                            - Valid errorcode otherwise
     *
     * Note:
     * Information on usage of DalBindInfo
     *  1. Valid instance of DalBindInfo with same table_index used in this API
     *  2. BindInput is mandatory for the interested attributes.
     *  3. BindMatch is optional.
     *     But it is recommended to provide all primary keys for match to
     *     update unique record in the user defined query.
     *     BindMatch if used, updates multiple records that match the given
     *     values in the user defined query.
     *     BindMatch if not used, updates all the records from the table.
     *  4. BindOutput if used for any attributes in the user defiuned query, ignored.
     */
    DalResultCode ExecuteAppQuery(std::string query_stmt,
                                  const UpllCfgType cfg_type,
                                  const DalTableIndex table_index,
                                  const DalBindInfo *bind_info,
                                  const unc_keytype_operation_t dirty_op,
                                  const CfgModeType cfg_mode,
                                  const uint8_t* vtn_name) const;

    /**
     * GetDeletedRecords
     *   Fetches the records from cfg_type_2 which are not in cfg_type_1
     *
     * @param[in] cfg_type_1      - Configuration Type 1
     *                              (not equal to cfg_type_2)
     * @param[in] cfg_type_2      - Configuration Type 2
     *                              (not equal to cfg_type_1)
     * @param[in] table_index     - Valid Index of the table
     * @param[in] max_record_count- Will be filled later. Under discussion
     * @param[in] output_attr_info
     *                            - Bind Information for output records
     * @param[in] cfg_mode        - Configuration mode
     * @param[in] vtn_name        - VTN name of corresponding VTN config mode
     * @param[in/out] cursor      - reference to the unallocated DalCursor
     *                              pointer
     *                            - Output - cursor pointer with valid instance
     *                              of DalCursor
     *
     * @return DalResultCode      - kDalRcSuccess in case of success
     *                            - Valid errorcode otherwise
     *
     * Note:
     * Information on usage of DalBindInfo
     *  1. Valid instance of DalBindInfo with same table_index used in this API
     *  2. BindInput if used for any attributes, ignored.
     *  3. BindMatch if used for any attributes, ignored.
     *  4. BindOutput is mandatory for the interested attributes.
     *
     * Information on usage of cursor
     *  1. Input - Reference to the unallocated DalCursor pointer.
     *  2. Output - Allocated cursor pointer with valid instance of DalCursor
     *  3. GetNextRecord with this cursor as input populates the output
     *     records to the output_attr_info
     *  4. Only the attributes bound for output are populated in
     *     output_attr_info
     *  5. CloseCursor with this cursor as input will destroy the cursor object
     */
    DalResultCode GetDeletedRecords(const UpllCfgType cfg_type_1,
                                    const UpllCfgType cfg_type_2,
                                    const DalTableIndex table_index,
                                    const size_t max_record_count,
                                    const DalBindInfo *output_attr_info,
                                    DalCursor **cursor,
                                    const CfgModeType cfg_mode,
                                    const uint8_t* vtn_name) const;

    /**
     * GetCreatedRecords
     *   Fetches the records from cfg_type_1 which are not in cfg_type_2
     *
     * @param[in] cfg_type_1      - Configuration Type 1
     *                              (not equal to cfg_type_2)
     * @param[in] cfg_type_2      - Configuration Type 2
     *                              (not equal to cfg_type_1)
     * @param[in] table_index     - Valid Index of the table
     * @param[in] max_record_count- Will be filled later. Under discussion
     * @param[in] output_attr_info
     *                            - Bind Information for output records
     * @param[in/out] cursor      - reference to the unallocated DalCursor
     *                              pointer
     *                            - Output - cursor pointer with valid instance
     *                              of DalCursor
     * @param[in] cfg_mode        - Configuration mode
     * @param[in] vtn_name        - VTN name of corresponding VTN config mode
     *
     * @return DalResultCode      - kDalRcSuccess in case of success
     *                            - Valid errorcode otherwise
     *
     * Note:
     * Information on usage of DalBindInfo
     *  1. Valid instance of DalBindInfo with same table_index used in this API
     *  2. BindInput if used for any attributes, ignored.
     *  3. BindMatch if used for any attributes, ignored.
     *  4. BindOutput is mandatory for the interested attributes.
     *
     * Information on usage of cursor
     *  1. Input - Reference to the unallocated DalCursor pointer.
     *  2. Output - Allocated cursor pointer with valid instance of DalCursor
     *  3. GetNextRecord with this cursor as input populates the output
     *     records to the output_attr_info
     *  4. Only the attributes bound for output are populated in
     *     output_attr_info
     *  5. CloseCursor with this cursor as input will destroy the cursor object
     */
    DalResultCode GetCreatedRecords(const UpllCfgType cfg_type_1,
                                    const UpllCfgType cfg_type_2,
                                    const DalTableIndex table_index,
                                    const size_t max_record_count,
                                    const DalBindInfo *output_attr_info,
                                    DalCursor **cursor,
                                    const CfgModeType cfg_mode,
                                    const uint8_t* vtn_name) const;

    DalResultCode ClearCreateUpdateFlags(const DalTableIndex table_index,
                                         const UpllCfgType cfg_type,
                                         const CfgModeType cfg_mode,
                                         const uint8_t* vtn_name,
                                         const bool creat,
                                         const bool update) const;

    DalResultCode DeleteRecordsInVtnMode(const UpllCfgType cfg_type,
                                         const DalTableIndex table_index,
                                         const DalBindInfo *bind_info,
                                         const bool truncate,
                                         const CfgModeType cfg_mode,
                                         const uint8_t* vtn_name) const;

    /**
     * GetUpdatedRecords
     *   Fetches the records from cfg_type_1 and cfg_type_2,
     *   that are updated in cfg_type_1 as compared to cfg_type_2
     *
     * @param[in] cfg_type_1      - Configuration Type 1
     *                              (not equal to cfg_type_2)
     * @param[in] cfg_type_2      - Configuration Type 2
     *                              (not equal to cfg_type_1)
     * @param[in] table_index     - Valid Index of the table
     * @param[in] max_record_count- Will be filled later. Under discussion
     * @param[in] cfg_1_output_and_match_attr_info
     *                            - Bind Information for output records of
     *                            - cfg_type_1
     * @param[in] cfg_2_output_and_match_attr_info
     *                            - Bind Information for output records of
     *                            - cfg_type_2
     * @param[in/out] cursor      - reference to the unallocated DalCursor
     *                              pointer
     *                            - Output - cursor pointer with valid instance
     *                              of DalCursor
     * @param[in] cfg_mode        - Configuration mode
     * @param[in] vtn_name        - VTN name of corresponding VTN config mode
     *
     * @return DalResultCode      - kDalRcSuccess in case of success
     *                            - Valid errorcode otherwise
     *
     * Note:
     * Information on usage of DalBindInfo
     *  1. Valid instance of DalBindInfo with same table_index used in this API
     *  2. BindInput if used for any attributes, ignored.
     *  3. BindMatch is optional.
     *     BindMatch if used, Primary Key attributes are mandtory.
     *     BindMatch if used, comparison is done only for the bound columns
     *     BindMatch if not used, comparison is done only for all the columns.
     *     Bound value is not used for comparison. The comparison is performed
     *     between the values in the cfg_type_1 and cfg_type_2 for the specific
     *     columns.
     *     Both cfg_type_1 and cfg_type_2 should bind same columns for match
     *     Since the bound value is not used for this API, it is ok to bind
     *     dummy address. Do not pass NULL address.
     *  4. BindOutput is mandatory for the interested attributes.
     *
     * Information on usage of cursor
     *  1. Input - Reference to the unallocated DalCursor pointer.
     *  2. Output - Allocated cursor pointer with valid instance of DalCursor
     *  3. GetNextRecord with this cursor as input populates the output records
     *     of cfg_type_1 and cfg_type_2 to the correponding DalBindInfo
     *     instances.
     *  4. Only the attributes bound for output are populated in both
     *     cfg_1_output_attr_info and cfg_2_output_attr_info.
     *  5. CloseCursor with this cursor as input will destroy the cursor object
     */
    DalResultCode GetUpdatedRecords(
                    const UpllCfgType cfg_type_1,
                    const UpllCfgType cfg_type_2,
                    const DalTableIndex table_index,
                    const size_t max_record_count,
                    const DalBindInfo *cfg_1_output_and_match_attr_info,
                    const DalBindInfo *cfg_2_output_and_match_attr_info,
                    DalCursor **cursor,
                    const CfgModeType cfg_mode,
                    const uint8_t* vtn_name) const;

    /**
     * CopyEntireRecords
     *   Copies the entire records of table from source configuration to
     *   destination configuration.
     *
     * @param[in] dest_cfg_type   - Configuration Type where the records to be
     *                              copied (not equal to src_cfg_type)
     * @param[in] src_cfg_type    - Configuration Type from where the records
     *                              will be copied (not equal to dest_cfg_type)
     * @param[in] table_index     - Valid Index of the table
     * @param[in] output_attr_info
     *                            - Bind Information for output columns
     *
     * @return DalResultCode      - kDalRcSuccess in case of success
     *                            - Valid errorcode otherwise
     *                              On successful execution, both the
     *                              configurations have same records.
     *
     * Note:
     * Information on Copy Logic
     * 1. Remove the entire records of the dest_cfg_type
     * 2. Copy the entire records from src_cfg_type to dest_cfg_type
     * 3. Recommended to use this API, where difference between both the
     *    configurations are drastically more.
     *
     * Information on usage of DalBindInfo
     *  1. Valid instance of DalBindInfo with same table_index used in this API
     *  2. BindInput if used for any attributes, ignored.
     *  3. BindMatch if used for any attributes, ignored.
     *  4. BindOutput is optional.
     *     BindOutput, if used, copy the values of bound columns from
     *     src_cfg_type to dst_cfg_type
     *     BindOutput, if not used, copy the values of all columns from
     *     src_cfg_type to dst_cfg_type
     *     Since the bound value is not used for this API, it is ok to bind
     *     dummy address. Do not pass NULL address.
     */
    DalResultCode CopyEntireRecords(const UpllCfgType dest_cfg_type,
                                    const UpllCfgType src_cfg_type,
                                    const DalTableIndex table_index,
                                    const DalBindInfo *output_attr_info) const;

    /**
     * CopyModifiedRecords
     *   Copies the modified records of table from source configuration to
     *   destination configuration based on the operation.
     *
     * @param[in] dest_cfg_type   - Configuration Type where the records to be
     *                              copied (not equal to src_cfg_type)
     * @param[in] src_cfg_type    - Configuration Type from where the records
     *                              will be copied (not equal to dest_cfg_type)
     * @param[in] table_index     - Valid Index of the table
     * @param[in] output_and_match_attr_info
     *                            - Bind Information for output and match
     *                            - columns
     * @param[in] op              - Operation to be performed for Copy.
     * @param[in] cfg_mode        - Configuration mode
     * @param[in] vtn_name        - VTN name of corresponding VTN config mode
     *
     * @return DalResultCode      - kDalRcSuccess in case of success
     *                            - Valid errorcode otherwise
     *                              On successful execution, both the
     *                              configurations have same records.
     *
     * Note:
     * Information on Copy Logic
     * 1. For DELETE operation, Remove the records from dest_cfg_type that are
     *    result of GetDeletedRecords(dest_cfg_type, src_cfg_type, ...)
     *    This should be called for Child schema first and then parent schema
     *    if Parent-Child relationship is established.
     * 2. For CREATE operation, Add the records in dest_cfg_type that are
     *    result of GetCreatedRecords(dest_cfg_type, src_cfg_type, ...)
     * 3. For UPDATE operation, Update the records in dest_cfg_type with the
     *    records from src_cfg_type that are result of
     *    GetUpdatedRecords(dest_cfg_type, src_cfg_type, ...)
     * 4. Recommended to use this API, where difference between both the
     *    configurations are comparitively lesser.
     *
     * Information on usage of DalBindInfo
     *  1. Valid instance of DalBindInfo with same table_index used in this API
     *  2. BindInput if used for any attributes, ignored.
     *  3. BindMatch is optional. Used in step 3 of Copy Logic.
     *     BindMatch if used, Primary Key attributes are mandtory.
     *     BindMatch if used, comparison is done only for the bound columns
     *     BindMatch if not used, comparison is done only for all the columns.
     *     Bound value is not used for comparison. The comparison is performed
     *     between the values in the cfg_type_1 and cfg_type_2 for the specific
     *     columns.
     *     Both src_cfg_type and dst_cfg_type_2 should bind same columns
     *     for match
     *     Since the bound value is not used for this API, it is ok to bind
     *     dummy address. Do not pass NULL.
     *  4. BindOutput is optional.
     *     BindOutput, if used, copy the values of bound columns from
     *     src_cfg_type to dst_cfg_type
     *     BindOutput, if not used, copy the values of all columns from
     *     src_cfg_type to dst_cfg_type
     *     Since the bound value is not used for this API, it is ok to bind
     *     dummy address. Do not pass NULL address.
     *
     */
    DalResultCode CopyModifiedRecords(
                    const UpllCfgType dest_cfg_type,
                    const UpllCfgType src_cfg_type,
                    const DalTableIndex table_index,
                    const DalBindInfo *output_and_match_attr_info,
                    const unc_keytype_operation_t op,
                    const CfgModeType cfg_mode,
                    const uint8_t* vtn_name) const;

    /**
     * CopyMatchingRecords
     *   Copies all the matching records of table from source configuration to
     *   destination configuration.
     *
     * @param[in] dest_cfg_type   - Configuration Type where the records to be
     *                              copied (not equal to src_cfg_type)
     * @param[in] src_cfg_type    - Configuration Type from where the records
     *                              will be copied (not equal to dest_cfg_type)
     * @param[in] table_index     - Valid Index of the table
     * @param[in] output_and_match_attr_info
     *                            - Bind Information for match and ouput columns
     * @param[in] cfg_mode        - Configuration mode
     * @param[in] vtn_name        - VTN name of corresponding VTN config mode
     *
     * @return DalResultCode      - kDalRcSuccess in case of success
     *                            - Valid errorcode otherwise
     *
     * Note:
     * Information on usage of DalBindInfo
     *  1. Valid instance of DalBindInfo with same table_index used in this API
     *  2. BindInput if used for any attributes, ignored.
     *  3. BindMatch is Mandatory.
     *  4. BindOutput is optional.
     *     BindOutput, if used, Primary Key columns are mandatory.
     *     BindOutput, if used, copy the values of bound columns from
     *     src_cfg_type to dst_cfg_type
     *     BindOutput, if not used, copy the values of all columns from
     *     src_cfg_type to dst_cfg_type
     *     Since the bound value is not used for this API, it is ok to bind
     *     dummy address. Do not pass NULL address.
     */
    DalResultCode CopyMatchingRecords(
                    const UpllCfgType dest_cfg_type,
                    const UpllCfgType src_cfg_type,
                    const DalTableIndex table_index,
                    const DalBindInfo *output_and_match_attr_info,
                    const CfgModeType cfg_mode,
                    const uint8_t* vtn_name) const;

    /**
     * CheckRecordsIdentical
     *   Checks whether cfg_type_1 and cfg_type_2 configurations contains same
     *   records in the given table.
     *
     * @param[in] cfg_type_1      - Configuration Type 1
     *                              (not equal to cfg_type_2)
     * @param[in] cfg_type_2      - Configuration Type 2
     *                              (not equal to cfg_type_1)
     * @param[in] table_index     - Valid Index of the table
     * @param[in] matching_attr_info
     *                            - Bind Information for comparing columns
     * @param[in/out] identical   - Pointer to the identical value.
     *                            - On successful execution, contains
     *                              true, if both config have same records
     *                              false, if not
     * @param[in] cfg_mode        - Configuration mode
     * @param[in] vtn_name        - VTN name of corresponding VTN config mode
     *
     * @return DalResultCode      - kDalRcSuccess in case of success
     *                            - Valid errorcode otherwise
     *
     * Note:
     * Information on usage of DalBindInfo
     *  1. Valid instance of DalBindInfo with same table_index used in this API
     *  2. BindInput if used for any attributes, ignored.
     *  3. BindMatch is Optional.
     *     If used, comparison is done only for bound columns.
     *     If not used, comparison is done for all the columns.
     *     Bound value is not used for comparison. The comparison is performed
     *     between the values in the cfg_type_1 and cfg_type_2 for the specific
     *     columns.
     *     Since the bound value is not used for this API, it is ok to bind
     *     dummy address. Do not pass NULL address.
     *  4. BindOutput if used for any attributes, ignored.
     */
    DalResultCode CheckRecordsIdentical(const UpllCfgType cfg_type_1,
                                        const UpllCfgType cfg_type_2,
                                        const DalTableIndex table_index,
                                        const DalBindInfo *matching_attr_info,
                                        bool *identical,
                                        const CfgModeType cfg_mode,
                                        const uint8_t* vtn_name) const;
    /**
     * ExecuteAppQuerySingleRecord
     *   Execute the user supplied query statement to generate single record as
     *   output
     *
     * @param[in] query_stmt      - User supplied executable query statement
     * @param[in] bind_info       - Corresponding bind information for the query
     *
     * @return DalResultCode      - kDalRcSuccess in case of success
     *                            - Valid errorcode otherwise
     */
    DalResultCode ExecuteAppQuerySingleRecord(
                         const std::string query_stmt,
                         const DalBindInfo *bind_info) const;
    /**
     * ExecuteAppQueryMultipleRecords
     *   Execute the user supplied query statement to generate multiple records
     *   in a given cursor as output
     *
     * @param[in] query_stmt      - User supplied executable query statement
     * @param[in] bind_info       - Corresponding bind information for the query
     * @param[in] max_record_count- Count of output records expected from the
     *                              user.
     * @param[in/out] cursor      - reference to the unallocated DalCursor
     *                              pointer
     *                            - Output - cursor pointer with valid instance
     *                              of DalCursor
     *
     * @return DalResultCode      - kDalRcSuccess in case of success
     *                            - Valid errorcode otherwise
     */
    DalResultCode ExecuteAppQueryMultipleRecords(
                         const std::string query_stmt,
                         const size_t max_record_count,
                         const DalBindInfo *bind_info,
                         DalCursor **cursor) const;

    // Clears all the tables from dirty list
    inline void ClearGlobalDirtyTblCache() const {
      delete_dirty.clear();
      create_dirty.clear();
      update_dirty.clear();
    }

    // Clear specific table index from global dirty cache.
    // Currently used to clear specific tables in virtual mode commit
    DalResultCode ClearGlobalDirtyTblCacheAndDB(
        const DalTableIndex table_index,
        const unc_keytype_operation_t op) const;

    // Clear all the tables from VTN dirty list
    inline void ClearVtnDirtyTblCache() const {
      delete_vtn_dirty.clear();
      create_vtn_dirty.clear();
      update_vtn_dirty.clear();
    }

    DalResultCode  ClearDirtyTblCache(const CfgModeType cfg_mode,
                                      const uint8_t* vtn_name) const;

    // Add all the tables to dirty list
    inline void MakeAllTableDirtyInCache() const {
      UPLL_FUNC_TRACE;
      delete_dirty.clear();
      create_dirty.clear();
      update_dirty.clear();
      for (uint16_t tbl_idx = schema::table::kDbiVtnTbl;
           tbl_idx < schema::table::kDalNumTables; tbl_idx++) {
        delete_dirty.insert(tbl_idx);
        create_dirty.insert(tbl_idx);
        update_dirty.insert(tbl_idx);
      }
    }  // DalOdbcMgr::MakeAllTableDirtyInCache

    inline bool IsAnyTableDirtyShallow() const {
      return ((delete_dirty.size() > 0) ||
              (create_dirty.size() > 0) ||
              (update_dirty.size() > 0));
    }
    bool IsTableDirtyShallow(const DalTableIndex table_index,
                             const CfgModeType cfg_mode,
                             const uint8_t* vtn_name) const;

    bool IsTableDirtyShallowForOp(const DalTableIndex table_index,
                                  const unc_keytype_operation_t op,
                                  const CfgModeType cfg_mode,
                                  const uint8_t* vtn_name) const;

    DalResultCode SetTableDirty(const UpllCfgType cfg_type,
                                const DalTableIndex table_index,
                                const unc_keytype_operation_t op,
                                const CfgModeType cfg_mode,
                                const uint8_t* vtn_name) const;

    DalResultCode  ExecuteAppQueryModifyRecord(
      const UpllCfgType cfg_type,
      const DalTableIndex table_index,
      const std::string query_stmt,
      const DalBindInfo *bind_info,
      const unc_keytype_operation_t op,
      const CfgModeType cfg_mode,
      const uint8_t* vtn_name) const;

    DalResultCode UpdateDirtyTblCacheFromDB() const;
    DalResultCode ClearAllDirtyTblInDB(UpllCfgType cfg_type,
                                       const CfgModeType cfg_mode,
                                       const uint8_t* vtn_name) const;
    static bool FillTableName2IndexMap();

    inline bool get_wr_exclusion_on_runn() { return wr_exclusion_on_runn_; }

    // NOTE: wr_exclusion_on_runn_ should be set immediately after
    // construction else the behavior is undefined
    inline void set_wr_exclusion_on_runn() { wr_exclusion_on_runn_ = true; }

  private:
    /**
     * SetConnAttributes
     *   Sets necessary attributes for the connection from the configuration file
     *
     * @param[in] conn_handle     - Valid connection handle before connecting
     *                              to DB
     * @param[in] conn_type       - Type of the connection
     *                              (Read-Only or Read/Write)
     *
     * @return DalResultCode      - kDalRcSuccess in case of success
     *                            - Valid errorcode otherwise
     */
    DalResultCode SetConnAttributes(const SQLHANDLE conn_handle,
                                    const DalConnType conn_type) const;

    /**
     * SetStmtAttributes
     *   Sets necessary attributes for the statement from the configuration file
     *
     * @param[in] stmt_handle     - Valid Statment handle before binding query
     *
     * @return DalResultCode      - kDalRcSuccess in case of success
     *                            - Valid errorcode otherwise
     */
    void SetStmtAttributes(const SQLHANDLE stmt_handle) const;

    /**
     * SetCursorAttributes
     *   Sets necessary cursor properites for the statement from the
     *   configuration file
     *
     * @param[in] stmt_handle     - Valid Statment handle before binding query
     * @return DalResultCode      - kDalRcSuccess in case of success
     *                            - Valid errorcode otherwise
     */
    void SetCursorAttributes(const SQLHANDLE stmt_handle,
                             uint32_t max_count) const;

    /**
     * BindToQuery
     *   Binds the information provided by dal user with the Query statement
     *   Used by DAL layer
     * @param[in] stmt_handle     - Valid Statment handle for binding query
     * @param[in] bind_info       - Valid DalBindInfo instance for binding
     *
     * @return DalResultCode      - kDalRcSuccess in case of success
     *                            - Valid errorcode otherwise
     */
    DalResultCode BindToQuery(const SQLHANDLE *stmt_handle,
                              const DalBindInfo *bind_info) const;

    // Wrapper for the above APIs
    /**
     * BindInputToQuery
     *   Binds all the Input parameters with the Query statement
     *   from DalBindInfo
     *
     * @param[in] stmt_handle     - Valid Statment handle for binding query
     * @param[in] bind_info       - Valid DalBindInfo instance for binding
     * @param[in/out] param_idx   - Input: Parameter Index for binding
     *                              Output: Updated Parameter Index to use for
     *                                      next input parameter
     *
     * @return DalResultCode      - kDalRcSuccess in case of success
     *                            - Valid errorcode otherwise
     */
    DalResultCode BindInputToQuery(const SQLHANDLE *stmt_handle,
                                   const DalBindInfo *bind_info,
                                   uint16_t *param_idx) const;

    /**
     * BindMatchToQuery
     *   Binds all the Match parameters with the Query statement
     *   from DalBindInfo
     *
     * @param[in] stmt_handle     - Valid Statment handle for binding query
     * @param[in] bind_info       - Valid DalBindInfo instance for binding
     * @param[in/out] param_idx   - Input: Parameter Index for binding
     *                              Output: Updated Parameter Index to use for
     *                                      next match parameter
     *
     * @return DalResultCode      - kDalRcSuccess in case of success
     *                            - Valid errorcode otherwise
     */
    DalResultCode BindMatchToQuery(const SQLHANDLE *stmt_handle,
                                   const DalBindInfo *bind_info,
                                   uint16_t *param_idx) const;

    /**
     * BindOutputToQuery
     *   Binds all the Output parameters with the Query statement
     *   from DalBindInfo
     *
     * @param[in] stmt_handle     - Valid Statment handle for binding query
     * @param[in] bind_info       - Valid DalBindInfo instance for binding
     * @param[in/out] param_idx   - Input: Parameter Index for binding
     *                              Output: Updated Parameter Index to use for
     *                                      next output parameter
     *
     * @return DalResultCode      - kDalRcSuccess in case of success
     *                            - Valid errorcode otherwise
     */
    DalResultCode BindOutputToQuery(const SQLHANDLE *stmt_handle,
                                    const DalBindInfo *bind_info,
                                    uint16_t *param_idx) const;

    /**
     * ExecuteQuery
     *   Wrapper to execute the query statement for all the APIs
     *
     * @param[in] dal_stmt_handle - Corresponding Statement Handle
     * @param[in] query_stmt      - Corresponding query statement to execute
     * @param[in] bind_info       - Corresponding bind information of the API
     * @param[in] max_count       - Corresponding max_count from the API input
     *                            - default - 1 for single record queries
     *
     * @return DalResultCode      - kDalRcSuccess in case of success
     *                            - Valid errorcode otherwise
     */
    DalResultCode ExecuteQuery(SQLHANDLE *dal_stmt_handle,
                               const std::string *query_stmt,
                               const DalBindInfo *bind_info = NULL,
                               const uint32_t max_count = 1) const;

    /**
     * CheckParentInstance
     *   Checks the parent of the given instance with parent key values from
     *   the given DalBindInfo in Database for the given cfg_type
     *
     * @param[in] cfg_type        - Configuration Type for which the record
     *                              has to be created
     * @param[in] table_index     - Valid Index of the table
     * @param[in] input_attr_info - Bind Information for the given table_index
     *
     * @return DalResultCode      - kDalRcSuccess in case of success
     *                            - Valid errorcode otherwise
     *
     * Note:
     * Information on usage of DalBindInfo
     *  1. Valid instance of DalBindInfo with same table_index used in this API
     *  2. BindInput is mandatory for the interested attributes.
     *     It is mandatory to bind input for all primary keys.
     *  3. BindMatch if used for any attributes, ignored.
     *  4. BindOutput if used for any attributes, ignored.
     */
    DalResultCode CheckParentInstance(const UpllCfgType cfg_type,
                                      const DalTableIndex table_index,
                                      const DalBindInfo *input_attr_info) const;


    /**
     * CheckInstance
     *   Checks the instance with primary key values from the given
     *   DalBindInfo in Database for the given cfg_type
     *
     * @param[in] cfg_type        - Configuration Type for which the record
     *                              has to be created
     * @param[in] table_index     - Valid Index of the table
     * @param[in] input_attr_info - Bind Information for the given table_index
     *
     * @return DalResultCode      - kDalRcSuccess in case of success
     *                            - Valid errorcode otherwise
     *
     * Note:
     * Information on usage of DalBindInfo
     *  1. Valid instance of DalBindInfo with same table_index used in this API
     *  2. BindInput is mandatory for the interested attributes.
     *     It is mandatory to bind input for all primary keys.
     *  3. BindMatch if used for any attributes, ignored.
     *  4. BindOutput if used for any attributes, ignored.
     */
    DalResultCode CheckInstance(const UpllCfgType cfg_type,
                                const DalTableIndex table_index,
                                const DalBindInfo *input_attr_info) const;

    /**
     * BackupToCandDel
     *  During Merge, the records which are going to be deleted from
     *  DT_CANDIDATE needs to be backedup in DT_CANDIDATE_DEL for commit
     *  operation(to get deleted records).
     *
     * @param[in] cfg_type        - Configuration Type from where the records
     *                              will be copied
     * @param[in] table_index     - Valid Index of the table
     * @param[in] bind_info
     *                            - Bind Information for match and ouput columns
     *
     * @return DalResultCode      - kDalRcSuccess in case of success
     *                            - Valid errorcode otherwise
     *
     * Information on usage of DalBindInfo
     * This function is called from CopyModifiedRecords
     * so the bind information is same as CopyModifiedRecords.
     *
     * XXX: NOTE: It is mandatory that candidate and running needs to be in
     * sync at this time. If not, the result is undefined.
     */
    DalResultCode BackupToCandDel(const UpllCfgType cfg_type,
                                  const DalTableIndex table_index,
                                  const DalBindInfo *bind_info) const;

    inline DalResultCode FreeHandle(const SQLSMALLINT handle_type,
                                    SQLHANDLE handle) const;

    std::string GetConnString(const DalConnType conn_type) const;

    DalResultCode SetCfgTblDirtyInDB(const UpllCfgType cfg_type,
                                     const unc_keytype_operation_t op,
                                     const DalTableIndex table_index,
                                     bool set_flag) const;
    DalResultCode UpdateGlobalDirtyTblCache(
        const UpllCfgType cfg_type,
        const char *tbl_name,
        const unc_keytype_operation_t op) const;
    DalResultCode GetTableIndex(const char* tbl_name,
                                DalTableIndex *tbl_idx) const;

    bool CheckRunnUpdateQuery(const std::string *query_stmt) const;

    void AcquireRunnExclusiveLock() const {
      wr_exclusion_runn_mutex_.lock();
      wr_exclusion_runn_mutex_acqd_ = true;
    }

    void ReleaseRunnExclusiveLock() const {
      wr_exclusion_runn_mutex_.unlock();
      wr_exclusion_runn_mutex_acqd_ = false;
    }

    DalResultCode SetTableDirtyForGblAndVirMode(
        const UpllCfgType cfg_type,
        const DalTableIndex table_index,
        const unc_keytype_operation_t op) const;
    DalResultCode SetTableDirtyForVtnMode(
        const UpllCfgType cfg_type,
        const DalTableIndex table_index,
        const unc_keytype_operation_t op,
        const uint8_t* vtn_name) const;
    DalResultCode SetVtnCfgTblDirtyInDB(const UpllCfgType cfg_type,
                                        const unc_keytype_operation_t op,
                                        const DalTableIndex table_index,
                                        const uint8_t* vtn_name) const;
    // To check if table is dirty or not in any mode
    DalResultCode IsTblDirty(const unc_keytype_operation_t op,
                             const DalTableIndex table_index,
                             const CfgModeType cfg_mode,
                             const uint8_t* vtn_name) const;
    // To check if table is dirty or not in global cache in global mode
    DalResultCode IsTblInGlobalDirty(const unc_keytype_operation_t op,
                                     const DalTableIndex table_index) const;
    // To check if table is dirty or not in vtn cache and db in vtn mode
    DalResultCode IsTblInVtnDirty(const unc_keytype_operation_t op,
                                  const DalTableIndex table_index,
                                  const CfgModeType cfg_mode,
                                  const uint8_t* vtn_name) const;
    // To check if table is dirty or not in vtn dirty db in vtn or global mode
    DalResultCode IsTblInVtnDirtyInDB(const unc_keytype_operation_t op,
                                      const DalTableIndex table_index,
                                      const CfgModeType cfg_mode,
                                      const uint8_t* vtn_name) const;
    // Clear all the records in global dirty tbl
    DalResultCode ClearGlobalDirtyTblInDB(UpllCfgType cfg_type) const;
    // Clear all the records in vtn dirty tbl
    DalResultCode ClearVtnDirtyTblInDB(UpllCfgType cfg_type,
                                       const CfgModeType cfg_mode,
                                       const uint8_t* vtn_name) const;
    DalResultCode UpdateGlobalDirtyTblCacheFromDB() const;
    DalResultCode UpdateVtnDirtyTblCacheFromDB() const;
    DalResultCode UpdateVtnDirtyTblCache(const UpllCfgType cfg_type,
                                         const DalTableIndex table_index,
                                         const unc_keytype_operation_t op,
                                         const uint8_t* vtn_name) const;

    bool IsVtnNamePresentInBind(const DalTableIndex table_index,
                                DalBindInfo *bind_info,
                                const uint8_t* vtn_name) const;
    bool  IsVtnNamePresentInCol(const DalTableIndex table_index) const;
    bool  BindVtnNameForMatch(const DalTableIndex table_index,
                              DalBindInfo *bind_info,
                              const uint8_t* vtn_name) const;
    // Get maximum configuration session possible from dal.conf
    uint32_t GetMaxcfgSession() const;

    mutable std::set<uint32_t> create_dirty;
       // List of tables modified by candidate create operation
    mutable std::set<uint32_t> delete_dirty;
       // List of tables modified by candidate delete operation
    mutable std::set<uint32_t> update_dirty;
       // List of tables modified by candidate update operation
    SQLHANDLE dal_env_handle_;   // Environment Handle
    SQLHANDLE dal_conn_handle_;  // Connection Handle
    mutable DalConnType conn_type_;  // Connection Type
    mutable DalConnState conn_state_;  // Connection State
    mutable uint32_t write_count_;
    typedef std::pair<DalTableIndex, std::string> TblVtnNamePair;
    mutable std::set<TblVtnNamePair> create_vtn_dirty;
    mutable std::set<TblVtnNamePair> delete_vtn_dirty;
    mutable std::set<TblVtnNamePair> update_vtn_dirty;
    static std::map<std::string, DalTableIndex> tbl_name_to_idx_map_;
    static std::string db_rw_dsn;  // RW DSN connection string
    static std::string db_ro_dsn;  // RO DSN connection string

    static pfc::core::Mutex wr_exclusion_runn_mutex_;

    // If true, wr_exclusion_runn_mutex_ lock needs be taken in UPDATE to
    // running configuration in ExecuteQuery() & should be released in
    // after commit/abort
    bool wr_exclusion_on_runn_;
    // If wr_exclusion_runn_mutex_acqd_ is true, then wr_exclusion_runn_mutex_
    // is currently owned by this connection.
    mutable bool wr_exclusion_runn_mutex_acqd_;

    mutable pfc::core::Mutex wr_exclusion_var_mutex_;
    // Maximum cache limit is calculated based on max configure session
    static uint32_t max_cache_limit_;
    // Default configure session(64)
    static uint32_t default_max_session_;
    // If true, vtn dirty cache limit is reached for respective operation
    static bool max_cache_reached_cr_;
    static bool max_cache_reached_up_;
    static bool max_cache_reached_dl_;

    DalResultCode print_vtn_cache(const unc_keytype_operation_t op) const;
};  // class DalOdbcMgr

}  // namespace dal
}  // namespace upll
}  // namespace unc
#endif  // __DAL_ODBC_MGR_HH__
