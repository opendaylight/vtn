/*
 * Copyright (c) 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.vnode.xml;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;

import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.verifyNoMoreInteractions;

import static org.opendaylight.vtn.manager.internal.TestBase.createInteger;
import static org.opendaylight.vtn.manager.internal.TestBase.getFieldValue;
import static org.opendaylight.vtn.manager.internal.vnode.xml.VInterfaceConfigList.toVinterfaceList;
import static org.opendaylight.vtn.manager.internal.vnode.xml.XmlVBridgeTest.XML_ROOT;

import org.mockito.Mockito;

import java.util.Random;

import org.opendaylight.vtn.manager.internal.util.log.VTNLogLevel;
import org.opendaylight.vtn.manager.internal.util.vnode.BridgeIdentifier;
import org.opendaylight.vtn.manager.internal.util.vnode.VBridgeIdentifier;

import org.opendaylight.vtn.manager.internal.XmlNode;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.vtn.vbridge.info.VbridgeConfig;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.vtn.vbridge.info.VbridgeConfigBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.vtn.vbridge.info.VbridgeInputFilter;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.vtn.vbridge.info.VbridgeInputFilterBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.vtn.vbridge.info.VbridgeOutputFilter;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.vtn.vbridge.info.VbridgeOutputFilterBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.vtn.vbridge.list.Vbridge;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.vtn.vbridge.list.VbridgeBuilder;

/**
 * {@code VBridgeConfig} describes configuration about a vBridge.
 */
public final class VBridgeConfig extends VirtualBridgeConfig {
    /**
     * The minimum value of age-interval.
     */
    private static final int  MIN_AGE_INTERVAL = 10;

    /**
     * The maximum value of age-interval.
     */
    private static final int  MAX_AGE_INTERVAL = 1000000;

    /**
     * The default value of age-interval.
     */
    private static final int  DEFAULT_AGE_INTERVAL = 600;

    /**
     * The number of seconds between MAC address table aging.
     */
    private int  ageInterval = DEFAULT_AGE_INTERVAL;

    /**
     * A list of VLAN mapping configurations.
     */
    private XmlVlanMapConfigList  vlanMaps;

    /**
     * MAC mapping.
     */
    private XmlMacMapConfig  macMap;

    /**
     * A list of flow filters applied to packets received from this vBridge.
     */
    private XmlFlowFilterList  inputFilters;

    /**
     * A list of flow filters applied to packets transmitted from this vBridge.
     */
    private XmlFlowFilterList  outputFilters;

    /**
     * Return "ageInterval" field value in the given {@link XmlVBridge}
     * instance.
     *
     * @param xvbr  A {@link XmlVBridge} instance.
     * @return  The value of "ageInterval" field.
     */
    public static int getAgeInterval(XmlVBridge xvbr) {
        try {
            Integer age = getFieldValue(xvbr, Integer.class, "ageInterval");
            assertNotNull(age);
            return age.intValue();
        } catch (Exception e) {
            throw new IllegalStateException(
                "Failed to get ageInterval field in XmlVBridge.", e);
        }
    }

    /**
     * Construct a new instance.
     *
     * @param name  The name of the vBridge.
     */
    public VBridgeConfig(String name) {
        super(name, null);
    }

    /**
     * Construct a new instance.
     *
     * @param name  The name of the vBridge.
     * @param desc  Description about the vBridge.
     */
    public VBridgeConfig(String name, String desc) {
        super(name, desc);
    }

    /**
     * Construct a new instance using the given random number generator.
     *
     * @param name  The name of the vBridge.
     * @param rand  A pseudo random number generator.
     */
    public VBridgeConfig(String name, Random rand) {
        super(name, "vBridge: " + rand.nextInt());
        ageInterval = createInteger(rand, MIN_AGE_INTERVAL, MAX_AGE_INTERVAL);

        if (rand.nextBoolean()) {
            vlanMaps = new XmlVlanMapConfigList(rand);
        }
        if (rand.nextBoolean()) {
            macMap = new XmlMacMapConfig(rand);
        }
        if (rand.nextBoolean()) {
            inputFilters = new XmlFlowFilterList(rand);
        }
        if (rand.nextBoolean()) {
            outputFilters = new XmlFlowFilterList(rand);
        }
    }

    /**
     * Return the age-interval value.
     *
     * @return  The age-interval value.
     */
    public int getAgeInterval() {
        return ageInterval;
    }

    /**
     * Set the age-interval value.
     *
     * @param age  The age-interval value.
     * @return  This instance.
     */
    public VBridgeConfig setAgeInterval(int age) {
        ageInterval = age;
        return this;
    }

    /**
     * Return a list of VLAN mapping configurations.
     *
     * @return  A {@link XmlVlanMapConfigList} instance if configured.
     *          {@code null} if not configured.
     */
    public XmlVlanMapConfigList getVlanMaps() {
        return vlanMaps;
    }

    /**
     * Set a list of VLAN mapping configurations.
     *
     * @param xvmaps  A {@link XmlVlanMapConfigList} instance.
     * @return  This instance.
     */
    public VBridgeConfig setVlanMaps(XmlVlanMapConfigList xvmaps) {
        vlanMaps = xvmaps;
        return this;
    }

    /**
     * Returh the MAC mapping configuration.
     *
     * @return  A {@link XmlMacMapConfig} instance if configured.
     *          {@code null} if not configured.
     */
    public XmlMacMapConfig getMacMap() {
        return macMap;
    }

    /**
     * Set the MAC mapping configuration.
     *
     * @param xmmap A {@link XmlMacMapConfig} instance.
     * @return  This instance.
     */
    public VBridgeConfig setMacMap(XmlMacMapConfig xmmap) {
        macMap = xmmap;
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
    public VBridgeConfig setInputFilters(XmlFlowFilterList xffl) {
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
    public VBridgeConfig setOutputFilters(XmlFlowFilterList xffl) {
        outputFilters = xffl;
        return this;
    }

    /**
     * Convert this instance into a vbridge-config instance.
     *
     * @return  A {@link VbridgeConfig} instance.
     */
    public VbridgeConfig toVbridgeConfig() {
        return new VbridgeConfigBuilder().
            setDescription(getDescription()).
            setAgeInterval(ageInterval).
            build();
    }

    /**
     * Convert this instance into a vbridge instance.
     *
     * @return  A {@link Vbridge} instance.
     */
    public Vbridge toVbridge() {
        VbridgeBuilder builder = new VbridgeBuilder().
            setName(getVnodeName()).
            setVbridgeConfig(toVbridgeConfig()).
            setBridgeStatus(newBridgeStatus()).
            setVinterface(toVinterfaceList(getInterfaces()));

        if (vlanMaps != null) {
            builder.setVlanMap(vlanMaps.toDataObjects());
        }
        if (macMap != null) {
            builder.setMacMap(macMap.toMacMap());
        }
        if (inputFilters != null) {
            VbridgeInputFilter in = new VbridgeInputFilterBuilder().
                setVtnFlowFilter(inputFilters.toVtnFlowFilterList()).
                build();
            builder.setVbridgeInputFilter(in);
        }
        if (outputFilters != null) {
            VbridgeOutputFilter out = new VbridgeOutputFilterBuilder().
                setVtnFlowFilter(outputFilters.toVtnFlowFilterList()).
                build();
            builder.setVbridgeOutputFilter(out);
        }

        return builder.build();
    }

    /**
     * Ensure that the given {@link XmlVBridge} instance is identical to
     * this instance.
     *
     * @param xvbr  A {@link XmlVBridge} instance.
     * @param jaxb  {@code true} indicates {@code xvbr} was deserialized from
     *              XML.
     */
    public void verify(XmlVBridge xvbr, boolean jaxb) {
        verify((XmlVNode)xvbr);
        assertEquals(ageInterval, getAgeInterval(xvbr));

        VInterfaceConfigList.
            verify(getInterfaces(), xvbr.getInterfaces(), jaxb);
        XmlVlanMapConfigList.verify(vlanMaps, xvbr.getVlanMaps());
        XmlMacMapConfig.verify(macMap, xvbr.getMacMap());
        XmlFlowFilterList.verify(inputFilters, xvbr.getInputFilters(), jaxb);
        XmlFlowFilterList.verify(outputFilters, xvbr.getOutputFilters(), jaxb);
    }

    /**
     * Test case for
     * {@link XmlVBridge#toVbridgeBuilder(XmlLogger, BridgeIdentifier)}.
     *
     * @param xvbr  A {@link XmlVBridge} instance that contains the same
     *              configuration as this instance.
     * @throws Exception  An error occurred.
     */
    public void testToVbridgeBuilder(XmlVBridge xvbr) throws Exception {
        XmlLogger xlogger = mock(XmlLogger.class);
        String tname = "vtn";
        String bname = getName();
        BridgeIdentifier<Vbridge> vbrId =
            VBridgeIdentifier.create(tname, bname, false);
        VbridgeBuilder builder = xvbr.toVbridgeBuilder(xlogger, vbrId);
        assertEquals(bname, builder.getName().getValue());

        VbridgeConfig vbrc = builder.getVbridgeConfig();
        assertEquals(getDescription(), vbrc.getDescription());
        assertEquals(ageInterval, getAgeInterval(xvbr));

        String fmt = "{}: {} flow filters have been loaded.";
        VbridgeInputFilter in = builder.getVbridgeInputFilter();
        if (inputFilters == null || inputFilters.isEmpty()) {
            assertEquals(null, in);
        } else {
            inputFilters.verify(in);
            Mockito.verify(xlogger).log(VTNLogLevel.DEBUG, fmt, vbrId, "IN");
        }

        VbridgeOutputFilter out = builder.getVbridgeOutputFilter();
        if (outputFilters == null || outputFilters.isEmpty()) {
            assertEquals(null, out);
        } else {
            outputFilters.verify(out);
            Mockito.verify(xlogger).log(VTNLogLevel.DEBUG, fmt, vbrId, "OUT");
        }
        verifyNoMoreInteractions(xlogger);

        // Other fields should be always null.
        assertEquals(null, builder.getBridgeStatus());
        assertEquals(null, builder.getVinterface());
        assertEquals(null, builder.getVlanMap());
        assertEquals(null, builder.getMacMap());
    }

    /**
     * Convert this instance into a XML node.
     * @return  A {@link XmlNode} instance that indicates the vBridge
     *          configuration in this instance.
     */
    public XmlNode toXmlNode() {
        XmlNode xnode = new XmlNode(XML_ROOT).
            add(new XmlNode("age-interval", ageInterval));
        setXml(xnode);
        setInterfacesAsXml(xnode);

        if (vlanMaps != null) {
            vlanMaps.setXml(xnode);
        }
        if (macMap != null) {
            macMap.setXml(xnode);
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
