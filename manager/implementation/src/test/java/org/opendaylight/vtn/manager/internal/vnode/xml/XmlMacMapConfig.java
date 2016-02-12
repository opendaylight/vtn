/*
 * Copyright (c) 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.vnode.xml;

import static org.junit.Assert.assertEquals;

import static org.opendaylight.vtn.manager.internal.TestBase.MAX_RANDOM;
import static org.opendaylight.vtn.manager.internal.TestBase.createEtherAddress;
import static org.opendaylight.vtn.manager.internal.TestBase.createVlanId;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashSet;
import java.util.Iterator;
import java.util.List;
import java.util.Random;
import java.util.Set;

import org.opendaylight.vtn.manager.util.EtherAddress;

import org.opendaylight.vtn.manager.internal.util.inventory.MacVlan;
import org.opendaylight.vtn.manager.internal.util.vnode.VTNMacMapConfig;

import org.opendaylight.vtn.manager.internal.XmlNode;

import org.opendaylight.yang.gen.v1.urn.opendaylight.l2.types.rev130827.VlanId;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.mac.rev150907.vtn.mac.map.config.AllowedHosts;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.mac.rev150907.vtn.mac.map.config.AllowedHostsBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.mac.rev150907.vtn.mac.map.config.DeniedHosts;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.mac.rev150907.vtn.mac.map.config.DeniedHostsBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.mac.rev150907.vtn.mac.map.info.MacMapConfig;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.mac.rev150907.vtn.mac.map.info.MacMapConfigBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.mac.rev150907.vtn.mac.map.info.MacMapStatusBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.mac.rev150907.vtn.mac.map.status.MappedHost;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.mac.rev150907.vtn.mac.map.status.MappedHostBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.mac.rev150907.vtn.mac.mappable.MacMap;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.mac.rev150907.vtn.mac.mappable.MacMapBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.vlan.host.desc.set.VlanHostDescList;

/**
 * {@code XmlMacMapConfig} describes configuration information about a
 * MAC mapping configured in a vBridge.
 */
public final class XmlMacMapConfig {
    /**
     * A set of hosts to be mapped by MAC mapping.
     */
    private final Set<MacVlan>  allowedHosts = new HashSet<>();

    /**
     * A set of hosts not to be mapped by MAC mapping.
     */
    private final Set<MacVlan>  deniedHosts = new HashSet<>();

    /**
     * Verify the given MAC mapping configuration.
     *
     * @param expected  A {@link XmlMacMapConfig} instance that contains
     *                  the expected MAC mapping configuration.
     * @param vmmc      A {@link VTNMacMapConfig} instance to be verified.
     */
    public static void verify(XmlMacMapConfig expected, VTNMacMapConfig vmmc) {
        if (expected == null) {
            assertEquals(null, vmmc);
        } else {
            expected.verify(vmmc);
        }
    }

    /**
     * Convert the given host set into a list of vlan-host-desc-list instance.
     *
     * @param acl  A set of L2 hosts.
     * @return  A list of {@link VlanHostDescList} instances.
     *          {@code null} if the given access control list is empty.
     */
    public static List<VlanHostDescList> toVlanHostDescList(Set<MacVlan> acl) {
        List<VlanHostDescList> list;
        if (acl.isEmpty()) {
            list = null;
        } else {
            list = new ArrayList<>();
            for (MacVlan mv: acl) {
                list.add(mv.getVlanHostDescList());
            }
        }

        return list;
    }

    /**
     * Construct an empty instance.
     */
    public XmlMacMapConfig() {
    }

    /**
     * Construct a new instance using the given pseudo random number generator.
     *
     * @param rand  A pseudo random number generator.
     */
    public XmlMacMapConfig(Random rand) {
        int n = rand.nextInt(3);
        if (n <= 1) {
            // Configure the allowed host set.
            setAllowedHosts(rand);
        }
        if (n >= 1) {
            // Configure the denied host set.
            setDeniedHosts(rand);
        }
    }

    /**
     * Return a set of hosts to be mapped by MAC mapping.
     *
     * @return  A set of hosts to be mapped by MAC mapping.
     */
    public Set<MacVlan> getAllowedHosts() {
        return Collections.unmodifiableSet(allowedHosts);
    }

    /**
     * Add the given hosts to the allowed host set.
     *
     * @param hosts  An array of L2 hosts to be added to the allowed host set.
     * @return  This instance.
     */
    public XmlMacMapConfig addAllowedHosts(MacVlan ... hosts) {
        for (MacVlan mv: hosts) {
            allowedHosts.add(mv);
        }
        return this;
    }

    /**
     * Return a set of hosts not to be mapped by MAC mapping.
     *
     * @return  A set of hosts not to be mapped by MAC mapping.
     */
    public Set<MacVlan> getDeniedHosts() {
        return Collections.unmodifiableSet(deniedHosts);
    }

    /**
     * Add the given hosts to the denied host set.
     *
     * @param hosts  An array of L2 hosts to be added to the denied host set.
     * @return  This instance.
     */
    public XmlMacMapConfig addDeniedHosts(MacVlan ... hosts) {
        for (MacVlan mv: hosts) {
            deniedHosts.add(mv);
        }
        return this;
    }

    /**
     * Convert this instance into a mac-map-config instance.
     *
     * @return  A {@link MacMap} instance.
     */
    public MacMap toMacMap() {
        // Create MAC mapping status.
        // This should be always ignored.
        Iterator<MacVlan> it = allowedHosts.iterator();
        MacMapStatusBuilder mstb = new MacMapStatusBuilder();
        if (it.hasNext()) {
            MacVlan mv = it.next();
            MappedHost mhost = new MappedHostBuilder().
                setMacAddress(mv.getMacAddress()).
                setVlanId(new VlanId(mv.getVlanId())).
                build();
            List<MappedHost> mhosts = Collections.singletonList(mhost);
            mstb.setMappedHost(mhosts);
        }

        return new MacMapBuilder().
            setMacMapConfig(toMacMapConfig()).
            setMacMapStatus(mstb.build()).
            build();
    }

    /**
     * Convert this instance into a mac-map-config instance.
     *
     * @return  A {@link MacMapConfig} instance.
     */
    public MacMapConfig toMacMapConfig() {
        MacMapConfigBuilder builder = new MacMapConfigBuilder();
        List<VlanHostDescList> hosts = toVlanHostDescList(allowedHosts);
        if (hosts != null) {
            AllowedHosts allowed = new AllowedHostsBuilder().
                setVlanHostDescList(hosts).
                build();
            builder.setAllowedHosts(allowed);
        }

        hosts = toVlanHostDescList(deniedHosts);
        if (hosts != null) {
            DeniedHosts denied = new DeniedHostsBuilder().
                setVlanHostDescList(hosts).
                build();
            builder.setDeniedHosts(denied);
        }

        return builder.build();
    }

    /**
     * Set MAC mapping configuration in this instance into the specified
     * XML node.
     *
     * @param xnode   A {@link XmlNode} instance.
     */
    public void setXml(XmlNode xnode) {
        XmlNode xmmc = new XmlNode("mac-map");
        setXml(xmmc, allowedHosts, "allowed-hosts");
        setXml(xmmc, deniedHosts, "denied-hosts");
        xnode.add(xmmc);
    }

    /**
     * Ensure that the given {@link VTNMacMapConfig} instance is identical to
     * this instance.
     *
     * @param vmmc  A {@link VTNMacMapConfig} instance.
     */
    public void verify(VTNMacMapConfig vmmc) {
        assertEquals(allowedHosts, vmmc.getAllowedHosts());
        assertEquals(deniedHosts, vmmc.getDeniedHosts());
    }

    /**
     * Configure the allowed host set using the given random number generator.
     *
     * @param rand  A pseudo random number generator.
     */
    private void setAllowedHosts(Random rand) {
        int count = rand.nextInt(MAX_RANDOM) + 1;
        Set<EtherAddress> macSet = new HashSet<>();
        do {
            // Determine MAC address.
            long mac;
            int n = rand.nextInt(5);
            if (n < 2) {
                // Use wildcard mapping.
                mac = MacVlan.UNDEFINED;
            } else {
                EtherAddress eaddr;
                do {
                    eaddr = createEtherAddress(rand);
                } while (!macSet.add(eaddr));
                mac = eaddr.getAddress();
            }

            // Determine VLAN ID.
            int vid = createVlanId(rand);
            allowedHosts.add(new MacVlan(mac, vid));
        } while (allowedHosts.size() < count);
    }

    /**
     * Configure the denied host set using the given random number generator.
     *
     * @param rand  A pseudo random number generator.
     */
    private void setDeniedHosts(Random rand) {
        int count = rand.nextInt(MAX_RANDOM) + 1;
        do {
            // Determine MAC address.
            EtherAddress eaddr = createEtherAddress(rand);

            // Determine VLAN ID.
            int vid = createVlanId(rand);
            deniedHosts.add(new MacVlan(eaddr.getAddress(), vid));
        } while (deniedHosts.size() < count);
    }

    /**
     * Convert the given host set into a XML node, and set it to the specified
     * XML node.
     *
     * @param xnode  A {@link XmlNode} instance to set host information.
     * @param acl    A set of L2 hosts.
     * @param root   The name of the root node.
     */
    private void setXml(XmlNode xnode, Set<MacVlan> acl, String root) {
        if (!acl.isEmpty()) {
            XmlNode hosts = new XmlNode(root);
            for (MacVlan mv: acl) {
                int vid = mv.getVlanId();
                XmlNode xhost = new XmlNode("host").
                    add(new XmlNode("vlan-id", vid));
                EtherAddress eaddr = mv.getEtherAddress();
                if (eaddr != null) {
                    xhost.add(new XmlNode("address", eaddr.getText()));
                }
                hosts.add(xhost);
            }
            xnode.add(hosts);
        }
    }
}
