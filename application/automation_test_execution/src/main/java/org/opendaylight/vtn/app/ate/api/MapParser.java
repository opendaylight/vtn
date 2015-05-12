/**
 * Copyright (c) 2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.app.ate.api;

import java.util.Map;

/**
 * Map-Key, value mapping to Object.
 */
public interface MapParser {

    /**
     * Input list identifier key from YML file.
     */
    String INPUT_LIST_KEY = "Inputs";

    /**
     * Input identifier key from YML file.
     */
    String INPUT_KEY = "Input";

    /**
     * Verification list identifier key from YML file.
     */
    String VERIFICATION_LIST_KEY = "Outputs";

    /**
     * Verification identifier key from YML file.
     */
    String VERIFICATION_KEY = "Output";

    /**
     * Post command identifier key from YML file.
     */
    String POST_COMMAND = "isPost";

    /**
     * Container name identifier key from YML file.
     */
    String CONTAINER_NAME = "containerName";

    /**
     * Get input elements from map values(Read from YML file).
     */
    Object getInputDatas(Map<String, Object> map);

    /**
     * Get verifying elements from map values(Read from YML file).
     */
    Object getVerifyingDatas(Map<String, Object> map);

    /**
     * Parse map values(Read from YML file) to objects for Controller.
     */
    Object setAnnotatedFields(Map<String, Object> map, Object requestObject);
}
