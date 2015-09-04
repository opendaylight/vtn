/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.config;

import java.util.ArrayList;
import java.util.List;
import java.util.Objects;

import javax.xml.bind.annotation.XmlAccessType;
import javax.xml.bind.annotation.XmlAccessorType;
import javax.xml.bind.annotation.XmlElement;
import javax.xml.bind.annotation.XmlRootElement;

import org.opendaylight.vtn.manager.util.EtherAddress;

import org.opendaylight.vtn.manager.internal.VTNConfig;
import org.opendaylight.vtn.manager.internal.util.MiscUtils;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.config.rev150209.VtnConfig;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.config.rev150209.VtnConfigBuilder;

import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.yang.types.rev100924.MacAddress;

/**
 * Implementation of {@code VTNConfig} interface.
 *
 * <p>
 *   This class has XML bindings, so an instance of this class can be
 *   serialized as XML.
 * </p>
 */
@XmlRootElement(name = "vtn-config")
@XmlAccessorType(XmlAccessType.NONE)
public final class VTNConfigImpl implements VTNConfig {
    /**
     * An integer value which represents an undefined value.
     */
    private static final int  UNDEFINED = -1;

    /**
     * Default value for "node-edge-wait".
     */
    private static final int  DEFAULT_NODE_EDGE_WAIT = 3000;

    /**
     * Default value for "l2-flow-priority".
     */
    private static final int  DEFAULT_L2_FLOW_PRIORITY = 10;

    /**
     * Default value for "flow-mod-timeout".
     */
    private static final int  DEFAULT_FLOW_MOD_TIMEOUT = 3000;

    /**
     * Default value for "bulk-flow-mod-timeout".
     */
    private static final int  DEFAULT_BULK_FLOW_MOD_TIMEOUT = 10000;

    /**
     * Default value for "init-timeout".
     */
    private static final int  DEFAULT_INIT_TIMEOUT = 3000;

    /**
     * Default value for "max-redirections".
     */
    private static final int  DEFAULT_MAX_REDIRECTIONS = 100;

    /**
     * Default MAC address of the local node.
     */
    private static final EtherAddress  DEFAULT_MAC_ADDRESS =
        new EtherAddress(0x00000c600d10L);

    /**
     * A string that represents a right arrow.
     */
    private static final String  RIGHT_ARROW = "->";

    /**
     * The number of milliseconds to wait for node edges to be detected.
     */
    private int  nodeEdgeWait;

    /**
     * Priority value of layer 2 flow entries.
     */
    private int  l2FlowPriority;

    /**
     * The number of milliseconds to wait for completion of a single FLOW_MOD
     * request.
     */
    private int  flowModTimeout;

    /**
     * The number of milliseconds to wait for completion of bulk FLOW_MOD
     * requests.
     */
    private int  bulkFlowModTimeout;

    /**
     * The number of milliseconds to wait for another controller in the cluster
     * to complete initialization.
     */
    private int  initTimeout;

    /**
     * The maximum number of virtual node hops per a flow.
     * In other words, the maximum number of packet redirections by REDIRECT
     * flow filter per a flow.
     */
    private int  maxRedirections;

    /**
     * MAC address of the controller used as source MAC address of ARP packet.
     * It is determined by the controller if omitted.
     */
    @XmlElement(name = "controller-mac-address")
    private EtherAddress  controllerMacAddress;

    /**
     * A {@link VtnConfigBuilder} that contains values loaded via JAXB.
     */
    private VtnConfigBuilder  jaxbValue;

    /**
     * Fill the given configuration with default values.
     *
     * @param builder  A {@link VtnConfigBuilder} instance.
     * @param mac      MAC address of the local node.
     * @return  {@code builder} is always returned.
     */
    public static VtnConfigBuilder fillDefault(VtnConfigBuilder builder,
                                               EtherAddress mac) {
        if (builder.getNodeEdgeWait() == null) {
            builder.setNodeEdgeWait(DEFAULT_NODE_EDGE_WAIT);
        }

        if (builder.getL2FlowPriority() == null) {
            builder.setL2FlowPriority(DEFAULT_L2_FLOW_PRIORITY);
        }

        if (builder.getFlowModTimeout() == null) {
            builder.setFlowModTimeout(DEFAULT_FLOW_MOD_TIMEOUT);
        }

        if (builder.getBulkFlowModTimeout() == null) {
            builder.setBulkFlowModTimeout(DEFAULT_BULK_FLOW_MOD_TIMEOUT);
        }

        if (builder.getInitTimeout() == null) {
            builder.setInitTimeout(DEFAULT_INIT_TIMEOUT);
        }

        if (builder.getMaxRedirections() == null) {
            builder.setMaxRedirections(DEFAULT_MAX_REDIRECTIONS);
        }

        if (builder.getControllerMacAddress() == null) {
            builder.setControllerMacAddress(mac.getMacAddress());
        }

        return builder;
    }

    /**
     * Create a new {@link VtnConfigBuilder} instance filled with default
     * values.
     *
     * @param mac  MAC address of the local node.
     * @return  A {@link VtnConfigBuilder} instance.
     */
    public static VtnConfigBuilder builder(EtherAddress mac) {
        return fillDefault(new VtnConfigBuilder(), mac);
    }

    /**
     * Create a new {@link VtnConfigBuilder} instance that contains the given
     * parameters.
     *
     * <ul>
     *   <li>
     *     If a {@code null} is specified to {@code vcfg}, this method returns
     *     a {@link VtnConfigBuilder} instance filled with default values.
     *   </li>
     *   <li>
     *     If a {@link VtnConfig} is specified to {@code vcfg}, this method
     *     returns a {@link VtnConfigBuilder} instance which contains the
     *     given parameter. Missing parameters in {@code vcfg} will be filled
     *     with default values.
     *   </li>
     * </ul>
     *
     * @param vcfg  A {@link VtnConfig} instance or {@code null}.
     * @param mac   MAC address of the local node.
     * @return  A {@link VtnConfigBuilder} instance.
     */
    public static VtnConfigBuilder builder(VtnConfig vcfg, EtherAddress mac) {
        VtnConfigBuilder b = new VtnConfigBuilder();
        if (vcfg != null) {
            // This code can be more simplified if RESTCONF implements
            // value restriction check.
            b.setNodeEdgeWait(vcfg.getNodeEdgeWait()).
                setL2FlowPriority(vcfg.getL2FlowPriority()).
                setFlowModTimeout(vcfg.getFlowModTimeout()).
                setBulkFlowModTimeout(vcfg.getBulkFlowModTimeout()).
                setInitTimeout(vcfg.getInitTimeout()).
                setMaxRedirections(vcfg.getMaxRedirections()).
                setControllerMacAddress(vcfg.getControllerMacAddress());
        }

        return fillDefault(b, mac);
    }

    /**
     * Create a diff of the given two configuration.
     *
     * @param oldConf  Old configuration.
     * @param newConf  New configuration.
     * @return  A string that represents difference of the given two instances.
     *          {@code null} if the given two instances are identical.
     */
    public static String diff(VTNConfig oldConf, VTNConfig newConf) {
        List<String> list = new ArrayList<>();

        diff(list, "node-edge-wait", oldConf.getNodeEdgeWait(),
             newConf.getNodeEdgeWait());
        diff(list, "l2-flow-priority", oldConf.getL2FlowPriority(),
             newConf.getL2FlowPriority());
        diff(list, "flow-mod-timeout", oldConf.getFlowModTimeout(),
             newConf.getFlowModTimeout());
        diff(list, "bulk-flow-mod-timeout",
             oldConf.getBulkFlowModTimeout(), newConf.getBulkFlowModTimeout());
        diff(list, "init-timeout", oldConf.getInitTimeout(),
             newConf.getInitTimeout());
        diff(list, "max-redirections", oldConf.getMaxRedirections(),
             newConf.getMaxRedirections());

        EtherAddress oldMac = oldConf.getControllerMacAddress();
        EtherAddress newMac = newConf.getControllerMacAddress();
        if (!oldMac.equals(newMac)) {
            String omac = oldMac.getText();
            String nmac = newMac.getText();
            StringBuilder builder =
                new StringBuilder("controller-mac-address=(");
            builder.append(omac).append(RIGHT_ARROW).append(nmac).append(')');
            list.add(builder.toString());
        }

        return (list.isEmpty()) ? null : MiscUtils.join(", ", list);
    }

    /**
     * Add a string that indicates the change of parameter to the given list.
     *
     * @param list  A list of strings that indicates diferrences of parameters.
     * @param name  The name of the parameter.
     * @param o     Old value of the parameter.
     * @param n     New value of the parameter.
     */
    private static void diff(List<String> list, String name, int o, int n) {
        if (o != n) {
            StringBuilder builder = new StringBuilder(name);
            builder.append("=(").append(o).append(RIGHT_ARROW).append(n).
                append(')');
            list.add(builder.toString());
        }
    }

    /**
     * Construct a new instance which contains default parameter values.
     *
     * <p>
     *   Note that this constructor never configures MAC address of the
     *   local node.
     * </p>
     */
    public VTNConfigImpl() {
        nodeEdgeWait = UNDEFINED;
        l2FlowPriority = UNDEFINED;
        flowModTimeout = UNDEFINED;
        bulkFlowModTimeout = UNDEFINED;
        initTimeout = UNDEFINED;
        maxRedirections = UNDEFINED;
    }

    /**
     * Construct a new instance which contains default parameter values.
     *
     * @param mac   MAC address of the local node.
     */
    public VTNConfigImpl(EtherAddress mac) {
        this();
        controllerMacAddress = mac;
    }

    /**
     * Construct a new instance.
     *
     * @param vcfg  A {@link VtnConfig} instance.
     */
    public VTNConfigImpl(VtnConfig vcfg) {
        this(vcfg, null);
    }

    /**
     * Construct a new instance.
     *
     * @param vcfg  A {@link VtnConfig} instance.
     * @param mac   MAC address of the local node.
     *              This parameter is used only if a non {@code null} value
     *              is specified and MAC address is not configured in
     *              {@code vcfg}.
     */
    public VTNConfigImpl(VtnConfig vcfg, EtherAddress mac) {
        nodeEdgeWait = decode(vcfg.getNodeEdgeWait());
        l2FlowPriority = decode(vcfg.getL2FlowPriority());
        flowModTimeout = decode(vcfg.getFlowModTimeout());
        bulkFlowModTimeout = decode(vcfg.getBulkFlowModTimeout());
        initTimeout = decode(vcfg.getInitTimeout());
        maxRedirections = decode(vcfg.getMaxRedirections());
        controllerMacAddress = decode(vcfg.getControllerMacAddress());
        if (mac != null && controllerMacAddress == null) {
            controllerMacAddress = mac;
        }
    }

    /**
     * Convert this instance into a {@link VtnConfig} instance.
     *
     * @return  A {@link VtnConfig} instance.
     */
    public VtnConfig toVtnConfig() {
        VtnConfigBuilder builder = new VtnConfigBuilder();
        builder.setNodeEdgeWait(encode(nodeEdgeWait)).
            setL2FlowPriority(encode(l2FlowPriority)).
            setFlowModTimeout(encode(flowModTimeout)).
            setBulkFlowModTimeout(encode(bulkFlowModTimeout)).
            setInitTimeout(encode(initTimeout)).
            setMaxRedirections(encode(maxRedirections)).
            setControllerMacAddress(encode(controllerMacAddress));

        return builder.build();
    }

    /**
     * Return a {@link VtnConfigBuilder} that contains values loaded via
     * JAXB.
     *
     * @return  A {@link VtnConfigBuilder} instance if this instance is created
     *          by JAXB. Note that an empty {@link VtnConfigBuilder} is
     *          returned if no value is configured via JAXB.
     */
    public VtnConfigBuilder getJaxbValue() {
        return createJaxbValue().
            setControllerMacAddress(encode(controllerMacAddress));
    }

    /**
     * Determine whether flow related parameters in the given configuration
     * are identical to parameters in this instance.
     *
     * @param vconf  A {@link VTNConfigImpl} instance to be compared.
     * @return  {@code true} only if flow related parameters in {@code vconf}
     *          are identical to parameters in this instance.
     */
    private boolean equalsFlowParams(VTNConfigImpl vconf) {
        return (l2FlowPriority == vconf.l2FlowPriority &&
                flowModTimeout == vconf.flowModTimeout &&
                bulkFlowModTimeout == vconf.bulkFlowModTimeout);
    }

    // Object

    /**
     * Determine whether the given object is identical to this object.
     *
     * @param o  An object to be compared.
     * @return   {@code true} if identical. Otherwise {@code false}.
     */
    @Override
    public boolean equals(Object o) {
        if (o == this) {
            return true;
        }

        boolean result = false;
        if (o != null && getClass().equals(o.getClass())) {
            VTNConfigImpl vconf = (VTNConfigImpl)o;
            if (nodeEdgeWait == vconf.nodeEdgeWait &&
                initTimeout == vconf.initTimeout &&
                maxRedirections == vconf.maxRedirections) {
                result = (equalsFlowParams(vconf) &&
                          Objects.equals(controllerMacAddress,
                                         vconf.controllerMacAddress));
            }
        }

        return result;
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public int hashCode() {
        return Objects.hash(nodeEdgeWait, l2FlowPriority, flowModTimeout,
                            bulkFlowModTimeout, initTimeout,
                            maxRedirections, controllerMacAddress);
    }

    // JAXB methods.

    /**
     * Return an {@link Integer} instance which represents the current value
     * of "node-edge-wait".
     *
     * @return  An {@link Integer} value or {@code null}.
     * @deprecated
     *     Only for JAXB. Use {@link #getNodeEdgeWait()} instead.
     */
    @XmlElement(name = "node-edge-wait")
    public Integer getJaxbNodeEdgeWait() {
        return encode(nodeEdgeWait);
    }

    /**
     * Set an {@link Integer} instance which represents the value of
     * "node-edge-wait".
     *
     * <p>
     *   This method is called by JAXB.
     * </p>
     *
     * @param value  An {@link Integer} value.
     */
    void setJaxbNodeEdgeWait(Integer value) {
        if (value == null) {
            nodeEdgeWait = UNDEFINED;
        } else {
            nodeEdgeWait = value.intValue();
            createJaxbValue().setNodeEdgeWait(value);
        }
    }

    /**
     * Return an {@link Integer} instance which represents the current value
     * of "l2-flow-priority".
     *
     * @return  An {@link Integer} value or {@code null}.
     * @deprecated
     *     Only for JAXB. Use {@link #getL2FlowPriority()} instead.
     */
    @XmlElement(name = "l2-flow-priority")
    public Integer getJaxbL2FlowPriority() {
        return encode(l2FlowPriority);
    }

    /**
     * Set an {@link Integer} instance which represents the value of
     * "l2-flow-priority".
     *
     * <p>
     *   This method is called by JAXB.
     * </p>
     *
     * @param value  An {@link Integer} value.
     */
    void setJaxbL2FlowPriority(Integer value) {
        if (value == null) {
            l2FlowPriority = UNDEFINED;
        } else {
            l2FlowPriority = value.intValue();
            createJaxbValue().setL2FlowPriority(value);
        }
    }

    /**
     * Return an {@link Integer} instance which represents the current value
     * of "flow-mod-timeout".
     *
     * @return  An {@link Integer} value or {@code null}.
     * @deprecated
     *     Only for JAXB. Use {@link #getFlowModTimeout()} instead.
     */
    @XmlElement(name = "flow-mod-timeout")
    public Integer getJaxbFlowModTimeout() {
        return encode(flowModTimeout);
    }

    /**
     * Set an {@link Integer} instance which represents the value of
     * "flow-mod-timeout".
     *
     * <p>
     *   This method is called by JAXB.
     * </p>
     *
     * @param value  An {@link Integer} value.
     */
    void setJaxbFlowModTimeout(Integer value) {
        if (value == null) {
            flowModTimeout = UNDEFINED;
        } else {
            flowModTimeout = value.intValue();
            createJaxbValue().setFlowModTimeout(value);
        }
    }

    /**
     * Return an {@link Integer} instance which represents the current value
     * of "bulk-flow-mod-timeout".
     *
     * @return  An {@link Integer} value or {@code null}.
     * @deprecated
     *     Only for JAXB. Use {@link #getBulkFlowModTimeout()} instead.
     */
    @XmlElement(name = "bulk-flow-mod-timeout")
    public Integer getJaxbBulkFlowModTimeout() {
        return encode(bulkFlowModTimeout);
    }

    /**
     * Set an {@link Integer} instance which represents the value of
     * "bulk-flow-mod-timeout".
     *
     * <p>
     *   This method is called by JAXB.
     * </p>
     *
     * @param value  An {@link Integer} value.
     */
    void setJaxbBulkFlowModTimeout(Integer value) {
        if (value == null) {
            bulkFlowModTimeout = UNDEFINED;
        } else {
            bulkFlowModTimeout = value.intValue();
            createJaxbValue().setBulkFlowModTimeout(value);
        }
    }

    /**
     * Return an {@link Integer} instance which represents the current value
     * of "init-timeout".
     *
     * @return  An {@link Integer} value or {@code null}.
     * @deprecated
     *     Only for JAXB. Use {@link #getInitTimeout()} instead.
     */
    @XmlElement(name = "init-timeout")
    public Integer getJaxbInitTimeout() {
        return encode(initTimeout);
    }

    /**
     * Set an {@link Integer} instance which represents the value of
     * "init-timeout".
     *
     * <p>
     *   This method is called by JAXB.
     * </p>
     *
     * @param value  An {@link Integer} value.
     */
    void setJaxbInitTimeout(Integer value) {
        if (value == null) {
            initTimeout = UNDEFINED;
        } else {
            initTimeout = value.intValue();
            createJaxbValue().setInitTimeout(value);
        }
    }

    /**
     * Return an {@link Integer} instance which represents the current value
     * of "max-redirections".
     *
     * @return  An {@link Integer} value or {@code null}.
     * @deprecated
     *     Only for JAXB. Use {@link #getMaxRedirections()} instead.
     */
    @XmlElement(name = "max-redirections")
    public Integer getJaxbMaxRedirections() {
        return encode(maxRedirections);
    }

    /**
     * Set an {@link Integer} instance which represents the value of
     * "max-redirections".
     *
     * <p>
     *   This method is called by JAXB.
     * </p>
     *
     * @param value  An {@link Integer} value.
     */
    void setJaxbMaxRedirections(Integer value) {
        if (value == null) {
            maxRedirections = UNDEFINED;
        } else {
            maxRedirections = value.intValue();
            createJaxbValue().setMaxRedirections(value);
        }
    }

    /**
     * Convert the given integer value into an {@link Integer} instance.
     *
     * @param i  An integer value.
     * @return   An {@link Integer} instance or {@code null}.
     */
    private Integer encode(int i) {
        return (i == UNDEFINED) ? null : Integer.valueOf(i);
    }

    /**
     * Convert the given {@link EtherAddress} instance into a
     * {@link MacAddress} instance.
     *
     * @param ea  An {@link EtherAddress} instance.
     * @return  A {@link MacAddress} instance or null.
     */
    private MacAddress encode(EtherAddress ea) {
        return (ea == null) ? null : ea.getMacAddress();
    }

    /**
     * Return an integer value in the given {@link Integer} instance.
     *
     * @param i  An {@link Integer} instance.
     * @return   An integer value in the given instance.
     */
    private int decode(Integer i) {
        return (i == null) ? UNDEFINED : i.intValue();
    }

    /**
     * Return an {@link EtherAddress} instance that represents the given
     * {@link MacAddress} instance.
     *
     * @param mac  A {@link MacAddress} instance.
     * @return  An {@link EtherAddress} instance.
     *          {@code null} is returned if no MAC address is configured.
     */
    private EtherAddress decode(MacAddress mac) {
        if (mac == null) {
            return null;
        }

        try {
            return new EtherAddress(mac);
        } catch (RuntimeException e) {
            // Ignore invalid MacAddress.
            return null;
        }
    }

    /**
     * Return a value of an integer parameter.
     *
     * @param value  An integer value.
     * @param def    A dault value.
     * @return   {@code value} if its value is not {@link #UNDEFINED}.
     *           {@code value} if its value is {@link #UNDEFINED}.
     */
    private int intValue(int value, int def) {
        return (value == UNDEFINED) ? def : value;
    }

    /**
     * Create a {@link VtnConfigBuilder} instance to keep values loaded via
     * JAXB.
     *
     * @return  A {@link VtnConfigBuilder} instance.
     */
    private VtnConfigBuilder createJaxbValue() {
        VtnConfigBuilder builder = jaxbValue;
        if (builder == null) {
            builder = new VtnConfigBuilder();
            jaxbValue = builder;
        }

        return builder;
    }

    // VTNConfig

    /**
     * {@inheritDoc}
     */
    @Override
    public int getNodeEdgeWait() {
        return intValue(nodeEdgeWait, DEFAULT_NODE_EDGE_WAIT);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public int getL2FlowPriority() {
        return intValue(l2FlowPriority, DEFAULT_L2_FLOW_PRIORITY);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public int getFlowModTimeout() {
        return intValue(flowModTimeout, DEFAULT_FLOW_MOD_TIMEOUT);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public int getBulkFlowModTimeout() {
        return intValue(bulkFlowModTimeout, DEFAULT_BULK_FLOW_MOD_TIMEOUT);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public int getInitTimeout() {
        return intValue(initTimeout, DEFAULT_INIT_TIMEOUT);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public int getMaxRedirections() {
        return intValue(maxRedirections, DEFAULT_MAX_REDIRECTIONS);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public EtherAddress getControllerMacAddress() {
        EtherAddress addr = controllerMacAddress;
        if (addr == null) {
            addr = DEFAULT_MAC_ADDRESS;
        }

        return addr;
    }
}
