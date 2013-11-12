/*
 * Copyright (c) 2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal;

import java.net.InetAddress;
import java.net.UnknownHostException;
import java.util.ArrayList;
import java.util.List;

import org.opendaylight.controller.sal.connection.ConnectionLocality;
import org.opendaylight.controller.sal.core.Node;

/**
 * Stub module for Unit test of VTNManager.
 * This stub provides APIs implemented in Bundle in controller project.
 *
 * <p>
 *  This Stub emulate cluster mode.
 * </p>
 */
public class TestStubCluster extends TestStub {

    public TestStubCluster(int stubMode) {
        super(stubMode);
    }

    @Override
    public List<InetAddress> getClusteredControllers() {
        List<InetAddress> list = new ArrayList<InetAddress>();

        InetAddress ipaddr = null;
        try {
            ipaddr = InetAddress.getByAddress(new byte[] {(byte) 192, (byte) 168, (byte) 0, (byte) 1});
        } catch (UnknownHostException e) {

        }
        list.add(ipaddr);

        ipaddr = null;
        try {
            ipaddr = InetAddress.getByAddress(new byte[] {(byte) 192, (byte) 168, (byte) 0, (byte) 2});
        } catch (UnknownHostException e) {

        }
        list.add(ipaddr);

        return list;
    }

    @Override
    public InetAddress getMyAddress() {
        InetAddress ipaddr = null;
        try {
            ipaddr = InetAddress.getByAddress(new byte[] {(byte) 192, (byte) 168, (byte) 0, (byte) 1});
        } catch (UnknownHostException e) {

        }
        return ipaddr;
    }

    @Override
    public boolean amICoordinator() {
        return true;
    }

    @Override
    public ConnectionLocality getLocalityStatus(Node arg0) {
        if (arg0.getID().equals(Long.valueOf("0"))) {
            return ConnectionLocality.LOCAL;
        } else if (arg0.getID().equals(Long.valueOf("1"))) {
            return ConnectionLocality.NOT_LOCAL;
        } else {
            return ConnectionLocality.NOT_CONNECTED;
        }
    }

}
