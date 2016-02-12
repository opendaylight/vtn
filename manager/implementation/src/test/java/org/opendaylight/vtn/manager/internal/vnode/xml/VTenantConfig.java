/*
 * Copyright (c) 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.vnode.xml;

import static org.junit.Assert.assertEquals;

import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.verifyNoMoreInteractions;

import static org.opendaylight.vtn.manager.internal.TestBase.getFieldValue;
import static org.opendaylight.vtn.manager.internal.vnode.xml.XmlVTenantTest.XML_ROOT;

import org.mockito.Mockito;

import org.opendaylight.vtn.manager.internal.util.log.VTNLogLevel;
import org.opendaylight.vtn.manager.internal.util.vnode.VTenantIdentifier;

import org.opendaylight.vtn.manager.internal.XmlNode;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.vtn.info.VtenantConfig;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.vtn.info.VtenantConfigBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.vtn.info.VtnInputFilter;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.vtn.info.VtnInputFilterBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.vtns.Vtn;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.vtns.VtnBuilder;

/**
 * {@code VTenantConfig} describes configuration about a VTN.
 */
public final class VTenantConfig extends VNodeConfig {
    /**
     * Idle timeout value for flow entries.
     */
    private Integer  idleTimeout = 0;

    /**
     * Hard timeout value for flow entries.
     */
    private Integer  hardTimeout = 0;

    /**
     * A list of vBridges.
     */
    private VBridgeConfigList  vBridges;

    /**
     * A list of vTerminals.
     */
    private VTerminalConfigList  vTerminals;

    /**
     * A list of VTN path maps.
     */
    private PathMapConfigList  pathMaps;

    /**
     * A list of flow filters applied to packets mapped to this VTN.
     */
    private XmlFlowFilterList  inputFilters;

    /**
     * Return "idleTimeout" field value in the given {@link XmlVTenant}
     * instance.
     *
     * @param xvtn  A {@link XmlVTenant} instance.
     * @return  The value of "idleTimeout" field.
     */
    public static Integer getIdleTimeout(XmlVTenant xvtn) {
        return getIntegerField(xvtn, "idleTimeout");
    }

    /**
     * Return "hardTimeout" field value in the given {@link XmlVTenant}
     * instance.
     *
     * @param xvtn  A {@link XmlVTenant} instance.
     * @return  The value of "hardTimeout" field.
     */
    public static Integer getHardTimeout(XmlVTenant xvtn) {
        return getIntegerField(xvtn, "hardTimeout");
    }

    /**
     * Return the value of the integer field in the given {@link XmlVTenant}
     * instance.
     *
     * @param xvtn  A {@link XmlVTenant} instance.
     * @param name  The name of the target field.
     * @return  The value of the specified field.
     */
    private static Integer getIntegerField(XmlVTenant xvtn, String name) {
        try {
            return getFieldValue(xvtn, Integer.class, name);
        } catch (Exception e) {
            throw new IllegalStateException(
                "Failed to get " + name + " field in XmlVTenant.", e);
        }
    }

    /**
     * Construct a new instance.
     *
     * @param name  The name of the VTN.
     */
    public VTenantConfig(String name) {
        super(name, null);
    }


    /**
     * Construct a new instance.
     *
     * @param name  The name of the VTN.
     * @param desc  Description about the VTN.
     */
    public VTenantConfig(String name, String desc) {
        super(name, desc);
    }

    /**
     * Return the idle-timeout value.
     *
     * @return  The idle-timeout value.
     */
    public Integer getIdleTimeout() {
        return idleTimeout;
    }

    /**
     * Set the idle-timeout value.
     *
     * @param idle  The idle-timeout value.
     * @return  This instance.
     */
    public VTenantConfig setIdleTimeout(Integer idle) {
        idleTimeout = idle;
        return this;
    }

    /**
     * Return the hard-timeout value.
     *
     * @return  The hard-timeout value.
     */
    public Integer getHardTimeout() {
        return hardTimeout;
    }

    /**
     * Set the hard-timeout value.
     *
     * @param hard  The hard-timeout value.
     * @return  This instance.
     */
    public VTenantConfig setHardTimeout(Integer hard) {
        hardTimeout = hard;
        return this;
    }

    /**
     * Return a list of vBridge configurations.
     *
     * @return  A {@link VBridgeConfigList} instance if configured.
     *          {@code null} if not configured.
     */
    public VBridgeConfigList getBridges() {
        return vBridges;
    }

    /**
     * Set a list of vBridge configurations.
     *
     * @param bconfs  A {@link VBridgeConfigList} instance.
     * @return  This instance.
     */
    public VTenantConfig setBridges(VBridgeConfigList bconfs) {
        vBridges = bconfs;
        return this;
    }

    /**
     * Return a list of vTerminal configurations.
     *
     * @return  A {@link VTerminalConfigList} instance if configured.
     *          {@code null} if not configured.
     */
    public VTerminalConfigList getTerminals() {
        return vTerminals;
    }

    /**
     * Set a list of vTerminal configurations.
     *
     * @param vtconfs  A {@link VTerminalConfigList} instance.
     * @return  This instance.
     */
    public VTenantConfig setTerminals(VTerminalConfigList vtconfs) {
        vTerminals = vtconfs;
        return this;
    }

    /**
     * Return a list of path map configurations.
     *
     * @return  A {@link PathMapConfigList} instance if configured.
     *          {@code null} if not configured.
     */
    public PathMapConfigList getPathMaps() {
        return pathMaps;
    }

    /**
     * Set a list of path map configurations.
     *
     * @param pmaps  A {@link PathMapConfigList} instance.
     * @return  This instance.
     */
    public VTenantConfig setPathMaps(PathMapConfigList pmaps) {
        pathMaps = pmaps;
        return this;
    }

    /**
     * Return a list of flow filters for incoming packets.
     *
     * @return  A {@link XmlFlowFilterList} instance or {@code null}.
     */
    public XmlFlowFilterList getInputFilters() {
        return inputFilters;
    }

    /**
     * Set a list of flow filters for incoming packets.
     *
     * @param xffl  A flow filter list.
     * @return  This instance.
     */
    public VTenantConfig setInputFilters(XmlFlowFilterList xffl) {
        inputFilters = xffl;
        return this;
    }

    /**
     * Convert this instance into a vtenant-config instance.
     *
     * @return  A {@link VtenantConfig} instance.
     */
    public VtenantConfig toVtenantConfig() {
        return new VtenantConfigBuilder().
            setDescription(getDescription()).
            setIdleTimeout(idleTimeout).
            setHardTimeout(hardTimeout).
            build();
    }

    /**
     * Convert this instance into a vtn instance.
     *
     * @return  A {@link Vtn} instance.
     */
    public Vtn toVtn() {
        VtnBuilder builder = new VtnBuilder().
            setName(getVnodeName()).
            setVtenantConfig(toVtenantConfig()).
            setVbridge(VBridgeConfigList.toVbridgeList(vBridges)).
            setVterminal(VTerminalConfigList.toVterminalList(vTerminals)).
            setVtnPathMaps(PathMapConfigList.toVtnPathMaps(pathMaps));

        if (inputFilters != null) {
            VtnInputFilter in = new VtnInputFilterBuilder().
                setVtnFlowFilter(inputFilters.toVtnFlowFilterList()).
                build();
            builder.setVtnInputFilter(in);
        }

        return builder.build();
    }

    /**
     * Ensure that the given {@link XmlVTenant} instance is identical to
     * this instance.
     *
     * @param xvtn  A {@link XmlVTenant} instance.
     * @param jaxb  {@code true} indicates {@code xvbr} was deserialized from
     *              XML.
     */
    public void verify(XmlVTenant xvtn, boolean jaxb) {
        verify((XmlVNode)xvtn);
        assertEquals(idleTimeout, getIdleTimeout(xvtn));
        assertEquals(hardTimeout, getHardTimeout(xvtn));

        VBridgeConfigList.verify(vBridges, xvtn.getBridges(), jaxb);
        VTerminalConfigList.verify(vTerminals, xvtn.getTerminals(), jaxb);
        PathMapConfigList.verify(pathMaps, xvtn.getPathMaps(), jaxb);
        XmlFlowFilterList.verify(inputFilters, xvtn.getInputFilters(), jaxb);
    }

    /**
     * Test case for {@link XmlVTenant#toVtnBuilder(XmlLogger, String)}.
     *
     * @param xvtn  A {@link XmlVTenant} instance that contains the same
     *              configuration as this instance.
     * @throws Exception  An error occurred.
     */
    public void testToVtnBuilder(XmlVTenant xvtn) throws Exception {
        XmlLogger xlogger = mock(XmlLogger.class);
        String tname = getName();
        VtnBuilder builder = xvtn.toVtnBuilder(xlogger, tname);
        assertEquals(tname, builder.getName().getValue());

        VtenantConfig vtnc = builder.getVtenantConfig();
        assertEquals(getDescription(), vtnc.getDescription());
        assertEquals(idleTimeout, vtnc.getIdleTimeout());
        assertEquals(hardTimeout, vtnc.getHardTimeout());

        VtnInputFilter in = builder.getVtnInputFilter();
        if (inputFilters == null || inputFilters.isEmpty()) {
            assertEquals(null, in);
        } else {
            inputFilters.verify(in);
            VTenantIdentifier vtnId = VTenantIdentifier.create(tname, false);
            String fmt = "{}: {} flow filters have been loaded.";
            Mockito.verify(xlogger).log(VTNLogLevel.DEBUG, fmt, vtnId, "IN");
        }
        verifyNoMoreInteractions(xlogger);

        // Other fields should be always null.
        assertEquals(null, builder.getVbridge());
        assertEquals(null, builder.getVterminal());
        assertEquals(null, builder.getVtnPathMaps());
    }

    /**
     * Convert this instance into a XML node.
     * @return  A {@link XmlNode} instance that indicates the vBridge
     *          configuration in this instance.
     */
    public XmlNode toXmlNode() {
        XmlNode xnode = new XmlNode(XML_ROOT).
            add(new XmlNode("idle-timeout", idleTimeout)).
            add(new XmlNode("hard-timeout", hardTimeout));
        setXml(xnode);

        if (vBridges != null) {
            vBridges.setXml(xnode);
        }
        if (vTerminals != null) {
            vTerminals.setXml(xnode);
        }
        if (pathMaps != null) {
            pathMaps.setXml(xnode);
        }
        if (inputFilters != null) {
            inputFilters.setXml(xnode, false);
        }

        return xnode;
    }
}
