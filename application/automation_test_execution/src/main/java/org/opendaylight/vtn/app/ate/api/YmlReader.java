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
 * YML data reader from .yml file.
 */
public interface YmlReader {

    /**
     * Indentation incrementor.
     */
    int INDENTATION_INCREMENTOR = 1;

    /**
     * Indentation width.
     */
    int INDENTATION_WIDTH = 2;

    /**
     * Key Index.
     */
    int KEY_INDEX = 0;

    /**
     * Value Index.
     */
    int VALUE_INDEX = 1;

    /**
     * Key-Value pair count.
     */
    int KEY_VALUE_PAIR_COUNT = 2;

    /**
     * Key-Value pair separator.
     */
    String KEY_VALUE_PAIR_SEPERATOR = ":";

    /**
     * List identifier.
     */
    String LIST_IDENTIFIER = "--";

    /**
     * Object identifier.
     */
    char OBJECT_IDENTIFIER = ':';

    /**
     * Parse and read all datas from yml file.
     *
     * @return
     *         Map object having the annotated field's key and value.
     */
    Map<String, Object> getYmlDatas() throws Exception;
}
