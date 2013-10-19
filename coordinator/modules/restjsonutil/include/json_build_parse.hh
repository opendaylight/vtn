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

#ifndef RESTJSON_JSON_BUILD_PARSE_H_
#define RESTJSON_JSON_BUILD_PARSE_H_

#include <json/json.h>
#include <json_type.hh>
#include <rest_common_defs.hh>
#include <string>

namespace unc {
namespace restjson {

class JsonBuildParse {
 public:
  /**
   * @brief                   -Creates json object
   * @param[out] json_object* - json_object pointer
   */
  static json_object* create_json_obj();

  /**
   * @brief                    - Template method which build the json request
   * @param[out] json_object*  - json object pointer to which the data should
   *                             be added
   * @param[in] key            - string key to be added to the jsonobject
   * @param[in] T data         - template type which is the value to be added
   *                             for the specific key. Accepts type int,
   *                             string, bool, json_object
   * @return                   - returns 0 on SUCCESS / 1 on FAILURE
   */
  template<typename T>
static int build(json_object* jparent, const std::string &key, T data);

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
   * return int                 - returns 0 on SUCCESS / 1 on FAILURE
   */
  template<typename T>
static int parse(json_object* jobj, const std::string &key,
                 int arrindex, T &val);

  /**
   * @brief                   - Gets the type of the value for the specified key
   * @param[in] json_object   - json object from which the val to be retieved
   * @param[in] key           - value retrieved from the specified key
   * return int               - returns enum value json type
   */
  static int get_type(json_object* jobj, const std::string &key);

  /**
   * GetArrayLength           - the length of the array
   * @param[in] json_object   - json object from which the val to be retieved
   * @param[in] key           - value retrieved from the specified key
   * @return int              - the length of the array
   */
  static int get_array_length(json_object* jobj, const std::string &key);

  /**
   * @brief                      - Converts th ejson object to json string
   * @param[in]  json_object*    - which needs to converted to json string
   * return const char*          - converted from json object
   */
  static const char* json_obj_to_json_string(json_object* jobj);

  /**
   * @brief                      - Converts string to json object
   * @param[in]  data            - char pointer which needs to be converted to
   *                               json obj
   * @param[out] json_object*    - returns converted json object
   */
  static json_object* string_to_json_object(char *data);
};

template<typename T>
int JsonBuildParse::build(json_object* jparent, const std::string &key,
                          T data) {
  if (json_object_is_type(jparent, json_type_null)) {
    return FAILURE;
  }

  if (json_object_is_type(jparent, json_type_object)) {
    json_object* jobj;
    JsonType jsonutil_obj(data);
    jobj = jsonutil_obj.get_json_data();
    if (json_object_is_type(jobj, json_type_null)) {
      return FAILURE;
    }
    json_object_object_add(jparent, key.c_str(), jobj);
    return SUCCESS;
  }
  return FAILURE;
}

template<typename T>
int JsonBuildParse::parse(json_object* jobj, const std::string &key,
                          int arrindex, T &val) {
  if (json_object_is_type(jobj, json_type_null)) {
    return FAILURE;
  }

  json_object * jobj_getval;
  if (-1 != arrindex) {
    json_object *jobj_array = json_object_array_get_idx(jobj, arrindex);
    jobj_getval = json_object_object_get(jobj_array, key.c_str());
  } else {
    jobj_getval = json_object_object_get(jobj, key.c_str());
  }
  if (json_object_is_type(jobj_getval, json_type_null)) {
    return SUCCESS;
  }

  JsonType::get_value(jobj_getval, val);
  return SUCCESS;
}
}  //  namespace restjson
}  //  namespace unc
#endif  // RESTJSON_JSON_BUILD_PARSE_H_
