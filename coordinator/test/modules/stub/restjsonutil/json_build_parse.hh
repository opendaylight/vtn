/*
 * Copyright (c) 2013-2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef RESTJSON_JSON_BUILD_PARSE_H_
#define RESTJSON_JSON_BUILD_PARSE_H_

#include <json_type_util.hh>
#include <rest_common_defs.hh>
#include <pfc/log.h>
#include <uncxx/odc_log.hh>
#include <string>

namespace unc {
namespace restjson {

class JsonBuildParse {
 public:
  /**
   * @brief                   -Creates json object
   * @param[out] json_object* - json_object pointer
   */
  static json_object* create_json_obj() {
    return json_object_new_object();
  }

  static json_object* create_json_array_obj() {
    return json_object_new_array();
  }

  static void add_to_array (json_object *array,
                            json_object* value) {
      json_object_array_add (array,value);
  }

  static json_object* create_json_int_obj(uint32_t val) {
       ODC_FUNC_TRACE;
       return json_object_new_int (val);
  }

  /**
   * @brief                    - Template method which build the json request
   * @param[in] key            - string key to be added to the jsonobject
   * @param[in] T data         - template type which is the value to be added
   *                             for the specific key. Accepts type int,
   *                             string, bool, json_object
   * @param[out] json_object*  - json object pointer to which the data should
   *                             be added
   * @return                   - returns 0 on REST_OP_SUCCESS / 1 on REST_OP_FAILURE
   */
  template<typename T>
  static int build(const std::string &key, T data, json_object* jparent);

  /**
   * @brief                     - Template method which parse the json object
   * @param[in] json_object*    - json object pointer from which the data
   *                              should be retrieved
   * @param[in] key             - string key to be parsed from the jsonobject
   * @param[in] arrindex        - if the value to be parsed inside array, give
   *                              the array index otherwise -1
   * @param[out] T data         - template type which is the value to be added
   *                              for the specific key accepts type int,
   *                              string, bool, json_object
   * return int                 - returns 0 on REST_OP_SUCCESS / 1 on REST_OP_FAILURE
   */
  template<typename T>
  static int parse(json_object* jobj, const std::string &key,
                 int arrindex, T &val);

  template<typename T>
  static int parse(json_object* jobj, const std::string &key, T &val);

  /**
   * GetArrayLength           - the length of the array
   * @param[in] json_object   - json object from which the val to be retieved
   * @param[in] key           - value retrieved from the specified key
   * @return int              - the length of the array
   */
  static int get_array_length(json_object* jobj, const std::string &key) {
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

  static int get_array_length(json_object* jobj){

    // If the json object is not of type array return ZERO as longth
  if (!(json_object_is_type(jobj, json_type_array))) {
    pfc_log_error("json object is not of type array");
    return ZERO_ARRAY_LENGTH;
  }
  int arr_length = json_object_array_length(jobj);
  pfc_log_debug("%d array length", arr_length);
  return arr_length;
  }

  /**
   * @brief                      - Converts the json object to json string
   * @param[in]  json_object*    - which needs to converted to json string
   * return const char*          - converted from json object
   */
  static const char* get_json_string(json_object* jobj){
    return json_object_to_json_string(jobj);
  }

  /**
   * @brief                      - Converts string to json object
   * @param[in]  data            - char pointer which needs to be converted to
   *                               json obj
   * @return json_object*        - returns converted json object
   */
  static json_object* get_json_object(char *data){
    return json_tokener_parse(data);
  }
};

// Build the json request with the key and the template data
template<typename T>
int JsonBuildParse::build(const std::string &key,
                          T data, json_object* jparent) {
  ODC_FUNC_TRACE;

  // Check the jparent is null
  if (json_object_is_type(jparent, json_type_null)) {
    pfc_log_error("json object is NULL ... ");
    return REST_OP_FAILURE;
  }

  // If japarent is of type object then procees else return FAILURE
  if (json_object_is_type(jparent, json_type_object)) {
    json_object* jobj = NULL;
    JsonTypeUtil jsonutil_obj(data);
    jobj = jsonutil_obj.get_json_data();
    if (json_object_is_type(jobj, json_type_null)) {
      pfc_log_error("json object for value is NULL");
      return REST_OP_FAILURE;
    }
    // Adds the data to the json object with key
    json_object_object_add(jparent, key.c_str(), jobj);
    return REST_OP_SUCCESS;
  }
  pfc_log_error("jparent is not of the type json_object");
  return REST_OP_FAILURE;
}

// Parse the json object with the given key and gets the value and assign it to
// template val instance
template<typename T>
int JsonBuildParse::parse(json_object* jobj, const std::string &key,
                          int arrindex, T &val) {
  ODC_FUNC_TRACE;
  if (json_object_is_type(jobj, json_type_null)) {
    pfc_log_error("json object is NULL ... ");
    return REST_OP_FAILURE;
  }

  // arrindex != -1 means json_object is of type array
  json_object *o;
  if (-1 != arrindex) {
    o = json_object_array_get_idx(jobj, arrindex);
    if (json_object_is_type(o, json_type_null)) {
      pfc_log_error("json array object is NULL ... ");
      return REST_OP_FAILURE;
    }
  } else {
    o = jobj;
  }

  // Checks the json object value is not null
  json_object *jobj_getval(NULL);
  if (json_object_object_get_ex(o, key.c_str(), &jobj_getval) &&
      !json_object_is_type(jobj_getval, json_type_null)) {
    JsonTypeUtil::get_value(jobj_getval, val);
  }
  // If json object is NULL , get_value is not called return REST_OP_SUCCESS
  return REST_OP_SUCCESS;
}


// Parse the json object with the given key and gets the value and assign it to
// template val instance
template<typename T>
int JsonBuildParse::parse(json_object* jobj,
                          const std::string &key,
                          T &val) {
  return parse(jobj, key, -1, val);
}


}  //  namespace restjson
}  //  namespace unc
#endif  // RESTJSON_JSON_BUILD_PARSE_H_
