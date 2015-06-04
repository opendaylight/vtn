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
import java.util.Map;

import org.json.JSONException;
import org.json.JSONObject;
import org.opendaylight.vtn.app.run.config.rest.client.VTNClientException;
import org.opendaylight.vtn.app.run.config.rest.enumgroups.ApplicationType;
import org.opendaylight.vtn.app.run.config.rest.enumgroups.HttpResponse;
import org.opendaylight.vtn.app.run.config.rest.parser.CRUDImpl;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import com.sun.jersey.api.client.Client;
import com.sun.jersey.api.client.ClientHandlerException;
import com.sun.jersey.api.client.ClientResponse;
import com.sun.jersey.api.client.filter.HTTPBasicAuthFilter;

/**
 * CRUD implementation for the rest request.
 */
public class ExtendedCRUDImpl extends CRUDImpl {

    /**
     * Logger instance.
     */
    static final Logger LOG = LoggerFactory.getLogger(ExtendedCRUDImpl.class);


    private Client client = null;
    private ApplicationType appType = null;
    private String userName;
    private String password;


    /**
     * CRUDImpl - constructor with arguments.
     * @param appType - content type of the http response.
     * @param userName - username of the controller to connect to the server.
     * @param password - password of the controller to connect to the server.
     */
    public ExtendedCRUDImpl(ApplicationType appType, String userName,
            String password) {
        super(appType, userName, password);

        this.client = Client.create();
        this.appType = appType;
        this.userName = userName;
        this.password = password;
        this.client.addFilter(new HTTPBasicAuthFilter(userName, password));
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
     * To create a JSON object for a particular HTTP response
     * @param response
     *         HttpResponse
     * @return
     *         JSONObject
     * @throws JSONException
     */
    private JSONObject getHttpResponseObject(HttpResponse response) throws JSONException {
        JSONObject object = new JSONObject();
        object.put(HTTP_STATUS_DESCR, response.getDescription());
        object.put(HTTP_STATUS, response.getStatus());
        return object;
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
     * @throws VTNClientException
     * @throws JSONException
     */
    public String doPOST(String url, Object obj, Map<String, Object> headers) throws VTNClientException, ConnectException, JSONException {

        try {
            ClientResponse response = getWebResource(url, null, headers).type(ApplicationType.JSON.getType()).post(ClientResponse.class, obj.toString());

            if (response.getStatus() == HttpResponse.NO_CONTENT.getStatus()) {
                return getHttpResponseObject(HttpResponse.NO_CONTENT).toString();
            } else {
                if (response.getStatus() == HttpResponse.UNAUTHORIZED.getStatus()) {
                    LOG.error("Unauthorized access :HTTP error code : {}", response.getStatus());
                    LOG.error(getHttpResponseObject(HttpResponse.UNAUTHORIZED).toString());
                    throw new VTNClientException("\n\nFailed to connect to ODL Controller due to unauthorized access..."
                                                    + "\nPlease check Username/Password...");
                } else if (response.getStatus() != HttpResponse.CREATED.getStatus()) {
                    LOG.error("POST RequestFailed :HTTP error code : {}", response.getStatus());
                    throw new VTNClientException("\n\nPage not found - " + response.getStatus());
                }
                return response.getEntity(String.class);
            }
        } catch (ClientHandlerException e) {
            LOG.error("An exception occured in Rest Post request - ", e.getMessage());
            throw new VTNClientException("\n\nFailed to connect to ODL Controller..."
                                                    + "\nPlease check Controller is running or IP address/Port number is incorrect...");
        }
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

        try {
            ClientResponse response = getWebResource(url, null, headers).type(ApplicationType.JSON.getType()).put(ClientResponse.class, obj.toString());

            if (response.getStatus() == HttpResponse.NO_CONTENT.getStatus()) {
                return getHttpResponseObject(HttpResponse.NO_CONTENT).toString();
            } else {
                if (response.getStatus() == HttpResponse.UNAUTHORIZED.getStatus()) {
                    LOG.error("Unauthorized access :HTTP error code : {}", response.getStatus());
                    LOG.error(getHttpResponseObject(HttpResponse.UNAUTHORIZED).toString());
                    throw new VTNClientException("\n\nFailed to connect to ODL Controller due to unauthorized access..."
                                                    + "\nPlease check Username/Password...");
                } else if (response.getStatus() != HttpResponse.OPERATION_SUCCESS.getStatus()) {
                    LOG.error("PUT RequestFailed :HTTP error code : {}", response.getStatus());
                    throw new VTNClientException("\n\nPage not found - " + response.getStatus());
                }
                return response.getEntity(String.class);
            }
        } catch (ClientHandlerException e) {
            LOG.error("An exception occured in Rest Put request - ", e.getMessage());
            throw new VTNClientException("\n\nFailed to connect to ODL Controller..."
                                                    + "\nPlease check Controller is running or IP address/Port number is incorrect...");
        }
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

        try {
            ClientResponse response = getWebResource(url, null, headers).type(ApplicationType.JSON.getType()).delete(ClientResponse.class);

            if (response.getStatus() == HttpResponse.NO_CONTENT.getStatus()) {
                return getHttpResponseObject(HttpResponse.NO_CONTENT).toString();
            } else {
                if (response.getStatus() == HttpResponse.UNAUTHORIZED.getStatus()) {
                    LOG.error("Unauthorized access :HTTP error code : {}", response.getStatus());
                    LOG.error(getHttpResponseObject(HttpResponse.UNAUTHORIZED).toString());
                    throw new VTNClientException("\n\nFailed to connect to ODL Controller due to unauthorized access..."
                                                    + "\nPlease check Username/Password...");
                } else if (response.getStatus() != HttpResponse.OPERATION_SUCCESS.getStatus()) {
                    LOG.error("DELETE RequestFailed :HTTP error code : {}", response.getStatus());
                    throw new VTNClientException("\n\nPage not found - " + response.getStatus());
                }
                return response.getEntity(String.class);
            }
        } catch (ClientHandlerException e) {
            LOG.error("An exception occured in Rest Delete request - ", e.getMessage());
            throw new VTNClientException("\n\nFailed to connect to ODL Controller..."
                                                    + "\nPlease check Controller is running or IP address/Port number is incorrect...");
        }
    }
}

