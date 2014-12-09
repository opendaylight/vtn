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

/**
 * JUnit test for {@link PathMapComparator}.
 */

public class PathMapComparatorTest extends TestBase {

    /**
     * Test method for
     * {@link PathMapComparator#hashCode(),
     *  @link PathMapComparator#equals(Object),
     *  @link PathMapComparator#compare(PathMap, PathMap)}.
     */
    @Test
    public void testComparator() {
        try {
            PathMapComparator pathcomparator = new PathMapComparator();

            assertTrue(pathcomparator.equals(pathcomparator));
            assertFalse(pathcomparator.equals(null));
            assertFalse(pathcomparator.equals(new Integer(1)));
            assertNotNull(pathcomparator.hashCode());

            for (PathMap pmap : createPathMapsWithIndex()) {
                for (PathMap pmapobj : createPathMapsWithIndex()) {
                    try {
                        assertEquals(0, pathcomparator.compare(pmapobj, pmapobj));
                        assertNotNull(pathcomparator.compare(pmapobj, pmap));
                    } catch (Exception ex) {
                        //ex.printStackTrace();
                    }
                }
            }
        } catch (Exception ex) {
            //ex.printStackTrace();
        }
    }
}
