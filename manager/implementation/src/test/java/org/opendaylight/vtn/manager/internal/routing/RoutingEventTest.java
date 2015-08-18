/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.routing;

import org.junit.Test;

import org.opendaylight.vtn.manager.internal.TestBase;
import org.opendaylight.vtn.manager.internal.VTNManagerImpl;

/**
 * JUnit test for {@link RoutingEvent}
 *
*/
public class RoutingEventTest extends TestBase {

    /**
     * Method to test notifyEvent
     */
    @Test
    public void testNotify() {
        try {
            VTNManagerImpl mgr = new VTNManagerImpl();
            RoutingEvent event = new RoutingEvent(mgr);
            event.notifyEvent();
        } catch (Exception ex) {
            assertEquals(null, ex.getMessage());
        }
    }

}
