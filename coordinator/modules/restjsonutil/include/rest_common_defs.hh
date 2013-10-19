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

#ifndef RESTJSON_REST_COMMON_DEFS_H_
#define RESTJSON_REST_COMMON_DEFS_H_

#include <string>

#define kContentTypeJson "Content-type: application/json"
#define kAcceptTypeJson "Accept: application/json"

namespace unc {
namespace restjson {

const char kProtocol[] = "http://";
const char colonSeparator[] = ":";
const size_t kSizeNull = 0;
const int connection_timeout = 100;
const int req_timeout = 100;

typedef enum {
  HTTP_METHOD_POST = 1, HTTP_METHOD_PUT, HTTP_METHOD_DELETE, HTTP_METHOD_GET,
  HTTP_METHOD_UNKNOWN
} HttpMethod;

typedef struct {
  char *memory;
  int size;
} HttpContent_t;

typedef struct {
  int code;
  HttpContent_t *write_data;
} HttpResponse_t;

typedef enum {
  SUCCESS = 0, FAILURE
} rest_resp_code_t;

typedef enum {
  RESP_BAD_REQUEST = 400,
  RESP_UN_AUTHORISED = 401,
  RESP_NOT_FOUND = 404,
  RESP_NOT_ACCEPTABLE = 406,
  RESP_CONFLICT = 409,
  RESP_UN_SUPPORTED_MEDIA_TYPE = 415,
  RESP_INTERNAL_SERVER_ERROR = 500,
  RESP_SERVICE_UNAVAILABLE = 503,
  RESP_CREATED = 201,
  RESP_OK = 200,
  RESP_UNKNOWN = 1
} ServerResponseCode;
}  // namespace restjson
}  // namespace unc
#endif  // RESTJSON_REST_COMMON_DEFS_H_
