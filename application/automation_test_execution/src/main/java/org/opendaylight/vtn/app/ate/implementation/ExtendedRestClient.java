/**
 * Copyright (c) 2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.app.ate.implementation;

import java.net.ConnectException;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;
import org.opendaylight.vtn.app.run.config.rest.client.CRUDOperation;
import org.opendaylight.vtn.app.run.config.rest.client.RESTClient;
import org.opendaylight.vtn.app.run.config.rest.client.VTNClientException;
import org.opendaylight.vtn.app.run.config.rest.enumgroups.ApplicationType;
import org.opendaylight.vtn.app.run.config.rest.parser.Parser;
import org.opendaylight.vtn.app.run.config.rest.parser.RestRequest;
import org.opendaylight.vtn.app.ate.json.annotations.JsonRestStatus;
import org.opendaylight.vtn.app.ate.json.annotations.RestStatus;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

/**
 * Rest client side implementation.
 */
public class ExtendedRestClient extends RESTClient {

    /**
     * Logger instance.
     */
    static final Logger LOG = LoggerFactory.getLogger(ExtendedRestClient.class);

    /**
     * CRUDOperation interface reference.
     */
    private CRUDOperation client = null;

    /**
     * Json Parser class Parser reference.
     */
    private ExtendedJsonParser parser = null;

    public ExtendedRestClient(String serverIp, String serverPort,
            String userName, String password) throws VTNClientException {
        super(serverIp, serverPort, userName, password);
        client = new ExtendedCRUDImpl(ApplicationType.JSON, userName, password);
        parser = new ExtendedJsonParser();
    }

    public RestStatus post(Object restObject, Map<String, String> paramMap,
            Map<String, Object> headers) throws IllegalArgumentException,
            JSONException, IllegalAccessException, InstantiationException,
            Exception, ConnectException {
        RestRequest request = getRestRequest(restObject, paramMap);
        String jsonString = parseRequestParams(restObject).toString();
        if (request != null) {
            if (jsonString.startsWith("[")) {
                /*
                 * TODO :Implement the required base interfaces if POST request
                 * accepts JSONArray type
                 */
                /*
                 * JSONArray array = new JSONArray(jsonString);
                 * request.setResponse(client.doPOST(request.getUrl(), array));
                 */
            } else {
                JSONObject json = new JSONObject(jsonString);
                request.setResponse(client.doPOST(request.getUrl(), json,
                        headers));
            }

        }
        return (RestStatus)parseResponse(new JsonRestStatus(), request);
    }

    public Object post(Object restObject, Object responseObj,
            Map<String, Object> headers) throws IllegalArgumentException,
            JSONException, IllegalAccessException, InstantiationException,
            Exception, ConnectException {
        Map<String, String> paramMap = new HashMap<String, String>();
        return this.post(restObject, responseObj, paramMap, headers);
    }

    public Object post(Object restObject, Object responseObj,
            Map<String, String> paramMap, Map<String, Object> headers) throws
            IllegalArgumentException, JSONException,
            IllegalAccessException, InstantiationException,
            Exception, ConnectException {
        RestRequest request = getRestRequest(restObject, paramMap);
        String jsonString = parseRequestParams(restObject).toString();
        if (request != null) {
            if (jsonString.startsWith("[")) {
                /*
                 * TODO :Implement the required base interfaces if POST request
                 * accepts JSONArray type
                 */
                /*
                 * JSONArray array = new JSONArray(jsonString);
                 * request.setResponse(client.doPOST(request.getUrl(), array));
                 */
            } else {
                JSONObject json = new JSONObject(jsonString);
                request.setResponse(client.doPOST(request.getUrl(), json,
                        headers));
            }

        }
        LOG.info("Post Response - {} ", request.getResponse());
        return parseResponse(responseObj, request);
    }

    public RestStatus put(Object restObject, Map<String, String> paramMap,
            Map<String, Object> headers) throws IllegalArgumentException,
            JSONException, IllegalAccessException, InstantiationException,
            Exception, ConnectException {
        RestRequest request = getRestRequest(restObject, paramMap);
        String jsonString = parseRequestParams(restObject).toString();
        if (request != null) {
            if (jsonString.startsWith("[")) {
                /*
                 * TODO :Implement the required base interfaces if POST request
                 * accepts JSONArray type
                 */
                /*
                 * JSONArray array = new JSONArray(jsonString);
                 * request.setResponse(client.doPOST(request.getUrl(), array));
                 */
            } else {
                JSONObject json = new JSONObject(jsonString);
                request.setResponse(client.doPUT(request.getUrl(), json,
                        headers));
            }

        }
        return (RestStatus)parseResponse(new JsonRestStatus(), request);
    }

    public RestStatus delete(Object restObject, Map<String, String> paramMap,
            Map<String, Object> headers) throws IllegalArgumentException,
            JSONException, IllegalAccessException, InstantiationException,
            Exception, ConnectException {
        RestRequest request = getRestRequest(restObject, paramMap);
        String jsonString = parseRequestParams(restObject).toString();
        if (request != null) {
            if (jsonString.startsWith("[")) {
                /*
                 * TODO :Implement the required base interfaces if POST request
                 * accepts JSONArray type
                 */
                /*
                 * JSONArray array = new JSONArray(jsonString);
                 * request.setResponse(client.doPOST(request.getUrl(), array));
                 */
            } else {
                JSONObject json = new JSONObject(jsonString);
                request.setResponse(client.doDELETE(request.getUrl(), json,
                        headers));
            }

        }
        return (RestStatus)parseResponse(new JsonRestStatus(), request);
    }

    private Object parseRequestParams(Object source) throws
        IllegalArgumentException, IllegalAccessException,
        JSONException, InstantiationException {
        if (source instanceof List) {
            return parser.parseArray((List<?>)source).toString();
        } else {
            return parser.parseObject(source).toString();
        }
    }

    /**
     * parseResponse - parse response which received from the get opertaion.
     * @param target
     * @param request
     * @return
     * @throws JSONException
     * @throws IllegalArgumentException
     * @throws IllegalAccessException
     * @throws InstantiationException
     */
    private Object parseResponse(Object target, RestRequest request)
        throws JSONException, IllegalArgumentException,
            IllegalAccessException, InstantiationException, VTNClientException {

        if (request.getResponse() != null && !request.getResponse().equals("")
                && !request.getResponse().equals("null")) {
            LOG.debug(request.getResponse());
            if (request.getResponse().startsWith("[")) {
                JSONArray jsonArray = new JSONArray(request.getResponse());
                return parser.parseJsonArray(target.getClass(), jsonArray);
            } else {
                String str = request.getResponse();
                JSONObject jsonObject = new JSONObject(str);
                Parser parse = new Parser();
                return parse.parseJsonObject(target, jsonObject);
            }
        } else {
            return target;
        }
    }
}

