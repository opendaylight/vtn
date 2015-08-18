/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.config;

import java.lang.reflect.Method;

import org.opendaylight.vtn.manager.util.EtherAddress;

import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.yang.types.rev100924.MacAddress;

/**
 * A symbol which indicates the type of parameters defined by
 * {@link org.opendaylight.vtn.manager.internal.VTNConfig}.
 */
public enum ConfigType {
    /**
     * A symbol which indicates the value returned by
     * {@link org.opendaylight.vtn.manager.internal.VTNConfig#getNodeEdgeWait()}.
     */
    NODE_EDGE_WAIT("getNodeEdgeWait", "node-edge-wait", Integer.valueOf(3000),
                   Integer.valueOf(0), Integer.valueOf(600000)),

    /**
     * A symbol which indicates the value returned by
     * {@link org.opendaylight.vtn.manager.internal.VTNConfig#getL2FlowPriority()}.
     */
    L2_FLOW_PRIORITY("getL2FlowPriority", "l2-flow-priority",
                     Integer.valueOf(10), Integer.valueOf(1),
                     Integer.valueOf(999)),

    /**
     * A symbol which indicates the value returned by
     * {@link org.opendaylight.vtn.manager.internal.VTNConfig#getFlowModTimeout()}.
     */
    FLOW_MOD_TIMEOUT("getFlowModTimeout", "flow-mod-timeout",
                     Integer.valueOf(3000), Integer.valueOf(100),
                     Integer.valueOf(60000)),

    /**
     * A symbol which indicates the value returned by
     * {@link org.opendaylight.vtn.manager.internal.VTNConfig#getRemoteFlowModTimeout()}.
     */
    REMOTE_FLOW_MOD_TIMEOUT("getRemoteFlowModTimeout",
                            "remote-flow-mod-timeout", Integer.valueOf(5000),
                            Integer.valueOf(1000), Integer.valueOf(60000)),

    /**
     * A symbol which indicates the value returned by
     * {@link org.opendaylight.vtn.manager.internal.VTNConfig#getBulkFlowModTimeout()}.
     */
    BULK_FLOW_MOD_TIMEOUT("getBulkFlowModTimeout", "bulk-flow-mod-timeout",
                          Integer.valueOf(10000), Integer.valueOf(3000),
                          Integer.valueOf(600000)),

    /**
     * A symbol which indicates the value returned by
     * {@link org.opendaylight.vtn.manager.internal.VTNConfig#getInitTimeout()}.
     */
    INIT_TIMEOUT("getInitTimeout", "init-timeout", Integer.valueOf(3000),
                 Integer.valueOf(100), Integer.valueOf(600000)),

    /**
     * A symbol which indicates the value returned by
     * {@link org.opendaylight.vtn.manager.internal.VTNConfig#getCacheTransactionTimeout()}.
     */
    CACHE_TRANSACTION_TIMEOUT("getCacheTransactionTimeout",
                              "cache-transaction-timeout",
                              Integer.valueOf(10000), Integer.valueOf(100),
                              Integer.valueOf(600000)),

    /**
     * A symbol which indicates the value returned by
     * {@link org.opendaylight.vtn.manager.internal.VTNConfig#getMaxRedirections()}.
     */
    MAX_REDIRECTIONS("getMaxRedirections", "max-redirections",
                     Integer.valueOf(100), Integer.valueOf(10),
                     Integer.valueOf(100000)),

    /**
     * A symbol which indicates the value returned by
     * {@link org.opendaylight.vtn.manager.internal.VTNConfig#getControllerMacAddress()}.
     */
    CONTROLLER_MAC_ADDRESS("getControllerMacAddress",
                           "controller-mac-address",
                           new MacAddress("00:00:0c:60:0d:10"));

    /**
     * The name of the accessor method.
     */
    private final String  methodName;

    /**
     * The default value.
     */
    private final Object  defaultValue;

    /**
     * The minimum value.
     */
    private final Object  minimumValue;

    /**
     * The maximum value.
     */
    private final Object  maximumValue;

    /**
     * The name of the XML element associated with this type.
     */
    private final String  elementName;

    /**
     * Consruct a new instance.
     *
     * @param name    The name of the accessor method.
     * @param elname  The name of the XML element.
     * @param def     The default value.
     */
    private ConfigType(String name, String elname, Object def) {
        this(name, elname, def, null, null);
    }

    /**
     * Consruct a new instance.
     *
     * @param name    The name of the accessor method.
     * @param elname  The name of the XML element.
     * @param def     The default value.
     * @param min     The minimum value.
     * @param max     The maximum value.
     */
    private ConfigType(String name, String elname, Object def, Object min,
                       Object max) {
        methodName = name;
        defaultValue = def;
        elementName = elname;
        minimumValue = min;
        maximumValue = max;
    }

    /**
     * Return the value configured in the given object.
     *
     * @param obj  An object that contains the parameter.
     * @return  The value associated with this parameter type.
     */
    public Object get(Object obj) {
        return get(obj, obj.getClass());
    }

    /**
     * Return the value configured in the given object.
     *
     * @param obj  An object that contains the parameter.
     * @param cls  A class of the given object.
     * @return  The value associated with this parameter type.
     */
    public Object get(Object obj, Class<?> cls) {
        try {
            Method m = cls.getMethod(methodName);
            return m.invoke(obj);
        } catch (Exception e) {
            String msg = toString() + ": Failed get parameter: " + e;
            throw new AssertionError(msg, e);
        }
    }

    /**
     * Return the default value of this parameter.
     *
     * @return  The default value.
     */
    public Object getDefaultValue() {
        return defaultValue;
    }

    /**
     * Return the minimum value of this parameter.
     *
     * @return  The minimum value.
     *          Note that {@code null} is returned if the minimum value is not
     *          defined.
     */
    public Object getMinimumValue() {
        return minimumValue;
    }

    /**
     * Return the maximum value of this parameter.
     *
     * @return  The maximum value.
     *          Note that {@code null} is returned if the maximum value is not
     *          defined.
     */
    public Object getMaximumValue() {
        return maximumValue;
    }

    /**
     * Return an XML element that represents this parameter.
     *
     * @param value  The value of the parameter.
     * @return  An XML element.
     */
    public String getXmlElement(Object value) {
        StringBuilder builder = new StringBuilder("<");
        Object v = value;
        if (value instanceof EtherAddress) {
            v = ((EtherAddress)v).getText();
        }
        return builder.append(elementName).append('>').append(v).
            append("</").append(elementName).append('>').toString();
    }
}
