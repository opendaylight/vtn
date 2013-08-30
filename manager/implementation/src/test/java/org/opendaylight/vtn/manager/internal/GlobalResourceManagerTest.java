/**
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
import java.util.Timer;
import java.util.concurrent.ConcurrentMap;

import org.apache.felix.dm.impl.ComponentImpl;
import org.junit.Before;
import org.junit.Test;
import org.opendaylight.controller.clustering.services.IClusterGlobalServices;
import org.opendaylight.controller.sal.core.Node;
import org.opendaylight.controller.sal.core.NodeConnector;
import org.opendaylight.controller.sal.utils.GlobalConstants;
import org.opendaylight.controller.sal.utils.NodeConnectorCreator;
import org.opendaylight.controller.sal.utils.NodeCreator;
import org.opendaylight.vtn.manager.VBridgeIfPath;
import org.opendaylight.vtn.manager.VBridgePath;
import org.opendaylight.vtn.manager.internal.cluster.PortVlan;

/**
 * @author kawagish
 *
 */
public class GlobalResourceManagerTest extends TestBase {

    @Before
    public void before() {
    }


    /**
     * Test method for
     * {@link GlobalResourceManager#init(org.apache.felix.dm.Component)},
     * {@link GlobalResourceManager#destroy()},
     * {@link GlobalResourceManager#getTimer()}.
     */
    @Test
    public void testInitDestroy() {
        ComponentImpl c = new ComponentImpl(null, null, null);
        GlobalResourceManager grsc = new GlobalResourceManager();
        TestStub stubObj = new TestStub(0);
        IClusterGlobalServices cs = (IClusterGlobalServices)stubObj;

        Hashtable<String, String> properties = new Hashtable<String, String>();
        properties.put("containerName", "default");
        c.setServiceProperties(properties);

        grsc.destroy();

        grsc.setClusterGlobalService(stubObj);
        grsc.init(c);

        Timer tm = grsc.getTimer();
        assertNotNull(tm);

        ConcurrentMap<Short, String> vmap = (ConcurrentMap<Short, String>)cs.getCache("vtn.vlanmap");
        assertNotNull(vmap);
        assertEquals(0, vmap.size());

        ConcurrentMap<Short, String> pmap = (ConcurrentMap<Short, String>)cs.getCache("vtn.portmap");
        assertNotNull(pmap);
        assertEquals(0, pmap.size());

        grsc.init(c);


        grsc.destroy();

        grsc.unsetClusterGlobalService(stubObj);
        grsc.init(c);
    }


    /**
     * Test method for
     * {@link GlobalResourceManager#setClusterGlobalService(IClusterGlobalServices)},
     * {@link GlobalResourceManager#unsetClusterGlobalService(IClusterGlobalServices)}.
     */
    @Test
    public void testSetUnsetClusterGlobalService() {
        ComponentImpl c = new ComponentImpl(null, null, null);
        GlobalResourceManager grsc = new GlobalResourceManager();
        TestStub stubObj = new TestStub(0);
        TestStub stubObj2 = new TestStub(0);

        Hashtable<String, String> properties = new Hashtable<String, String>();
        properties.put("containerName", "default");
        c.setServiceProperties(properties);

        grsc.setClusterGlobalService(stubObj);

        grsc.unsetClusterGlobalService(stubObj2);

        grsc.unsetClusterGlobalService(stubObj);
    }

    /**
     * Test method for
     * {@link GlobalResourceManager#registerVlanMap(String, VBridgePath, short)},
     * {@link GlobalResourceManager#unregisterVlanMap(short)}.
     */
    @Test
    public void testRegisterVlanMap() {
        String containerName = "default";
        TestStub stubObj = new TestStub(0);
        GlobalResourceManager grsc = setupGlobalResourceManager(containerName, stubObj);
        IClusterGlobalServices cs = (IClusterGlobalServices)stubObj;

        String tname = "tenant";
        String bname = "bridge";
        VBridgePath bpath = new VBridgePath(tname, bname);
        short vlan = 0;
        String reg = grsc.registerVlanMap(containerName, bpath, vlan);
        assertEquals(null, reg);
        checkMapCache(cs, "vtn.vlanmap", containerName, bpath, vlan);

        String bnamenew = "bridgenew";
        VBridgePath bpathnew = new VBridgePath(tname, bnamenew);
        reg = grsc.registerVlanMap(containerName, bpathnew, vlan);
        String required = containerName + ":" + bpath.toString();
        assertEquals(required, reg);
        checkMapCache(cs, "vtn.vlanmap", containerName, bpath, vlan);

        reg = grsc.registerVlanMap(containerName, bpathnew, (short)4095);
        required = containerName + ":" + bpath.toString();
        assertEquals(null, reg);
        checkMapCache(cs, "vtn.vlanmap", containerName, bpathnew, (short)4095);

        grsc.unregisterVlanMap((short)0);

        reg = grsc.registerVlanMap(containerName, bpath, (short)4095);
        required = containerName + ":" + bpathnew.toString();
        assertEquals(required, reg);
        checkMapCache(cs, "vtn.vlanmap", containerName, bpathnew, (short)4095);

        reg = grsc.registerVlanMap(containerName, bpathnew, (short)0);
        required = containerName + ":" + bpathnew.toString();
        assertEquals(null, reg);
        checkMapCache(cs, "vtn.vlanmap", containerName, bpathnew, (short)0);

        try {
            grsc.unregisterVlanMap((short)1);
            fail("throwing exception was expected.");
        } catch (IllegalStateException e) {

        }

        grsc.unregisterVlanMap((short)0);
        grsc.unregisterVlanMap((short)4095);
    }


    /**
     * Test method for
     * {@link GlobalResourceManager#registerPortMap(String, VBridgeIfPath, PortVlan)},
     * {@link GlobalResourceManager#unregisterPortMap(PortVlan)}.
     */
    @Test
    public void testRegisterPortMap() {
        short[] vlans = {0, 10, 4095};
        String containerName = "default";
        TestStub stubObj = new TestStub(0);
        GlobalResourceManager grsc = setupGlobalResourceManager(containerName, stubObj);
        IClusterGlobalServices cs = (IClusterGlobalServices)stubObj;

        for (NodeConnector nc : createNodeConnectors(3, false)) {
            for (short vlan : vlans) {
                String tname = "tenant";
                String bname = "bridge";
                String ifname = "interface";
                VBridgeIfPath bifpath = new VBridgeIfPath(tname, bname, ifname);
                PortVlan pv = new PortVlan(nc, vlan);
                String reg = grsc.registerPortMap(containerName, bifpath, pv);
                assertEquals(null, reg);
                checkMapCache(cs, "vtn.portmap", containerName, bifpath, pv);

                String bnamenew = "bridgenew";
                String ifnamenew = "interfacenew";
                VBridgeIfPath bifpathnew = new VBridgeIfPath(tname, bnamenew, ifnamenew);
                reg = grsc.registerPortMap(containerName, bifpathnew, pv);
                String required = containerName + ":" + bifpath.toString();
                assertEquals(required, reg);
                checkMapCache(cs, "vtn.portmap", containerName, bifpath, pv);

                PortVlan pvnew = new PortVlan(nc, (short)4094);
                reg = grsc.registerPortMap(containerName, bifpathnew, pvnew);
                required = containerName + ":" + bifpathnew.toString();
                assertEquals(null, reg);
                checkMapCache(cs, "vtn.portmap", containerName, bifpathnew, pvnew);

                grsc.unregisterPortMap(pv);

                reg = grsc.registerPortMap(containerName, bifpath, pvnew);
                required = containerName + ":" + bifpathnew.toString();
                assertEquals(required, reg);
                checkMapCache(cs, "vtn.portmap", containerName, bifpathnew, pvnew);

                reg = grsc.registerPortMap(containerName, bifpathnew, pv);
                required = containerName + ":" + bifpathnew.toString();
                assertEquals(null, reg);
                checkMapCache(cs, "vtn.portmap", containerName, bifpathnew, pv);

                try {
                    grsc.unregisterPortMap(new PortVlan(nc, (short)1));
                    fail("throwing exception was expected.");
                } catch (IllegalStateException e) {

                }

                grsc.unregisterPortMap(pvnew);
                grsc.unregisterPortMap(pv);
            }
        }
    }

    /**
     * Test method for
     * {@link GlobalResourceManager#cleanUp(String)}.
     */
    @Test
    public void testCleanUp() {
        String containerName = "default";
        TestStub stubObj = new TestStub(0);
        GlobalResourceManager grsc = setupGlobalResourceManager(containerName, stubObj);
        IClusterGlobalServices cs = (IClusterGlobalServices)stubObj;

        String tname = "tenant";
        String bname = "bridge";
        VBridgePath bpath = new VBridgePath(tname, bname);
        short vlan = 0;
        String reg = grsc.registerVlanMap(containerName, bpath, vlan);
        assertEquals(null, reg);
        checkMapCache(cs, "vtn.vlanmap", containerName, bpath, vlan);

        Node node = NodeCreator.createOFNode(Long.valueOf("0"));
        NodeConnector nc = NodeConnectorCreator.createOFNodeConnector(Short.valueOf((short)10), node);
        String ifname = "interface";
        VBridgeIfPath bifpath = new VBridgeIfPath(tname, bname, ifname);
        PortVlan pv = new PortVlan(nc, vlan);
        reg = grsc.registerPortMap(containerName, bifpath, pv);
        assertEquals(null, reg);
        checkMapCache(cs, "vtn.portmap", containerName, bifpath, pv);

        grsc.cleanUp("clean");

        ConcurrentMap<Short, String> vmap = (ConcurrentMap<Short, String>)cs.getCache("vtn.vlanmap");
        assertEquals(1, vmap.size());
        ConcurrentMap<PortVlan, String> pmap = (ConcurrentMap<PortVlan, String>)cs.getCache("vtn.portmap");
        assertEquals(1, pmap.size());

        grsc.cleanUp(containerName);

        vmap = (ConcurrentMap<Short, String>)cs.getCache("vtn.vlanmap");
        assertEquals(0, vmap.size());
        pmap = (ConcurrentMap<PortVlan, String>)cs.getCache("vtn.portmap");
        assertEquals(0, pmap.size());
    }

    /**
     * setup GlobalResourceManager
     * @param containerName container name.
     * @param stubObj TestStub object
     * @return GlobalResourceManager
     */
    private GlobalResourceManager setupGlobalResourceManager (String containerName, TestStub stubObj) {
        ComponentImpl c = new ComponentImpl(null, null, null);
        GlobalResourceManager grsc = new GlobalResourceManager();

        Hashtable<String, String> properties = new Hashtable<String, String>();
        properties.put("containerName", containerName);
        c.setServiceProperties(properties);

        grsc.setClusterGlobalService(stubObj);
        grsc.init(c);

        return grsc;
    }

    /**
     * check Map cache.
     * @param cs    IClusgerGlobalServices
     * @param cacheName a cache name.
     * @param containerName a container name.
     * @param path  VBridgePath or VBridgeIfPath
     * @param key   key value
     */
    private <T, S> void checkMapCache (IClusterGlobalServices cs, String cacheName,
            String containerName, T path, S key) {
        ConcurrentMap<S, String> map = (ConcurrentMap<S, String>)cs.getCache(cacheName);
        assertNotNull(map);
        String required = containerName + ":" + path.toString();
        String value = map.get(key);
        assertEquals(required, value);
    }

}
