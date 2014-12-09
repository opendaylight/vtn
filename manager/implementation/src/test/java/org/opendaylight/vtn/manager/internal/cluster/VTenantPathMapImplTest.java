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
import org.opendaylight.vtn.manager.internal.TestBase;

/**
 * JUnit test for {@link VTenantPathMapImpl}.
 */
public class VTenantPathMapImplTest extends TestBase{

    /**
     * Name of the VTenantImpl instance.
     */
    private static final String VTENANT_IMPL_INSTANCE_NAME = "VTenantName";

    /**
     * Name of the VTenantImpl-Container instance.
     */
    private static final String VTENANT_IMPL_CONTAINER_NAME = "default";

    /**
     * Name of the VTenantConfig Description.
     */
    private static final String VTENANT_CONFIG_DESC = "desc";

    /**
     * VTenantPathMapImpl-Logger prefix identifier.
     */
    private static final String LOGGER_PREFIX = ":" + VTENANT_IMPL_INSTANCE_NAME + ".";

    /**
     * Test method for
     * {@link VTenantPathMapImpl#VTenantPathMapImpl(VTenantImpl, int, PathMap),
     *  @link VTenantPathMapImpl#setVTenant(VTenantImpl),
     *  @link VTenantPathMapImpl#getLogger(),
     *  @link VTenantPathMapImpl#getLogPrefix()}.
     */
    @Test
    public void testAllMethods() {
        try {
            // Checking for all the scenarios for the constructor in VTenantPathMapImpl.
            // VTenantImpl List
            for (VTenantImpl vTenantImpl : createVTenantImpl(VTENANT_CONFIG_DESC, VTENANT_IMPL_CONTAINER_NAME, VTENANT_IMPL_INSTANCE_NAME)) {
                // VTenantPathMapImpl.index List
                for (int idx : INDEX_ARRAY) {
                    // VTenantPathMapImpl.PathMap List
                    for (PathMap pathMap : createPathMapsWithIndex()) {
                        try {
                            VTenantPathMapImpl vTenantPathMapImpl = new VTenantPathMapImpl(vTenantImpl, idx, pathMap);
                            vTenantPathMapImpl.setVTenant(vTenantImpl);

                            assertNotNull(vTenantPathMapImpl.getLogger());
                            assertEquals(vTenantPathMapImpl.getLogPrefix(), new String(LOGGER_PREFIX + idx));

                            assertEquals(pathMap, vTenantPathMapImpl.getPathMap());
                            assertEquals(idx, vTenantPathMapImpl.getIndex());
                            assertNotNull(vTenantPathMapImpl.hashCode());

                            assertTrue(vTenantPathMapImpl.equals(vTenantPathMapImpl));
                            assertFalse(vTenantPathMapImpl.equals(pathMap));
                        } catch (Exception ex) {
                            //ex.printStackTrace();
                        }
                    }
                }
            }
        } catch (Exception ex) {
            //ex.printStackTrace();
        }
    }
}
