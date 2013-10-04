/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made
 * available under the
 * terms of the Eclipse Public License v1.0 which
 * accompanies this
 * distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 */

#include <json_build_parse.hh>

namespace restjson {

// Creates json object
json_object* JsonBuildParse::create_json_obj() {
  json_object* jsonobj = json_object_new_object();
  return jsonobj;
}

// Converts Json Obj to Json String
const char* JsonBuildParse::json_obj_to_json_string(json_object* jobj) {
  if (json_object_is_type(jobj, json_type_null)) {
    return NULL;
  }
  return json_object_to_json_string(jobj);
}

// Gets the enum type of the value
int JsonBuildParse::get_type(json_object* jobj, const std::string &key) {
  if (json_object_is_type(jobj, json_type_null)) {
    return FAILURE;
  }
  json_object *jobj_value = json_object_object_get(jobj, key.c_str());
  json_type type = json_object_get_type(jobj_value);
  return type;
}

// Gets the array length of the value
int JsonBuildParse::get_array_length(json_object* jobj,
                                     const std::string &key) {
  if (json_object_is_type(jobj, json_type_null)) {
    return FAILURE;
  }
  json_object *jobj_value = json_object_object_get(jobj, key.c_str());
  if (json_object_is_type(jobj_value, json_type_null)) {
    return FAILURE;
  }
  int arr_length = json_object_array_length(jobj_value);
  return arr_length;
}
}
