/*
 * Copyright (c) 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.vnode.xml;

import static org.junit.Assert.assertEquals;

import static org.opendaylight.vtn.manager.util.NumberUtils.MASK_SHORT;

import static org.opendaylight.vtn.manager.internal.TestBase.MAX_UNSIGNED_SHORT;
import static org.opendaylight.vtn.manager.internal.TestBase.createInteger;
import static org.opendaylight.vtn.manager.internal.TestBase.createUnsignedShort;
import static org.opendaylight.vtn.manager.internal.TestBase.createVtnIndex;
import static org.opendaylight.vtn.manager.internal.routing.xml.XmlPathMapTest.XML_ROOT;
import static org.opendaylight.vtn.manager.internal.util.pathpolicy.PathPolicyUtilsTest.PATH_POLICY_MAX;

import java.util.Random;

import org.opendaylight.vtn.manager.internal.routing.xml.XmlPathMap;

import org.opendaylight.vtn.manager.internal.XmlNode;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathmap.rev150328.vtn.path.map.list.VtnPathMap;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathmap.rev150328.vtn.path.map.list.VtnPathMapBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeName;

/**
 * {@code PathMapConfig} describes configuration about a path map.
 */
public final class PathMapConfig {
    /**
     * An index value assigned to this path map.
     */
    private final Integer  index;

    /**
     * The name of the flow condition that selects packets.
     */
    private final String  condition;

    /**
     * The identifier of the path policy which determines the route of packets.
     */
    private final Integer  policy;

    /**
     * The number of seconds to be configured in {@code idle_timeout} field
     * in flow entries.
     */
    private Integer  idleTimeout;

    /**
     * The number of seconds to be configured in {@code hard_timeout} field
     * in flow entries.
     */
    private Integer  hardTimeout;

    /**
     * Construct a new instance.
     *
     * @param idx   The index value for the path map.
     * @param cond  The name of the flow condition.
     * @param pid   The identifier of the path policy.
     */
    public PathMapConfig(Integer idx, String cond, Integer pid) {
        index = idx;
        condition = cond;
        policy = pid;
    }

    /**
     * Construct a new instance using the given random number generator.
     *
     * @param rand  A pseudo random number generator.
     */
    public PathMapConfig(Random rand) {
        index = createVtnIndex(rand, null);
        condition = "cond_" + rand.nextInt(100000);

        int pid = rand.nextInt(PATH_POLICY_MAX + 2);
        policy = (pid > PATH_POLICY_MAX) ? null : pid;

        int n = rand.nextInt(6);
        if (n == 1) {
            idleTimeout = 0;
            hardTimeout = 0;
        } else if (n == 2) {
            idleTimeout = createUnsignedShort(rand);
            hardTimeout = 0;
        } else if (n == 3) {
            idleTimeout = 0;
            hardTimeout = createUnsignedShort(rand);
        } else if (n > 3) {
            idleTimeout = createInteger(rand, 1, MASK_SHORT);
            int idle = idleTimeout.intValue();
            if (idle == MASK_SHORT) {
                hardTimeout = MAX_UNSIGNED_SHORT;
            } else {
                hardTimeout =
                    createInteger(rand, idle + 1, MAX_UNSIGNED_SHORT);
            }
        }
    }

    /**
     * Return an index number assigned to this path map.
     *
     * @return  An {@link Integer} instance which indicates the index number.
     */
    public Integer getIndex() {
        return index;
    }

    /**
     * Return the name of the flow condition that selects packets.
     *
     * @return  The name of the flow condition.
     */
    public String getCondition() {
        return condition;
    }

    /**
     * Return the identifier of the path policy which determines the route of
     * packets.
     *
     * @return  An {@link Integer} instance which indicates the path policy
     *          identifier if configured. {@code null} if not configured.
     */
    public Integer getPolicy() {
        return policy;
    }

    /**
     * Return the number of seconds to be set to {@code idle-timeout} field in
     * flow entries.
     *
     * @return  An {@link Integer} instance which indicates the value for
     *          {@code idle-timeout} if configured.
     *          {@code null} if not configured.
     */
    public Integer getIdleTimeout() {
        return idleTimeout;
    }

    /**
     * Set the number of seconds to be set to {@code idle-timeout} field in
     * flow entries.
     *
     * @param idle  The {@code idle-timeout} value.
     * @return  This instance.
     */
    public PathMapConfig setIdleTimeout(Integer idle) {
        idleTimeout = idle;
        return this;
    }

    /**
     * Return the number of seconds to be set to {@code hard-timeout} field in
     * flow entries.
     *
     * @return  An {@link Integer} instance which indicates the value for
     *          {@code hard-timeout} if configured.
     *          {@code null} if not configured.
     */
    public Integer getHardTimeout() {
        return hardTimeout;
    }

    /**
     * Set the number of seconds to be set to {@code hard-timeout} field in
     * flow entries.
     *
     * @param hard  The {@code hard-timeout} value.
     * @return  This instance.
     */
    public PathMapConfig setHardTimeout(Integer hard) {
        hardTimeout = hard;
        return this;
    }

    /**
     * Convert this instance into a vtn-path-map instance.
     *
     * @return  A {@link VtnPathMap} instance.
     */
    public VtnPathMap toVtnPathMap() {
        return new VtnPathMapBuilder().
            setIndex(index).
            setCondition(new VnodeName(condition)).
            setPolicy(policy).
            setIdleTimeout(idleTimeout).
            setHardTimeout(hardTimeout).
            build();
    }

    /**
     * Ensure that the given {@link XmlPathMap} instance is identical to
     * this instance.
     *
     * @param xpm   A {@link XmlPathMap} instance.
     */
    public void verify(XmlPathMap xpm) {
        assertEquals(index, xpm.getIndex());
        assertEquals(condition, xpm.getCondition().getValue());
        assertEquals(policy, xpm.getPolicy());
        assertEquals(idleTimeout, xpm.getIdleTimeout());
        assertEquals(hardTimeout, xpm.getHardTimeout());
    }

    /**
     * Convert this instance into a XML node.
     * @return  A {@link XmlNode} instance that indicates the vBridge
     *          configuration in this instance.
     */
    public XmlNode toXmlNode() {
        XmlNode xnode = new XmlNode(XML_ROOT).
            add(new XmlNode("index", index)).
            add(new XmlNode("condition", condition));

        if (policy != null) {
            xnode.add(new XmlNode("policy", policy));
        }
        if (idleTimeout != null) {
            xnode.add(new XmlNode("idle-timeout", idleTimeout));
        }
        if (hardTimeout != null) {
            xnode.add(new XmlNode("hard-timeout", hardTimeout));
        }

        return xnode;
    }
}
