/*
 * Copyright (c) 2013-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * dal_dml_intf.hh
 *   Contains definition of DalDmlIntf
 *   DML interface of database
 *
 * Implemented by DalOdbcMgr
 */

#ifndef __DAL_DML_INTF_HH__
#define __DAL_DML_INTF_HH__

#include <stdint.h>
#include "dal_defines.hh"
#include "dal_bind_info.hh"
#include "dal_schema.hh"
#include "dal_cursor.hh"

namespace unc {
namespace upll {
namespace dal {

/**
 *  DalConnIntf
 *    Database management query APIs for database
 *
 *  Inherited by DalOdbcMgr
 */
class DalDmlIntf {
  public:
    /**
     * DalDmlIntf - Constructor
     *
     * @return void             - None
     */
    DalDmlIntf() {
    }

    /**
     * DalDmlIntf - Constructor
     *
     * @return void             - None
     */

    virtual ~DalDmlIntf() {
    }

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
    virtual DalResultCode GetSingleRecord(
                    const UpllCfgType cfg_type,
                    const DalTableIndex table_index,
                    const DalBindInfo *ouput_and_matching_attr_info) const = 0;

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
    virtual DalResultCode GetMultipleRecords(
                    const UpllCfgType cfg_type,
                    const DalTableIndex table_index,
                    const size_t max_record_count,
                    const DalBindInfo *ouput_and_matching_attr_info,
                    DalCursor **cursor) const = 0;

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
    virtual DalResultCode GetNextRecord(const DalCursor *cursor) const = 0;

    /**
     * CloseCursor
     *   Close and destroy the cursor. After this call, cursor object is not
     *   aavailable.
     *
     * @param[in] cursor          - Pointer to the valid cursor instance
     *
     * @return DalResultCode      - kDalRcSuccess in case of success
     *                            - Valid errorcode otherwise
     */
    virtual DalResultCode CloseCursor(DalCursor *cursor) const = 0;

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
    virtual DalResultCode RecordExists(const UpllCfgType cfg_type,
                               const DalTableIndex table_index,
                               const DalBindInfo *matching_attr_info,
                               bool *existence) const = 0;

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
    virtual DalResultCode GetSiblingBegin(
                    const UpllCfgType cfg_type,
                    const DalTableIndex table_index,
                    const size_t max_record_count,
                    const DalBindInfo *ouput_and_matching_attr_info,
                    DalCursor **cursor) const = 0;

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
    virtual DalResultCode GetSiblingRecords(
                    const UpllCfgType cfg_type,
                    const DalTableIndex table_index,
                    const size_t max_record_count,
                    const DalBindInfo *ouput_and_matching_attr_info,
                    DalCursor **cursor) const = 0;

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
    virtual DalResultCode GetSiblingCount(const UpllCfgType cfg_type,
                                  const DalTableIndex table_index,
                                  const DalBindInfo *matching_attr_info,
                                  uint32_t *count) const = 0;

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
    virtual DalResultCode GetRecordCount(const UpllCfgType cfg_type,
                                 const DalTableIndex table_index,
                                 const DalBindInfo *matching_attr_info,
                                 uint32_t *count) const = 0;

    /**
     * DeleteRecords
     *   Deletes the records of table from the given cfg_type
     *
     * @param[in] cfg_type        - Configuration Type for which the records
     *                              have to be updated
     * @param[in] table_index     - Valid Index of the table
     * @param[in] matching_attr_info
     *                            - Bind Information for deleting records
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
     */
    virtual DalResultCode DeleteRecords(const UpllCfgType cfg_type,
                                const DalTableIndex table_index,
                                const DalBindInfo *matching_attr_info) const = 0;

    /**
     * CreateRecord
     *   Creates the record in table with the given input data for 
     *   the given cfg_type
     *
     * @param[in] cfg_type        - Configuration Type for which the record
     *                              has to be created
     * @param[in] table_index     - Valid Index of the table
     * @param[in] input_attr_info - Bind Information for creating record
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
    virtual DalResultCode CreateRecord(const UpllCfgType cfg_type,
                               const DalTableIndex table_index,
                               const DalBindInfo *input_attr_info) const = 0;

    /**
     * UpdateRecords
     *   Updates the records of table with the given input data for 
     *   the given cfg_type
     *
     * @param[in] cfg_type        - Configuration Type for which the records
     *                              have to be updated
     * @param[in] table_index     - Valid Index of the table
     * @param[in] input_and_matching_attr_info
     *                            - Bind Information for updating records
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
    virtual DalResultCode UpdateRecords(
                    const UpllCfgType cfg_type,
                    const DalTableIndex table_index,
                    const DalBindInfo *input_and_matching_attr_info) const = 0;

    virtual DalResultCode UpdateRecords(
                        string query_statement,
                        const UpllCfgType cfg_type,
                        const DalTableIndex table_index,
                        const DalBindInfo *input_and_matching_attr_info) const = 0;

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
    virtual DalResultCode GetDeletedRecords(const UpllCfgType cfg_type_1,
                                    const UpllCfgType cfg_type_2,
                                    const DalTableIndex table_index,
                                    const size_t max_record_count,
                                    const DalBindInfo *output_attr_info,
                                    DalCursor **cursor) const = 0;

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
    virtual DalResultCode GetCreatedRecords(const UpllCfgType cfg_type_1,
                                    const UpllCfgType cfg_type_2,
                                    const DalTableIndex table_index,
                                    const size_t max_record_count,
                                    const DalBindInfo *output_attr_info,
                                    DalCursor **cursor) const = 0;

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
    virtual DalResultCode GetUpdatedRecords(
                    const UpllCfgType cfg_type_1,
                    const UpllCfgType cfg_type_2,
                    const DalTableIndex table_index,
                    const size_t max_record_count,
                    const DalBindInfo *cfg_1_output_and_match_attr_info,
                    const DalBindInfo *cfg_2_output_and_match_attr_info,
                    DalCursor **cursor) const = 0;

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
    virtual DalResultCode CopyEntireRecords(const UpllCfgType dest_cfg_type,
                                    const UpllCfgType src_cfg_type,
                                    const DalTableIndex table_index,
                                    const DalBindInfo *output_attr_info) const = 0;

    /**
     * CopyModifiedRecords
     *   Copies the entire records of table from source configuration to
     *   destination configuration.
     *
     * @param[in] dest_cfg_type   - Configuration Type where the records to be
     *                              copied (not equal to src_cfg_type)
     * @param[in] src_cfg_type    - Configuration Type from where the records
     *                              will be copied (not equal to dest_cfg_type)
     * @param[in] table_index     - Valid Index of the table
     * @param[in] output_and_match_attr_info
     *                            - Bind Information for output and match
     *                            - columns
     *
     * @return DalResultCode      - kDalRcSuccess in case of success
     *                            - Valid errorcode otherwise
     *                              On successful execution, both the
     *                              configurations have same records.
     *
     * Note:
     * Information on Copy Logic
     * 1. Remvoe the records from dest_cfg_type that are result of
     *    GetDeletedRecords(dest_cfg_type, src_cfg_type, ...)
     * 2. Add the records in dest_cfg_type that are result of
     *    GetCreatedRecords(dest_cfg_type, src_cfg_type, ...)
     * 3. Update the records in dest_cfg_type with the records from 
     *    src_cfg_type that are result of
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
    virtual DalResultCode CopyModifiedRecords(
                    const UpllCfgType dest_cfg_type,
                    const UpllCfgType src_cfg_type,
                    const DalTableIndex table_index,
                    const DalBindInfo *output_and_match_attr_info) const = 0;

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
    virtual DalResultCode CopyMatchingRecords(
                    const UpllCfgType dest_cfg_type,
                    const UpllCfgType src_cfg_type,
                    const DalTableIndex table_index,
                    const DalBindInfo *output_and_match_attr_info) const = 0;

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
    virtual DalResultCode CheckRecordsIdentical(const UpllCfgType cfg_type_1,
                                        const UpllCfgType cfg_type_2,
                                        const DalTableIndex table_index,
                                        const DalBindInfo *matching_attr_info,
                                        bool *identical) const = 0;
};  // class DalDmlIntf

};  // namespace dal
};  // namespace upll
};  // namespace unc
#endif  // __DAL_DML_INTF_HH__
