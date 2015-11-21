/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.routing;

import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.verifyNoMoreInteractions;

import org.junit.Test;

import org.opendaylight.vtn.manager.internal.TestBase;

/**
 * JUnit test for {@link RoutingEvent}
 *
*/
public class RoutingEventTest extends TestBase {

    /**
     * Test case for {@link RoutingEvent#notifyEvent()}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testNotify() throws Exception {
        VTNRoutingListener listener = mock(VTNRoutingListener.class);
        RoutingEvent event = new RoutingEvent(listener);
        event.notifyEvent();
        verify(listener).routingUpdated(event);
        verifyNoMoreInteractions(listener);
    }
}
