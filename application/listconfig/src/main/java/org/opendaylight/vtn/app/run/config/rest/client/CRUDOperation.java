/*
 * Copyright (c) 2014, 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.app.run.config.rest.client;


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
     * @throws VTNClientException
     */
    String doGET(String url, Map<String, Object> headers)
        throws VTNClientException;

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
     * @throws VTNClientException
     */
    String doPOST(String url, Object obj, Map<String, Object> headers)
        throws VTNClientException;

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
     * @throws VTNClientException
     */
    String doPUT(String url, Object obj, Map<String, Object> headers)
        throws VTNClientException;

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
     * @throws VTNClientException
     */
    String doDELETE(String url, Object obj, Map<String, Object> headers)
        throws VTNClientException;
}
