/*
 * Copyright (c) 2014, 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.app.run.config.json.annotations;

import java.lang.annotation.ElementType;
import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.lang.annotation.Target;

/**
 * JsonElement Interface provides functions to for name and defaultvalue to set
 *    in the implementation.
 */
@Retention(RetentionPolicy.RUNTIME)
@Target(ElementType.FIELD)
public @interface JsonElement {

    /**
     * name - provides api to set the name in the implementation.
     * @return  The element name.
     */
    String name();

    /**
     * defaultValue - provide api to set the defaultValue in the implementation.
     * @return  The default value.
     */
    String defaultValue() default "";
}
