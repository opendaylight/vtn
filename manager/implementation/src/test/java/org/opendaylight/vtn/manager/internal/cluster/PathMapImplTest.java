/*
 * Copyright (c) 2014-2015 NEC Corporation
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

/**
 * JUnit test for the abstract clase{@link PathMapImpl}.
 * Using the extended class-ContainerPathMapImpl.
 */

public class PathMapImplTest extends TestBase {
    /**
     * Test method for
     * {@link PathMapImpl#PathMapImpl(int, PathMap)},
     * {@link PathMapImpl#getTimeout(Integer, String)},
     * {@link PathMapImpl#getIndex()},
     * {@link PathMapImpl#getPathMap()},
     * {@link PathMapImpl#hashCode()},
     * {@link PathMapImpl#equals(Object)},
     * {@link PathMapImpl#toString()}.
     */
    @Test
    public void testAllMethodExceptEvaluate() {
        ContainerPathMapImpl tempContainerPathMapImpl = null;

        for (int idx : INDEX_ARRAY) {
            for (PathMap pmaps : createPathMaps()) {
                PathMap[] pmapList = {pmaps};
                for (PathMap pmap : pmapList) {
                    try {
                        // Test - Constructor and getTimeout
                        ContainerPathMapImpl containerPathMapImpl = new ContainerPathMapImpl(idx, pmap);

                        // Test - getIndex()
                        assertEquals(idx, containerPathMapImpl.getIndex());

                        // Test - getPathMap()
                        assertNotNull(containerPathMapImpl.getPathMap());

                        // Test - hashCode()
                        assertNotNull(containerPathMapImpl.hashCode());

                        // Test - equals()
                        containerPathMapImpl.equals(tempContainerPathMapImpl);
                        containerPathMapImpl.equals(pmap);

                        // Test - toString()
                        assertNotNull(containerPathMapImpl.toString());

                        if (tempContainerPathMapImpl == null) {
                            if ((containerPathMapImpl != null) && (containerPathMapImpl.getPathMap() != null)) {
                                PathMap pathMap = containerPathMapImpl.getPathMap();
                                if ((pathMap.getIndex() != null) && (pathMap.getFlowConditionName() != null)
                                    && (pathMap.getIdleTimeout() != null) && (pathMap.getHardTimeout() != null)) {
                                    tempContainerPathMapImpl = containerPathMapImpl;
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

    /**
     * Test method for
     * {@link PathMapImpl#evaluate(VTNManagerImpl, PacketContext)}.
     */
    //@Test
    public void testEvaluate() {
        ContainerPathMapImpl tempContainerPathMapImpl = null;
        try {
            for (int idx : INDEX_ARRAY) {
                for (PathMap pmap : createPathMaps()) {
                    // Test - Constructor and getTimeout
                    ContainerPathMapImpl containerPathMapImpl = new ContainerPathMapImpl(idx, pmap);
                }
            }
        } catch (VTNException | NullPointerException ex) {
            //ex.printStackTrace();
        }
    }
}
