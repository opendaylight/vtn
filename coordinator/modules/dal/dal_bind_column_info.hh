/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */


/**
 * dal_bind_column_info.hh
 *   Contains bind information for each column defined in the schema
 */ 

#ifndef __DAL_BIND_COLUMN_INFO__HH__
#define __DAL_BIND_COLUMN_INFO__HH__

#include <stdint.h>
#include <sqltypes.h>
#include <string>
#include "dal_defines.hh"
#include "dal_schema.hh"
#include "dal_bind_column_info_private.hh"

namespace unc {
namespace upll {
namespace dal {

/**
 * DalBindColumnInfo
 *   Bind information of columns available in schema
 *   contained in DalBindInfo
 */
class DalBindColumnInfo {
  public:
    /**
     * DalBindColumnInfo - Constructor
     *   Initilizes the DalBindcolumnInfo properties for the specific column
     *
     * @param[in] column_index  - Valid Column index of the corresponding table
     * @return void             - None
     */
    explicit DalBindColumnInfo(const DalColumnIndex column_index);

    /**
     * ~DalBindColumnInfo - Destructor
     *   Clean up the DalBindColumnInfo properties and destroys the allocated
     *   memory.
     *   Memory occupied by output buffer bound by dal user is not released
     *   here. The dal user should take care of releasing the memory
     *
     * @return void             - None
     */
    ~DalBindColumnInfo();

    // Accessors
    /**
     * get_column_index
     *   Fetch the column index of this DalBindColumnInfo instance
     *
     * @return uint8_t   - column index of the corresponding column
     */
    inline uint16_t get_column_index() const {
      return (column_index_);
    }

    /**
     * get_app_data_type
     *   Fetch the datatype used by dal user for this DalBindColumnInfo
     *   instance
     *
     * @return DalCDataType - C datatype used by dal user for the corresponding
     *                        column
     */
    inline DalCDataType get_app_data_type() const {
      return (app_data_type_);
    }

    /**
     * get_app_array_size
     *   Fetch the array size used by dal user for this DalBindColumnInfo
     *   instance
     *
     * @return DalCDataType - Array size used by dal user for the corresponding
     *                        column
     */
    inline size_t get_app_array_size() const {
      return (app_array_size_);
    }

    /**
     * get_io_type
     *   Fetch the IO type of this DalBindColumnInfo instance
     *
     * @return DalIoType - IO type of the corresponding column
     */
    inline DalIoType get_io_type() const {
      return (io_type_);
    }

    /**
     * get_app_out_addr
     *   Fetch the address of output buffer of the dal user for this
     *   DalBindColumnInfo instance
     *
     * @return void *    - void pointer to the dal user output buffer of the
     *                     corresponding column
     */
    inline void * get_app_out_addr() const {
      return (app_out_addr_);
    }

    /**
     * get_db_in_out_addr
     *   Fetch the address of input/output buffer of DAL layer for this
     *   DalBindColumnInfo instance
     *
     * @return void *    - void pointer to the DAL buffer that contains
     *                     input values from the dal user or output value
     *                     from the DB for the corresponding column
     */
    inline void * get_db_in_out_addr() const {
      return (db_in_out_addr_);
    }

    /**
     * get_db_match_addr
     *   Fetch the address of match buffer of DAL layer for this
     *   DalBindColumnInfo instance
     *
     * @return void *    - void pointer to the DAL buffer that contains match
     *                     value from the dal user for the corresponding column
     */
    inline void * get_db_match_addr() const {
      return (db_match_addr_);
    }

    /**
     * get_buff_len_ptr
     *   Fetch the address of buffer length to be used in SQLBindParameter
     *   and SQLBindColumn
     *
     * @return SQLLEN *  - pointer to the buffer length
     */
    inline SQLLEN * get_buff_len_ptr() const {
      return (buff_len_ptr_);
    }

    /**
     * UpdateColumnInfo
     *   Updates the column info with the given data
     *
     * @param[in] table_index     - Valid Table index corresponding to this
     *                              column
     * @param[in] app_data_type   - C datatype used by DAL user
     * @param[in] dal_data_type   - C datatype used by DAL
     * @param[in] array_size      - Array size of the column
     *                              1, if array is not used
     * @param[in] io_code         - IO code to update the binding address
     * @param[in] bind_addr       - Reference to the DAL user buffer for the
     *                              corresponding io_code
     * @return bool               - true, if success. false, otherwise
     */
    bool UpdateColumnInfo(const DalTableIndex table_index,
                          const DalCDataType app_data_type,
                          const SQLSMALLINT dal_data_type,
                          const size_t array_size,
                          const DalIoCode io_code,
                          const void **bind_addr);
    /**
     * CopyResultToAppAddr
     *   Copies the result to the dal user buffer based on the DAL datatype
     *
     * @param[in] dal_data_type   - C datatype used by DAL from schema
     * @return bool               - true, if success. false, otherwise
     */
    bool CopyResultToAppAddr(const DalTableIndex table_index);

    /**
     * ResetDalOutputBuffer
     *   Resets the ouput buffer in DAL based on the DAL datatype and array size
     *
     * @param[in] dal_data_type   - C datatype used by DAL from schema
     *
     * @return bool               - true, if success. false, otherwise
     */
    bool ResetDalOutputBuffer(const DalTableIndex table_index);

    std::string ColInfoToStr(const DalTableIndex table_index) const;
    std::string ColInfoInputToStr(const DalTableIndex table_index) const;
    std::string ColInfoResultToStr(const DalTableIndex table_index) const;
    std::string ValueInBindAddrToStr(const DalTableIndex table_index,
                                     const void **addr) const;

    /**
     * GetBindAvailability
     *   Gets the availability of binding for the given io_code in the
     *   current instance
     *
     * @param[in] io_code       - IO code to bind the buffer
     * @param[out] avail        - true, if possible. false, if not.
     * @return bool             - true, if possible. false, if not.
     */
    bool GetBindAvailability(const DalIoCode io_code, bool *avail);

    static std::string DalIoCodeToStr(const DalIoCode io_code);

  private:
    /**
     * AllocateAndCopy
     *   Allocates dest buffer and copies data from src buffer based on the 
     *   DAL data type of the corresponding column
     *
     * @param[in] dest            - Reference to the destination buffer
     *                              DAL buffer to store input/match value
     * @param[in] src             - Reference to the source buffer
     *                              DAL user buffer contians input/match value
     * @param[in] app_data_type   - C datatype used by Application
     * @param[in] dal_data_type   - C datatype used by DAL from schema
     * @return bool               - true, if success. false, otherwise
     */
    static bool AllocateAndCopy(void **dest, const void **src,
                                const DalCDataType app_data_type,
                                const SQLSMALLINT dal_data_type,
                                const size_t arraysize);

    /**
     * AllocateColBuffer
     *   Allocates buffer based on the DAL data type and array size of the
     *   corresponding column
     *
     * @param[in] buff_ptr        - Reference to the buffer for allocation
     *                              DAL buffer to store output value
     * @param[in] dal_data_type   - C datatype used by DAL from schema
     * @param[in] db_array_size   - Array size of the column from schema
     * @return bool               - true, if success. false, otherwise
     */
    static bool AllocateColBuffer(void **buff_ptr,
                                  const SQLSMALLINT dal_data_type,
                                  const size_t db_array_size);

    /**
     * CalculateDalBufferSize
     *   Calculates buffer size based on the DAL data type and array size of
     *   the corresponding column
     *
     * @param[in] dal_data_type   - C datatype used by DAL from schema
     * @param[in] db_array_size   - Array size of the column from schema
     * @return bool               - true, if success. false, otherwise
     */
    static size_t CalculateDalBufferSize(const SQLSMALLINT dal_data_type,
                                         const size_t db_array_size);

    /**
     * CalculateAppBufferSize
     *   Calculates buffer size based on the DAL data type and array size of
     *   the corresponding column
     *
     * @param[in] app_data_type   - C datatype used by DAL User.
     * @param[in] db_array_size   - Array size of the column from schema
     * @return bool               - true, if success. false, otherwise
     */
    static size_t CalculateAppBufferSize(const DalCDataType app_data_type,
                                         const size_t db_array_size);

    /**
     * GetCopyDataType
     *   Computes data type (DAL/ DAL user) for Copy input/ouput/match values
     *
     * @param[in] app_data_type   - C datatype used by DAL user
     * @param[in] dal_data_type   - C datatype used by DAL from schema
     * @param[in/out] data_type_code
     *                            - Reference to the datatype code
     * @return bool               - true, if success. false, otherwise
     */
    static void GetCopyDataType(const DalCDataType app_data_type,
                                const SQLSMALLINT dal_data_type,
                                DalDataTypeCode *data_type_code);

    /**
     * CopyUsingDalDataType
     *   Copies the data from src to dest buffer based on the DAL data type
     *   and array size of the corresponding column
     *
     * @param[in] dest            - Reference to the destination buffer
     * @param[in] src             - Reference to the source buffer
     * @param[in] dal_data_type   - C datatype used by DAL from schema
     * @param[in] array_size      - Array size of the column from schema
     * @param[in] input           - If true, copying input.
     *                              If false, copying output.
     *                              required for byte-ordering for binary data
     * @return bool               - true, if success. false, otherwise
     */
    static bool CopyUsingDalDataType(void **dest, const void **src,
                                     const SQLSMALLINT dal_data_type,
                                     const DalCDataType app_data_type,
                                     const size_t db_array_size,
                                     const bool input);

    /**
     * CopyUsingAppDataType
     *   Copies the data from src to dest buffer based on the C data type
     *   and array size used by DAL user
     *
     * @param[in] dest            - Reference to the destination buffer
     * @param[in] src             - Reference to the source buffer
     * @param[in] dal_data_type   - C datatype used by DAL user
     * @param[in] array_size      - Array size of the column from schema
     * @return bool               - true, if success. false, otherwise
     */
    static bool CopyUsingAppDataType(void **dest, const void **src,
                                     const DalCDataType app_data_type,
                                     const size_t db_array_size);

    /**
     * ResetColBuffer
     *   Resets the given buffer based on the DAL datatype and array size
     *
     * @param[in] col_buff        - Reference to the buffer in DalBindColumnInfo
     *                              corresponding io_code
     * @param[in] dal_data_type   - C datatype used by DAL from schema
     * @param[in] array_size      - Array size of the column from schema
     * @return bool               - true, if success. false, otherwise
     */
    static bool ResetColBuffer(void **col_buff,
                               const SQLSMALLINT dal_data_type,
                               const size_t array_size);

    /**
     * ComputeIoType
     *   Computes the IO type by current IO type of this instance and the given
     *   IO code
     *
     * @param[in] io_code       - IO code to bind the buffer
     * @return DalIoType        - computed IO type
     */
    static DalIoType ComputeIoType(const DalIoType curr_io_type,
                                   const DalIoCode io_code);

    /**
     * IsDataTypeCompatible
     *   Checks the data_type used by DAL and dal user are matching
     *
     * @param[in] app_data_type - C datattype used by dal user
     * @param[in] dal_data_type - C datattype used by DAL
     *
     * @return bool             - returns true, if matching
     *                            false, otherwise
     */
    static bool IsDataTypeCompatible(const DalCDataType app_data_type,
                                     const SQLSMALLINT dal_data_type);

    std::string AppDataTypeToStr(const DalCDataType app_data_type) const;
    std::string DalDataTypeToStr(const SQLSMALLINT dal_data_type) const;
    std::string DalIoTypeToStr(const DalIoType io_type) const;
    std::string AppValueToStr(const DalCDataType app_data_type,
                              const void **addr) const;
    std::string DalValueToStr(const SQLSMALLINT dal_data_type,
                              const void **addr) const;


    uint16_t column_index_;       // Column Index as given in the schema
                                  // from the corresponding table

    DalCDataType app_data_type_;  // C datatype used by DAL user

    size_t app_array_size_;       // Array size of the column used by DAL user

    DalIoType io_type_;           // IO type to specify the column binding

    void *app_out_addr_;          // DAL user buffer for output. The result
                                  // will be stored to this address after
                                  // successful query execution/fetch results

    void *db_in_out_addr_;        // DAL buffer contains input/output value
                                  // of DAL user to DB.
                                  // For input, this address contains the input
                                  // data for query execution.
                                  // For output, this address stores the result
                                  // of successful query execution. This value
                                  // will then be copied to app_out_addr which
                                  // is user buffer for output value.

    void *db_match_addr_;         // DAL buffer contains match value of DAL user
                                  // to DB for filtering results.

    SQLLEN *buff_len_ptr_;        // Buffer length or Indicator to specify the
                                  // number of bytes, the input data has for DB
                                  // On successful query execution, it has the
                                  // number of bytes, the output value has.
};  // class DalBindColumnInfo
}  // namespace dal
}  // namespace upll
}  // namespace unc
#endif  // __DAL_BIND_COLUMN_INFO_HH__
