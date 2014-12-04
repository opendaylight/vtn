/*
 * Copyright (c) 2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution,and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.cluster;

import java.util.List;

import org.junit.Test;
import org.opendaylight.vtn.manager.VTNException;
import org.opendaylight.vtn.manager.VTenantConfig;
import org.opendaylight.vtn.manager.VTenantPath;
import org.opendaylight.vtn.manager.internal.LockStack;
import org.opendaylight.vtn.manager.internal.PacketContext;
import org.opendaylight.vtn.manager.internal.TestBase;
import org.opendaylight.vtn.manager.flow.filter.FlowFilter;
import org.opendaylight.vtn.manager.flow.filter.FlowFilterId;
import org.opendaylight.vtn.manager.internal.VTNManagerImpl;
import org.opendaylight.controller.sal.core.NodeConnector;
import org.opendaylight.controller.sal.packet.ARP;
import org.opendaylight.controller.sal.packet.address.EthernetAddress;

/**
 * JUnit test for {@link DropFlowFilterImpl}.
 */
public class DropFlowFilterImplTest extends TestBase {
    /**
     * Testing the Get methods in {@link DropFlowFilterImpl}..
    */
    @Test
    public void testGetter() {
        DropFlowFilterImpl dropFlowFilterImpl = null;
        try {
            //Checking for all the scenarios for all the methods in  DropFlowFilterImpl.
            for (int idx:INDEX_ARRAY) {
                List<FlowFilter> filterList = createFlowFilter();
                for (FlowFilter flowfilter : filterList) {
                    try {
                        dropFlowFilterImpl = new DropFlowFilterImpl(idx, flowfilter);
                        assertTrue(dropFlowFilterImpl.isMulticastSupported());
                        assertFalse(dropFlowFilterImpl.needFlowAction());
                        assertNotNull(dropFlowFilterImpl.getFilterType());

                        VTNManagerImpl mgr = new VTNManagerImpl();
                        List<NodeConnector> connectors = createNodeConnectors(2);
                        byte[] addr = {(byte)0x00, (byte)0x00, (byte)0x00,
                                       (byte)0x00, (byte)0x00, (byte)0x01};
                        EthernetAddress ea = new EthernetAddress(addr);
                        byte [] bytes = ea.getValue();
                        byte [] src = new byte[] {bytes[0], bytes[1], bytes[2],
                                                  bytes[3], bytes[4], bytes[5]};
                        byte [] sender = new byte[] {(byte)192, (byte)168,
                                                     (byte)0, (byte)1};
                        byte[] dst = new byte[] {(byte)0xff, (byte)0xff, (byte)0xff,
                                                 (byte)0xff, (byte)0xff, (byte)0xff};
                        byte[] target = new byte[] {(byte)192, (byte)168, (byte)0, (byte)250};

                        PacketContext pctx = createARPPacketContext(src, dst, sender, target,
                                                          (short)-1, connectors.get(0), ARP.REQUEST);
                        String containerName = "default";
                        String description = "description";

                        LockStack lstack = new LockStack();
                        VTenantPath path = new VTenantPath(TENANT_NAME[0]);
                        VTenantConfig tenantconfig = new VTenantConfig(description);
                        VTenantImpl vtn =  new VTenantImpl(containerName, TENANT_NAME[0], tenantconfig);
                        FlowFilterId flowFilterId = new FlowFilterId(path);

                        FlowFilterMap ffmap = vtn.getFlowFilterMap(lstack, flowFilterId, false);
                        dropFlowFilterImpl.apply(mgr, pctx, ffmap);
                    } catch (VTNException | NullPointerException ex) {
                        ex.printStackTrace();
                    }
                }
            }
        } catch (Exception ex) {
        }
    }
}
