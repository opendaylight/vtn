/*
 * Copyright (c) 2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef RESTJSON_JSON_TYPE_H_
#define RESTJSON_JSON_TYPE_H_

#include <json/json.h>
#include <string>

namespace unc {
namespace restjson {
class JsonType {
 private:
  /*
   * json object
   */
  json_object* jobj_;

 public:
  explicit JsonType(std::string strdata) {
    jobj_ = json_object_new_string(strdata.c_str());
  }
  explicit JsonType(int intdata) {
    jobj_ = json_object_new_int(intdata);
  }
  explicit JsonType(json_object* jsondata) {
    jobj_ = jsondata;
  }

  static void get_value(json_object* jobjval, std::string &val) {
    val = json_object_get_string(jobjval);
  }
  static void get_value(json_object* jobjval, int &val) {
    val = json_object_get_int(jobjval);
  }
  static void get_value(json_object* jobjval, json_object*& jobjgetval) {
    jobjgetval = jobjval;
  }

  json_object* get_json_data() {
    return jobj_;
  }
};
}  // namespace restjson
}  // namespace unc
#endif  // RESTJSON_JSON_TYPE_H_
