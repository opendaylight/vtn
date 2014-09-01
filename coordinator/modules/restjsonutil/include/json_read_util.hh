/*
 * Copyright (c) 2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */


#ifndef JSON_READ_UTIL_H_
#define JSON_READ_UTIL_H_

#include <json_build_parse.hh>
#include <uncxx/odc_log.hh>
#include <string>

namespace unc {
namespace restjson {
class json_object_parse_util {
  public:
    static int read_int_value(json_object* in,
                               std::string name,
                               int &value) {
      return unc::restjson::JsonBuildParse::parse(in,
                                                  name.c_str(),
                                                  value);
    }

    static int read_unsigned_int_value(json_object* in,
                                        std::string name,
                                        uint &value ) {
      return unc::restjson::JsonBuildParse::parse(in,
                                                  name.c_str(),
                                                  value);
    }

    static int read_long_value(json_object* in,
                                std::string name,
                                unsigned long long &value) {
      return unc::restjson::JsonBuildParse::parse(in,
                                                  name.c_str(),
                                                  value);
    }

    static int read_string(json_object* in,
                            std::string name,
                            std::string& value ) {
      return unc::restjson::JsonBuildParse::parse(in,
                                                  name.c_str(),
                                                  -1,
                                                  value);
    }

    static int read_boolean(json_object* in,
                             std::string name,
                             bool& value ) {
      return unc::restjson::JsonBuildParse::parse(in,
                                                  name.c_str(),
                                                  value);
    }

    static int extract_json_object(char *buffer,
                                   json_object** value) {
      *value = restjson::JsonBuildParse::get_json_object(buffer);
      if ( *value != NULL )
        return REST_OP_SUCCESS;
      else
        return REST_OP_FAILURE;
    }

    static int extract_json_object(json_object* in,
                                   std::string search,
                                   json_object** out) {
      restjson::JsonBuildParse::parse(in,
                                      search,
                                      -1,
                                      *out);

      if ( out )
        return REST_OP_SUCCESS;

      return REST_OP_FAILURE;
    }

    static bool is_array(json_object* object) {
      return json_object_is_type(object,
                                 json_type_array);
    }

    static uint32_t get_array_length(json_object* object) {
      return restjson::JsonBuildParse::get_array_length(object);
    }
};

class json_obj_destroy_util {
  private:
    json_object* obj_;


  public:
    json_obj_destroy_util(): obj_(NULL) {}
    json_obj_destroy_util(json_object* json_obj): obj_(json_obj) {}
    void set_object(json_object *set) {
      if (!set)
        obj_= set;
    }
    ~json_obj_destroy_util() {
      if ( obj_ )
        json_object_put(obj_);
    }
};

class json_array_object_parse_util {
  private:
    json_object* array_obj_;

  public:
    json_array_object_parse_util(json_object* obj): array_obj_(obj) {}

    virtual int start_array_read(int length_of_array,
                                 json_object* json_instance) = 0;

    virtual int read_array_iteration(uint32_t index,
                                     json_object* json_instance) = 0;

    virtual int end_array_read(uint32_t index,
                               json_object* json_instance) = 0;

    int extract_values() {
      ODC_FUNC_TRACE;
      if ( !restjson::json_object_parse_util::is_array(array_obj_) ) {
        return REST_OP_FAILURE;
      }
      uint32_t array_length = restjson::JsonBuildParse::
                                get_array_length(array_obj_);
      if ( start_array_read(array_length, array_obj_) == REST_OP_FAILURE )
        return REST_OP_FAILURE;

      for (uint32_t iter = 0; iter < array_length; iter++) {
        pfc_log_info("Iteration %d", iter);
        json_object* array_inst = json_object_array_get_idx(array_obj_, iter);
        if ( read_array_iteration(iter, array_inst) == REST_OP_FAILURE ) {
          pfc_log_info("Failed in parsing the array index %d", iter);
          return REST_OP_FAILURE;
        }
      }
      if ( end_array_read(array_length, array_obj_) == REST_OP_FAILURE ) {
        pfc_log_info("Failed in End array read");
        return REST_OP_FAILURE;
      }

      pfc_log_info("Extract values success");
      return REST_OP_SUCCESS;
    }

    int get_array_length() {
      return restjson::JsonBuildParse::get_array_length(array_obj_);
    }
};
}  // namespace restjson
}  // namespace unc

#endif
