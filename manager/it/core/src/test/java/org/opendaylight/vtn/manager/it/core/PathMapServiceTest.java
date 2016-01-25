/*
 * Copyright (c) 2015, 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.it.core;

import static org.opendaylight.vtn.manager.it.core.VTNManagerIT.LOG;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map.Entry;
import java.util.Map;
import java.util.Random;
import java.util.Set;

import org.opendaylight.vtn.manager.it.util.RpcErrorTag;
import org.opendaylight.vtn.manager.it.util.VirtualNetwork;
import org.opendaylight.vtn.manager.it.util.pathmap.PathMap;
import org.opendaylight.vtn.manager.it.util.pathmap.PathMapSet;
import org.opendaylight.vtn.manager.it.util.vnode.VTenantConfig;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathmap.rev150328.ClearPathMapInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathmap.rev150328.ClearPathMapInputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathmap.rev150328.RemovePathMapInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathmap.rev150328.RemovePathMapInputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathmap.rev150328.SetPathMapInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathmap.rev150328.SetPathMapInputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathmap.rev150328.VtnPathMapService;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathmap.rev150328.set.path.map.input.PathMapList;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnErrorTag;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateType;

/**
 * Test case for {@link VtnPathMapService}.
 */
public final class PathMapServiceTest extends TestMethodBase {
    /**
     * The number used to generate random path policy ID.
     */
    private static final int  PATH_POLICY_BOUNDARY = PATH_POLICY_ID_MAX + 2;

    /**
     * Construct a new instance.
     *
     * @param vit  A {@link VTNManagerIT} instance.
     */
    public PathMapServiceTest(VTNManagerIT vit) {
        super(vit);
    }

    /**
     * Generate a random path policy ID.
     *
     * @param rand  A pseudo random generator.
     * @return  An integer that specifies the path policy or {@code null}.
     */
    private Integer randomPolicy(Random rand) {
        int num = rand.nextInt(PATH_POLICY_BOUNDARY);
        return (num <= PATH_POLICY_ID_MAX) ? Integer.valueOf(num) : null;
    }

    /**
     * Test case for {@link VtnPathMapService}.
     *
     * @param pmSrv  vtn-path-map service.
     * @param rand   A pseudo random generator.
     * @param tname  The name of the target VTN.
     *               {@code null} indicates the global path map.
     * @throws Exception  An error occurred.
     */
    private void testPathMapService(VtnPathMapService pmSrv, Random rand,
                                    String tname) throws Exception {
        LOG.debug("testPathMapService: VTN={}",
                  (tname == null) ? "<global>" : tname);

        VirtualNetwork vnet = getVirtualNetwork();
        PathMapSet pmSet;
        if (tname == null) {
            // The target is the global path map.
            pmSet = vnet.getPathMaps();
        } else {
            // Ensure that path map RPCs return NOTFOUND error if the specified
            // VTN is not present.
            notFoundTest(pmSrv, rand, tname);

            // Errors should never affect path maps.
            vnet.verify();

            // Create the target VTN.
            VTenantConfig tconf = new VTenantConfig();
            vnet.addTenant(tname, tconf).apply();
            pmSet = tconf.getPathMaps();
        }

        // Configure path maps.
        Set<Integer> idxSet = new HashSet<>();
        PathMap pmap1 = new PathMap(VTN_INDEX_MIN, "cond_1");
        assertEquals(true, idxSet.add(pmap1.getIndex()));
        assertEquals(VtnUpdateType.CREATED, pmSet.addMap(pmSrv, tname, pmap1));
        vnet.verify();

        PathMap pmap2 = new PathMap(VTN_INDEX_MAX, "cond_2",
                                    PATH_POLICY_ID_MAX);
        assertEquals(true, idxSet.add(pmap2.getIndex()));
        Map<Integer, VtnUpdateType> pmResult = new HashMap<>();
        assertEquals(null, pmResult.put(pmap1.getIndex(), null));
        assertEquals(null,
                     pmResult.put(pmap2.getIndex(), VtnUpdateType.CREATED));
        pmSet.add(pmap2);
        assertEquals(pmResult, pmSet.update(pmSrv, tname));
        vnet.verify();

        PathMap pmap3 = new PathMap(
            createVtnIndex(rand, idxSet), "cond_3", 0, 0, 0);
        PathMap pmap4 = new PathMap(
            createVtnIndex(rand, idxSet), "cond_4", randomPolicy(rand),
            65534, 65535);
        PathMap pmap5 = new PathMap(
            createVtnIndex(rand, idxSet), "cond_5", randomPolicy(rand),
            100, 101);
        PathMap pmap6 = new PathMap(
            createVtnIndex(rand, idxSet), "cond_6", randomPolicy(rand),
            3333, 4444);
        pmResult.clear();
        assertEquals(null,
                     pmResult.put(pmap3.getIndex(), VtnUpdateType.CREATED));
        assertEquals(null,
                     pmResult.put(pmap4.getIndex(), VtnUpdateType.CREATED));
        assertEquals(null,
                     pmResult.put(pmap5.getIndex(), VtnUpdateType.CREATED));
        assertEquals(null,
                     pmResult.put(pmap6.getIndex(), VtnUpdateType.CREATED));
        assertEquals(pmResult,
                     pmSet.addMaps(pmSrv, tname, pmap3, pmap4, pmap5, pmap6));
        vnet.verify();

        // Update pmap1.
        pmap1.setPolicy(PATH_POLICY_ID_MIN);
        assertEquals(VtnUpdateType.CHANGED, pmSet.addMap(pmSrv, tname, pmap1));
        vnet.verify();

        // Update pmap2 and pmap6.
        pmResult.clear();
        pmap2.setPolicy(null).
            setCondition("cond_22").
            setIdleTimeout(12345).
            setHardTimeout(23456);
        assertEquals(null,
                     pmResult.put(pmap2.getIndex(), VtnUpdateType.CHANGED));

        pmap6.setIdleTimeout(1000).
            setHardTimeout(0);
        assertEquals(null,
                     pmResult.put(pmap6.getIndex(), VtnUpdateType.CHANGED));

        assertEquals(null, pmResult.put(pmap1.getIndex(), null));
        assertEquals(null, pmResult.put(pmap3.getIndex(), null));
        assertEquals(pmResult,
                     pmSet.addMaps(pmSrv, tname, pmap1, pmap2, pmap3, pmap6));
        vnet.verify();

        // Update pmap3, and create pmap7.
        pmResult.clear();
        pmap3.setPolicy(PATH_POLICY_ID_MIN + 1).
            setIdleTimeout(null).
            setHardTimeout(null);
        assertEquals(null,
                     pmResult.put(pmap3.getIndex(), VtnUpdateType.CHANGED));

        PathMap pmap7 = new PathMap(
            createVtnIndex(rand, idxSet), "cond_7", randomPolicy(rand),
            9999, 10000);
        assertEquals(null,
                     pmResult.put(pmap7.getIndex(), VtnUpdateType.CREATED));

        assertEquals(null, pmResult.put(pmap5.getIndex(), null));
        assertEquals(pmResult,
                     pmSet.addMaps(pmSrv, tname, pmap3, pmap5, pmap7));
        vnet.verify();

        // Remove pmap4.
        assertEquals(VtnUpdateType.REMOVED,
                     pmSet.removeMap(pmSrv, tname, pmap4));
        vnet.verify();

        // Remove pmap2 and pmap5.
        List<Integer> indices = new ArrayList<>();
        pmResult.clear();
        assertEquals(null,
                     pmResult.put(pmap2.getIndex(), VtnUpdateType.REMOVED));
        assertEquals(null,
                     pmResult.put(pmap5.getIndex(), VtnUpdateType.REMOVED));
        assertEquals(null, pmResult.put(pmap4.getIndex(), null));

        // Duplicate indices should be ignored.
        Collections.addAll(
            indices, pmap2.getIndex(), pmap4.getIndex(), pmap5.getIndex(),
            pmap2.getIndex(), pmap4.getIndex(), pmap5.getIndex());

        // Invalid indices in remove-path-map input should be ignored.
        for (int idx = -10; idx <= 10; idx++) {
            if (idx < VTN_INDEX_MIN || idx > VTN_INDEX_MAX) {
                pmResult.put(idx, null);
                indices.add(idx);
            }
        }

        assertEquals(pmResult, pmSet.removeMaps(pmSrv, tname, indices));
        vnet.verify();

        // Error tests.

        // No path map in set-path-map input.
        SetPathMapInput input = new SetPathMapInputBuilder().
            setTenantName(tname).
            build();
        checkRpcError(pmSrv.setPathMap(input),
                      RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);

        input = new SetPathMapInputBuilder().
            setTenantName(tname).
            setPathMapList(Collections.<PathMapList>emptyList()).
            build();
        checkRpcError(pmSrv.setPathMap(input),
                      RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);

        // Null path map.
        input = new SetPathMapInputBuilder().
            setTenantName(tname).
            setPathMapList(Collections.singletonList((PathMapList)null)).
            build();
        checkRpcError(pmSrv.setPathMap(input),
                      RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);

        // No path map index.
        String cond = "cond";
        PathMap pmap = new PathMap(null, cond);
        input = PathMapSet.newInputBuilder(pmap).
            setTenantName(tname).
            build();
        checkRpcError(pmSrv.setPathMap(input),
                      RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);

        // No flow condition name.
        Integer index = 1;
        pmap = new PathMap(index, null);
        input = PathMapSet.newInputBuilder(pmap).
            setTenantName(tname).
            build();
        checkRpcError(pmSrv.setPathMap(input),
                      RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);

        Integer policy = PATH_POLICY_ID_MIN;
        int[] timeouts = {1, 3000, 65535};
        for (int t: timeouts) {
            // Specifying idle timeout without hard timeout.
            Integer timeout = Integer.valueOf(t);
            pmap = new PathMap(index, cond, policy, timeout, null);
            input = PathMapSet.newInputBuilder(pmap).
                setTenantName(tname).
                build();
            checkRpcError(pmSrv.setPathMap(input),
                          RpcErrorTag.BAD_ELEMENT, VtnErrorTag.BADREQUEST);

            // Specifying hard timeout without specifying idle timeout.
            pmap = new PathMap(index, cond, policy, null, timeout);
            input = PathMapSet.newInputBuilder(pmap).
                setTenantName(tname).
                build();
            checkRpcError(pmSrv.setPathMap(input),
                          RpcErrorTag.BAD_ELEMENT, VtnErrorTag.BADREQUEST);

            // Inconsistent timeout.
            pmap = new PathMap(index, cond, policy, timeout, timeout);
            input = PathMapSet.newInputBuilder(pmap).
                setTenantName(tname).
                build();
            checkRpcError(pmSrv.setPathMap(input),
                          RpcErrorTag.BAD_ELEMENT, VtnErrorTag.BADREQUEST);

            if (timeout != 65535) {
                pmap = new PathMap(index, cond, policy, t + 1, timeout);
                input = PathMapSet.newInputBuilder(pmap).
                    setTenantName(tname).
                    build();
                checkRpcError(pmSrv.setPathMap(input),
                              RpcErrorTag.BAD_ELEMENT, VtnErrorTag.BADREQUEST);
            }
        }

        // Duplicate path map index.
        input = PathMapSet.newInputBuilder(
            new PathMap(index, cond),
            new PathMap(VTN_INDEX_MAX, cond, policy),
            new PathMap(index, cond, policy)).
            build();
        checkRpcError(pmSrv.setPathMap(input),
                      RpcErrorTag.BAD_ELEMENT, VtnErrorTag.BADREQUEST);

        // No path map index in remove-path-map input.
        RemovePathMapInput rinput = new RemovePathMapInputBuilder().
            setTenantName(tname).
            build();
        checkRpcError(pmSrv.removePathMap(rinput),
                      RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);

        rinput = new RemovePathMapInputBuilder().
            setTenantName(tname).
            setMapIndex(Collections.<Integer>emptyList()).
            build();
        checkRpcError(pmSrv.removePathMap(rinput),
                      RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);

        // Null map index.
        rinput = new RemovePathMapInputBuilder().
            setTenantName(tname).
            setMapIndex(Collections.singletonList((Integer)null)).
            build();
        checkRpcError(pmSrv.removePathMap(rinput),
                      RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);

        // Errors should never affect path maps.
        vnet.verify();
    }

    /**
     * Ensure that path map RPCs return NOTFOUND error if the specified VTN
     * is not present.
     *
     * @param pmSrv  vtn-path-map service.
     * @param rand   A pseudo random generator.
     * @param tname  The name of the target VTN.
     * @throws Exception  An error occurred.
     */
    private void notFoundTest(VtnPathMapService pmSrv, Random rand,
                              String tname) throws Exception {
        PathMapSet pset = new PathMapSet().add(rand);
        List<Integer> idxList = Collections.singletonList(Integer.valueOf(1));

        SetPathMapInput input = pset.newInputBuilder().
            setTenantName(tname).
            build();
        checkRpcError(pmSrv.setPathMap(input),
                      RpcErrorTag.DATA_MISSING, VtnErrorTag.NOTFOUND);

        RemovePathMapInput rinput = new RemovePathMapInputBuilder().
            setTenantName(tname).
            setMapIndex(idxList).
            build();
        checkRpcError(pmSrv.removePathMap(rinput),
                      RpcErrorTag.DATA_MISSING, VtnErrorTag.NOTFOUND);

        ClearPathMapInput cinput = new ClearPathMapInputBuilder().
            setTenantName(tname).
            build();
        checkRpcError(pmSrv.clearPathMap(cinput),
                      RpcErrorTag.DATA_MISSING, VtnErrorTag.NOTFOUND);
    }

    // TestMethodBase

    /**
     * Run the test.
     *
     * @throws Exception  An error occurred.
     */
    @Override
    protected void runTest() throws Exception {
        Random rand = new Random(111222333444555L);
        VTNManagerIT vit = getTest();
        VtnPathMapService pmSrv = vit.getPathMapService();

        // Test for global path map.
        testPathMapService(pmSrv, rand, null);

        // Test for VTN path map.
        String[] tenants = {
            "vtn_1", "vtn_2", "vtn_3",
        };
        for (String tname: tenants) {
            testPathMapService(pmSrv, rand, tname);
        }

        // Error tests.

        // Null input.
        checkRpcError(pmSrv.setPathMap(null),
                      RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);
        checkRpcError(pmSrv.removePathMap(null),
                      RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);
        checkRpcError(pmSrv.clearPathMap(null),
                      RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);

        // Invalid VTN name.
        for (String tname: INVALID_VNODE_NAMES) {
            notFoundTest(pmSrv, rand, tname);
        }

        // Errors should never affect path maps.
        VirtualNetwork vnet = getVirtualNetwork();
        vnet.verify();

        // Remove all the VTN path maps using remove-path-map.
        Map<String, PathMapSet> vtnMaps = new HashMap<>();
        for (String tname: tenants) {
            PathMapSet pmSet = vnet.getTenant(tname).getPathMaps();
            assertEquals(null, vtnMaps.put(tname, pmSet.clone()));
            pmSet.removeAll(pmSrv, tname);
            vnet.verify();
        }

        // Remove all the global path maps using remove-path-map.
        PathMapSet gpmSet = vnet.getPathMaps();
        PathMapSet gpmSaved = gpmSet.clone();
        gpmSet.removeAll(pmSrv, null);
        vnet.verify();

        // Restore all the global and VTN path maps.
        gpmSet.add(gpmSaved).restore(pmSrv, null);
        vnet.verify();

        for (Entry<String, PathMapSet> entry: vtnMaps.entrySet()) {
            String tname = entry.getKey();
            PathMapSet saved = entry.getValue();
            PathMapSet pmSet = vnet.getTenant(tname).getPathMaps();
            pmSet.add(saved).restore(pmSrv, tname);
            vnet.verify();
        }

        // Remove all the global path maps using clear-path-map.
        assertEquals(VtnUpdateType.REMOVED, clearPathMap(null));
        gpmSet.clear();
        vnet.verify();
        assertEquals(null, clearPathMap(null));

        // Remove all the VTN path maps using clear-path-map.
        for (String tname: tenants) {
            PathMapSet pmSet = vnet.getTenant(tname).getPathMaps();
            assertEquals(VtnUpdateType.REMOVED, clearPathMap(tname));
            pmSet.clear();
            vnet.verify();
            assertEquals(null, clearPathMap(tname));
        }

        // Restore VTN path maps again.
        for (Entry<String, PathMapSet> entry: vtnMaps.entrySet()) {
            String tname = entry.getKey();
            PathMapSet saved = entry.getValue();
            PathMapSet pmSet = vnet.getTenant(tname).getPathMaps();
            pmSet.add(saved).restore(pmSrv, tname);
            vnet.verify();
        }

        // Remove VTNs.
        for (String tname: tenants) {
            removeVtn(tname);
            vnet.removeTenant(tname).verify();
        }
    }
}
