/**
 * Copyright (c) 2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package com.vtn.app.run.config.rest.parser;

import java.net.ConnectException;
import java.util.Map;
import java.util.Map.Entry;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import org.json.JSONException;
import org.json.JSONObject;

import com.sun.jersey.api.client.Client;
import com.sun.jersey.api.client.ClientResponse;
import com.sun.jersey.api.client.ClientHandlerException;
import com.sun.jersey.api.client.WebResource;
import com.sun.jersey.api.client.WebResource.Builder;
import com.sun.jersey.api.client.filter.HTTPBasicAuthFilter;
import com.vtn.app.run.config.rest.client.CRUDOperation;
import com.vtn.app.run.config.rest.client.VTNClientException;
import com.vtn.app.run.config.rest.enumgroups.APPLICATION_TYPE;
import com.vtn.app.run.config.rest.enumgroups.HTTP_RESPONSE;

public class CRUDImpl implements CRUDOperation {

    /**
     * Logger instance.
     */
    static final Logger LOG = LoggerFactory.getLogger(CRUDImpl.class);

    private Client client = null;

    public static final String HTTP_STATUS_DESCR  = "response_description";
    public static final String HTTP_STATUS  = "response_status";
    private APPLICATION_TYPE appType = null;

    /**
     * CRUDImpl - constructor with arguments.
     * @param appType
     *         APPLICATION_TYPE
     */
    public CRUDImpl(APPLICATION_TYPE appType) {
        this.appType = appType;
        this.client = Client.create();
        this.client.addFilter(new HTTPBasicAuthFilter("admin", "admin"));
    }

    /**
     * Get the Jersey client
     * @return
     *         Client instance
     */
    private Client getClient() {
        return this.client;
    }

    /**
     * Create the web resource with the URL
     * @param url
     *         URL
     * @param headers
     *         Headers
     * @return
     *         Web resource instance
     */
    protected Builder getWebResource(String url, Map<String, String> queryParams, Map<String, Object> headers) {
        WebResource webResource = getClient().resource(url);

        /*
         * Set query parameters
         */
        if (queryParams != null) {
            for (Entry<String, String> entry: queryParams.entrySet()) {
                webResource.queryParam(entry.getKey(), entry.getValue());
            }
        }

        /*
         * Set request headers
         */
        Builder builder = null;
        if (headers != null) {
            for (Entry<String, Object> entry: headers.entrySet()) {
                if (builder == null) {
                    builder = webResource.header(entry.getKey(), entry.getValue());
                } else {
                    builder = builder.header(entry.getKey(), entry.getValue());
                }
            }
        }
        return (builder != null ? builder : webResource.getRequestBuilder());
    }

    /**
     * To create a JSON object for a particular HTTP response
     * @param response
     *         HTTP_RESPONSE
     * @return
     *         JSONObject
     * @throws JSONException
     */
    private JSONObject getHttpResponseObject(HTTP_RESPONSE response) throws JSONException {
        JSONObject object = new JSONObject();
        object.put(HTTP_STATUS_DESCR, response.getDescription());
        object.put(HTTP_STATUS, response.getStatus());
        return object;
    }

    /**
     * Send a GET request
     * @param url
     *         URL
     * @return
     *         Response as String
     * @throws RequestFailureException
     *         Throws RequestFailureException if request does not return 'success'
     * @throws VTNClientException, JSONException, ConnectException
     */
    public String doGET(String url, Map<String, Object> headers) throws VTNClientException, JSONException, ConnectException {
        try {
            ClientResponse response = getWebResource(url, null, headers).accept(appType.getType()).get(ClientResponse.class);
            if (response.getStatus() == HTTP_RESPONSE.NO_CONTENT.getStatus()) {
                return getHttpResponseObject(HTTP_RESPONSE.NO_CONTENT).toString();
            } else {
                if (response.getStatus() == HTTP_RESPONSE.UNAUTHORIZED.getStatus()) {
                    return getHttpResponseObject(HTTP_RESPONSE.UNAUTHORIZED).toString();
                } else if (response.getStatus() != HTTP_RESPONSE.OPERATION_SUCCESS.getStatus()) {
                    LOG.error("GET RequestFailed :HTTP error code : {}", response.getStatus());
                    throw new VTNClientException("Page not found - " + response.getStatus());
                }
                return response.getEntity(String.class);
            }
        } catch (ClientHandlerException e) {
            LOG.error("An exception occured - {}", e);
            throw new VTNClientException("Failed to connect to ODL Controller...\nPlease check IP addrees/Port number...");
        }
    }

    /**
     * Send a POST request
     * @param url
     *         URL
     * @param obj
     *         Object to send along POST request
     * @param heders
     *         Headers
     * @return
     *         Response as String
     * @throws RequestFailureException
     *         Throws RequestFailureException if request does not return 'success'
     * @throws JSONException
     */
    public String doPOST(String url, Object obj, Map<String, Object> headers) throws VTNClientException, ConnectException, JSONException {
        return "";
    }

    /**
     * Send a PUT request
     * @param url
     *         URL
     * @param obj
     *         Object to send along PUT request
     * @param heders
     *         Headers
     * @return
     *         Response as String
     * @throws RequestFailureException
     *         Throws RequestFailureException if request does not return 'success'
     * @throws JSONException
     */
    public String doPUT(String url, Object obj, Map<String, Object> headers) throws VTNClientException, ConnectException, JSONException {
        return "";
    }

    /**
     * Send a DELETE request
     * @param url
     *         URL
     * @param obj
     *         Object to send along DELETE request
     * @param heders
     *         Headers
     * @return
     *         Response as String
     * @throws RequestFailureException
     *         Throws RequestFailureException if request does not return 'success'
     * @throws JSONException
     */
    public String doDELETE(String url, Object obj, Map<String, Object> headers) throws VTNClientException, ConnectException, JSONException {
        return "";
    }
}
