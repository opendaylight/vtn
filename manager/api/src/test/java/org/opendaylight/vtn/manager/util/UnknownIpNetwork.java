/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.util;

import java.net.InetAddress;

import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.address.Address;

import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.inet.types.rev100924.IpAddress;
import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.inet.types.rev100924.IpPrefix;

/**
 * An implementation of {@link IpNetwork} only for test.
 *
 * <p>
 *   This class does not represent any IP network.
 * </p>
 */
public final class UnknownIpNetwork extends IpNetwork {
    /**
     * Version number for serialization.
     */
    private static final long serialVersionUID = 1;

    /**
     * Construct a new instance.
     */
    public UnknownIpNetwork() {
    }

    // IpNetwork

    /**
     * {@inheritDoc}
     */
    @Override
    public int getMaxPrefix() {
        return 0;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public IpPrefix getIpPrefix() {
        return null;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public IpAddress getIpAddress() {
        return null;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public Address getMdAddress() {
        return null;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public String getCidrText() {
        return null;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public String getHostAddress() {
        return null;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public byte[] getBytes() {
        return null;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public boolean contains(IpNetwork inw) {
        return false;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    InetAddress init(InetAddress iaddr, int prefix) {
        return null;
    }
}
