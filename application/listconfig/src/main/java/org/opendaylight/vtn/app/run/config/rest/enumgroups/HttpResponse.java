/*
 * Copyright (c) 2014, 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.app.run.config.rest.enumgroups;

/**
 * HttpResponse - http response codes are maintained to return the
 * corresponding values.
 *
 */
public enum HttpResponse {

    /**
     * For Success operation.
     */
    SUCCESS(0, "Success"),

    /**
     * For HTTP request sucess operation.
     */
    OPERATION_SUCCESS(200, "No error, operation successful"),

    /**
     * for creating the resource in the server.
     */
    CREATED(201, "Successful creation of a resource"),

    /**
     * for the request received.
     */
    ACCEPTED(202, "The request was received"),

    /**
     * for no response body.
     */
    NO_CONTENT(204, "The request was processed successfully, but no response body is needed"),

    /**
     * for not authorized request.
     */
    UNAUTHORIZED(401, "Action requires user authentication");

    /**
     * HttpResponse  parameterized constructor.
     * @param status {@link Integer}
     * @param description {@link String}
     */
    private HttpResponse(int status, String description) {
        this.status = status;
        this.description = description;
    }

    /**
     * Status code.
     */
    private int status = 0;

    /**
     * status description.
     */
    private String description = null;

    /**
     * get the status of the operation.
     * @return  The status of the operation.
     */
    public int getStatus() {
        return status;
    }

    /**
     * get description of the operation.
     * @return  Description of the operation
     */
    public String getDescription() {
        return description;
    }

    /**
     * HTTP_getResponse - function to get the response of the request.
     * @param status
     * @return  The response of the request.
     */
    public HttpResponse getResponse(int status) {
        for (HttpResponse resp :values()) {
            if (resp.getStatus() == status) {
                return resp;
            }
        }
        return null;
    }
}
