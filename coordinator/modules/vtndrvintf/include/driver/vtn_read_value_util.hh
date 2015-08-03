/*
 * Copyright (c) 2014-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */



#ifndef __VTN_READ_UTIL_HH__
#define __VTN_READ_UTIL_HH__
#include <uncxx/dataflow.hh>
#include <pfcxx/ipc_server.hh>
#include <unc/unc_base.h>
#include <uncxx/odc_log.hh>
#include <pfc/ipc_struct.h>
#include <list>

#define INPUT_KEY_STRUCT_INDEX 10
#define INPUT_VAL_STRUCT_INDEX 11



namespace unc {
namespace vtnreadutil {


class read_cache_element {
  public:
    virtual void write_to_sess(pfc::core::ipc::ServerSession &sess) = 0;
    virtual ~read_cache_element() {}
};



template < typename ipc_val_structure >
class cache_value_item : public read_cache_element {
  private:
    ipc_val_structure* cache_val_;

  public:
    cache_value_item(ipc_val_structure* value) {
      if (value == NULL) {  // add NULL element for sending empty breaks
        cache_val_= NULL;
      } else {
        cache_val_ = new ipc_val_structure();
        memcpy(cache_val_, value, sizeof(ipc_val_structure));
      }
    }

    ~cache_value_item() {
      if ( cache_val_ )
        delete cache_val_;
    }

    void write_to_sess(pfc::core::ipc::ServerSession &sess) {
      ODC_FUNC_TRACE;
      if (cache_val_) {
        if (sess.addOutput(*cache_val_)) {
          pfc_log_error("Write to Session Failed");
        } else {
          pfc_log_info("write to cache succes");
        }
      } else {
        pfc_log_trace("No element in cache to write to session");
        // Write Empty to signify break
        if (sess.addOutput()) {
          pfc_log_error("Write to Session Failed");
        }
      }
    }
};

class driver_read_util {
  public:
    std::list <unc::vtnreadutil::read_cache_element*> drv_read_results_;
    unc::dataflow::DataflowUtil *df_util_;
    unc::dataflow::DataflowCmn *df_cmn_;
    pfc::core::ipc::ServerSession &sess_;

    driver_read_util(pfc::core::ipc::ServerSession& sess);
    ~driver_read_util();

     void write_response_to_session();
     uint32_t get_arg_count();
     uint32_t get_option1();
     uint32_t get_option2();
     const char* get_ctr_id();
     pfc_bool_t alternate_flow;
};

class driver_dataflow_read_util {
  public:
    static void add_read_value(unc::dataflow::DataflowCmn* value,
                           unc::vtnreadutil::driver_read_util *read_util);
};

template<typename ipc_val_structure>
class driver_read_util_io {
  public:
    static UncRespCode read_key_val(ipc_val_structure *instance,
                               pfc_bool_t is_key,
                               unc::vtnreadutil::driver_read_util*,
                               pfc_bool_t add_key = PFC_TRUE);

    static UncRespCode read_additional_args(ipc_val_structure *instance,
                                        int index,
                                        unc::vtnreadutil::driver_read_util*);

    static void add_read_value(ipc_val_structure* value,
                              unc::vtnreadutil::driver_read_util*);

    // Zero pos indicates the top of the list
    static void add_read_value_top(ipc_val_structure* value,
                              unc::vtnreadutil::driver_read_util*);
};


template <typename ipc_key_val_st>
void driver_read_util_io<ipc_key_val_st>::add_read_value(
    ipc_key_val_st* value,
    unc::vtnreadutil::driver_read_util *read_util) {
  unc::vtnreadutil::read_cache_element *read_value=
      new cache_value_item<ipc_key_val_st> (value);
  PFC_ASSERT(read_value != NULL);
  read_util->drv_read_results_.push_back(read_value);
}

template <typename ipc_key_val_st>
void driver_read_util_io<ipc_key_val_st>::add_read_value_top(
    ipc_key_val_st* value,
    unc::vtnreadutil::driver_read_util *read_util) {
  unc::vtnreadutil::read_cache_element *read_value=
      new cache_value_item<ipc_key_val_st> (value);
  PFC_ASSERT(read_value != NULL);
  read_util->drv_read_results_.push_front(read_value);
}

template<typename ipc_key_val_st>
    UncRespCode driver_read_util_io<ipc_key_val_st>::read_key_val(
                                             ipc_key_val_st *instance,
                                             pfc_bool_t is_key,
                              unc::vtnreadutil::driver_read_util *read_util,
                              pfc_bool_t add_key) {
      if ( is_key ) {
        if (read_util->sess_.getArgument(INPUT_KEY_STRUCT_INDEX, *instance)) {
          return UNC_DRV_RC_MISSING_KEY_STRUCT;
        }
        // To append key tothe output
        //
        if ( add_key == PFC_TRUE ) {
          pfc_log_info("Key to be added to the output!!");
          unc::vtnreadutil::read_cache_element *key_value=
                              new cache_value_item<ipc_key_val_st> (instance);
          PFC_ASSERT(key_value != NULL);
          read_util->drv_read_results_.push_back(key_value);
        }
      } else {
        if (read_util->sess_.getArgument(INPUT_VAL_STRUCT_INDEX, *instance))
          return UNC_DRV_RC_MISSING_VAL_STRUCT;
      }
      return UNC_RC_SUCCESS;
    }


  template<typename ipc_key_val_st>
    UncRespCode driver_read_util_io<ipc_key_val_st>::read_additional_args(
                                        ipc_key_val_st *instance,
                                        int index,
                    unc::vtnreadutil::driver_read_util *read_util) {
      int arg_index = INPUT_VAL_STRUCT_INDEX+index;
      if ( read_util->sess_.getArgument(arg_index, *instance))
        return UNC_RC_INTERNAL_ERR;
    }


}  // namespace vtnreadutil
}  // namespace unc

#endif
