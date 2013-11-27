/*
 * Copyright (c) 2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef RESTJSON_JSON_BUILD_PARSE_H_
#define RESTJSON_JSON_BUILD_PARSE_H_

#include <uncxx/odc_log.hh>
#include <json/json.h>
#include <json_type_util.hh>
#include <rest_common_defs.hh>
#include <pfc/log.h>
#include <string>

namespace unc {
namespace restjson {

class JsonBuildParse {
 public:
      template < typename T >
      static int build
      (const std::string & key, T data, json_object * jparent);

      template < typename T >
        static int parse(json_object * jobj, const std::string & key,
              int arrindex, T & val);

static json_object* create_json_obj() {
ODC_FUNC_TRACE;
  return json_object_new_object();
}

// Converts Json Obj to Json String
static const char* get_string(json_object* jobj) {
  ODC_FUNC_TRACE;
  if (json_object_is_type(jobj, json_type_null)) {
    pfc_log_error("json object is NULL");
    return NULL;
  }
  return json_object_to_json_string(jobj);
}

// Converts String to Json Object
static json_object* get_json_object(char *data) {
  ODC_FUNC_TRACE;
  return json_tokener_parse(data);
}

// Gets the array length of the value
static int get_array_length(json_object* jobj,
                                     const std::string &key) {
  ODC_FUNC_TRACE;
  if (json_object_is_type(jobj, json_type_null)) {
    pfc_log_error("json object is NULL");
    return ZERO_ARRAY_LENGTH;
  }

  json_object *jobj_value = json_object_object_get(jobj, key.c_str());
  if (json_object_is_type(jobj_value, json_type_null)) {
    pfc_log_error("json object value is NULL");
    return ZERO_ARRAY_LENGTH;
  }

  // If the json object is not of type array return ZERO as longth
  if (!(json_object_is_type(jobj_value, json_type_array))) {
    pfc_log_error("json object is not of type array");
    return ZERO_ARRAY_LENGTH;
  }
  int arr_length = json_object_array_length(jobj_value);
  pfc_log_debug("%d array length", arr_length);
  return arr_length;
}
};

template < typename T > int
JsonBuildParse::build(const std::string & key, T data, json_object * jparent) {
  if (json_object_is_type(jparent, json_type_null)) {
    return REST_OP_FAILURE;
  }

  if (json_object_is_type(jparent, json_type_object)) {
    json_object *jobj;
    JsonTypeUtil jsonutil_obj(data);
    jobj = jsonutil_obj.get_json_data();
    if (json_object_is_type(jobj, json_type_null)) {
      return REST_OP_FAILURE;
    }
    json_object_object_add(jparent, key.c_str(), jobj);
    return REST_OP_SUCCESS;
  }
  return REST_OP_FAILURE;
}

template < typename T > int
JsonBuildParse::parse(json_object * jobj, const std::string & key,
                       int arrindex, T & val) {
  if (json_object_is_type(jobj, json_type_null)) {
    return REST_OP_FAILURE;
  }
  json_object * jobj_getval;

  if (-1 != arrindex) {
    json_object *jobj_array = json_object_array_get_idx(jobj, arrindex);
    jobj_getval = json_object_object_get(jobj_array, key.c_str());
  } else {
    jobj_getval = json_object_object_get(jobj, key.c_str());
  }
  if (json_object_is_type(jobj_getval, json_type_null)) {
    return REST_OP_SUCCESS;
  }

  JsonTypeUtil::get_value(jobj_getval, val);


  return REST_OP_SUCCESS;
}

}  //  namespace restjson
}  //  namespace unc
#endif  // RESTJSON_JSON_BUILD_PARSE_H_
