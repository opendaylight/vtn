/*
 * Copyright (c) 2013-2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef JSON_TYPE_UTIL_H_
#define JSON_TYPE_UTIL_H_

#include <json.h>
#include <uncxx/odc_log.hh>
#include <string>

namespace unc {
namespace restjson {
class JsonTypeUtil {
 private:
  /**
   * json object
   */
  json_object* jobj_;

 public:
  /**
   * @brief             - parametrised constructor takes arg as string
   *                    - creates json object of type string
   * @param[in] strdata - string value
   */
  explicit JsonTypeUtil(std::string strdata) {
    ODC_FUNC_TRACE;
    jobj_ = json_object_new_string(strdata.c_str());
  }

  /**
   * @brief               - parametrised constructor takes arg as unsigned int
   *                        and  creates json object of type unsigned int
   * @param[in] uintdata  - unsigned integer value
   */
  explicit JsonTypeUtil(uint uintdata) {
    ODC_FUNC_TRACE;
    jobj_ = json_object_new_int(uintdata);
  }

  /**
   * @brief               - parametrised constructor takes arg as int
   *                        and  creates json object of type int
   * @param[in] intdata  - integer value
   */
  explicit JsonTypeUtil(int intdata) {
    ODC_FUNC_TRACE;
    jobj_ = json_object_new_int(intdata);
  }

  /**
   * @brief               - Parametrised constructor takes arg as json object
   *                      - Creates json object of type json object
   * @param[in] jsondata  - json object
   */
  explicit JsonTypeUtil(json_object* jsondata) {
    ODC_FUNC_TRACE;
    jobj_ = jsondata;
  }
  /**
   * @brief               - Parametrised constructor takes arg as json object
   *                      - Creates json object of type json object
   * @param[in] jsondata  - json object
   */

  explicit JsonTypeUtil(unsigned long long longdata) {
    ODC_FUNC_TRACE;
    jobj_ = json_object_new_int64(longdata);
  }

  explicit JsonTypeUtil(bool value) {
    ODC_FUNC_TRACE;
    jobj_ = json_object_new_boolean(value);
  }
  /**
   * @brief                 - Function overloading getValue - returns the value
   *                          of type string
   * @param[in] jobjval     - json object from which the value to be retrieved
   * @param[out] val        - the value stored in this string reference
   * return                 - None
   */
  static void get_value(json_object* jobjval, bool &val) {
    ODC_FUNC_TRACE;
    val = json_object_get_boolean(jobjval);
  }
  static void get_value(json_object* jobjval, std::string &val) {
    ODC_FUNC_TRACE;
    val = json_object_get_string(jobjval);
  }

  /**
   * @brief                  - Function overloading getValue - returns the
   *                           value of type unsigned int
   * @param[in] jobjval      - json object from which the value to be retrieved
   * @param[out] val         - the value stored in this unsigned int reference
   * return                  - None
   */
  static void get_value(json_object* jobjval, uint &val) {
    ODC_FUNC_TRACE;
    val = json_object_get_int(jobjval);
  }

  /**
   * @brief                  - Function overloading getValue - returns the
   *                           value of type int
   * @param[in] jobjval      - json object from which the value to be retrieved
   * @param[out] val         - the value stored in this int reference
   * return                  - None
   */
  static void get_value(json_object* jobjval, int &val) {
    ODC_FUNC_TRACE;
    val = json_object_get_int(jobjval);
  }

  /**
   * @brief                   - Function overloading getValue - returns the
   *                            value of type json_object
   * @param[in] jobjval       - json obj from which the value to be retrive
   * @param[out] val          - the value stored in this json_object reference
   * return                   - None
   */

  static void get_value(json_object* jobjval, json_object*& jobjgetval) {
    ODC_FUNC_TRACE;
    jobjgetval = jobjval;
  }

  /**
   * @brief                   - Function overloading getValue - returns the
   *                            value of type unsigned long long
   * @param[in] jobjval       - json obj from which the value to be retrive
   * @param[out] val          - the value stored in this json_object reference
   * return                   - None
   */

  static void get_value(json_object* jobjval, unsigned long long& val) {
    ODC_FUNC_TRACE;
    val = json_object_get_int64(jobjval);
  }

  /**
   * @brief                   - gets the JsonData
   * @return     json_object* - json_object
   */
  json_object* get_json_data() {
    ODC_FUNC_TRACE;
    return jobj_;
  }
};
}  // namespace restjson
}  // namespace unc
#endif  // RESTJSON_JSON_TYPE_H_
