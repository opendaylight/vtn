/*
 * Copyright (c) 2013-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef __DAL_BIND_INFO__HH__
#define __DAL_BIND_INFO__HH__

#include <stdint.h>
#include <string.h>
#include <vector>
#include <map>
#include "include/dal_defines.hh"
#include "include/dal_schema.hh"
#include "include/dal_bind_column_info.hh"

namespace unc {
namespace upll {
namespace dal {

// Type definition for vector of DalBindColumnInfo
typedef std::vector<DalBindColumnInfo *> DalBindList;

class DalBindInfo {
  public:

	enum Method
	{
		BIND_INPUT,
		BIND_OUTPUT,
		BIND_MATCH,
		COPY,
		RESET
	};

    explicit DalBindInfo(const DalTableIndex table_index);

    ~DalBindInfo();

    bool BindInput(const DalColumnIndex column_index,
                   const DalCDataType app_data_type,
                   const size_t array_size,
                   const void *bind_addr);

    bool BindOutput(const DalColumnIndex column_index,
                    const DalCDataType app_data_type,
                    const size_t array_size,
                    const void *bind_addr);

    bool BindMatch(const DalColumnIndex column_index,
                   const DalCDataType app_data_type,
                   const size_t array_size,
                   const void *bind_addr);

    inline DalTableIndex get_table_index() const {
      return (table_index_);
    }

    inline DalBindList get_bind_list() const {
      return (bind_list_);
    }

    inline uint16_t get_input_bind_count() const {
      return (input_bind_count_);
    }

    inline uint16_t get_output_bind_count() const {
      return (output_bind_count_);
    }

    inline uint16_t get_match_bind_count() const {
      return (match_bind_count_);
    }

    static void stub_set_table_index(DalTableIndex tableIndex) {
    	table_index_ = tableIndex;
    }

    static void stub_set_bind_list( DalBindList dalBindList) {
      bind_list_ = dalBindList;
    }

    static void  stub_set_input_bind_count(uint16_t bind_count) {
       input_bind_count_= bind_count;
    }

    static void  stub_set_output_bind_count( uint16_t output_bind) {
         output_bind_count_=output_bind;
    }

    static void stub_set_match_bind_count(uint16_t bind_count) {
    	match_bind_count_ = bind_count;
    }

    static void stub_setResultcode(DalBindInfo::Method methodType ,DalResultCode res_code) {
    	method_result_map.insert(std::make_pair(methodType,res_code));
    }

    static void clearStubData() {
    	method_result_map.clear();
    }


    bool CopyResultToApp();

    bool ResetDalOutBuffer();
  private:
    bool stub_getMappedResultCode(DalBindInfo::Method);
    static DalBindList bind_list_;
    static DalTableIndex table_index_;
    static uint16_t input_bind_count_;

    static uint16_t output_bind_count_;

    static uint16_t match_bind_count_;
    static std::map<DalBindInfo::Method,bool> method_result_map;

};  // class DalBindInfo
}  // namespace dal
}  // namespace upll
}  // namespace unc
#endif  // __DAL_BIND_INFO_HH__
