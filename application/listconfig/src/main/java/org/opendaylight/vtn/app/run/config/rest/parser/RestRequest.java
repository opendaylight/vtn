/*
 * Copyright (c) 2014, 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.app.run.config.rest.parser;

import java.util.HashMap;
import java.util.Map;
import java.util.Map.Entry;

public class RestRequest {

    /**
     * Json Response Start Delimiter.
     */

    public static final String PARAM_DELIM_START = " {";

    /**
     * Json Response End Delimiter.
     */
    public static final String PARAM_DELIM_END = "}";

    /**
     * REST Service server URL address.
     */
    private String url = null;

    /**
     * Response string value received from the request.
     */
    private String response = null;

    /**
     * Map of Response parameters.
     */
    private Map<String, String> params = null;

    /**
     * Default contructor
     */
    public RestRequest() {
        params = new HashMap<String, String>();
    }

    /**
     * getUrl - to get the server URL.
     * @return {@link String}
     */
    public String getUrl() {
        return url;
    }

    /**
     * setUrl - to set the server URL.
     *  @param url
     */
    public void setUrl(String url) {
        this.url = url;
    }

    /**
     * getParams - to get the response parameter.
     * @return {@link String}
     */
    public Map<String, String> getParams() {
        return params;
    }

    /**
     * setParams - to set the response parameter.
     * @param params
     */
    public void setParams(Map<String, String> params) {
        this.params = params;
    }

    /**
     * getResponse - to get the response .
     * @return response
     */
    public String getResponse() {
        return response;
    }

    /**
     * setResponse - to set the response.
     * @param response
     */
    public void setResponse(String response) {
        this.response = response;
    }

    /**
     * setResponse - to set the URL parameters.
     *
     */
    public void setURLParams() {
        for (Entry<String, String> entry : params.entrySet()) {
            setURLParam(entry.getKey(), entry.getValue());
        }
    }

    /**
     * setResponse - to set the response.
     * @param paramName
     * @param paramValue
     */
    public void setURLParam(String paramName, String paramValue) {
        if (url.contains(PARAM_DELIM_START + paramName + PARAM_DELIM_END)) {
            url = url.replace(PARAM_DELIM_START + paramName + PARAM_DELIM_END,
                    paramValue);
        }
    }
}
