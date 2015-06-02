/**
 * Copyright (c) 2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.app.run.config.rest.client;


import java.net.ConnectException;
import java.util.Map;

/**
 * CRUDOperation - Interface which provides all the HTTP methods to perform the
 *    create,read,update and delete operation.
 *
 */
public interface CRUDOperation {

    /**
     * Send a GET request.
     * @param url
     *         URL
     * @param headers
     *         Headers
     * @return
     *         Response as String.
     * @throws ConnectException
     */
    String doGET(String url, Map<String, Object> headers) throws Exception, ConnectException;

    /**
     * Send a POST request.
     * @param url
     *         URL
     * @param obj
     *         Object to send along POST request
     * @param headers
     *         Headers
     * @return
     *         Response as String
     * @throws ConnectException
     */
    String doPOST(String url, Object obj, Map<String, Object> headers) throws Exception, ConnectException;

    /**
     * Send PUT request.
     * @param url
     *         URL
     * @param obj
     *         Object to send along PUT request
     * @param headers
     *         Headers
     * @return
     *         Response as String
     * @throws ConnectException
     */
    String doPUT(String url, Object obj, Map<String, Object> headers) throws Exception, ConnectException;

    /**
      * Send a DELETE request.
     * @param url
     *         URL
     * @param obj
     *         Object to send along DELETE request
     * @param headers
     *         Headers
      * @return
     *         Response as String
     * @throws ConnectException
     */
    String doDELETE(String url, Object obj, Map<String, Object> headers) throws Exception, ConnectException;
}
