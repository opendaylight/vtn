/*
 * Copyright (c) 2013-2016 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <json_build_parse.hh>

namespace unc {
namespace restjson {

// Creates json object
json_object* JsonBuildParse::create_json_obj() {
  ODC_FUNC_TRACE;
  return json_object_new_object();
}

json_object* JsonBuildParse::create_json_array_obj() {
  ODC_FUNC_TRACE;
  return json_object_new_array();
}

void JsonBuildParse::add_to_array (json_object *array,
                                   json_object* value) {
  ODC_FUNC_TRACE;
  json_object_array_add (array,value);
}

json_object* JsonBuildParse::create_json_int_obj(uint32_t val) {
  ODC_FUNC_TRACE;
  return json_object_new_int (val);

}
const char* JsonBuildParse::get_json_string(json_object* jobj) {
  if (json_object_is_type(jobj, json_type_null)) {
    return NULL;
  }
  return json_object_to_json_string(jobj);
}

// Converts String to Json Object
json_object* JsonBuildParse::get_json_object(char *data) {
  ODC_FUNC_TRACE;
  return json_tokener_parse(data);
}

// Gets the array length of the value
int JsonBuildParse::get_array_length(json_object* jobj,
                                     const std::string &key) {
  ODC_FUNC_TRACE;
  if (json_object_is_type(jobj, json_type_null)) {
    pfc_log_error("json object is NULL");
    return ZERO_ARRAY_LENGTH;
  }

  json_object *jobj_value(NULL);
  if (!json_object_object_get_ex(jobj, key.c_str(), &jobj_value) ||
      json_object_is_type(jobj_value, json_type_null)) {
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

// Gets array length of json object
int JsonBuildParse::get_array_length(json_object* jobj) {
  // If the json object is not of type array return ZERO as longth
  if (!(json_object_is_type(jobj, json_type_array))) {
    pfc_log_error("json object is not of type array");
    return ZERO_ARRAY_LENGTH;
  }
  int arr_length = json_object_array_length(jobj);
  pfc_log_debug("%d array length", arr_length);
  return arr_length;
}


}  // namespace restjson
}  // namespace unc
