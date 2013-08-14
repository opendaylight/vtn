/*
 * Copyright (c) 2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */
package org.opendaylight.vtn.manager.internal;

import static org.junit.Assert.*;

import java.io.File;
import java.util.Hashtable;
import java.util.List;


import org.apache.felix.dm.impl.ComponentImpl;
import org.junit.After;
import org.junit.Before;


import org.opendaylight.controller.sal.packet.ARP;
import org.opendaylight.controller.sal.packet.Ethernet;
import org.opendaylight.controller.sal.packet.IEEE8021Q;
import org.opendaylight.controller.sal.utils.EtherTypes;
import org.opendaylight.controller.sal.utils.GlobalConstants;
import org.opendaylight.controller.sal.utils.Status;
import org.opendaylight.vtn.manager.IVTNManager;
import org.opendaylight.vtn.manager.VBridgeConfig;
import org.opendaylight.vtn.manager.VBridgeIfPath;
import org.opendaylight.vtn.manager.VBridgePath;
import org.opendaylight.vtn.manager.VInterfaceConfig;
import org.opendaylight.vtn.manager.VTenantConfig;
import org.opendaylight.vtn.manager.VTenantPath;

/**
 * Common class for {@link VTNManagerImplTest} and {@link VTNManagerImplWithNodesTest}.
 *
 */
public class VTNManagerImplTestCommon extends TestBase {
    protected VTNManagerImpl vtnMgr = null;
    protected TestStub stubObj = null;
    protected static int stubMode = 0;

    @Before
    public void before() {
        File confdir = new File(GlobalConstants.STARTUPHOME.toString());
        boolean result = confdir.exists();
        if (!result) {
            result = confdir.mkdirs();
        } else {
            File[] list = confdir.listFiles();
            for (File f : list) {
                f.delete();
            }
        }

        vtnMgr = new VTNManagerImpl();
        ComponentImpl c = new ComponentImpl(null, null, null);
        GlobalResourceManager grsc = new GlobalResourceManager();
        stubObj = new TestStub(stubMode);

        Hashtable<String, String> properties = new Hashtable<String, String>();
        properties.put("containerName", "default");
        c.setServiceProperties(properties);

        grsc.setClusterGlobalService(stubObj);
        grsc.init(c);
        vtnMgr.setResourceManager(grsc);
        vtnMgr.setClusterContainerService(stubObj);
        vtnMgr.setSwitchManager(stubObj);
        vtnMgr.setTopologyManager(stubObj);
        vtnMgr.setDataPacketService(stubObj);
        vtnMgr.setRouting(stubObj);
        vtnMgr.setHostTracker(stubObj);
        vtnMgr.init(c);
    }

    @After
    public void after() {

        vtnMgr.destroy();

        String currdir = new File(".").getAbsoluteFile().getParent();
        File confdir = new File(GlobalConstants.STARTUPHOME.toString());

        if (confdir.exists()) {
            File[] list = confdir.listFiles();
            for (File f : list) {
                f.delete();
            }

            while (confdir != null && confdir.getAbsolutePath() != currdir) {
                confdir.delete();
                String pname = confdir.getParent();
                if (pname == null) {
                    break;
                }
                confdir = new File(pname);
            }
        }
    }


    /**
     * method for setup enviroment.
     * create 1 Tenant and bridges
     */
    protected void createTenantAndBridge(IVTNManager mgr, VTenantPath tpath,
            List<VBridgePath> bpaths) {

        Status st = mgr.addTenant(tpath, new VTenantConfig(null));
        assertTrue(st.isSuccess());
        assertTrue(mgr.isActive());

        for (VBridgePath bpath : bpaths) {
            st = mgr.addBridge(bpath, new VBridgeConfig(null));
            assertTrue(st.isSuccess());
        }
    }

    /**
     * method for setup enviroment.
     * create 1 Tenant and bridges and vinterfaces
     */
    protected void createTenantAndBridgeAndInterface(IVTNManager mgr, VTenantPath tpath,
            List<VBridgePath> bpaths, List<VBridgeIfPath> ifpaths) {

        Status st = mgr.addTenant(tpath, new VTenantConfig(null));
        assertTrue(st.isSuccess());
        assertTrue(mgr.isActive());

        for (VBridgePath bpath : bpaths) {
            st = mgr.addBridge(bpath, new VBridgeConfig(null));
            assertTrue(st.isSuccess());
        }

        for (VBridgeIfPath ifpath : ifpaths) {
            VInterfaceConfig ifconf = new VInterfaceConfig(null, null);
            st = mgr.addBridgeInterface(ifpath, ifconf);
            assertTrue(st.isSuccess());
        }
    }

    /**
     * check a Ethernet packet whether setted expected parametor in the packet.
     * (for IPv4 packet)
     *
     * @param msg   if check is failed, report error with a message specified this.
     * @param eth   input ethernet frame data.
     * @param ethType   expected ethernet type.
     * @param destMac   expected destination mac address.
     * @param srcMac    expected source mac address.
     * @param vlan  expected vlan id. (if expected untagged, specify 0 or less than 0)
     */
    protected void checkOutEthernetPacketIPv4 (String msg, Ethernet eth, EtherTypes ethType,
            byte[] srcMac, byte[] destMac,  short vlan) {

        checkOutEthernetPacket(msg, eth, ethType, srcMac, destMac, vlan, null, (short)-1,
                null, null, null, null);
    }

    /**
     * check a Ethernet packet whether setted expected parametor in the packet.
     *
     * @param msg   if check is failed, report error with a message specified this.
     * @param eth   input ethernet frame data.
     * @param ethType   expected ethernet type.
     * @param destMac   expected destination mac address.
     * @param srcMac    expected source mac address.
     * @param vlan  expected vlan id. (if expected untagged, specify 0 or less than 0)
     * @param protoType expected protocol type.
     * @param opCode    expected opCode. if thish is not ARP, opCode is not checked.
     * @param senderMac expected sender HW address.
     * @param targetMac expected target HW address.
     * @param senderAddr    expected sender protocol address.
     * @param targetAddr    expected target protocol address.
     *
     */
    protected void checkOutEthernetPacket (String msg, Ethernet eth, EtherTypes ethType,
            byte[] srcMac, byte[] destMac,  short vlan, EtherTypes protoType, short opCode,
            byte[] senderMac, byte[] targetMac, byte[] senderAddr, byte [] targetAddr) {

        ARP arp = null;
        if (vlan > 0) {
            assertEquals(msg, EtherTypes.VLANTAGGED.shortValue(), eth.getEtherType());
            IEEE8021Q vlantag = (IEEE8021Q)eth.getPayload();
            assertEquals(msg, vlan, vlantag.getVid());
            assertEquals(msg, ethType.shortValue(), vlantag.getEtherType());
            if (ethType.shortValue() == EtherTypes.ARP.shortValue()) {
                arp = (ARP)vlantag.getPayload();
            }
        } else {
            assertEquals(msg, ethType.shortValue(), eth.getEtherType());
            if (ethType.shortValue() == EtherTypes.ARP.shortValue()) {
                arp = (ARP)eth.getPayload();
            }
        }

        if (srcMac != null) {
            assertArrayEquals(msg, srcMac, eth.getSourceMACAddress());
        }
        if (destMac != null) {
            assertArrayEquals(msg, destMac, eth.getDestinationMACAddress());
        }

        if (ethType.shortValue() == EtherTypes.ARP.shortValue()) {
            assertEquals(msg, protoType.shortValue(), arp.getProtocolType());
            assertEquals(msg, opCode, arp.getOpCode());
            if (senderMac != null) {
                assertArrayEquals(msg, senderMac, arp.getSenderHardwareAddress());
            }
            if (targetMac != null) {
                assertArrayEquals(msg, targetMac, arp.getTargetHardwareAddress());
            }
            if (senderAddr != null) {
                assertArrayEquals(msg, senderAddr, arp.getSenderProtocolAddress());
            }
            if (targetAddr != null) {
                assertArrayEquals(msg, targetAddr, arp.getTargetProtocolAddress());
            }
        }
    }

}
