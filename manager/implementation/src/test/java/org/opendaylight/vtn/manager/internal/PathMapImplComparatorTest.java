/*
 * Copyright (c) 2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal;

import org.junit.Test;
import org.opendaylight.vtn.manager.PathMap;
import org.opendaylight.vtn.manager.internal.cluster.ContainerPathMapImpl;

/**
 * JUnit test for {@link PathMapImplComparator}.
 */

public class PathMapImplComparatorTest extends TestBase {

    /**
     * Test method for
     * {@link PathMapImplComparator#hashCode(),
     *  @link PathMapImplComparator#equals(Object),
     *  @link PathMapImplComparator#compare(PathMapImpl, PathMapImpl)}.
     */
    @Test
    public void testComparator() {
        try {
            PathMapImplComparator pathMapImplComparator = new PathMapImplComparator();

            assertTrue(pathMapImplComparator.equals(pathMapImplComparator));
            assertFalse(pathMapImplComparator.equals(null));
            assertFalse(pathMapImplComparator.equals(new Integer(1)));
            assertNotNull(pathMapImplComparator.hashCode());

            for (PathMap pathMap : createPathMapsWithIndex()) {
                if ((pathMap != null) && (pathMap.getIndex() != null)) {
                    ContainerPathMapImpl containerPathMapImpl1 = new ContainerPathMapImpl(pathMap.getIndex(), pathMap);
                    for (PathMap objPathMap : createPathMapsWithIndex()) {
                        if ((objPathMap != null) && (objPathMap.getIndex() != null)) {
                            ContainerPathMapImpl containerPathMapImpl2 = new ContainerPathMapImpl(objPathMap.getIndex(), objPathMap);
                            try {
                                assertEquals(0, pathMapImplComparator.compare(containerPathMapImpl2, containerPathMapImpl2));
                                assertNotNull(pathMapImplComparator.compare(containerPathMapImpl2, containerPathMapImpl1));
                            } catch (Exception ex) {
                                //ex.printStackTrace();
                            }
                        }
                    }
                }
            }
        } catch (Exception ex) {
            //ex.printStackTrace();
        }
    }
}
