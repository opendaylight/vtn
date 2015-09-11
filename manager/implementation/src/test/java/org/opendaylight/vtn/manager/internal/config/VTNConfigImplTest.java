/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.config;

import java.io.ByteArrayInputStream;
import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

import javax.xml.bind.UnmarshalException;
import javax.xml.bind.Unmarshaller;

import org.junit.Test;

import org.opendaylight.vtn.manager.util.EtherAddress;

import org.opendaylight.vtn.manager.internal.VTNConfig;

import org.opendaylight.vtn.manager.internal.TestBase;
import org.opendaylight.vtn.manager.internal.XmlValueType;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.config.rev150209.VtnConfig;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.config.rev150209.VtnConfigBuilder;

import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.yang.types.rev100924.MacAddress;

/**
 * JUnit test for {@link VTNConfigImpl}.
 */
public class VTNConfigImplTest extends TestBase {
    /**
     * Root XML element name associated with {@link VTNConfigImpl} class.
     */
    private static final String  XML_ROOT = "vtn-config";

    /**
     * Test case for
     * {@link VTNConfigImpl#fillDefault(VtnConfigBuilder, EtherAddress)}.
     */
    @Test
    public void testFillDefault() {
        EtherAddress mac = new EtherAddress(0xa0b1c2d3e4f5L);
        MacAddress defMac = new MacAddress("a0:b1:c2:d3:e4:f5");
        VtnConfigBuilder builder = new VtnConfigBuilder();
        assertSame(builder, VTNConfigImpl.fillDefault(builder, mac));
        new TestVtnConfigBuilder().fillDefault().
            set(ConfigType.CONTROLLER_MAC_ADDRESS, defMac).verify(builder);

        VtnConfigBuilder all = new VtnConfigBuilder();
        TestVtnConfigBuilder allTest = new TestVtnConfigBuilder();

        // Set topology-wait.
        Integer value = 600000;
        builder = new VtnConfigBuilder().setTopologyWait(value);
        assertSame(builder, VTNConfigImpl.fillDefault(builder, mac));
        new TestVtnConfigBuilder().set(ConfigType.TOPOLOGY_WAIT, value).
            set(ConfigType.CONTROLLER_MAC_ADDRESS, defMac).fillDefault().
            verify(builder);
        assertEquals(null, builder.isInitState());

        value = 0;
        all.setTopologyWait(value);
        allTest.set(ConfigType.TOPOLOGY_WAIT, value);

        // Set l2-flow-priority.
        value = 999;
        builder = new VtnConfigBuilder().setL2FlowPriority(value);
        assertSame(builder, VTNConfigImpl.fillDefault(builder, mac));
        new TestVtnConfigBuilder().set(ConfigType.L2_FLOW_PRIORITY, value).
            set(ConfigType.CONTROLLER_MAC_ADDRESS, defMac).fillDefault().
            verify(builder);
        assertEquals(null, builder.isInitState());

        value = 1;
        all.setL2FlowPriority(value);
        allTest.set(ConfigType.L2_FLOW_PRIORITY, value);

        // Set flow-mod-timeout.
        value = 60000;
        builder = new VtnConfigBuilder().setFlowModTimeout(value);
        assertSame(builder, VTNConfigImpl.fillDefault(builder, mac));
        new TestVtnConfigBuilder().set(ConfigType.FLOW_MOD_TIMEOUT, value).
            set(ConfigType.CONTROLLER_MAC_ADDRESS, defMac).fillDefault().
            verify(builder);
        assertEquals(null, builder.isInitState());

        value = 100;
        all.setFlowModTimeout(value);
        allTest.set(ConfigType.FLOW_MOD_TIMEOUT, value);

        // Set bulk-flow-mod-timeout.
        value = 600000;
        builder = new VtnConfigBuilder().setBulkFlowModTimeout(value);
        assertSame(builder, VTNConfigImpl.fillDefault(builder, mac));
        new TestVtnConfigBuilder().
            set(ConfigType.BULK_FLOW_MOD_TIMEOUT, value).
            set(ConfigType.CONTROLLER_MAC_ADDRESS, defMac).fillDefault().
            verify(builder);
        assertEquals(null, builder.isInitState());

        value = 3000;
        all.setBulkFlowModTimeout(value);
        allTest.set(ConfigType.BULK_FLOW_MOD_TIMEOUT, value);

        // Set init-timeout.
        value = 600000;
        builder = new VtnConfigBuilder().setInitTimeout(value);
        assertSame(builder, VTNConfigImpl.fillDefault(builder, mac));
        new TestVtnConfigBuilder().set(ConfigType.INIT_TIMEOUT, value).
            set(ConfigType.CONTROLLER_MAC_ADDRESS, defMac).fillDefault().
            verify(builder);
        assertEquals(null, builder.isInitState());

        value = 100;
        all.setInitTimeout(value);
        allTest.set(ConfigType.INIT_TIMEOUT, value);

        // Set max-redirections.
        value = 100000;
        builder = new VtnConfigBuilder().setMaxRedirections(value);
        assertSame(builder, VTNConfigImpl.fillDefault(builder, mac));
        new TestVtnConfigBuilder().set(ConfigType.MAX_REDIRECTIONS, value).
            set(ConfigType.CONTROLLER_MAC_ADDRESS, defMac).fillDefault().
            verify(builder);
        assertEquals(null, builder.isInitState());

        value = 10;
        all.setMaxRedirections(value);
        allTest.set(ConfigType.MAX_REDIRECTIONS, value);

        // Set controller-mac-address.
        MacAddress macAddr = new MacAddress("12:34:56:78:9a:bc");
        builder = new VtnConfigBuilder().setControllerMacAddress(macAddr);
        assertSame(builder, VTNConfigImpl.fillDefault(builder, mac));
        new TestVtnConfigBuilder().
            set(ConfigType.CONTROLLER_MAC_ADDRESS, macAddr).fillDefault().
            verify(builder);
        assertEquals(null, builder.isInitState());

        macAddr = new MacAddress("aa:bb:cc:dd:ee:00");
        all.setControllerMacAddress(macAddr);
        assertSame(all, VTNConfigImpl.fillDefault(all, mac));
        allTest.set(ConfigType.CONTROLLER_MAC_ADDRESS, macAddr).verify(all);
        assertEquals(null, all.isInitState());
    }

    /**
     * Test case for
     * {@link VTNConfigImpl#builder(EtherAddress)} and
     * {@link VTNConfigImpl#builder(VtnConfig, EtherAddress)} and
     */
    @Test
    public void testBuilder() {
        EtherAddress mac = new EtherAddress(0xa0b1c2d3e4f5L);
        MacAddress defMac = new MacAddress("a0:b1:c2:d3:e4:f5");
        VtnConfigBuilder builder = VTNConfigImpl.builder(mac);
        TestVtnConfigBuilder test = new TestVtnConfigBuilder().
            set(ConfigType.CONTROLLER_MAC_ADDRESS, defMac).fillDefault();
        test.verify(builder);
        assertEquals(null, builder.isInitState());

        builder = VTNConfigImpl.builder(null, mac);
        test.verify(builder);
        assertEquals(null, builder.isInitState());
        builder = VTNConfigImpl.builder(new VtnConfigBuilder().build(), mac);
        test.verify(builder);
        assertEquals(null, builder.isInitState());

        VtnConfigBuilder all = new VtnConfigBuilder();
        TestVtnConfigBuilder allTest = new TestVtnConfigBuilder();

        // Set topology-wait.
        Integer value = 600000;
        VtnConfig vcfg = new VtnConfigBuilder().
            setTopologyWait(value).build();
        builder = VTNConfigImpl.builder(vcfg, mac);
        new TestVtnConfigBuilder().set(ConfigType.TOPOLOGY_WAIT, value).
            set(ConfigType.CONTROLLER_MAC_ADDRESS, defMac).fillDefault().
            verify(builder);
        assertEquals(null, builder.isInitState());

        value = 0;
        all.setTopologyWait(value);
        allTest.set(ConfigType.TOPOLOGY_WAIT, value);

        // Set l2-flow-priority.
        value = 999;
        vcfg = new VtnConfigBuilder().setL2FlowPriority(value).build();
        builder = VTNConfigImpl.builder(vcfg, mac);
        new TestVtnConfigBuilder().set(ConfigType.L2_FLOW_PRIORITY, value).
            set(ConfigType.CONTROLLER_MAC_ADDRESS, defMac).fillDefault().
            verify(builder);
        assertEquals(null, builder.isInitState());

        value = 1;
        all.setL2FlowPriority(value);
        allTest.set(ConfigType.L2_FLOW_PRIORITY, value);

        // Set flow-mod-timeout.
        value = 60000;
        vcfg = new VtnConfigBuilder().setFlowModTimeout(value).build();
        builder = VTNConfigImpl.builder(vcfg, mac);
        new TestVtnConfigBuilder().set(ConfigType.FLOW_MOD_TIMEOUT, value).
            set(ConfigType.CONTROLLER_MAC_ADDRESS, defMac).fillDefault().
            verify(builder);
        assertEquals(null, builder.isInitState());

        value = 100;
        all.setFlowModTimeout(value);
        allTest.set(ConfigType.FLOW_MOD_TIMEOUT, value);

        // Set bulk-flow-mod-timeout.
        value = 600000;
        vcfg = new VtnConfigBuilder().setBulkFlowModTimeout(value).build();
        builder = VTNConfigImpl.builder(vcfg, mac);
        new TestVtnConfigBuilder().
            set(ConfigType.BULK_FLOW_MOD_TIMEOUT, value).
            set(ConfigType.CONTROLLER_MAC_ADDRESS, defMac).fillDefault().
            verify(builder);
        assertEquals(null, builder.isInitState());

        value = 3000;
        all.setBulkFlowModTimeout(value);
        allTest.set(ConfigType.BULK_FLOW_MOD_TIMEOUT, value);

        // Set init-timeout.
        value = 600000;
        vcfg = new VtnConfigBuilder().setInitTimeout(value).build();
        builder = VTNConfigImpl.builder(vcfg, mac);
        new TestVtnConfigBuilder().set(ConfigType.INIT_TIMEOUT, value).
            set(ConfigType.CONTROLLER_MAC_ADDRESS, defMac).fillDefault().
            verify(builder);
        assertEquals(null, builder.isInitState());

        value = 100;
        all.setInitTimeout(value);
        allTest.set(ConfigType.INIT_TIMEOUT, value);

        // Set max-redirections.
        value = 100000;
        vcfg = new VtnConfigBuilder().setMaxRedirections(value).build();
        builder = VTNConfigImpl.builder(vcfg, mac);
        new TestVtnConfigBuilder().set(ConfigType.MAX_REDIRECTIONS, value).
            set(ConfigType.CONTROLLER_MAC_ADDRESS, defMac).fillDefault().
            verify(builder);
        assertEquals(null, builder.isInitState());

        value = 10;
        all.setMaxRedirections(value);
        allTest.set(ConfigType.MAX_REDIRECTIONS, value);

        // Set controller-mac-address.
        MacAddress macAddr = new MacAddress("12:34:56:78:9a:bc");
        vcfg = new VtnConfigBuilder().setControllerMacAddress(macAddr).
            build();
        builder = VTNConfigImpl.builder(vcfg, mac);
        new TestVtnConfigBuilder().
            set(ConfigType.CONTROLLER_MAC_ADDRESS, macAddr).fillDefault().
            verify(builder);
        assertEquals(null, builder.isInitState());

        macAddr = new MacAddress("aa:bb:cc:dd:ee:00");
        vcfg = all.setControllerMacAddress(macAddr).build();
        builder = VTNConfigImpl.builder(vcfg, mac);
        allTest.set(ConfigType.CONTROLLER_MAC_ADDRESS, macAddr).verify(all);
        assertEquals(null, all.isInitState());
    }

    /**
     * Test case for {@link VTNConfigImpl#diff(VTNConfig, VTNConfig)}.
     */
    @Test
    public void testDiff() {
        VTNConfigImpl vconfOld = new VTNConfigImpl();
        VTNConfigImpl vconfNew = new VTNConfigImpl();
        assertEquals(null, VTNConfigImpl.diff(vconfOld, vconfNew));

        VtnConfigBuilder all1 = new VtnConfigBuilder();
        VtnConfigBuilder all2 = new VtnConfigBuilder();
        List<String> diffList = new ArrayList<>();

        // Change topology-wait.
        int ov = vconfOld.getTopologyWait();
        int nv = 600000;
        VtnConfig vcfg = new VtnConfigBuilder().setTopologyWait(nv).build();
        vconfNew = new VTNConfigImpl(vcfg);
        String expected = "topology-wait=(" + ov + "->" + nv + ")";
        assertEquals(expected, VTNConfigImpl.diff(vconfOld, vconfNew));

        int nv1 = 11111;
        all1.setTopologyWait(nv);
        all2.setTopologyWait(nv1);
        diffList.add("topology-wait=(" + nv + "->" + nv1 + ")");

        // Change l2-flow-priority.
        ov = vconfOld.getL2FlowPriority();
        nv = 999;
        vcfg = new VtnConfigBuilder().setL2FlowPriority(nv).build();
        vconfNew = new VTNConfigImpl(vcfg);
        expected = "l2-flow-priority=(" + ov + "->" + nv + ")";
        assertEquals(expected, VTNConfigImpl.diff(vconfOld, vconfNew));

        nv1 = 222;
        all1.setL2FlowPriority(nv);
        all2.setL2FlowPriority(nv1);
        diffList.add("l2-flow-priority=(" + nv + "->" + nv1 + ")");

        // Change flow-mod-timeout.
        ov = vconfOld.getFlowModTimeout();
        nv = 60000;
        vcfg = new VtnConfigBuilder().setFlowModTimeout(nv).build();
        vconfNew = new VTNConfigImpl(vcfg);
        expected = "flow-mod-timeout=(" + ov + "->" + nv + ")";
        assertEquals(expected, VTNConfigImpl.diff(vconfOld, vconfNew));

        nv1 = 33333;
        all1.setFlowModTimeout(nv);
        all2.setFlowModTimeout(nv1);
        diffList.add("flow-mod-timeout=(" + nv + "->" + nv1 + ")");

        // Change bulk-flow-mod-timeout.
        ov = vconfOld.getBulkFlowModTimeout();
        nv = 600000;
        vcfg = new VtnConfigBuilder().setBulkFlowModTimeout(nv).build();
        vconfNew = new VTNConfigImpl(vcfg);
        expected = "bulk-flow-mod-timeout=(" + ov + "->" + nv + ")";
        assertEquals(expected, VTNConfigImpl.diff(vconfOld, vconfNew));

        nv1 = 55555;
        all1.setBulkFlowModTimeout(nv);
        all2.setBulkFlowModTimeout(nv1);
        diffList.add("bulk-flow-mod-timeout=(" + nv + "->" + nv1 + ")");

        // Change init-timeout.
        ov = vconfOld.getInitTimeout();
        nv = 600000;
        vcfg = new VtnConfigBuilder().setInitTimeout(nv).build();
        vconfNew = new VTNConfigImpl(vcfg);
        expected = "init-timeout=(" + ov + "->" + nv + ")";
        assertEquals(expected, VTNConfigImpl.diff(vconfOld, vconfNew));

        nv1 = 66666;
        all1.setInitTimeout(nv);
        all2.setInitTimeout(nv1);
        diffList.add("init-timeout=(" + nv + "->" + nv1 + ")");

        // Change max-redirections.
        ov = vconfOld.getMaxRedirections();
        nv = 100000;
        vcfg = new VtnConfigBuilder().setMaxRedirections(nv).build();
        vconfNew = new VTNConfigImpl(vcfg);
        expected = "max-redirections=(" + ov + "->" + nv + ")";
        assertEquals(expected, VTNConfigImpl.diff(vconfOld, vconfNew));

        nv1 = 88888;
        all1.setMaxRedirections(nv);
        all2.setMaxRedirections(nv1);
        diffList.add("max-redirections=(" + nv + "->" + nv1 + ")");

        // Change controller-mac-address.
        EtherAddress omac = vconfOld.getControllerMacAddress();
        EtherAddress nmac = new EtherAddress(0xfafbfcfdfeffL);
        String oldMac = omac.getText();
        String newMac = nmac.getText();
        vcfg = new VtnConfigBuilder().
            setControllerMacAddress(new MacAddress(newMac)).build();
        vconfNew = new VTNConfigImpl(vcfg);
        expected = "controller-mac-address=(" + oldMac + "->" + newMac + ")";
        assertEquals(expected, VTNConfigImpl.diff(vconfOld, vconfNew));

        EtherAddress mac = new EtherAddress(0xa0b1c2d3e4f5L);
        String maddr = mac.getText();
        all1.setControllerMacAddress(new MacAddress(newMac));
        all2.setControllerMacAddress(new MacAddress(maddr));
        diffList.add("controller-mac-address=(" + newMac + "->" + maddr + ")");

        // Change all parameters.
        vconfOld = new VTNConfigImpl(all1.build());
        vconfNew = new VTNConfigImpl(all2.build());
        expected = joinStrings(null, null, ", ", diffList.toArray());
        assertEquals(expected, VTNConfigImpl.diff(vconfOld, vconfNew));
    }

    /**
     * Test case for {@link VTNConfigImpl#VTNConfigImpl()} and
     * {@link VTNConfigImpl#VTNConfigImpl(EtherAddress)}.
     */
    @Test
    public void testConstructor1() {
        VTNConfigImpl vconf = new VTNConfigImpl();
        new TestVTNConfig().verify(vconf);

        EtherAddress mac = new EtherAddress(0xa0b1c2d3e4f5L);
        MacAddress defMac = new MacAddress("a0:b1:c2:d3:e4:f5");
        vconf = new VTNConfigImpl(mac);
        new TestVTNConfig().set(ConfigType.CONTROLLER_MAC_ADDRESS, defMac).
            verify(vconf);
    }

    /**
     * Test case for {@link VTNConfigImpl#VTNConfigImpl(VtnConfig)} and
     * {@link VTNConfigImpl#VTNConfigImpl(VtnConfig, EtherAddress)}.
     */
    @Test
    public void testConstructor2() {
        EtherAddress[] macAddrs = {
            null,
            new EtherAddress(0xa0b1c2d3e4f5L),
        };

        for (EtherAddress mac: macAddrs) {
            VtnConfig vcfg = new VtnConfigBuilder().build();
            VTNConfigImpl vconf;
            MacAddress defMac;
            if (mac == null) {
                vconf = new VTNConfigImpl(vcfg);
                defMac = (MacAddress)ConfigType.CONTROLLER_MAC_ADDRESS.
                    getDefaultValue();
            } else {
                vconf = new VTNConfigImpl(vcfg, mac);
                defMac = new MacAddress("a0:b1:c2:d3:e4:f5");
            }
            TestVTNConfig test = new TestVTNConfig();
            VtnConfigBuilder all = new VtnConfigBuilder();
            TestVTNConfig allTest = new TestVTNConfig();

            if (mac != null) {
                test.set(ConfigType.CONTROLLER_MAC_ADDRESS, defMac);
                allTest.set(ConfigType.CONTROLLER_MAC_ADDRESS, defMac);
            }
            test.verify(vconf);

            // Set topology-wait.
            ConfigType type = ConfigType.TOPOLOGY_WAIT;
            Integer value = 600000;
            vcfg = new VtnConfigBuilder().setTopologyWait(value).build();
            vconf = (mac == null)
                ? new VTNConfigImpl(vcfg) : new VTNConfigImpl(vcfg, mac);
            test.resetIntegers().set(type, value).verify(vconf);

            value = 1;
            all.setTopologyWait(value);
            allTest.set(type, value);

            // Set l2-flow-priority.
            type = ConfigType.L2_FLOW_PRIORITY;
            value = 999;
            vcfg = new VtnConfigBuilder().setL2FlowPriority(value).build();
            vconf = (mac == null)
                ? new VTNConfigImpl(vcfg) : new VTNConfigImpl(vcfg, mac);
            test.resetIntegers().set(type, value).verify(vconf);

            value = 1;
            all.setL2FlowPriority(value);
            allTest.set(type, value);

            // Set flow-mod-timeout.
            type = ConfigType.FLOW_MOD_TIMEOUT;
            value = 60000;
            vcfg = new VtnConfigBuilder().setFlowModTimeout(value).build();
            vconf = (mac == null)
                ? new VTNConfigImpl(vcfg) : new VTNConfigImpl(vcfg, mac);
            test.resetIntegers().set(type, value).verify(vconf);

            value = 200;
            all.setFlowModTimeout(value);
            allTest.set(type, value);

            // Set bulk-flow-mod-timeout.
            type = ConfigType.BULK_FLOW_MOD_TIMEOUT;
            value = 60000;
            vcfg = new VtnConfigBuilder().setBulkFlowModTimeout(value).build();
            vconf = (mac == null)
                ? new VTNConfigImpl(vcfg) : new VTNConfigImpl(vcfg, mac);
            test.resetIntegers().set(type, value).verify(vconf);

            value = 3300;
            all.setBulkFlowModTimeout(value);
            allTest.set(type, value);

            // Set init-timeout.
            type = ConfigType.INIT_TIMEOUT;
            value = 60000;
            vcfg = new VtnConfigBuilder().setInitTimeout(value).build();
            vconf = (mac == null)
                ? new VTNConfigImpl(vcfg) : new VTNConfigImpl(vcfg, mac);
            test.resetIntegers().set(type, value).verify(vconf);

            value = 200;
            all.setInitTimeout(value);
            allTest.set(type, value);

            // Set max-redirections.
            type = ConfigType.MAX_REDIRECTIONS;
            value = 100000;
            vcfg = new VtnConfigBuilder().setMaxRedirections(value).build();
            vconf = (mac == null)
                ? new VTNConfigImpl(vcfg) : new VTNConfigImpl(vcfg, mac);
            test.resetIntegers().set(type, value).verify(vconf);

            value = 11;
            all.setMaxRedirections(value);
            allTest.set(type, value);

            // Set ccontroller-mac-address.
            MacAddress macAddr = new MacAddress("12:34:56:78:9a:bc");
            type = ConfigType.CONTROLLER_MAC_ADDRESS;
            vcfg = new VtnConfigBuilder().setControllerMacAddress(macAddr).
                build();
            vconf = (mac == null)
                ? new VTNConfigImpl(vcfg) : new VTNConfigImpl(vcfg, mac);
            test.resetIntegers().set(type, macAddr).verify(vconf);

            macAddr = new MacAddress("11:22:33:44:55:66");
            all.setControllerMacAddress(macAddr);
            vcfg = all.build();
            vconf = (mac == null)
                ? new VTNConfigImpl(vcfg) : new VTNConfigImpl(vcfg, mac);
            allTest.set(ConfigType.CONTROLLER_MAC_ADDRESS, macAddr).
                verify(vconf);
        }
    }

    /**
     * Test case for {@link VTNConfigImpl#toVtnConfig()}.
     */
    @Test
    public void testToVtnConfig() {
        VTNConfigImpl vconf = new VTNConfigImpl();
        new TestVtnConfigBuilder().verify(vconf.toVtnConfig());

        VtnConfigBuilder all = new VtnConfigBuilder();
        TestVtnConfigBuilder allTest = new TestVtnConfigBuilder();

        // Set topology-wait.
        Integer value = 600000;
        VtnConfig vcfg = new VtnConfigBuilder().setTopologyWait(value).build();
        vcfg = new VTNConfigImpl(vcfg).toVtnConfig();
        new TestVtnConfigBuilder().set(ConfigType.TOPOLOGY_WAIT, value).
            verify(vcfg);
        assertEquals(null, vcfg.isInitState());

        value = 0;
        all.setTopologyWait(value);
        allTest.set(ConfigType.TOPOLOGY_WAIT, value);

        // Set l2-flow-priority.
        value = 999;
        vcfg = new VtnConfigBuilder().setL2FlowPriority(value).build();
        vcfg = new VTNConfigImpl(vcfg).toVtnConfig();
        new TestVtnConfigBuilder().set(ConfigType.L2_FLOW_PRIORITY, value).
            verify(vcfg);
        assertEquals(null, vcfg.isInitState());

        value = 1;
        all.setL2FlowPriority(value);
        allTest.set(ConfigType.L2_FLOW_PRIORITY, value);

        // Set flow-mod-timeout.
        value = 60000;
        vcfg = new VtnConfigBuilder().setFlowModTimeout(value).build();
        vcfg = new VTNConfigImpl(vcfg).toVtnConfig();
        new TestVtnConfigBuilder().set(ConfigType.FLOW_MOD_TIMEOUT, value).
            verify(vcfg);
        assertEquals(null, vcfg.isInitState());

        value = 100;
        all.setFlowModTimeout(value);
        allTest.set(ConfigType.FLOW_MOD_TIMEOUT, value);

        // Set bulk-flow-mod-timeout.
        value = 600000;
        vcfg = new VtnConfigBuilder().setBulkFlowModTimeout(value).build();
        vcfg = new VTNConfigImpl(vcfg).toVtnConfig();
        new TestVtnConfigBuilder().
            set(ConfigType.BULK_FLOW_MOD_TIMEOUT, value).verify(vcfg);
        assertEquals(null, vcfg.isInitState());

        value = 3000;
        all.setBulkFlowModTimeout(value);
        allTest.set(ConfigType.BULK_FLOW_MOD_TIMEOUT, value);

        // Set init-timeout.
        value = 600000;
        vcfg = new VtnConfigBuilder().setInitTimeout(value).build();
        vcfg = new VTNConfigImpl(vcfg).toVtnConfig();
        new TestVtnConfigBuilder().set(ConfigType.INIT_TIMEOUT, value).
            verify(vcfg);
        assertEquals(null, vcfg.isInitState());

        value = 100;
        all.setInitTimeout(value);
        allTest.set(ConfigType.INIT_TIMEOUT, value);

        // Set max-redirections.
        value = 100000;
        vcfg = new VtnConfigBuilder().setMaxRedirections(value).build();
        vcfg = new VTNConfigImpl(vcfg).toVtnConfig();
        new TestVtnConfigBuilder().set(ConfigType.MAX_REDIRECTIONS, value).
            verify(vcfg);
        assertEquals(null, vcfg.isInitState());

        value = 10;
        all.setMaxRedirections(value);
        allTest.set(ConfigType.MAX_REDIRECTIONS, value);

        // Set controller-mac-address.
        MacAddress macAddr = new MacAddress("12:34:56:78:9a:bc");
        vcfg = new VtnConfigBuilder().setControllerMacAddress(macAddr).
            build();
        vcfg = new VTNConfigImpl(vcfg).toVtnConfig();
        new TestVtnConfigBuilder().
            set(ConfigType.CONTROLLER_MAC_ADDRESS, macAddr).verify(vcfg);
        assertEquals(null, vcfg.isInitState());

        macAddr = new MacAddress("aa:bb:cc:dd:ee:00");
        vcfg = all.setControllerMacAddress(macAddr).build();
        vcfg = new VTNConfigImpl(vcfg).toVtnConfig();
        allTest.set(ConfigType.CONTROLLER_MAC_ADDRESS, macAddr).verify(all);
        assertEquals(null, vcfg.isInitState());
    }

    /**
     * Test case for {@link VTNConfigImpl#equals(Object)} and
     * {@link VTNConfigImpl#hashCode()}.
     */
    @Test
    public void testEquals() {
        Set<Object> set = new HashSet<>();
        VtnConfigBuilder builder = new VtnConfigBuilder();
        testEquals(set, new VTNConfigImpl(), new VTNConfigImpl());

        Integer[] neWaits = {
            Integer.valueOf(0), Integer.valueOf(1), Integer.valueOf(2),
            Integer.valueOf(1000), Integer.valueOf(599999),
            Integer.valueOf(600000),
        };
        for (Integer v: neWaits) {
            builder.setTopologyWait(v);
            VTNConfigImpl vconf1 = new VTNConfigImpl(builder.build());
            VTNConfigImpl vconf2 = new VTNConfigImpl(builder.build());
            assertEquals(v.intValue(), vconf1.getTopologyWait());
            testEquals(set, vconf1, vconf2);
        }

        Integer[] l2Priorities = {
            Integer.valueOf(1), Integer.valueOf(2), Integer.valueOf(3),
            Integer.valueOf(400), Integer.valueOf(700),
            Integer.valueOf(997), Integer.valueOf(998), Integer.valueOf(999),
        };
        for (Integer v: l2Priorities) {
            builder.setL2FlowPriority(v);
            VTNConfigImpl vconf1 = new VTNConfigImpl(builder.build());
            VTNConfigImpl vconf2 = new VTNConfigImpl(builder.build());
            assertEquals(v.intValue(), vconf1.getL2FlowPriority());
            testEquals(set, vconf1, vconf2);
        }

        Integer[] flowModTimeouts = {
            Integer.valueOf(100), Integer.valueOf(101), Integer.valueOf(102),
            Integer.valueOf(10000), Integer.valueOf(33333),
            Integer.valueOf(59998), Integer.valueOf(59999),
            Integer.valueOf(60000),
        };
        for (Integer v: flowModTimeouts) {
            builder.setFlowModTimeout(v);
            VTNConfigImpl vconf1 = new VTNConfigImpl(builder.build());
            VTNConfigImpl vconf2 = new VTNConfigImpl(builder.build());
            assertEquals(v.intValue(), vconf1.getFlowModTimeout());
            testEquals(set, vconf1, vconf2);
        }

        Integer[] bulkFlowModTimeouts = {
            Integer.valueOf(3000), Integer.valueOf(3001),
            Integer.valueOf(3002), Integer.valueOf(12345),
            Integer.valueOf(400000), Integer.valueOf(599998),
            Integer.valueOf(599999), Integer.valueOf(600000),
        };
        for (Integer v: bulkFlowModTimeouts) {
            builder.setBulkFlowModTimeout(v);
            VTNConfigImpl vconf1 = new VTNConfigImpl(builder.build());
            VTNConfigImpl vconf2 = new VTNConfigImpl(builder.build());
            assertEquals(v.intValue(), vconf1.getBulkFlowModTimeout());
            testEquals(set, vconf1, vconf2);
        }

        Integer[] initTimeouts = {
            Integer.valueOf(100), Integer.valueOf(101), Integer.valueOf(102),
            Integer.valueOf(22222), Integer.valueOf(333333),
            Integer.valueOf(444555), Integer.valueOf(599998),
            Integer.valueOf(599999), Integer.valueOf(600000),
        };
        for (Integer v: initTimeouts) {
            builder.setInitTimeout(v);
            VTNConfigImpl vconf1 = new VTNConfigImpl(builder.build());
            VTNConfigImpl vconf2 = new VTNConfigImpl(builder.build());
            assertEquals(v.intValue(), vconf1.getInitTimeout());
            testEquals(set, vconf1, vconf2);
        }

        Integer[] maxRedirections = {
            Integer.valueOf(10), Integer.valueOf(11), Integer.valueOf(12),
            Integer.valueOf(2345), Integer.valueOf(34567),
            Integer.valueOf(56789), Integer.valueOf(99998),
            Integer.valueOf(99999), Integer.valueOf(100000),
        };
        for (Integer v: maxRedirections) {
            builder.setMaxRedirections(v);
            VTNConfigImpl vconf1 = new VTNConfigImpl(builder.build());
            VTNConfigImpl vconf2 = new VTNConfigImpl(builder.build());
            assertEquals(v.intValue(), vconf1.getMaxRedirections());
            testEquals(set, vconf1, vconf2);
        }

        String[] addrs = {
            "aa:bb:cc:dd:ee:ff",
            "99:88:77:66:55:44",
            "12:34:56:78:9A:BC",
            "DD:EE:FF:10:20:30",
        };
        for (String addr: addrs) {
            EtherAddress mac = new EtherAddress(addr);
            builder.setControllerMacAddress(new MacAddress(addr));
            VTNConfigImpl vconf1 = new VTNConfigImpl(builder.build());
            VTNConfigImpl vconf2 = new VTNConfigImpl(builder.build());
            assertEquals(mac, vconf1.getControllerMacAddress());
            testEquals(set, vconf1, vconf2);
        }

        int expected = neWaits.length + l2Priorities.length +
            flowModTimeouts.length + bulkFlowModTimeouts.length +
            initTimeouts.length + maxRedirections.length + addrs.length + 1;
        assertEquals(expected, set.size());
    }

    /**
     * Ensure that {@link VTNConfigImpl} is bound to XML root element.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testJAXB() throws Exception {
        VTNConfigImpl vconf =
            jaxbTest(new VTNConfigImpl(), VTNConfigImpl.class, XML_ROOT);
        TestVTNConfig test = new TestVTNConfig().verify(vconf);
        VtnConfigBuilder allBuilder = new VtnConfigBuilder();
        TestVTNConfig allTest = new TestVTNConfig();
        assertEquals(allBuilder.build(), vconf.getJaxbValue().build());

        ConfigType type = ConfigType.TOPOLOGY_WAIT;
        Integer[] neWaits = {
            Integer.valueOf(0), Integer.valueOf(1), Integer.valueOf(2),
            Integer.valueOf(1000), Integer.valueOf(599999),
            Integer.valueOf(600000),
        };
        for (Integer v: neWaits) {
            VtnConfig vcfg = new VtnConfigBuilder().setTopologyWait(v).build();
            vconf = jaxbTest(new VTNConfigImpl(vcfg), VTNConfigImpl.class,
                             XML_ROOT);
            test.resetIntegers().set(type, v).verify(vconf);
            assertEquals(vcfg, vconf.getJaxbValue().build());

            allBuilder.setTopologyWait(v);
            VtnConfig all = allBuilder.build();
            vconf = jaxbTest(new VTNConfigImpl(all), VTNConfigImpl.class,
                             XML_ROOT);
            allTest.set(type, v).verify(vconf);
            assertEquals(all, vconf.getJaxbValue().build());
        }
        vconf.setJaxbTopologyWait(null);
        assertEquals(null, vconf.getJaxbTopologyWait());

        type = ConfigType.L2_FLOW_PRIORITY;
        Integer[] l2Priorities = {
            Integer.valueOf(1), Integer.valueOf(2), Integer.valueOf(3),
            Integer.valueOf(400), Integer.valueOf(700),
            Integer.valueOf(997), Integer.valueOf(998), Integer.valueOf(999),
        };
        for (Integer v: l2Priorities) {
            VtnConfig vcfg = new VtnConfigBuilder().setL2FlowPriority(v).
                build();
            vconf = jaxbTest(new VTNConfigImpl(vcfg), VTNConfigImpl.class,
                             XML_ROOT);
            test.resetIntegers().set(type, v).verify(vconf);
            assertEquals(vcfg, vconf.getJaxbValue().build());

            allBuilder.setL2FlowPriority(v);
            VtnConfig all = allBuilder.build();
            vconf = jaxbTest(new VTNConfigImpl(all), VTNConfigImpl.class,
                             XML_ROOT);
            allTest.set(type, v).verify(vconf);
            assertEquals(all, vconf.getJaxbValue().build());
        }
        vconf.setJaxbL2FlowPriority(null);
        assertEquals(null, vconf.getJaxbL2FlowPriority());

        type = ConfigType.FLOW_MOD_TIMEOUT;
        Integer[] flowModTimeouts = {
            Integer.valueOf(100), Integer.valueOf(101), Integer.valueOf(102),
            Integer.valueOf(10000), Integer.valueOf(33333),
            Integer.valueOf(59998), Integer.valueOf(59999),
            Integer.valueOf(60000),
        };
        for (Integer v: flowModTimeouts) {
            VtnConfig vcfg = new VtnConfigBuilder().setFlowModTimeout(v).
                build();
            vconf = jaxbTest(new VTNConfigImpl(vcfg), VTNConfigImpl.class,
                             XML_ROOT);
            test.resetIntegers().set(type, v).verify(vconf);
            assertEquals(vcfg, vconf.getJaxbValue().build());

            allBuilder.setFlowModTimeout(v);
            VtnConfig all = allBuilder.build();
            vconf = jaxbTest(new VTNConfigImpl(all), VTNConfigImpl.class,
                             XML_ROOT);
            allTest.set(type, v).verify(vconf);
            assertEquals(all, vconf.getJaxbValue().build());
        }
        vconf.setJaxbFlowModTimeout(null);
        assertEquals(null, vconf.getJaxbFlowModTimeout());

        type = ConfigType.BULK_FLOW_MOD_TIMEOUT;
        Integer[] bulkFlowModTimeouts = {
            Integer.valueOf(3000), Integer.valueOf(3001),
            Integer.valueOf(3002), Integer.valueOf(12345),
            Integer.valueOf(400000), Integer.valueOf(599998),
            Integer.valueOf(599999), Integer.valueOf(600000),
        };
        for (Integer v: bulkFlowModTimeouts) {
            VtnConfig vcfg = new VtnConfigBuilder().setBulkFlowModTimeout(v).
                build();
            vconf = jaxbTest(new VTNConfigImpl(vcfg), VTNConfigImpl.class,
                             XML_ROOT);
            test.resetIntegers().set(type, v).verify(vconf);
            assertEquals(vcfg, vconf.getJaxbValue().build());

            allBuilder.setBulkFlowModTimeout(v);
            VtnConfig all = allBuilder.build();
            vconf = jaxbTest(new VTNConfigImpl(all), VTNConfigImpl.class,
                             XML_ROOT);
            allTest.set(type, v).verify(vconf);
            assertEquals(all, vconf.getJaxbValue().build());
        }
        vconf.setJaxbBulkFlowModTimeout(null);
        assertEquals(null, vconf.getJaxbBulkFlowModTimeout());

        type = ConfigType.INIT_TIMEOUT;
        Integer[] initTimeouts = {
            Integer.valueOf(100), Integer.valueOf(101), Integer.valueOf(102),
            Integer.valueOf(22222), Integer.valueOf(333333),
            Integer.valueOf(444555), Integer.valueOf(599998),
            Integer.valueOf(599999), Integer.valueOf(600000),
        };
        for (Integer v: initTimeouts) {
            VtnConfig vcfg = new VtnConfigBuilder().setInitTimeout(v).build();
            vconf = jaxbTest(new VTNConfigImpl(vcfg), VTNConfigImpl.class,
                             XML_ROOT);
            test.resetIntegers().set(type, v).verify(vconf);
            assertEquals(vcfg, vconf.getJaxbValue().build());

            allBuilder.setInitTimeout(v);
            VtnConfig all = allBuilder.build();
            vconf = jaxbTest(new VTNConfigImpl(all), VTNConfigImpl.class,
                             XML_ROOT);
            allTest.set(type, v).verify(vconf);
            assertEquals(all, vconf.getJaxbValue().build());
        }
        vconf.setJaxbInitTimeout(null);
        assertEquals(null, vconf.getJaxbInitTimeout());

        type = ConfigType.MAX_REDIRECTIONS;
        Integer[] maxRedirections = {
            Integer.valueOf(10), Integer.valueOf(11), Integer.valueOf(12),
            Integer.valueOf(2345), Integer.valueOf(34567),
            Integer.valueOf(56789), Integer.valueOf(99998),
            Integer.valueOf(99999), Integer.valueOf(100000),
        };
        for (Integer v: maxRedirections) {
            VtnConfig vcfg = new VtnConfigBuilder().setMaxRedirections(v).
                build();
            vconf = jaxbTest(new VTNConfigImpl(vcfg), VTNConfigImpl.class,
                             XML_ROOT);
            test.resetIntegers().set(type, v).verify(vconf);
            assertEquals(vcfg, vconf.getJaxbValue().build());

            allBuilder.setMaxRedirections(v);
            VtnConfig all = allBuilder.build();
            vconf = jaxbTest(new VTNConfigImpl(all), VTNConfigImpl.class,
                             XML_ROOT);
            allTest.set(type, v).verify(vconf);
            assertEquals(all, vconf.getJaxbValue().build());
        }
        vconf.setJaxbMaxRedirections(null);
        assertEquals(null, vconf.getJaxbMaxRedirections());

        type = ConfigType.CONTROLLER_MAC_ADDRESS;
        String[] addrs = {
            "aa:bb:cc:dd:ee:ff",
            "99:88:77:66:55:44",
            "12:34:56:78:9a:bc",
            "dd:ee:ff:10:20:30",
        };
        for (String addr: addrs) {
            MacAddress maddr = new MacAddress(addr);
            VtnConfig vcfg = new VtnConfigBuilder().
                setControllerMacAddress(maddr).build();
            vconf = jaxbTest(new VTNConfigImpl(vcfg), VTNConfigImpl.class,
                             XML_ROOT);
            test.resetIntegers().set(type, maddr).verify(vconf);
            assertEquals(vcfg, vconf.getJaxbValue().build());

            allBuilder.setControllerMacAddress(maddr);
            VtnConfig all = allBuilder.build();
            vconf = jaxbTest(new VTNConfigImpl(all), VTNConfigImpl.class,
                             XML_ROOT);
            allTest.set(type, maddr).verify(vconf);
            assertEquals(all, vconf.getJaxbValue().build());
        }

        // Specifying invalid MAC address.
        allBuilder.setControllerMacAddress(null);
        VTNConfigImpl expected = new VTNConfigImpl(allBuilder.build());
        String[] invalidAddrs = {
            "",
            "invalid_MAC_address",
            "aa:bb:cc:dd:ee:ff:00",
            "ff",
            "a0:b0:c0:d0:e0:g0",
        };

        Unmarshaller um = createUnmarshaller(VTNConfigImpl.class);
        for (String maddr: invalidAddrs) {
            StringBuilder sb = new StringBuilder(XML_DECLARATION).
                append('<').append(XML_ROOT).append('>');
            for (ConfigType ctype: ConfigType.values()) {
                Object value = (ctype == ConfigType.CONTROLLER_MAC_ADDRESS)
                    ? maddr : ctype.get(expected, VTNConfig.class);
                sb.append(ctype.getXmlElement(value));
            }

            String xml = sb.append("</").append(XML_ROOT).append('>').
                toString();
            ByteArrayInputStream in = new ByteArrayInputStream(xml.getBytes());
            try {
                um.unmarshal(in);
                unexpected();
            } catch (UnmarshalException e) {
            }
        }

        // Ensure that broken values in XML can be detected.
        jaxbErrorTest(VTNConfigImpl.class,
                      new XmlValueType("controller-mac-address",
                                       EtherAddress.class).add(XML_ROOT),
                      new XmlValueType("topology-wait", Integer.class).
                      add(XML_ROOT),
                      new XmlValueType("l2-flow-priority", Integer.class).
                      add(XML_ROOT),
                      new XmlValueType("flow-mod-timeout", Integer.class).
                      add(XML_ROOT),
                      new XmlValueType("bulk-flow-mod-timeout", Integer.class).
                      add(XML_ROOT),
                      new XmlValueType("init-timeout", Integer.class).
                      add(XML_ROOT),
                      new XmlValueType("max-redirections", Integer.class).
                      add(XML_ROOT));
    }
}
