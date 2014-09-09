/*
 * Copyright (c) 2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.flow;

import static org.junit.Assert.assertEquals;

import java.util.ArrayList;
import java.util.List;

import org.junit.Test;
import org.opendaylight.vtn.manager.DataLinkHost;
import org.opendaylight.vtn.manager.EthernetHost;
import org.opendaylight.vtn.manager.SwitchPort;
import org.opendaylight.controller.sal.packet.address.EthernetAddress;
import org.opendaylight.controller.sal.core.Node;
import org.opendaylight.controller.sal.utils.NodeCreator;

public class DataFlowFilterTest {
    /**
     * Test case for DataFlowFilter getter and setter methods
     */
    @Test
    public void testGetSetMethods() {
        DataFlowFilter dataFlowFilter = new DataFlowFilter();
        List<EthernetAddress> list = new ArrayList<EthernetAddress>();
        byte[][] addrbytes = {
            new byte[] {(byte)0x00, (byte)0x00, (byte)0x00,
                        (byte)0x00, (byte)0x00, (byte)0x00 },
            new byte[] {(byte)0x12, (byte)0x34, (byte)0x56,
                        (byte)0x78, (byte)0x9a, (byte)0xbc },
            new byte[] {(byte)0xfe, (byte)0xdc, (byte)0xba,
                        (byte)0x98, (byte)0x76, (byte)0x54 } };
        for (byte[] addr : addrbytes) {
            try {
                EthernetAddress ea = new EthernetAddress(addr);
                list.add(ea);
            } catch (Exception exception) {
                exception.printStackTrace();
                assertEquals(-1, 0);
            }
        }
        short vlan = 1;
        for (EthernetAddress ea : list) {
            DataLinkHost host = new EthernetHost(ea, vlan);
            dataFlowFilter.setSourceHost(host);
            assertEquals(host, dataFlowFilter.getSourceHost());
        }
        for (int i = 1; i <= 3; i++) {
            Node node = NodeCreator.createOFNode((long)i);
            dataFlowFilter.setNode(node);
            assertEquals(node, dataFlowFilter.getNode());
            String switchName = "Name" + i;
            String id = "ID" + i;
            String type = "Type" + i;
            SwitchPort port = new SwitchPort(switchName, id, type);
            dataFlowFilter.setSwitchPort(port);
            assertEquals(port, dataFlowFilter.getSwitchPort());
        }
    }
}
