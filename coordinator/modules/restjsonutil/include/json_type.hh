/*
 * copyright (c) 2012-2013 nec corporation
 * all rights reserved.
 *
 * this program and the accompanying materials are made
 * available under the
 * terms of the eclipse public license v1.0 which
 * accompanies this
 * distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef restjson_json_type_h_
#define restjson_json_type_h_

#include <json/json.h>
#include <string>

namespace unc {
namespace restjson {
class JsonType {
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
  explicit JsonType(std::string strdata) {
    jobj_ = json_object_new_string(strdata.c_str());
  }

  /**
   * @brief               - parametrised constructor takes arg as int
   *                      - creates json object of type int
   * @param[in] intdata   -  integer value
   */
  explicit JsonType(int intdata) {
    jobj_ = json_object_new_int(intdata);
  }

  /**
   * @brief               - Parametrised constructor takes arg as json object
   *                      - Creates json object of type json object
   * @param[in] jsondata  - json object
   */
  explicit JsonType(json_object* jsondata) {
    jobj_ = jsondata;
  }

  /**
   * @brief                 - Function overloading getValue - returns the value
   *                          of type string
   * @param[in] jobjval     - json object from which the value to be retrieved
   * @param[out] val        - the value stored in this string reference
   * return                 - None
   */
  static void get_value(json_object* jobjval, std::string &val) {
    val = json_object_get_string(jobjval);
  }

  /**
   * @brief                  - Function overloading getValue - returns the
   *                           value of type int
   * @param[in] jobjval      - json object from which the value to be retrieved
   * @param[out] val         - the value stored in this int reference
   * return                  - None
   */
  static void get_value(json_object* jobjval, int &val) {
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
    jobjgetval = jobjval;
  }

  /**
   * @brief                   - gets the JsonData
   * @param[out] json_object* - json_object
   */
  json_object* get_json_data() {
    return jobj_;
  }
};
}  // namespace restjson
}  // namespace unc
#endif  // RESTJSON_JSON_TYPE_H_
