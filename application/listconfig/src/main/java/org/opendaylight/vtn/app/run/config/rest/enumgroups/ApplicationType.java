/*
 * Copyright (c) 2014, 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.app.run.config.rest.enumgroups;

/**
 * ApplicationType - enum which helps to set the content type of the request.
 *
 */
public enum ApplicationType {
    /**
     * Content type formats either String /json format.
     */
    STRING("application/string"), JSON("application/json");

    /**
     * ApplicationType - parameterized constructor.
     * @param type {@link String}
     */
    private ApplicationType(String type) {
        this.type = type;
    }

    /**
     * type which used to set the contenttype.
     */
    private String type = null;

    /**
     * getType - to get the content type.
     * @return {@link String}
     */
    public String getType() {
        return type;
    }
}
