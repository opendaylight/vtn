/*
 * Copyright (c) 2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */
package org.opendaylight.vtn.manager.internal.cluster;

import static org.junit.Assert.*;

import java.util.HashSet;
import java.util.List;

import org.junit.Test;
import org.opendaylight.controller.sal.packet.address.EthernetAddress;
import org.opendaylight.controller.sal.utils.HexEncode;
import org.opendaylight.controller.sal.utils.NetUtils;
import org.opendaylight.vtn.manager.internal.TestBase;

/**
 * JUnit test for {@link MacVlan}
 */
public class MacVlanTest extends TestBase {

    /**
     * Test case for getter methods.
     */
    @Test
    public void testGetter() {
        short[] vlans = new short[] {-1, 0, 1, 4095, 4096};
        for (EthernetAddress ea : createEthernetAddresses(false)){
            for (short vlan : vlans) {
                byte[] mac = (ea == null) ? null : ea.getValue();
                MacVlan mv = new MacVlan(mac, vlan);
                long macLongVal = NetUtils.byteArray6ToLong(mac);
                assertEquals(macLongVal, mv.getMacAddress());
                assertEquals(vlan, mv.getVlan());

                mv = new MacVlan(macLongVal, vlan);
                assertEquals(macLongVal, mv.getMacAddress());
                assertEquals(vlan, mv.getVlan());
            }
        }
    }

    /**
     * Test method for
     * {@link MacVlan#equals(Object)} and
     * {@link MacVlan#hashCode()}.
     */
    @Test
    public void testEquals() {
        HashSet<Object> set = new HashSet<Object>();
        short vlans[] = {-1, 0, 1, 10, 4095, 4096};
        List<EthernetAddress> ethers = createEthernetAddresses();

        for (EthernetAddress ea : ethers) {
            for (short vlan: vlans) {
                byte[] mac = (ea == null) ? null : ea.getValue();
                MacVlan mv1 = new MacVlan(mac, vlan);
                MacVlan mv2 = new MacVlan(mac, vlan);
                testEquals(set, mv1, mv2);
            }
        }

        int required = ethers.size() * vlans.length;
        assertEquals(required, set.size());

        set.clear();
        for (EthernetAddress ea : ethers) {
            for (short vlan: vlans) {
                byte[] mac = (ea == null) ? null : ea.getValue();
                long macLongVal = NetUtils.byteArray6ToLong(mac);
                MacVlan mv1 = new MacVlan(macLongVal, vlan);
                MacVlan mv2 = new MacVlan(macLongVal, vlan);
                testEquals(set, mv1, mv2);
            }
        }

        required = ethers.size() * vlans.length;
        assertEquals(required, set.size());
    }

    /**
     * Test method for {@link MacVlan#toString()}.
     */
    @Test
    public void testToString() {
        String prefix = "MacVlan[";
        String suffix = "]";
        short vlans[] = {-1, 0, 1, 10, 4095, 10000};

        for (EthernetAddress ea : createEthernetAddresses(false)) {
            for (short vlan: vlans) {
                byte[] mac = (ea == null) ? null : ea.getValue();
                MacVlan mvlan = new MacVlan(mac, vlan);
                String c = "addr=" + HexEncode.bytesToHexStringFormat(mac);
                String v = "vlan=" + vlan;
                String required = joinStrings(prefix, suffix, ",", c, v);
                assertEquals(required, mvlan.toString());
            }
        }
    }

    /**
     * Ensure that {@link MacVlan} is serializable.
     */
    @Test
    public void testSerialize() {
        short vlans[] = {-1, 0, 1, 100, 4095, 10000};
        for (EthernetAddress ea : createEthernetAddresses()) {
            for (short vlan: vlans) {
                byte[] mac = (ea == null) ? null : ea.getValue();
                MacVlan mvlan = new MacVlan(mac, vlan);
                serializeTest(mvlan);
            }
        }
    }
}
