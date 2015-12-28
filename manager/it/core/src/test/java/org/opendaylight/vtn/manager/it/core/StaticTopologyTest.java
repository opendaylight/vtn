/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.it.core;

import java.math.BigInteger;
import java.util.ArrayList;
import java.util.List;

import org.opendaylight.vtn.manager.it.ofmock.OfMockService;

/**
 * Test case for the static network topology configuration.
 */
public final class StaticTopologyTest extends TestMethodBase {
    /**
     * Construct a new instance.
     *
     * @param vit  A {@link VTNManagerIT} instance.
     */
    public StaticTopologyTest(VTNManagerIT vit) {
        super(vit);
    }

    // TestMethodBase

    /**
     * Run the test.
     *
     * @throws Exception  An error occurred.
     */
    @Override
    protected void runTest() throws Exception {
        VTNManagerIT vit = getTest();
        OfMockService ofmock = vit.getOfMockService();

        // Below are initial topology configured by ofmock.
        SwitchLinkMap linkMap = new SwitchLinkMap(ofmock);
        String port1000n1 = "openflow:1000:1";
        String port1000n2 = "openflow:1000:2";
        String port1001n1 = "openflow:1001:1";
        String port1002n1 = "openflow:1002:1";
        linkMap.add(port1000n1, port1001n1);
        linkMap.add(port1001n1, port1000n1);
        linkMap.add(port1000n2, port1002n1);
        linkMap.add(port1002n1, port1000n2);
        linkMap.verify();
        SwitchLinkMap initial = new SwitchLinkMap(linkMap);

        // Configuration should be ignored if the port is not present.
        StaticTopology stopo = new StaticTopology(ofmock);
        stopo.addSwitchLink("openflow:111:222", "openflow:333:444");
        stopo.addSwitchLink("unknown:1:2", "unknown:3:4");
        stopo.addEdgePort("openflow:555:666");
        stopo.addEdgePort("unknown:5:6");
        sleep(10);
        initial.verify();

        // Invalid configuration should be ignored.
        stopo.addSwitchLink(port1000n1, port1000n1);
        stopo.addSwitchLink(port1000n2, port1000n2);
        sleep(10);
        initial.verify();

        // Configure a static link from openflow:1000:1 to openflow:1001:2.
        String port1001n2 = "openflow:1001:2";
        stopo.addSwitchLink(port1000n1, port1001n2);
        stopo.apply();
        linkMap.add(port1000n1, port1001n2, true);
        linkMap.addIgnored(port1000n1, port1001n1);
        linkMap.verify();

        // Configure a static link from openflow:1001:2 to openflow:1000:1.
        stopo.addSwitchLink(port1001n2, port1000n1);
        stopo.apply();
        linkMap.add(port1001n2, port1000n1, true);
        linkMap.verify();

        // Configure "openflow:1001:1" as an edge port.
        stopo.addEdgePort(port1001n1);
        stopo.apply();
        linkMap.remove(port1001n1);
        linkMap.addIgnored(port1001n1, port1000n1);
        linkMap.verify();

        // Add the following configuration at once.
        //   add link between openflow:1000:2 and openflow:1002:2
        //   make openflow:1002:1 as an edge port
        String port1002n2 = "openflow:1002:2";
        stopo.addSwitchLink(port1000n2, port1002n2);
        stopo.addSwitchLink(port1002n2, port1000n2);
        stopo.addEdgePort(port1002n1);
        stopo.apply();
        linkMap.add(port1000n2, port1002n2, true);
        linkMap.add(port1002n2, port1000n2, true);
        linkMap.remove(port1002n1);
        linkMap.addIgnored(port1000n2, port1002n1);
        linkMap.addIgnored(port1002n1, port1000n2);
        linkMap.verify();

        // Add a static link between openflow:1001:3 and openflow:1002:3.
        String port1001n3 = "openflow:1001:3";
        String port1002n3 = "openflow:1002:3";
        stopo.addSwitchLink(port1001n3, port1002n3);
        stopo.addSwitchLink(port1002n3, port1001n3);
        stopo.apply();
        linkMap.add(port1001n3, port1002n3, true);
        linkMap.add(port1002n3, port1001n3, true);
        linkMap.verify();

        // Add the following configuration at once.
        //   add link between openflow:1000:1 and openflow:1001:3
        //   add link between openflow:1000:2 and openflow:1002:3
        //   add link between openflow:1001:2 and openflow:1002:2
        stopo.addSwitchLink(port1000n1, port1001n3);
        stopo.addSwitchLink(port1001n3, port1000n1);
        stopo.addSwitchLink(port1000n2, port1002n3);
        stopo.addSwitchLink(port1002n3, port1000n2);
        stopo.addSwitchLink(port1001n2, port1002n2);
        stopo.addSwitchLink(port1002n2, port1001n2);
        stopo.apply();
        linkMap.add(port1000n1, port1001n3, true);
        linkMap.add(port1001n3, port1000n1, true);
        linkMap.add(port1000n2, port1002n3, true);
        linkMap.add(port1002n3, port1000n2, true);
        linkMap.add(port1001n2, port1002n2, true);
        linkMap.add(port1002n2, port1001n2, true);
        linkMap.verify();

        // Put all the switch ports to static-edge-ports.
        StaticTopology stopo1 = new StaticTopology(stopo);
        SwitchLinkMap linkMap1 = new SwitchLinkMap(linkMap);
        String[] allPorts = {
            port1000n1,
            port1000n2,
            port1001n1,
            port1001n2,
            port1001n3,
            port1002n1,
            port1002n2,
            port1002n3,
        };
        for (String port: allPorts) {
            stopo1.addEdgePort(port);
            linkMap1.remove(port);
        }
        stopo1.apply();
        linkMap1.verify();
        stopo1 = null;
        linkMap1 = null;

        // Restore the configuration.
        stopo.apply();
        linkMap.verify();

        // Configure static network topology for new switches.
        //   openflow:1:1 <-> openflow:2:1
        //   openflow:1:2 <-> openflow:3:1
        //   openflow:1:3 <-> openflow:4:1
        //   openflow:2:2 <-> openflow:3:2
        //   openflow:2:3 <-> openflow:4:2
        //   openflow:3:3 <-> openflow:4:3
        for (int i = 1; i <= 3; i++) {
            for (int j = i; j <= 3; j++) {
                String src = String.format("openflow:%d:%d", i, j);
                String dst = String.format("openflow:%d:%d", j + 1, i);
                stopo.addSwitchLink(src, dst);
                stopo.addSwitchLink(dst, src);
            }
        }

        stopo.apply();
        sleep(10);
        linkMap.verify();
        SwitchLinkMap savedLinks = new SwitchLinkMap(linkMap);

        // Create 4 nodes, and 4 ports per node.
        List<String> newNodes = new ArrayList<>();
        List<String> newPorts = new ArrayList<>();
        for (long dpid = 1L; dpid <= 4L; dpid++) {
            newNodes.add(ofmock.addNode(BigInteger.valueOf(dpid)));
            String nid = "openflow:" + dpid;
            for (long idx = 1; idx <= 4L; idx++) {
                newPorts.add(ofmock.addPort(nid, idx, false));
            }
        }

        // Static network topology should not be changed because all the
        // created ports are still in DOWN state.
        sleep(10);
        savedLinks.verify();

        // Up all the created ports.
        // Static network topology configuration should be appied to newly
        // detected inventories.
        for (String pid: newPorts) {
            ofmock.setPortState(pid, true, false);
        }
        for (int i = 1; i <= 3; i++) {
            for (int j = i; j <= 3; j++) {
                String src = String.format("openflow:%d:%d", i, j);
                String dst = String.format("openflow:%d:%d", j + 1, i);
                linkMap.add(src, dst, true);
                linkMap.add(dst, src, true);
            }
        }
        linkMap.verify();

        // Establish dynamic links.
        // They will be put into ignored-links container because static links
        // are already established.
        String port1n1 = "openflow:1:1";
        String port2n2 = "openflow:2:2";
        String port3n3 = "openflow:3:3";
        String port4n3 = "openflow:4:3";
        ofmock.setPeerIdentifier(port1n1, port2n2, false);
        ofmock.setPeerIdentifier(port2n2, port1n1, false);
        ofmock.setPeerIdentifier(port3n3, port4n3, false);
        ofmock.setPeerIdentifier(port4n3, port3n3, false);

        linkMap.addIgnored(port1n1, port2n2);
        linkMap.addIgnored(port2n2, port1n1);
        linkMap.addIgnored(port3n3, port4n3);
        linkMap.addIgnored(port4n3, port3n3);
        linkMap.verify();

        // Down port openflow:1:1, openflow:1:3, and openflow:2:2.
        // Below static links should be removed.
        //   openflow:1:1 <-> openflow:2:1
        //   openflow:1:3 <-> openflow:4:1
        //   openflow:2:2 <-> openflow:3:2
        String port1n3 = "openflow:1:3";
        String port2n1 = "openflow:2:1";
        String port3n2 = "openflow:3:2";
        String port4n1 = "openflow:4:1";
        ofmock.setPortState(port1n1, false, false);
        ofmock.setPortState(port1n3, false, false);
        ofmock.setPortState(port2n2, false, false);
        linkMap1 = new SwitchLinkMap(linkMap);
        linkMap1.remove(port1n1);
        linkMap1.remove(port2n1);
        linkMap1.remove(port1n3);
        linkMap1.remove(port4n1);
        linkMap1.remove(port2n2);
        linkMap1.remove(port3n2);

        // openflow:1:1 <-> openflow:2:2 in ignored-links should also be
        // removed because topology-manager will remove it.
        linkMap1.removeIgnored(port1n1);
        linkMap1.removeIgnored(port2n2);
        linkMap1.verify();
        linkMap1 = null;

        // Restore port state.
        ofmock.setPortState(port1n1, true, false);
        ofmock.setPortState(port1n3, true, false);
        ofmock.setPortState(port2n2, true, false);
        linkMap.verify();

        // Delete static link configuration for openflow:3:3 <-> openflow:4:3.
        stopo.removeSwitchLink(port3n3);
        stopo.removeSwitchLink(port4n3);
        stopo.apply();
        linkMap.add(port3n3, port4n3);
        linkMap.add(port4n3, port3n3);
        linkMap.removeIgnored(port3n3);
        linkMap.removeIgnored(port4n3);
        linkMap.verify();

        // Add openflow:2:1 and openflow:2:3 to the static edge port
        // configuration. This should remove below static links.
        //   openflow:1:1 <-> openflow:2:1
        //   openflow:2:3 <-> openflow:4:2
        String port2n3 = "openflow:2:3";
        String port4n2 = "openflow:4:2";
        stopo.addEdgePort(port2n1);
        stopo.addEdgePort(port2n3);
        stopo.apply();
        linkMap1 = new SwitchLinkMap(linkMap);
        linkMap1.remove(port2n1);
        linkMap1.remove(port1n1);
        linkMap1.remove(port2n3);
        linkMap1.remove(port4n2);

        // openflow:1:1 -> openflow:2:2 in the ignored-links container should
        // be restored. Note that openflow:2:2 -> openflow:1:1 should not be
        // restored because static link configuration is present for
        // openflow:2:2. Although static link configuration for openflow:1:1
        // (openflow:1:1 -> openflow:2:1) is present, it will be ignored
        // because the destination port (openflow:2:1) is configured as a
        // static edge port.
        linkMap1.add(port1n1, port2n2);
        linkMap1.removeIgnored(port1n1);
        linkMap1.verify();
        linkMap1 = null;

        // Restore configuration.
        stopo.removeEdgePort(port2n1);
        stopo.removeEdgePort(port2n3);
        stopo.apply();
        linkMap.verify();

        // Remove all nodes created by this test.
        for (String nid: newNodes) {
            ofmock.removeNode(nid);
        }
        savedLinks.verify();

        // Delete static network topology configuration.
        // This should restore initial network topology.
        stopo.clear();
        stopo.apply();
        initial.verify();
    }
}
