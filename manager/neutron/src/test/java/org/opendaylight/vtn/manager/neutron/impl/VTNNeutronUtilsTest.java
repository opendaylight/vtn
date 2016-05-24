/*
 * Copyright (c) 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.neutron.impl;

import static org.opendaylight.vtn.manager.neutron.impl.VTNNeutronUtils.convertUUIDToKey;
import static org.opendaylight.vtn.manager.neutron.impl.VTNNeutronUtils.getBridgeId;
import static org.opendaylight.vtn.manager.neutron.impl.VTNNeutronUtils.getInterfaceId;
import static org.opendaylight.vtn.manager.neutron.impl.VTNNeutronUtils.getNonNullValue;
import static org.opendaylight.vtn.manager.neutron.impl.VTNNeutronUtils.getTenantId;

import java.util.Map.Entry;
import java.util.Map;

import com.google.common.collect.ImmutableMap;

import org.junit.Test;

import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.yang.types.rev130715.Uuid;

import org.opendaylight.yang.gen.v1.urn.opendaylight.neutron.attrs.rev150712.BaseAttributes;
import org.opendaylight.yang.gen.v1.urn.opendaylight.neutron.networks.rev150712.networks.attributes.networks.Network;
import org.opendaylight.yang.gen.v1.urn.opendaylight.neutron.networks.rev150712.networks.attributes.networks.NetworkBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.neutron.ports.rev150712.ports.attributes.ports.Port;
import org.opendaylight.yang.gen.v1.urn.opendaylight.neutron.ports.rev150712.ports.attributes.ports.PortBuilder;

/**
 * JUnit test for {@link VTNNeutronUtils}.
 */
public final class VTNNeutronUtilsTest extends TestBase {
    /**
     * Pairs of UUIDs and expected IDs.
     */
    private static final Map<Uuid, String>  UUID_CASES;

    /**
     * Initialize static fields.
     */
    static {
        UUID_CASES = ImmutableMap.<Uuid, String>builder().
            put(new Uuid("c801210b-a030-1887-9174-fc02d842ab23"),
                "c801210ba0308879174fc02d842ab23").
            put(new Uuid("d035bcdd-4057-1e35-9337-bcc9514069ca"),
                "d035bcdd4057e359337bcc9514069ca").
            put(new Uuid("6c58bc63-cd86-1c3f-9acd-17627fcb1507"),
                "6c58bc63cd86c3f9acd17627fcb1507").
            put(new Uuid("4430cece-63fd-2fb2-a04f-d398bb4dc7b4"),
                "4430cece63fdfb2a04fd398bb4dc7b4").
            put(new Uuid("08e03cdf-34cf-205a-8c9b-187f777efce2"),
                "08e03cdf34cf05a8c9b187f777efce2").
            put(new Uuid("960ade08-1895-2e19-9631-245f8170bac2"),
                "960ade081895e199631245f8170bac2").
            put(new Uuid("da59d91c-391a-3397-b58c-0b39d262bc29"),
                "da59d91c391a397b58c0b39d262bc29").
            put(new Uuid("3bfea7af-2a30-3f4d-90cd-f0d579e640be"),
                "3bfea7af2a30f4d90cdf0d579e640be").
            put(new Uuid("2edf20d3-46d0-39da-9097-05382fc162ef"),
                "2edf20d346d09da909705382fc162ef").
            put(new Uuid("5eb40630-667d-4cd9-ac7f-ec80523a7b7a"),
                "5eb40630667dcd9ac7fec80523a7b7a").
            put(new Uuid("1201cf3a-86dc-4c9e-a244-6a7cec678598"),
                "1201cf3a86dcc9ea2446a7cec678598").
            put(new Uuid("347239d5-e0f6-4927-8b80-36de7e5343cc"),
                "347239d5e0f69278b8036de7e5343cc").
            build();
    }

    /**
     * Custom UUID only for test.
     */
    private static final class TestUuid extends Uuid {
        /**
         * A string representation of UUID.
         */
        private final String  uuid;

        /**
         * Construct an empty instance.
         */
        private TestUuid() {
            this(null);
        }

        /**
         * Construct a new instance.
         *
         * @param id  A string representation of UUID.
         */
        private TestUuid(String id) {
            super("00000000-0000-4000-0000-000000000000");
            uuid = id;
        }

        // Uuid

        /**
         * Return the value of this UUID.
         *
         * @return  A string representation of UUID.
         */
        @Override
        public String getValue() {
            return uuid;
        }
    }

    /**
     * Test case for {@link VTNNeutronUtils#convertUUIDToKey(Uuid)}.
     */
    @Test
    public void testCOnvertUUIDToKey() {
        for (Entry<Uuid, String> entry: UUID_CASES.entrySet()) {
            Uuid uuid = entry.getKey();
            assertEquals(entry.getValue(), convertUUIDToKey(uuid));
        }
    }

    /**
     * Test case for {@link VTNNeutronUtils#getTenantId(BaseAttributes)}.
     */
    @Test
    public void testGetTenantId() {
        Uuid uuid1 = new Uuid("7357df1a-7479-3663-bf26-0a3b23fc8df1");
        Uuid uuid2 = new Uuid("e676a1f1-c56c-41da-85d6-e9eae05de3a8");
        NetworkBuilder nb = new NetworkBuilder().
            setUuid(uuid1);
        PortBuilder pb = new PortBuilder().
            setNetworkId(uuid1).
            setUuid(uuid2);

        for (Entry<Uuid, String> entry: UUID_CASES.entrySet()) {
            Uuid uuid = entry.getKey();
            String id = entry.getValue();

            BaseAttributes obj = nb.setTenantId(uuid).build();
            assertEquals(id, getTenantId(obj));

            obj = pb.setTenantId(uuid).build();
            assertEquals(id, getTenantId(obj));
        }

        Uuid[] uuids = {null, new TestUuid()};
        for (Uuid uuid: uuids) {
            assertEquals(null, getTenantId(nb.setTenantId(uuid).build()));
            assertEquals(null, getTenantId(pb.setTenantId(uuid).build()));
        }
    }

    /**
     * Test case for {@link VTNNeutronUtils#getBridgeId(Network)}.
     */
    @Test
    public void testGetBridgeIdNetwork() {
        Uuid uuid1 = new Uuid("864a1a62-0089-3b7b-a0f0-1f402cf245b4");
        NetworkBuilder nb = new NetworkBuilder().
            setTenantId(uuid1);
        for (Entry<Uuid, String> entry: UUID_CASES.entrySet()) {
            Uuid uuid = entry.getKey();
            Network nw = nb.setUuid(uuid).build();
            assertEquals(entry.getValue(), getBridgeId(nw));
        }

        Uuid[] uuids = {null, new TestUuid()};
        for (Uuid uuid: uuids) {
            assertEquals(null, getBridgeId(nb.setUuid(uuid).build()));
        }
    }

    /**
     * Test case for {@link VTNNeutronUtils#getBridgeId(Port)}.
     */
    @Test
    public void testGetBridgeIdPort() {
        Uuid uuid1 = new Uuid("d3271b00-b2c6-371b-bcbd-27efd9a55f72");
        Uuid uuid2 = new Uuid("ff17e0b4-bd4d-402a-802c-36f68bf84d80");
        PortBuilder pb = new PortBuilder().
            setTenantId(uuid1).
            setUuid(uuid2);
        for (Entry<Uuid, String> entry: UUID_CASES.entrySet()) {
            Uuid uuid = entry.getKey();
            Port port = pb.setNetworkId(uuid).build();
            assertEquals(entry.getValue(), getBridgeId(port));
        }

        Uuid[] uuids = {null, new TestUuid()};
        for (Uuid uuid: uuids) {
            assertEquals(null, getBridgeId(pb.setNetworkId(uuid).build()));
        }
    }

    /**
     * Test case for {@link VTNNeutronUtils#getInterfaceId(Port)}.
     */
    @Test
    public void testGetInterfaceId() {
        Uuid uuid1 = new Uuid("ba20f28a-1850-306e-8094-70450c261bac");
        Uuid uuid2 = new Uuid("907a0a78-ede3-4186-93ee-5a3495b9a5d8");
        PortBuilder pb = new PortBuilder().
            setTenantId(uuid1).
            setNetworkId(uuid2);
        for (Entry<Uuid, String> entry: UUID_CASES.entrySet()) {
            Uuid uuid = entry.getKey();
            Port port = pb.setUuid(uuid).build();
            assertEquals(entry.getValue(), getInterfaceId(port));
        }

        Uuid[] uuids = {null, new TestUuid()};
        for (Uuid uuid: uuids) {
            assertEquals(null, getInterfaceId(pb.setUuid(uuid).build()));
        }
    }

    /**
     * Test case for {@link VTNNeutronUtils#getNonNullValue(Object,Object)}.
     */
    @Test
    public void testGetNonNullValue() {
        String def = "default-value";
        String[] values = {null, "value-1", "value-2"};
        for (String value: values) {
            String expected = (value == null) ? def : value;
            assertSame(expected, getNonNullValue(value, def));
        }
    }
}
