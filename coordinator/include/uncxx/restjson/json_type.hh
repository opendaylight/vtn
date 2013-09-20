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

#ifndef RESTJSON_JSON_TYPE_H_
#define RESTJSON_JSON_TYPE_H_

#include <json/json.h>
#include <string>

namespace librestjson {
class JsonType {
  private:
    /*
     * json object
     */
    json_object* jobj_;

  public:
    /*
     * @brief - Parametrised constructor takes arg as string
     *        - Creates json object of type string
     */
    explicit JsonType(std::string strdata) {
        jobj_ = json_object_new_string(strdata.c_str());
    }
    /*
     * @brief - Parametrised constructor takes arg as int
     *        - Creates json object of type int
     */
    explicit JsonType(int intdata) {
      jobj_ = json_object_new_int(intdata);
    }
    /*
     * @brief - Parametrised constructor takes arg as json object
     *        - Creates json object of type json object
     */
    explicit JsonType(json_object* jsondata) {
      jobj_ = jsondata;
    }

    /**
     * Function overloading getValue - returns the value of type string
     * @param[in] - jobjval - json object from which the value to be retrieved
     * @param[out] - val - the value stored in this string reference
     */
    static void GetValue(json_object* jobjval, std::string &val) {
      val = json_object_get_string(jobjval);
    }
    /**
     * Function overloading getValue - returns the value of type int
     * @param[in] - jobjval - json object from which the value to be retrieved
     * @param[out] - val - the value stored in this int reference
     */
    static void GetValue(json_object* jobjval, int &val) {
      val = json_object_get_int(jobjval);
    }
    /**
     * Function overloading getValue - returns the value of type json_object
     * @param[in] - jobjval - json object from which the value to be retrieved
     * @param[out] - val - the value stored in this json_object reference
     */
    static void GetValue(json_object* jobjval, json_object*& jobjgetval) {
      jobjgetval = jobjval;
    }

    /*
     * @brief - gets the JsonData
     * @param[out] - json_object
     */
    json_object* GetJsonData() {
      return jobj_;
    }
};
}
#endif  // RESTJSON_JSON_TYPE_H_
