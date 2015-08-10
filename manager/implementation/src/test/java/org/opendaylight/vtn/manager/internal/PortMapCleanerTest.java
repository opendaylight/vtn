/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal;
import java.util.ArrayList;
import java.util.List;

import org.junit.Test;

import org.opendaylight.vtn.manager.internal.cluster.PortVlan;
import org.opendaylight.controller.sal.core.NodeConnector;

/**
 * JUnit test for {@link PortMapCleaner}
 */
public class PortMapCleanerTest extends TestBase{

    /**
     * Test case for add and purge methods.
     */
    @Test
    public void testSetters() {
        PortMapCleaner cleaner = new PortMapCleaner();
        for (PortVlan vlan: createPortVlan(10)) {
            try {
                cleaner.add(vlan);
            } catch (Exception ex) {
                assertEquals(null, ex.getLocalizedMessage());
            }
        }

        for (VTNManagerImpl mangerImpl: createVtnManagerImplobj()) {
            for (String name : createStrings("portVlan")) {
                try {
                    cleaner.purge(mangerImpl, name);
                } catch (Exception ex) {
                    assertEquals(null, ex.getLocalizedMessage());
                }
            }
        }
    }

    /**
     * Create PortVlan  methods.
     */
    private List<PortVlan> createPortVlan(int num) {
        ArrayList<PortVlan> portVlanObj = new ArrayList<PortVlan>();
        for (NodeConnector nc: createNodeConnectors(num, false)) {
            short[] vlans = {-10, -1, 0, 1, 10, 100, 4095, 10000};
            for (short vlan : vlans) {
                PortVlan pvlan = new PortVlan(nc, vlan);
                portVlanObj.add(pvlan);
            }
            portVlanObj.add(null);
        }
        return portVlanObj;
    }
}
