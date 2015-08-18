/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager;

import javax.xml.bind.ValidationEvent;
import javax.xml.bind.ValidationEventHandler;

/**
 * JAXB validation event handler that terminates the current operation on
 * an error event.
 */
public final class TestXmlEventHandler implements ValidationEventHandler {
    /**
     * Handle XML validation event.
     *
     * @param event  A {@link ValidationEvent} instance.
     * @return  {@code true} if the JAXB provider should attempt to continue
     *          the current operation.
     *          {@code false} if the JAXB provider should terminate the current
     *          operation.
     * @throws IllegalArgumentException
     *    {@code event} is {@code null}.
     */
    @Override
    public boolean handleEvent(ValidationEvent event) {
        if (event == null) {
            throw new IllegalArgumentException();
        }

        return (event.getSeverity() == ValidationEvent.WARNING);
    }
}
