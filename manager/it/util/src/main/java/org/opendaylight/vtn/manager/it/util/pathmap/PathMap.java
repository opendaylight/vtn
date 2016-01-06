/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.it.util.pathmap;

import static org.junit.Assert.assertEquals;

import static org.opendaylight.vtn.manager.it.util.ModelDrivenTestBase.getRpcOutput;
import static org.opendaylight.vtn.manager.it.util.TestBase.createPathPolicyReference;
import static org.opendaylight.vtn.manager.it.util.TestBase.createUnsignedShort;
import static org.opendaylight.vtn.manager.it.util.TestBase.createVnodeName;
import static org.opendaylight.vtn.manager.it.util.TestBase.createVtnIndex;

import java.util.Collections;
import java.util.List;
import java.util.Map;
import java.util.Random;
import java.util.Set;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathmap.rev150328.SetPathMapInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathmap.rev150328.SetPathMapInputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathmap.rev150328.SetPathMapOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathmap.rev150328.VtnPathMapService;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathmap.rev150328.set.path.map.input.PathMapList;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathmap.rev150328.set.path.map.input.PathMapListBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathmap.rev150328.vtn.path.map.list.VtnPathMap;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeName;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateType;

/**
 * {@code PathMap} describes the configuration of a path map.
 */
public final class PathMap {
    /**
     * The default value of policy.
     */
    public static final Integer  DEFAULT_POLICY = 0;

    /**
     * The value of index.
     */
    private final Integer  index;

    /**
     * The value of condition.
     */
    private String  condition;

    /**
     * The value of policy.
     */
    private Integer  policy;

    /**
     * The value of idle-timeout.
     */
    private Integer  idleTimeout;

    /**
     * The value of hard-timeout.
     */
    private Integer  hardTimeout;

    /**
     * Construct a new instance without specifying policy ID and flow timeout.
     *
     * @param idx   The index of the path map.
     * @param cond  The name of the flow condition.
     */
    public PathMap(Integer idx, String cond) {
        this(idx, cond, null, null, null);
    }

    /**
     * Construct a new instance without specifying flow timeout.
     *
     * @param idx   The index of the path map.
     * @param cond  The name of the flow condition.
     * @param pid   The identifier for the path policy.
     */
    public PathMap(Integer idx, String cond, Integer pid) {
        this(idx, cond, pid, null, null);
    }

    /**
     * Construct a new instance.
     *
     * @param idx   The index of the path map.
     * @param cond  The name of the flow condition.
     * @param pid   The identifier for the path policy.
     * @param idle  The value of idle-timeout.
     * @param hard  The value of hard-timeout.
     */
    public PathMap(Integer idx, String cond, Integer pid, Integer idle,
                   Integer hard) {
        index = idx;
        condition = cond;
        policy = pid;
        idleTimeout = idle;
        hardTimeout = hard;
    }

    /**
     * Construct a new instance with specifying random parameters.
     *
     * @param rand     A pseudo random generator.
     * @param indices  A set of index values to store generated values.
     */
    public PathMap(Random rand, Set<Integer> indices) {
        index = createVtnIndex(rand, indices);
        condition = createVnodeName("cond_", rand);
        policy = createPathPolicyReference(rand);

        int idle = createUnsignedShort(rand);
        int hard = createUnsignedShort(rand);
        if (idle >= hard) {
            idleTimeout = null;
            hardTimeout = null;
        } else {
            idleTimeout = idle;
            hardTimeout = hard;
        }
    }

    /**
     * Return the index of the path map.
     *
     * @return  The index of the path map.
     */
    public Integer getIndex() {
        return index;
    }

    /**
     * Return the name of the flow condition.
     *
     * @return  The name of the flow condition.
     */
    public String getCondition() {
        return condition;
    }

    /**
     * Set the name of the flow condition.
     *
     * @param cond  The name of the flow condition.
     * @return  This instance.
     */
    public PathMap setCondition(String cond) {
        condition = cond;
        return this;
    }

    /**
     * Return the identifier for the path policy.
     *
     * @return  The identifier for the path policy.
     */
    public Integer getPolicy() {
        return policy;
    }

    /**
     * Set the identifier for the path policy.
     *
     * @param id  The identifier for the path policy.
     * @return  This instance.
     */
    public PathMap setPolicy(Integer id) {
        policy = id;
        return this;
    }

    /**
     * Return the value of idle-timeout.
     *
     * @return  The value of idle-timeout.
     */
    public Integer getIdleTimeout() {
        return idleTimeout;
    }

    /**
     * Set the value of idle-timeout.
     *
     * @param idle  The value of the idle-timeout.
     * @return  This instance.
     */
    public PathMap setIdleTimeout(Integer idle) {
        idleTimeout = idle;
        return this;
    }

    /**
     * Return the value of hard-timeout.
     *
     * @return  The value of hard-timeout.
     */
    public Integer getHardTimeout() {
        return hardTimeout;
    }

    /**
     * Set the value of hard-timeout.
     *
     * @param hard  The value of the hard-timeout.
     * @return  This instance.
     */
    public PathMap setHardTimeout(Integer hard) {
        hardTimeout = hard;
        return this;
    }

    /**
     * Update the global path map.
     *
     * @param service  The vtn-path-map service.
     * @return  A {@link VtnUpdateType} instance that indicates the result
     *          of RPC.
     */
    public VtnUpdateType update(VtnPathMapService service) {
        return update(service, null);
    }

    /**
     * Update the path map.
     *
     * @param service  The vtn-path-map service.
     * @param tname    The name of the VTN.
     *                 {@code null} implies the global path map.
     * @return  A {@link VtnUpdateType} instance that indicates the result
     *          of RPC.
     */
    public VtnUpdateType update(VtnPathMapService service, String tname) {
        List<PathMapList> pmaps = Collections.singletonList(toPathMapList());
        SetPathMapInput input = new SetPathMapInputBuilder().
            setTenantName(tname).
            setPathMapList(pmaps).
            build();

        SetPathMapOutput output = getRpcOutput(service.setPathMap(input));
        Map<Integer, VtnUpdateType> result =
            PathMapSet.getResultMap(output.getSetPathMapResult());
        assertEquals(Collections.<Integer>singleton(index), result.keySet());
        return result.get(index);
    }

    /**
     * Verify the given path map.
     *
     * @param vpm  The path map to be verified.
     */
    public void verify(VtnPathMap vpm) {
        assertEquals(index, vpm.getIndex());
        assertEquals(condition, vpm.getCondition().getValue());
        Integer pid = (policy == null) ? DEFAULT_POLICY : policy;
        assertEquals(pid, vpm.getPolicy());
        assertEquals(idleTimeout, vpm.getIdleTimeout());
        assertEquals(hardTimeout, vpm.getHardTimeout());
    }

    /**
     * Convert this instance into a {@link PathMapList} instance.
     *
     * @return  A {@link PathMapList} instance.
     */
    public PathMapList toPathMapList() {
        VnodeName vcond = (condition == null)
            ? null : new VnodeName(condition);
        return new PathMapListBuilder().
            setIndex(index).
            setCondition(vcond).
            setPolicy(policy).
            setIdleTimeout(idleTimeout).
            setHardTimeout(hardTimeout).
            build();
    }
}
