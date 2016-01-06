/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.it.util.pathmap;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;

import static org.opendaylight.vtn.manager.it.util.ModelDrivenTestBase.getRpcOutput;
import static org.opendaylight.vtn.manager.it.util.ModelDrivenTestBase.getRpcResult;
import static org.opendaylight.vtn.manager.it.util.TestBase.RANDOM_ADD_MAX;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.Random;
import java.util.Set;

import org.opendaylight.vtn.manager.it.util.VTNServices;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathmap.rev150328.ClearPathMapInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathmap.rev150328.ClearPathMapInputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathmap.rev150328.RemovePathMapInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathmap.rev150328.RemovePathMapInputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathmap.rev150328.RemovePathMapOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathmap.rev150328.SetPathMapInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathmap.rev150328.SetPathMapInputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathmap.rev150328.SetPathMapOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathmap.rev150328.VtnPathMapList;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathmap.rev150328.VtnPathMapResult;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathmap.rev150328.VtnPathMapService;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathmap.rev150328.set.path.map.input.PathMapList;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathmap.rev150328.vtn.path.map.list.VtnPathMap;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateType;

/**
 * {@code PathMapSet} describes a set of path map configurations.
 */
public final class PathMapSet implements Cloneable {
    /**
     * A map that keeps path map configurations.
     */
    private Map<Integer, PathMap> pathMaps = new HashMap<>();

    /**
     * Remove all the path maps in the specified path map list.
     *
     * @param service  The vtn-path-map RPC service.
     * @param tname    The name of the VTN.
     *                 {@code null} implies the global path map list.
     * @return  A {@link VtnUpdateType} instance.
     */
    public static VtnUpdateType clearPathMap(
        VtnPathMapService service, String tname) {
        ClearPathMapInput input = new ClearPathMapInputBuilder().
            setTenantName(tname).
            build();
        return getRpcResult(service.clearPathMap(input));
    }

    /**
     * Remove the specified path map in the specified path map list.
     *
     * @param service  The vtn-path-map RPC service.
     * @param tname    The name of the VTN.
     *                 {@code null} implies the global path map list.
     * @param index    The index of the path map to be removed.
     * @return  A {@link VtnUpdateType} instance that indicates the result of
     *          RPC.
     */
    public static VtnUpdateType removePathMap(
        VtnPathMapService service, String tname, Integer index) {
        Map<Integer, VtnUpdateType> result = removePathMap(
            service, tname, Collections.singletonList(index));
        assertEquals(Collections.singleton(index), result.keySet());
        return result.get(index);
    }

    /**
     * Remove the specified path maps in the specified path map list.
     *
     * @param service  The vtn-path-map RPC service.
     * @param tname    The name of the VTN.
     *                 {@code null} implies the global path map list.
     * @param indices  A list of path map indices to be removed.
     * @return  A map that specifies the removed path maps.
     */
    public static Map<Integer, VtnUpdateType> removePathMap(
        VtnPathMapService service, String tname, List<Integer> indices) {
        RemovePathMapInput input = new RemovePathMapInputBuilder().
            setTenantName(tname).
            setMapIndex(indices).
            build();

        RemovePathMapOutput output =
            getRpcOutput(service.removePathMap(input));
        return getResultMap(output.getRemovePathMapResult());
    }

    /**
     * Create a new input builder for set-path-map RPC.
     *
     * @param pmaps  An array of {@link PathMap} instances.
     * @return  A {@link SetPathMapInputBuilder} instance.
     */
    public static SetPathMapInputBuilder newInputBuilder(PathMap ... pmaps) {
        List<PathMap> list = (pmaps == null) ? null : Arrays.asList(pmaps);
        return newInputBuilder(list);
    }

    /**
     * Create a new input builder for set-path-map RPC.
     *
     * @param pmaps  A collection of {@link PathMap} instances.
     * @return  A {@link SetPathMapInputBuilder} instance.
     */
    public static SetPathMapInputBuilder newInputBuilder(
        Collection<PathMap> pmaps) {
        SetPathMapInputBuilder builder = new SetPathMapInputBuilder();
        if (pmaps != null) {
            List<PathMapList> list = new ArrayList<>(pmaps.size());
            for (PathMap pmap: pmaps) {
                list.add(pmap.toPathMapList());
            }
            builder.setPathMapList(list);
        }

        return builder;
    }

    /**
     * Create a map that indicates the given result of path map RPC.
     *
     * @param list  A list of {@link VtnPathMapResult} instances.
     * @param <T>   The type of the RPC result.
     * @return  A map that indicates the given result of path map RPC.
     *          Note that {@code null} is returned if the given list is
     *          {@code null}.
     */
    static <T extends VtnPathMapResult> Map<Integer, VtnUpdateType> getResultMap(
        List<T> list) {
        Map<Integer, VtnUpdateType> result;
        if (list == null) {
            result = null;
        } else {
            result = new HashMap<>();
            for (VtnPathMapResult vpres: list) {
                Integer index = vpres.getIndex();
                VtnUpdateType status = vpres.getStatus();
                assertEquals(null, result.put(index, status));
            }
        }

        return result;
    }

    /**
     * Add random path maps using the given random generator.
     *
     * @param rand  A pseudo random generator.
     * @return  This instance.
     */
    public PathMapSet add(Random rand) {
        Set<Integer> indices = new HashSet<>(pathMaps.keySet());
        int count = rand.nextInt(RANDOM_ADD_MAX);
        for (int i = 0; i <= count; i++) {
            add(new PathMap(rand, indices));
        }

        return this;
    }

    /**
     * Add the given path map configuration.
     *
     * @param pmaps  An array of {@link PathMap} instances.
     * @return  This instance.
     */
    public PathMapSet add(PathMap ... pmaps) {
        for (PathMap pmap: pmaps) {
            pathMaps.put(pmap.getIndex(), pmap);
        }
        return this;
    }

    /**
     * Add all the path maps in the given path map set.
     *
     * @param pmSet  A {@link PathMapSet} instance.
     * @return  This instance.
     */
    public PathMapSet add(PathMapSet pmSet) {
        for (PathMap pmap: pmSet.pathMaps.values()) {
            pathMaps.put(pmap.getIndex(), pmap);
        }
        return this;
    }

    /**
     * Remove the path map configuration specified by the given index.
     *
     * @param pmaps  An array of {@link PathMap} instances.
     * @return  This instance.
     */
    public PathMapSet remove(PathMap ... pmaps) {
        for (PathMap pmap: pmaps) {
            pathMaps.remove(pmap.getIndex());
        }
        return this;
    }

    /**
     * Remove the path map configuration specified by the given index.
     *
     * @param indices  An array of the path map indices to be removed.
     * @return  This instance.
     */
    public PathMapSet remove(Integer ... indices) {
        for (Integer idx: indices) {
            pathMaps.remove(idx);
        }
        return this;
    }

    /**
     * Remove all the path map configurations.
     *
     * @return  This instance.
     */
    public PathMapSet clear() {
        pathMaps.clear();
        return this;
    }

    /**
     * Return the path map configuration specified by the given index.
     *
     * @param idx  The index of the path map.
     * @return  A {@link PathMap} instance if found.
     *          {@code null} if not found.
     */
    public PathMap get(Integer idx) {
        return pathMaps.get(idx);
    }

    /**
     * Return an unmodifiable collection of path map configurations.
     *
     * @return  An unmodifiable collection of {@link PathMap} instances.
     */
    public Collection<PathMap> getPathMaps() {
        return Collections.unmodifiableCollection(pathMaps.values());
    }

    /**
     * Create a new input builder for set-path-map RPC.
     *
     * @return  A {@link SetPathMapInputBuilder} instance.
     */
    public SetPathMapInputBuilder newInputBuilder() {
        Collection<PathMap> pmaps = pathMaps.values();
        if (pmaps.isEmpty()) {
            pmaps = null;
        }

        return newInputBuilder(pmaps);
    }

    /**
     * Update the global path maps.
     *
     * @param service  The vtn-path-map service.
     * @return  A map that specifies the updated path maps.
     */
    public Map<Integer, VtnUpdateType> update(VtnPathMapService service) {
        return update(service, null);
    }

    /**
     * Update the path maps.
     *
     * @param service  The vtn-path-map service.
     * @param tname    The name of the VTN.
     *                 {@code null} implies the global path map.
     * @return  A map that specifies the updated path maps.
     */
    public Map<Integer, VtnUpdateType> update(VtnPathMapService service,
                                              String tname) {
        SetPathMapInput input = newInputBuilder().
            setTenantName(tname).
            build();

        SetPathMapOutput output = getRpcOutput(service.setPathMap(input));
        return getResultMap(output.getSetPathMapResult());
    }

    /**
     * Restore all the path maps in this instance.
     *
     * <p>
     *   This method expects that this instance contains at least one path map,
     *   and all the path maps in this instance are not configured yet.
     * </p>
     *
     * @param service  The vtn-path-map service.
     * @param tname    The name of the VTN.
     *                 {@code null} implies the global path map.
     */
    public void restore(VtnPathMapService service, String tname) {
        assertEquals(false, pathMaps.isEmpty());

        Map<Integer, VtnUpdateType> expected = new HashMap<>();
        for (Integer idx: pathMaps.keySet()) {
            assertEquals(null, expected.put(idx, VtnUpdateType.CREATED));
        }

        assertEquals(expected, update(service, tname));
    }

    /**
     * Add the given path map using set-path-map RPC, and then add it to
     * this instance.
     *
     * @param service  The vtn-path-map service.
     * @param tname    The name of the VTN.
     *                 {@code null} implies the global path map.
     * @param pmap     A {@link PathMap} instance.
     * @return  A {@link VtnUpdateType} instance returned by the RPC.
     */
    public VtnUpdateType addMap(VtnPathMapService service, String tname,
                                PathMap pmap) {
        Map<Integer, VtnUpdateType> result = addMaps(service, tname, pmap);
        Integer idx = pmap.getIndex();
        assertEquals(Collections.singleton(idx), result.keySet());
        return result.get(idx);
    }

    /**
     * Add the given path maps using set-path-map RPC, and then add them to
     * this instance.
     *
     * @param service  The vtn-path-map service.
     * @param tname    The name of the VTN.
     *                 {@code null} implies the global path map.
     * @param pmaps     An array of {@link PathMap} instances to be added.
     * @return  A map that contains the RPC result.
     */
    public Map<Integer, VtnUpdateType> addMaps(
        VtnPathMapService service, String tname, PathMap ... pmaps) {
        List<PathMap> list = (pmaps == null) ? null : Arrays.asList(pmaps);
        return addMaps(service, tname, list);
    }

    /**
     * Add the given path maps using set-path-map RPC, and then add them to
     * this instance.
     *
     * @param service  The vtn-path-map service.
     * @param tname    The name of the VTN.
     *                 {@code null} implies the global path map.
     * @param pmaps     A collection of {@link PathMap} instances to be added.
     * @return  A map that contains the RPC result.
     */
    public Map<Integer, VtnUpdateType> addMaps(
        VtnPathMapService service, String tname, Collection<PathMap> pmaps) {
        SetPathMapInput input = newInputBuilder(pmaps).
            setTenantName(tname).
            build();
        SetPathMapOutput output = getRpcOutput(service.setPathMap(input));
        Map<Integer, VtnUpdateType> result =
            getResultMap(output.getSetPathMapResult());

        if (pmaps != null) {
            for (PathMap pmap: pmaps) {
                pathMaps.put(pmap.getIndex(), pmap);
            }
        }

        return result;
    }

    /**
     * Remove the given path map using remove-path-map RPC, and then remove it
     * from this instance.
     *
     * @param service  The vtn-path-map service.
     * @param tname    The name of the VTN.
     *                 {@code null} implies the global path map.
     * @param pmap     A {@link PathMap} instance.
     * @return  A {@link VtnUpdateType} instance returned by the RPC.
     */
    public VtnUpdateType removeMap(VtnPathMapService service, String tname,
                                   PathMap pmap) {
        return removeMap(service, tname, pmap.getIndex());
    }

    /**
     * Remove the given path map using remove-path-map RPC, and then remove it
     * from this instance.
     *
     * @param service  The vtn-path-map service.
     * @param tname    The name of the VTN.
     *                 {@code null} implies the global path map.
     * @param idx      The index for the path map to be removed.
     * @return  A {@link VtnUpdateType} instance returned by the RPC.
     */
    public VtnUpdateType removeMap(VtnPathMapService service, String tname,
                                   Integer idx) {
        Map<Integer, VtnUpdateType> result = removeMaps(service, tname, idx);
        assertEquals(Collections.singleton(idx), result.keySet());
        return result.get(idx);
    }

    /**
     * Remove the given path maps using remove-path-map RPC, and then remove
     * them from this instance.
     *
     * @param service  The vtn-path-map service.
     * @param tname    The name of the VTN.
     *                 {@code null} implies the global path map.
     * @param indices  An array of path map indices.
     * @return  A map that contains the RPC result.
     */
    public Map<Integer, VtnUpdateType> removeMaps(
        VtnPathMapService service, String tname, Integer ... indices) {
        List<Integer> list = (indices == null) ? null : Arrays.asList(indices);
        return removeMaps(service, tname, list);
    }

    /**
     * Remove the given path maps using remove-path-map RPC, and then remove
     * them from this instance.
     *
     * @param service  The vtn-path-map service.
     * @param tname    The name of the VTN.
     *                 {@code null} implies the global path map.
     * @param indices  A list of path map indices.
     * @return  A map that contains the RPC result.
     */
    public Map<Integer, VtnUpdateType> removeMaps(
        VtnPathMapService service, String tname, List<Integer> indices) {
        Map<Integer, VtnUpdateType> result =
            removePathMap(service, tname, indices);

        if (indices != null) {
            for (Integer idx: indices) {
                pathMaps.remove(idx);
            }
        }

        return result;
    }

    /**
     * Remove all the path maps in this instance using remove-path-map RPC.
     *
     * @param service  The vtn-path-map service.
     * @param tname    The name of the VTN.
     *                 {@code null} implies the global path map.
     */
    public void removeAll(VtnPathMapService service, String tname) {
        if (!pathMaps.isEmpty()) {
            List<Integer> indices = new ArrayList<>();
            Map<Integer, VtnUpdateType> resMap = new HashMap<>();
            for (Iterator<Integer> it = pathMaps.keySet().iterator();
                 it.hasNext();) {
                Integer idx = it.next();
                indices.add(idx);
                assertEquals(null, resMap.put(idx, VtnUpdateType.REMOVED));
                it.remove();
            }

            assertEquals(resMap, removePathMap(service, tname, indices));
        }
    }

    /**
     * Verify the given path map container.
     *
     * @param vpmaps  A {@link VtnPathMapList} instance.
     */
    public void verify(VtnPathMapList vpmaps) {
        if (!pathMaps.isEmpty()) {
            assertNotNull(vpmaps);
            List<VtnPathMap> vpms = vpmaps.getVtnPathMap();
            assertNotNull(vpms);
            Set<Integer> checked = new HashSet<>();
            for (VtnPathMap vpm: vpms) {
                Integer idx = vpm.getIndex();
                PathMap pmap = pathMaps.get(idx);
                assertNotNull(pmap);
                pmap.verify(vpm);
                assertEquals(true, checked.add(idx));
            }
            assertEquals(checked, pathMaps.keySet());
        } else if (vpmaps != null) {
            List<VtnPathMap> vpms = vpmaps.getVtnPathMap();
            if (vpms != null) {
                assertEquals(Collections.<VtnPathMap>emptyList(), vpms);
            }
        }
    }

    /**
     * Apply all the configurations in this instance.
     *
     * @param service  A {@link VTNServices} instance.
     * @param tname    The name of the VTN.
     *                 {@code null} implies the global path map list.
     * @param vpmaps   A {@link VtnPathMapList} instance that contains
     *                 path maps present in the specified path map list.
     */
    public void apply(VTNServices service, String tname,
                      VtnPathMapList vpmaps) {
        VtnPathMapService vpmSrv = service.getPathMapService();
        removeUnwanted(vpmSrv, tname, vpmaps);

        if (!pathMaps.isEmpty()) {
            SetPathMapInput input = newInputBuilder().
                setTenantName(tname).
                build();
            getRpcOutput(vpmSrv.setPathMap(input));
        }
    }

    /**
     * Remove all the path maps that are not assocaited with the configuration
     * in this instance.
     *
     * @param service  The vtn-path-map RPC service.
     * @param tname    The name of the VTN.
     *                 {@code null} implies the global path map list.
     * @param vpmaps   A {@link VtnPathMapList} instance that contains
     *                 path maps present in the specified path map list.
     */
    private void removeUnwanted(
        VtnPathMapService service, String tname, VtnPathMapList vpmaps) {
        if (vpmaps != null) {
            List<VtnPathMap> vpms = vpmaps.getVtnPathMap();
            if (vpms != null) {
                if (pathMaps.isEmpty()) {
                    assertEquals(VtnUpdateType.REMOVED,
                                 clearPathMap(service, tname));
                } else {
                    List<Integer> unwanted = new ArrayList<>();
                    Map<Integer, VtnUpdateType> resMap = new HashMap<>();
                    for (VtnPathMap vpm: vpms) {
                        Integer index = vpm.getIndex();
                        if (!pathMaps.containsKey(index)) {
                            unwanted.add(index);
                            resMap.put(index, VtnUpdateType.REMOVED);
                        }
                    }

                    if (!unwanted.isEmpty()) {
                        assertEquals(resMap,
                                     removePathMap(service, tname, unwanted));
                    }
                }
            }
        }
    }

    // Object

    /**
     * Create a shallow copy of this instance.
     *
     * @return  A shallow copy of this instance.
     */
    @Override
    public PathMapSet clone() {
        try {
            PathMapSet pmSet = (PathMapSet)super.clone();
            pmSet.pathMaps = new HashMap<>(pathMaps);
            return pmSet;
        } catch (CloneNotSupportedException e) {
            // This should never happen.
            throw new IllegalStateException("clone() failed", e);
        }
    }
}
