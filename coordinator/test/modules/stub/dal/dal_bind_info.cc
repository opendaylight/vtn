/*
 * Copyright (c) 2013-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include "dal_bind_info.hh"

namespace unc {
namespace upll {
namespace dal {

 DalBindList DalBindInfo::bind_list_;
 DalTableIndex DalBindInfo::table_index_;
 uint16_t DalBindInfo::input_bind_count_;
 uint16_t DalBindInfo::output_bind_count_;
 std::map<DalBindInfo::Method,bool>  DalBindInfo::method_result_map;

 uint16_t match_bind_count_;
DalBindInfo::DalBindInfo(const DalTableIndex table_index) {

}
DalBindInfo::~DalBindInfo() {

}

bool DalBindInfo::BindInput(const DalColumnIndex column_index,
                  const DalCDataType app_data_type,
                  const size_t array_size,
                  const void *bind_addr) {
	return stub_getMappedResultCode(DalBindInfo::BIND_INPUT);

}

bool DalBindInfo::BindOutput(const DalColumnIndex column_index,
                   const DalCDataType app_data_type,
                   const size_t array_size,
                   const void *bind_addr) {
	return stub_getMappedResultCode(DalBindInfo::BIND_OUTPUT);

}

bool DalBindInfo::BindMatch(const DalColumnIndex column_index,
                  const DalCDataType app_data_type,
                  const size_t array_size,
                  const void *bind_addr) {
	return stub_getMappedResultCode(DalBindInfo::BIND_MATCH);

}

bool  DalBindInfo::CopyResultToApp() {
	return stub_getMappedResultCode(DalBindInfo::COPY);
}

 bool  DalBindInfo::ResetDalOutBuffer() {
	 return stub_getMappedResultCode(DalBindInfo::RESET);
 }

 bool DalBindInfo::stub_getMappedResultCode(DalBindInfo::Method methodType) {
	 if (0 != method_result_map.count(methodType))  {
		 return method_result_map[methodType];
	 }
	 return false;
 }

}
}
}



