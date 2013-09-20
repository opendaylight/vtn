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
#include <uncxx/restjson/json_type.hh>
#include <uncxx/restjson/rest_common_defs.hh>
#include <string>

namespace librestjson {

class JsonBuildParse {
  public:
    /*
     * @brief Creates json object
     * @param[out] - retval - json_object pointer
     */
    static json_object* CreateJsonObj();

    /**
     * @brief - Template method which build the json request
     * @param[out] - json_object* - json object pointer to which the data should be added
     * @param[in] - const std::string &key - string key to be added to the jsonobject
     * @param[in] T data - template type which is the value to be added for the specific key
     *         - accepts type int, string, bool, json_object
     */
    template<typename T>
    static int Build(json_object* jparent, const std::string &key, T data);

    /**
     * @brief - Template method which parse the json object
     * @param[in] -json_object* - json object pointer from which the data should be retrieved
     * @param[in] - const std::string &key - string key to be parsed from the jsonobject
     * @param[in] - const int arrindex - if the value to be parsed inside array, give the array index,
     *                     - otherwise -1
     * @param[out] - T data - template type which is the value to be added for the specific key
     *         - accepts type int, string, bool, json_object
     * @param[out] - int - SUCCESS - 0 , FAILURE - 1
     */
    template<typename T>
    static int Parse(json_object* jobj, const std::string &key,
                                         int arrindex, T &val);

    /**
     * @Gets the type of the value for the specified key
     * @param[in] json_object - json object from which the val to be retieved
     * @param[in] - key - value retrieved from the specified key
     * @param[out] - int enum value json type
     */
    static int GetType(json_object* jobj, const std::string &key);

    /**
     * GetArrayLength - the length of the array
     * @param[in] json_object - json object from which the val to be retieved
     * @param[in] - key - value retrieved from the specified key
     * @param[out] - the length of the array
     */
    static int GetArrayLength(json_object* jobj, const std::string &key);

    /*
     * Converts th ejson object to json string
     * @param[in] - json_object* which needs to converted to json string
     * @param[out] - const char* - converted from json object
     */
    static const char* JsonObjToJsonString(json_object* jobj);
};

template<typename T>
int JsonBuildParse::Build(json_object* jparent, const std::string &key,
                           T data) {
  if (json_object_is_type(jparent, json_type_null)) {
    return FAILURE;
  }

  if (json_object_is_type(jparent, json_type_object)) {
    json_object* jobj;
    JsonType jsonutil_obj(data);
    jobj = jsonutil_obj.GetJsonData();
    if (json_object_is_type(jobj, json_type_null)) {
      return FAILURE;
    }
    json_object_object_add(jparent, key.c_str(), jobj);
    return SUCCESS;
  }
  return FAILURE;
}

template<typename T>
int JsonBuildParse::Parse(json_object* jobj, const std::string &key,
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
    return FAILURE;
  }
  JsonType::GetValue(jobj_getval, val);
  return SUCCESS;
}
}
#endif  // RESTJSON_JSON_BUILD_PARSE_H_
