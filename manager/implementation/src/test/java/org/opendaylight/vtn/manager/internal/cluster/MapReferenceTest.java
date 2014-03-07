/*
 * Copyright (c) 2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.cluster;

import java.util.HashSet;
import java.util.List;
import java.util.ArrayList;

import org.junit.Test;

import org.opendaylight.vtn.manager.VBridgeIfPath;
import org.opendaylight.vtn.manager.VBridgePath;
import org.opendaylight.vtn.manager.internal.TestBase;

import org.opendaylight.controller.sal.core.Node;

/**
 * JUnit test for {@link MapReference}.
 */
public class MapReferenceTest extends TestBase {
    /**
     * Test case for getter methods.
     */
    @Test
    public void testGetter() {
        for (String cname: createStrings("ctr", false)) {
            for (String tname: createStrings("tnt", false)) {
                for (String bname: createStrings("brg", false)) {
                    for (MapType type: MapType.values()) {
                        VBridgePath path = new VBridgePath(tname, bname);
                        MapReference ref = new MapReference(type, cname, path);

                        assertEquals(type, ref.getMapType());
                        assertEquals(cname, ref.getContainerName());
                        assertEquals(path, ref.getPath());

                        StringBuilder builder = new StringBuilder(cname);
                        builder.append(':').append(path.toString());
                        assertEquals(builder.toString(),
                                     ref.getAbsolutePath());

                        for (String name: createStrings("nm", false)) {
                            // If the target is vBridge interface, it should be
                            // embedded in its absolute path.
                            VBridgeIfPath ifpath =
                                new VBridgeIfPath(path, name);
                            MapReference iref =
                                new MapReference(type, cname, ifpath);
                            assertFalse(ref.equals(iref));
                            builder = new StringBuilder(cname);
                            builder.append(':').append(ifpath.toString());
                            assertEquals(builder.toString(),
                                         iref.getAbsolutePath());

                            // Otherwise, path to the vBridge should be used.
                            VlanMapPath vpath = new VlanMapPath(path, name);
                            MapReference vref =
                                new MapReference(type, cname, vpath);
                            assertFalse(ref.equals(vref));
                            assertEquals(ref.getAbsolutePath(),
                                         vref.getAbsolutePath());
                        }
                    }
                }
            }
        }
    }

    /**
     * Test case for {@link MapReference#isContained(String, VBridgePath)}.
     */
    @Test
    public void testIsContained() {
        String unknown = "unknown";
        for (MapReference ref: createMapReferences()) {
            VBridgePath path = ref.getPath();
            String cname = ref.getContainerName();
            String tname = path.getTenantName();
            String bname = path.getBridgeName();
            VBridgePath bpath = new VBridgePath(tname, bname);

            assertTrue(ref.isContained(cname, bpath));
            assertFalse(ref.isContained(unknown, bpath));

            bpath = new VBridgePath(unknown, bname);
            assertFalse(ref.isContained(cname, bpath));
            bpath = new VBridgePath(tname, unknown);
            assertFalse(ref.isContained(cname, bpath));
        }
    }

    /**
     * Test case for {@link MapReference#equals(Object)} and
     * {@link MapReference#hashCode()}.
     */
    @Test
    public void testEquals() {
        HashSet<Object> set = new HashSet<Object>();
        List<MapReference> references = createMapReferences();
        for (MapReference ref: references) {
            MapReference ref1 = copy(ref);
            testEquals(set, ref, ref1);
        }

        assertEquals(references.size(), set.size());
    }

    /**
     * Test case for {@link MapReference#toString()}.
     */
    @Test
    public void testToString() {
        for (MapReference ref: createMapReferences()) {
            MapType type = ref.getMapType();
            String required = type + "@" + ref.getContainerName() +
                ":" + ref.getPath();
            assertEquals(required, ref.toString());
        }
    }

    /**
     * Ensure that {@link MapReference} is serializable.
     */
    @Test
    public void testSerialize() {
        for (MapReference ref: createMapReferences()) {
            serializeTest(ref);
        }
    }

    /**
     * Create unique {@link MapReference} objects for test.
     *
     * @return  A list of {@link MapReference} objects.
     */
    private List<MapReference> createMapReferences() {
        ArrayList<MapReference> list = new ArrayList<MapReference>();
        for (String cname: createStrings("ctr", false)) {
            for (String tname: createStrings("tnt", false)) {
                for (String bname: createStrings("brg", false)) {
                    for (MapType type: MapType.values()) {
                        VBridgePath path = new VBridgePath(tname, bname);
                        MapReference ref = new MapReference(type, cname, path);
                        list.add(ref);

                        for (String name: createStrings("nm", false)) {
                            VBridgeIfPath ipath =
                                new VBridgeIfPath(path, name);
                            ref = new MapReference(type, cname, ipath);
                            list.add(ref);

                            VlanMapPath vpath = new VlanMapPath(path, name);
                            ref = new MapReference(type, cname, vpath);
                            list.add(ref);
                        }
                    }
                }
            }
        }

        return list;
    }
}
