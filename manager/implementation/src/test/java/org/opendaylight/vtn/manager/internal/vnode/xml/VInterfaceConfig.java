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
import static org.opendaylight.vtn.manager.internal.vnode.xml.XmlVInterfaceTest.XML_ROOT;

import org.mockito.Mockito;

import java.util.Random;

import org.opendaylight.vtn.manager.internal.util.log.VTNLogLevel;
import org.opendaylight.vtn.manager.internal.util.vnode.VInterfaceIdentifier;
import org.opendaylight.vtn.manager.internal.util.vnode.VTerminalIfIdentifier;

import org.opendaylight.vtn.manager.internal.XmlNode;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeState;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.vtn.mappable.vinterface.VinterfaceStatus;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.vtn.mappable.vinterface.VinterfaceStatusBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.vtn.mappable.vinterface.list.Vinterface;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.vtn.mappable.vinterface.list.VinterfaceBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.vtn.vinterface.info.VinterfaceConfig;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.vtn.vinterface.info.VinterfaceConfigBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.vtn.vinterface.info.VinterfaceInputFilter;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.vtn.vinterface.info.VinterfaceInputFilterBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.vtn.vinterface.info.VinterfaceOutputFilter;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.vtn.vinterface.info.VinterfaceOutputFilterBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vterminal.rev150907.vtn.vterminal.list.Vterminal;

/**
 * {@code VInterfaceConfig} describes configuration about a virtual interface.
 */
public final class VInterfaceConfig extends VNodeConfig {
    /**
     * A boolean value which determines whether the virtual interface is to
     * be enabled or not.
     */
    private Boolean  enabled;

    /**
     * Port mapping configuration.
     */
    private XmlPortMapConfig  portMap;

    /**
     * A list of flow filters applied to packets received from this virtual
     * interface.
     */
    private XmlFlowFilterList  inputFilters;

    /**
     * A list of flow filters applied to packets transmitted from this virtual
     * interface.
     */
    private XmlFlowFilterList  outputFilters;

    /**
     * Return the "enabled" field value in the given {@link XmlVInterface}
     * instance.
     *
     * @param xvif  A {@link XmlVInterface} instance.
     * @return  The value of "enabled" field.
     */
    public static Boolean isEnabled(XmlVInterface xvif) {
        try {
            return getFieldValue(xvif, Boolean.class, "enabled");
        } catch (Exception e) {
            throw new IllegalStateException(
                "Failed to get enabled field in XmlVInterface.", e);
        }
    }

    /**
     * Construct a new instance.
     *
     * @param name  The name of the virtual interface.
     */
    public VInterfaceConfig(String name) {
        super(name, null);
    }

    /**
     * Construct a new instance.
     *
     * @param name  The name of the virtual interface.
     * @param desc  Description about the virtual interface.
     */
    public VInterfaceConfig(String name, String desc) {
        super(name, desc);
    }

    /**
     * Construct a new instance using the given random number generator.
     *
     * @param name  The name of the virtual interface.
     * @param rand  A pseudo random number generator.
     */
    public VInterfaceConfig(String name, Random rand) {
        super(name, "Virtual interface: " + rand.nextInt());

        if (rand.nextBoolean()) {
            enabled = (rand.nextBoolean()) ? Boolean.TRUE : null;
        } else {
            enabled = Boolean.FALSE;
        }

        if (rand.nextBoolean()) {
            portMap = new XmlPortMapConfig(rand);
        }
        if (rand.nextBoolean()) {
            inputFilters = new XmlFlowFilterList(rand);
        }
        if (rand.nextBoolean()) {
            outputFilters = new XmlFlowFilterList(rand);
        }
    }

    /**
     * Determine whether the virtual interface is enabled or not.
     *
     * @return  {@link Boolean#TRUE} if enabled.
     *          {@link Boolean#FALSE} if disabled.
     *          {@code null} if not configured.
     */
    public Boolean isEnabled() {
        return enabled;
    }

    /**
     * Set a boolean value that indicates the administrative status of the
     * virtual interface.
     *
     * @param en  {@link Boolean#TRUE} indicates the virtual interface is
     *            enabled.
     *            {@link Boolean#FALSE} indicates the virtual interface is
     *            disabled.
     *            {@code null} indicates the default value.
     * @return  This instance.
     */
    public VInterfaceConfig setEnabled(Boolean en) {
        enabled = en;
        return this;
    }

    /**
     * Return the port mapping configuration.
     *
     * @return  A {@link XmlPortMapConfig} instance or {@code null}.
     */
    public XmlPortMapConfig getPortMap() {
        return portMap;
    }

    /**
     * Set the port mapping configuration.
     *
     * @param xpmc  A port mapping configuration.
     * @return  This instance.
     */
    public VInterfaceConfig setPortMap(XmlPortMapConfig xpmc) {
        portMap = xpmc;
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
    public VInterfaceConfig setInputFilters(XmlFlowFilterList xffl) {
        inputFilters = xffl;
        return this;
    }

    /**
     * Return a list of flow filters for outgoing packets.
     *
     * @return  A {@link XmlFlowFilterList} instance or {@code null}.
     */
    public XmlFlowFilterList getOutputFilters() {
        return outputFilters;
    }

    /**
     * Set a list of flow filters for outgoing packets.
     *
     * @param xffl  A flow filter list.
     * @return  This instance.
     */
    public VInterfaceConfig setOutputFilters(XmlFlowFilterList xffl) {
        outputFilters = xffl;
        return this;
    }

    /**
     * Convert this instance into a vinterface-config instance.
     *
     * @return  A {@link VinterfaceConfig} instance.
     */
    public VinterfaceConfig toVinterfaceConfig() {
        return new VinterfaceConfigBuilder().
            setDescription(getDescription()).
            setEnabled(enabled).
            build();
    }

    /**
     * Convert this instance into a vinterface instance.
     *
     * @return  A {@link Vinterface} instance.
     */
    public Vinterface toVinterface() {
        // Create vInterface status.
        // This should be always ignored.
        VinterfaceStatus vist = new VinterfaceStatusBuilder().
            setState(VnodeState.UNKNOWN).
            setEntityState(VnodeState.UNKNOWN).
            build();

        VinterfaceBuilder builder = new VinterfaceBuilder().
            setName(getVnodeName()).
            setVinterfaceConfig(toVinterfaceConfig()).
            setVinterfaceStatus(vist);

        if (portMap != null) {
            builder.setPortMapConfig(portMap.toPortMapConfig());
        }
        if (inputFilters != null) {
            VinterfaceInputFilter in = new VinterfaceInputFilterBuilder().
                setVtnFlowFilter(inputFilters.toVtnFlowFilterList()).
                build();
            builder.setVinterfaceInputFilter(in);
        }
        if (outputFilters != null) {
            VinterfaceOutputFilter out = new VinterfaceOutputFilterBuilder().
                setVtnFlowFilter(outputFilters.toVtnFlowFilterList()).
                build();
            builder.setVinterfaceOutputFilter(out);
        }

        return builder.build();
    }

    /**
     * Ensure that the given {@link XmlVInterface} instance is identical to
     * this instance.
     *
     * @param xvif  A {@link XmlVInterface} instance.
     * @param jaxb  {@code true} indicates {@code xvif} was deserialized from
     *              XML.
     */
    public void verify(XmlVInterface xvif, boolean jaxb) {
        verify((XmlVNode)xvif);

        Boolean en = enabled;
        if (en == null) {
            en = Boolean.TRUE;
        }
        assertEquals(en, isEnabled(xvif));

        if (portMap != null) {
            portMap.verify(xvif.getPortMap());
        }

        XmlFlowFilterList.verify(inputFilters, xvif.getInputFilters(), jaxb);
        XmlFlowFilterList.verify(outputFilters, xvif.getOutputFilters(), jaxb);
    }

    /**
     * Test case for {@link XmlVInterface#toVinterfaceBuilder(XmlLogger, VInterfaceIdentifier)}.
     *
     * @param xvif  A {@link XmlVInterface} instance that contains the same
     *              configuration as this instance.
     * @throws Exception  An error occurred.
     */
    public void testToVinterfaceBuilder(XmlVInterface xvif) throws Exception {
        XmlLogger xlogger = mock(XmlLogger.class);
        String tname = "vtn";
        String bname = "vterminal";
        String iname = getName();
        VInterfaceIdentifier<Vterminal> ifId =
            VTerminalIfIdentifier.create(tname, bname, iname, false);
        VinterfaceBuilder builder = xvif.toVinterfaceBuilder(xlogger, ifId);
        assertEquals(iname, builder.getName().getValue());

        VinterfaceConfig vifc = builder.getVinterfaceConfig();
        assertEquals(getDescription(), vifc.getDescription());

        Boolean en = enabled;
        if (en == null) {
            en = Boolean.TRUE;
        }
        assertEquals(en, vifc.isEnabled());

        String fmt = "{}: {} flow filters have been loaded.";
        VinterfaceInputFilter in = builder.getVinterfaceInputFilter();
        if (inputFilters == null || inputFilters.isEmpty()) {
            assertEquals(null, in);
        } else {
            inputFilters.verify(in);
            Mockito.verify(xlogger).log(VTNLogLevel.DEBUG, fmt, ifId, "IN");
        }

        VinterfaceOutputFilter out = builder.getVinterfaceOutputFilter();
        if (outputFilters == null || outputFilters.isEmpty()) {
            assertEquals(null, out);
        } else {
            outputFilters.verify(out);
            Mockito.verify(xlogger).log(VTNLogLevel.DEBUG, fmt, ifId, "OUT");
        }
        verifyNoMoreInteractions(xlogger);
    }

    /**
     * Convert this instance into a XML node.
     *
     * @return  A {@link XmlNode} instance that indicates the virtual interface
     *          configuration in this instance.
     */
    public XmlNode toXmlNode() {
        XmlNode xnode = new XmlNode(XML_ROOT);
        setXml(xnode);

        if (enabled != null) {
            xnode.add(new XmlNode("enabled", enabled));
        }
        if (portMap != null) {
            portMap.setXml(xnode);
        }

        if (inputFilters != null) {
            inputFilters.setXml(xnode, false);
        }
        if (outputFilters != null) {
            outputFilters.setXml(xnode, true);
        }

        return xnode;
    }
}
