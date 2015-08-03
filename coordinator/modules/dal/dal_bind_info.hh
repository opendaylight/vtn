/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * dal_info.hh
 *   Contians information and buffers from dal user for all the columns
 *   in a specific table
 *   The buffers store the input values from dal user to databse and
 *   the output values from database to dal user.
 */

#ifndef __DAL_BIND_INFO__HH__
#define __DAL_BIND_INFO__HH__

#include <stdint.h>
#include <sqltypes.h>
#include <vector>
#include <string>
#include "dal_schema.hh"
#include "dal_bind_column_info.hh"

namespace unc {
namespace upll {
namespace dal {
// Type definition for vector of DalBindColumnInfo
typedef std::vector<DalBindColumnInfo *> DalBindList;

/**
 * DalBindInfo
 *   Bind information of all columns of the specific table
 *   Used by DAL user to bind the buffers for input/output/match values
 */
class DalBindInfo {
  public:
    /**
     * DalBindInfo - Constructor
     *   Initilizes the DalBindInfo properties for the specific table
     *
     * @param[in] table_index   - Valid Table index available in the schema
     * @return void             - None
     */
    explicit DalBindInfo(const DalTableIndex table_index);

    /**
     * ~DalBindInfo - Destructor
     *   Clean up the DalBindInfo properties
     *
     * @return void             - None
     */
    ~DalBindInfo();

    /**
     * BindInput
     *   Binds the DAL user buffer that contains input value
     *
     * @param[in] column_index  - Valid Column index of the corresponding table
     * @param[in] app_data_type - C datatype used by DAL user
     * @param[in] array_size    - Array size of the column
     *                            1, if array is not used
     * @param[in] bind_addr     - Reference to the DAL user buffer that
     *                            contains input value
     * @return bool             - true, if success. false, otherwise
     */
    bool BindInput(const DalColumnIndex column_index,
                   const DalCDataType app_data_type,
                   const size_t array_size,
                   const void *bind_addr);

    /**
     * BindOutput
     *   Binds the DAL user buffer for output from DAL
     *
     * @param[in] column_index  - Valid Column index of the corresponding table
     * @param[in] app_data_type - C datatype used by DAL user
     * @param[in] array_size    - Array size of the column
     *                            1, if array is not used
     * @param[in] bind_addr     - Reference to the DAL user buffer for output
     * @return bool             - true, if success. false, otherwise
     */
    bool BindOutput(const DalColumnIndex column_index,
                    const DalCDataType app_data_type,
                    const size_t array_size,
                    const void *bind_addr);

    /**
     * BindMatch
     *   Binds the DAL user buffer that contains match value
     *
     * @param[in] column_index  - Valid Column index of the corresponding table
     * @param[in] app_data_type - C datatype used by DAL user
     * @param[in] array_size    - Array size of the column
     *                            1, if array is not used
     * @param[in] bind_addr     - Reference to the DAL user buffer that
     *                            contains match value
     * @return bool             - true, if success. false, otherwise
     */
    bool BindMatch(const DalColumnIndex column_index,
                   const DalCDataType app_data_type,
                   const size_t array_size,
                   const void *bind_addr);

    /* Accessors */
    /**
     * get_table_index
     *   Fetches the stored table_index of this instance
     *   Used by DAL layer
     *
     * @return DalTableIndex    - table_index of this instance
     */
    inline DalTableIndex get_table_index() const {
      return (table_index_);
    }

    /**
     * get_bind_list
     *   Fetches the stored bind list for the corresponding table index
     *   Used by DAL layer
     *   Bind List contains the bind information of all the columns of the
     *   given table
     *
     * @return DalBindList      - Bind List of this instance
     */
    inline DalBindList get_bind_list() const {
      return (bind_list_);
    }

    /**
     * get_input_bind_count
     *   Fetches the count of columns bound for input
     *   Used by DAL layer
     *
     * @return uint16_t         - count of input bound columns
     */
    inline uint16_t get_input_bind_count() const {
      return (input_bind_count_);
    }

    /**
     * get_output_bind_count
     *   Fetches the count of columns bound for output
     *   Used by DAL layer
     *
     * @return uint16_t         - count of output bound columns
     */
    inline uint16_t get_output_bind_count() const {
      return (output_bind_count_);
    }

    /**
     * get_match_bind_count
     *   Fetches the count of columns bound for match
     *   Used by DAL layer
     *
     * @return uint16_t         - count of match bound columns
     */
    inline uint16_t get_match_bind_count() const {
      return (match_bind_count_);
    }

    /**
     * CopyResultToApp
     *   Copies the result stored in DAL output buffer to the dal user output
     *   buffer
     *   Used by DAL layer
     *
     * @return bool             - true, if successfully copied
     *                            false, otherwise
     */
    bool CopyResultToApp();

    /**
     * ResetDalOutBuffer
     *   Resets the DAL output buffer to store the next result from the
     *   database
     *   Used by DAL layer
     *
     * @return bool             - true, if successfully copied
     *                            false, otherwise
     */
    bool ResetDalOutBuffer();

    /* Only for Debugging and Testing */
    std::string BindListToStr();
    std::string BindListInputToStr();
    std::string BindListResultToStr();

    /**
     * ValidateInputParams
     * Validates the input parameters to bind functions
     *
     * @return bool             - true, if all te params are valid
     *                            false, otherwise
     */
    bool ValidateInputParams(const DalColumnIndex column_index,
                             const DalCDataType app_data_type,
                             const size_t array_size,
                             const void *bind_addr);

  private:
    // Initialize the bind list for all columns with default values
    void InitBindList();

    /**
     * BindAttribute
     *   Wrapper for BindInput/Output/Match APIs
     *
     * @param[in] io_code       - IO code to bind the buffer
     * @param[in] column_index  - Valid Column index of the corresponding table
     * @param[in] app_data_type - C datatype used by DAL user
     * @param[in] array_size    - Array size of the column
     *                            1, if array is not used
     * @param[in] bind_addr     - Reference to the DAL user buffer for the
     *                            corresponding io_code
     * @return bool             - true, if success. false, otherwise
     */
    bool BindAttribute(const DalIoCode io_code,
                       const DalColumnIndex column_index,
                       const DalCDataType app_data_type,
                       const size_t array_size,
                       const void **bind_addr);

    DalBindList bind_list_;       // Bind information of all the columns for
                                  // the corresponding table index
    DalTableIndex table_index_;   // Index of the table from schema
    uint16_t input_bind_count_;   // Count of columns bound for input by
                                  // the dal user
    uint16_t output_bind_count_;  // Count of columns bound for output by
                                  // the dal user
    uint16_t match_bind_count_;   // Count of columns bound for match by
                                  // the dal user
};  // class DalBindInfo
}  // namespace dal
}  // namespace upll
}  // namespace unc
#endif  // __DAL_BIND_INFO_HH__
