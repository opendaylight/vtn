/*
 * Copyright (c) 2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.cluster;

import org.junit.Test;
import org.opendaylight.vtn.manager.PathMap;
import org.opendaylight.vtn.manager.VTNException;
import org.opendaylight.vtn.manager.internal.TestBase;
import org.opendaylight.vtn.manager.internal.VTNManagerImpl;

/**
 * JUnit test for {@link ContainerPathMapImpl}.
 */

public class ContainerPathMapImplTest extends TestBase {
    /**
     * Junit for method calls.
     */
    @Test
    public void testAllMethod() {
        for (PathMap pmap1 : createPathMapsWithAllParameters()) {
            PathMap[] pmaps = {pmap1};
            for (PathMap pmap : pmaps) {
                try {
                    int idx = pmap.getIndex();
                    ContainerPathMapImpl containerPathMapImpl = new ContainerPathMapImpl(idx, pmap);

                    assertEquals(idx, containerPathMapImpl.getIndex());
                    assertEquals(pmap, containerPathMapImpl.getPathMap());
                    assertTrue(containerPathMapImpl.equals(containerPathMapImpl));
                    assertNotNull(containerPathMapImpl.hashCode());
                    assertEquals(containerPathMapImpl.getLogPrefix(), "." + idx);
                    assertEquals(containerPathMapImpl.getLogger().getName(), ContainerPathMapImpl.class.getName());

                    for (VTNManagerImpl mgr1 : createVtnManagerImplobj()) {
                        VTNManagerImpl[] mgrs = {mgr1};
                        for (VTNManagerImpl mgr : mgrs) {
                            try {
                                assertNotNull(containerPathMapImpl.saveConfig(mgr));
                                containerPathMapImpl.destroy(mgr);
                                assertFalse(containerPathMapImpl.equals(mgr));
                                assertFalse(containerPathMapImpl.equals(pmap));
                                assertFalse(containerPathMapImpl.equals(null));
                            } catch (Exception ex) {
                                //ex.printStackTrace();
                            }
                        }
                    }
                } catch (VTNException | NullPointerException ex) {
                    //ex.printStackTrace();
                }
            }
        }
    }
}
