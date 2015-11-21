/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.Set;

import org.junit.Test;

import org.opendaylight.vtn.manager.internal.TestBase;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.vtns.Vtn;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.vtns.VtnBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeName;

/**
 * JUnit test for {@link CacheMap}.
 */
public class CacheMapTest extends TestBase {
    /**
     * Cache for VTN for test.
     */
    private static final class VtnCache {
        /**
         * The raw data.
         */
        private final Vtn  rawVtn;

        /**
         * Construct a new instance.
         *
         * @param vtn  A {@link Vtn} instance.
         */
        private VtnCache(Vtn vtn) {
            rawVtn = vtn;
        }

        /**
         * Return the name of the VTN.
         *
         * @return  The name of the VTN.
         */
        private VnodeName getName() {
            return rawVtn.getName();
        }

        /**
         * Return the raw VTN.
         *
         * @return  The raw VTN.
         */
        private Vtn getVtn() {
            return rawVtn;
        }
    }

    /**
     * Implementation of {@link CacheMap} for test.
     */
    private static final class VtnCacheMap
        extends CacheMap<Vtn, VnodeName, VtnCache> {
        /**
         * Construct a new instance.
         *
         * @param list  A list of {@link Vtn} instances.
         */
        private VtnCacheMap(List<Vtn> list) {
            super(list);
        }

        // CacheMap

        /**
         * Return a key in the given VTN.
         *
         * @param vtn  A {@link Vtn} instance.
         * @return  The name of the VTN.
         */
        @Override
        protected VnodeName getKey(Vtn vtn) {
            return vtn.getName();
        }

        /**
         * Create a new cached VTN.
         *
         * @param data  A raw VTN.
         * @return  A cached data for the given VTN.
         */
        @Override
        protected VtnCache newCache(Vtn data) {
            return new VtnCache(data);
        }
    }

    /**
     * Test case for iteration.
     */
    @Test
    public void testIteration() {
        // Null list.
        List<Vtn> vtnList = null;
        VtnCacheMap map = new VtnCacheMap(vtnList);
        Iterator<VtnCache> it = map.iterator();
        assertEquals(false, it.hasNext());
        List<VtnCache> cached = new ArrayList<>();
        for (VtnCache vc: map) {
            cached.add(vc);
        }
        assertEquals(true, cached.isEmpty());
        assertEquals(true, map.cachedValues().isEmpty());

        // Empty list.
        vtnList = Collections.<Vtn>emptyList();
        map = new VtnCacheMap(vtnList);
        it = map.iterator();
        assertEquals(false, it.hasNext());
        for (VtnCache vc: map) {
            cached.add(vc);
        }
        assertEquals(true, cached.isEmpty());
        assertEquals(true, map.cachedValues().isEmpty());

        // Singleton list.
        vtnList = new ArrayList<>();
        Vtn vtn1 = new VtnBuilder().setName(new VnodeName("vtn_1")).build();
        vtnList.add(vtn1);
        map = new VtnCacheMap(vtnList);
        for (VtnCache vc: map) {
            cached.add(vc);
        }
        assertEquals(1, cached.size());
        assertSame(vtn1, cached.get(0).getVtn());
        it = map.cachedValues().iterator();
        assertEquals(true, it.hasNext());
        assertSame(vtn1, it.next().getVtn());
        assertEquals(false, it.hasNext());

        // Multiple elements in a list.
        for (int i = 2; i <= 10; i++) {
            String name = "vtn_" + i;
            Vtn vtn = new VtnBuilder().setName(new VnodeName(name)).build();
            vtnList.add(vtn);
        }

        cached.clear();
        map = new VtnCacheMap(vtnList);
        for (VtnCache vc: map) {
            cached.add(vc);
        }
        assertEquals(vtnList.size(), cached.size());
        for (int i = 0; i < vtnList.size(); i++) {
            assertSame(vtnList.get(i), cached.get(i).getVtn());
        }

        Set<VtnCache> cacheSet = new HashSet<>(cached);
        Set<VtnCache> cacheSet1 = new HashSet<>(map.cachedValues());
        assertEquals(cacheSet, cacheSet1);

        cacheSet1.clear();
        for (VtnCache vc: map) {
            cacheSet1.add(vc);
        }
        assertEquals(cacheSet, cacheSet1);

        // Terminate iteration.
        cached.clear();
        cacheSet.clear();
        map = new VtnCacheMap(vtnList);
        int count = 0;
        for (VtnCache vc: map) {
            cached.add(vc);
            assertTrue(cacheSet.add(vc));
            count++;
            if (count >= 3) {
                break;
            }
        }

        for (int i = 0; i < count; i++) {
            assertSame(vtnList.get(i), cached.get(i).getVtn());
        }

        cacheSet1.clear();
        cacheSet1.addAll(map.cachedValues());
        assertEquals(cacheSet, cacheSet1);

        // Iterate again.
        count = 0;
        for (VtnCache vc: map) {
            if (count < cached.size()) {
                assertSame(cached.get(count), vc);
            } else {
                cached.add(vc);
                assertTrue(cacheSet.add(vc));
            }
            assertSame(vtnList.get(count), vc.getVtn());
            count++;
            if (count >= 7) {
                break;
            }
        }

        cacheSet1.clear();
        cacheSet1.addAll(map.cachedValues());
        assertEquals(cacheSet, cacheSet1);

        count = 0;
        for (VtnCache vc: map) {
            if (count < cached.size()) {
                assertSame(cached.get(count), vc);
            } else {
                cached.add(vc);
                assertTrue(cacheSet.add(vc));
            }
            assertSame(vtnList.get(count), vc.getVtn());
            count++;
        }

        cacheSet1.clear();
        cacheSet1.addAll(map.cachedValues());
        assertEquals(cacheSet, cacheSet1);
        assertEquals(vtnList.size(), cacheSet1.size());
    }

    /**
     * Test case for {@link CacheMap#get(Object)}.
     */
    @Test
    public void testGet() {
        // Null list.
        List<Vtn> vtnList = null;
        VtnCacheMap map = new VtnCacheMap(vtnList);
        List<VnodeName> keys = new ArrayList<>();
        for (int i = 0; i <= 10; i++) {
            keys.add(new VnodeName("vtn_" + i));
        }
        for (int i = 0; i < 5; i++) {
            for (VnodeName vname: keys) {
                assertEquals(null, map.get(vname));
            }
        }

        // Empty list.
        vtnList = Collections.<Vtn>emptyList();
        map = new VtnCacheMap(vtnList);
        for (int i = 0; i < 5; i++) {
            for (VnodeName vname: keys) {
                assertEquals(null, map.get(vname));
            }
        }

        // Singleton list.
        VnodeName key0 = keys.get(0);
        Vtn vtn0 = new VtnBuilder().setName(key0).build();
        vtnList = Collections.singletonList(vtn0);
        map = new VtnCacheMap(vtnList);
        VtnCache vc0 = map.get(key0);
        assertSame(vtn0, vc0.getVtn());
        Map<VnodeName, VtnCache> cached = new HashMap<>();
        cached.put(key0, vc0);
        for (int i = 0; i < 5; i++) {
            for (VnodeName vname: keys) {
                VtnCache vc = map.get(vname);
                if (vname.equals(key0)) {
                    assertSame(vc0, vc);
                } else {
                    assertEquals(null, vc);
                }
            }
        }

        // Multiple elements in a list.
        vtnList = new ArrayList<>();
        for (VnodeName vname: keys) {
            Vtn vtn = new VtnBuilder().setName(vname).build();
            vtnList.add(vtn);
        }
        map = new VtnCacheMap(vtnList);

        // Change order of the key list.
        List<VnodeName> keys1 = new ArrayList<>(keys.size());
        keys1.add(keys.get(3));
        keys1.add(keys.get(5));
        keys1.add(keys.get(2));
        keys1.add(keys.get(4));
        keys1.add(keys.get(0));
        keys1.add(keys.get(9));
        keys1.add(keys.get(8));
        keys1.add(keys.get(6));
        keys1.add(keys.get(7));
        keys1.add(keys.get(10));
        keys1.add(keys.get(1));

        cached.clear();
        for (int i = 0; i < 5; i++) {
            for (VnodeName vname: keys1) {
                VtnCache vc = map.get(vname);
                assertEquals(vname, vc.getName());
                VtnCache c = cached.get(vname);
                if (c == null) {
                    cached.put(vname, vc);
                } else {
                    assertSame(c, vc);
                }
            }
        }

        // Iteration order should be preserved.
        int count = 0;
        for (VtnCache vc: map) {
            assertSame(vtnList.get(count), vc.getVtn());
            assertSame(vc, map.get(vc.getName()));
            count++;
        }
        assertEquals(vtnList.size(), count);

        // In case where the key is not found.
        List<VnodeName> unknown = new ArrayList<>();
        for (int i = 100; i <= 120; i++) {
            unknown.add(new VnodeName("vtn_" + i));
        }
        for (int i = 0; i < 5; i++) {
            for (VnodeName vname: unknown) {
                assertEquals(null, map.get(vname));
            }
        }

        map = new VtnCacheMap(vtnList);
        for (int i = 0; i < 5; i++) {
            for (VnodeName vname: unknown) {
                assertEquals(null, map.get(vname));
            }
        }
    }


    /**
     * Test case for {@link CacheMap#put(Object, Object)} and
     * {@link CacheMap#getCached(Object)}.
     */
    @Test
    public void testPut() {
        List<VnodeName> keys = new ArrayList<>();
        List<Vtn> vtnList = new ArrayList<>();
        for (int i = 0; i <= 10; i++) {
            VnodeName vname = new VnodeName("vtn_" + i);
            keys.add(vname);
            Vtn vtn = new VtnBuilder().setName(vname).build();
            vtnList.add(vtn);
        }
        VtnCacheMap map = new VtnCacheMap(vtnList);

        Set<VtnCache> cacheSet = new HashSet<>();
        for (int i = 0; i <= 10; i += 2) {
            VnodeName vname = keys.get(i);
            Vtn vtn = vtnList.get(i);
            VtnCache vc = new VtnCache(vtn);
            assertSame(vc, map.put(vname, vc));
            assertSame(vc, map.get(vname));
            assertTrue(cacheSet.add(vc));

            // Cached object is returned if already cached.
            VtnCache vc1 = new VtnCache(vtn);
            assertSame(vc, map.put(vname, vc1));
            assertSame(vc, map.get(vname));
        }

        assertEquals(cacheSet, new HashSet<>(map.cachedValues()));

        // Read cached data.
        for (int i = 0; i <= 10; i++) {
            VnodeName vname = keys.get(i);
            VtnCache vc = map.getCached(vname);
            if ((i & 1) == 0) {
                assertEquals(true, cacheSet.contains(vc));
            } else {
                assertEquals(null, vc);
            }
        }

        // Iterate cached data.
        int index = 0;
        for (VtnCache vc: map) {
            assertSame(vtnList.get(index), vc.getVtn());
            assertSame(vc, map.get(keys.get(index)));

            boolean cached = ((index & 1) == 0);
            assertEquals(cached, cacheSet.contains(vc));
            index++;
        }
    }
}
