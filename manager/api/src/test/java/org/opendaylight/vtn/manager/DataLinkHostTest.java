/*
 * Copyright (c) 2014, 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager;

import java.util.HashSet;
import java.util.List;

import org.junit.Test;

import org.opendaylight.controller.sal.packet.address.DataLinkAddress;

/**
 * JUnit test for {@link DataLinkHost}.
 */
public class DataLinkHostTest extends TestBase {
    /**
     * Yet another {@link DataLinkHost} implementation.
     */
    private static class AnotherDataLinkHost extends DataLinkHost {
        /**
         * Version number for serialization.
         */
        private static final long serialVersionUID = 1L;

        /**
         * Construct a new host information.
         *
         * @param addr  A {@link DataLinkAddress} which represents a data link
         *              layer address.
         */
        public AnotherDataLinkHost(DataLinkAddress addr) {
            super(addr);
        }
    }

    /**
     * Test case for getter methods.
     */
    @Test
    public void testGetter() {
        for (DataLinkAddress dladdr: createDataLinkAddresses()) {
            DataLinkHost dlhost = new TestDataLinkHost(dladdr);
            assertEquals(dladdr, dlhost.getAddress());
        }
    }

    /**
     * Test case for {@link DataLinkHost#equals(Object)} and
     * {@link DataLinkHost#hashCode()}.
     */
    @Test
    public void testEquals() {
        HashSet<Object> set = new HashSet<Object>();
        List<DataLinkAddress> addresses = createDataLinkAddresses();
        for (DataLinkAddress dladdr1: addresses) {
            DataLinkAddress dladdr2 = copy(dladdr1);
            DataLinkHost dh1 = new TestDataLinkHost(dladdr1);
            DataLinkHost dh2 = new TestDataLinkHost(dladdr2);
            testEquals(set, dh1, dh2);

            // An instance of different class should be treated as different
            // instance.
            DataLinkHost adh1 = new AnotherDataLinkHost(dladdr1);
            DataLinkHost adh2 = new AnotherDataLinkHost(dladdr2);
            testEquals(set, adh1, adh2);
        }

        assertEquals(addresses.size() * 2, set.size());
    }

    /**
     * Test case for {@link DataLinkHost#toString()}.
     */
    @Test
    public void testToString() {
        String prefix = "DataLinkHost[";
        String suffix = "]";
        for (DataLinkAddress dladdr: createDataLinkAddresses()) {
            DataLinkHost dlhost = new TestDataLinkHost(dladdr);
            String a = (dladdr == null) ? null : "address=" + dladdr;
            String required = joinStrings(prefix, suffix, ",", a);
            assertEquals(required, dlhost.toString());
        }
    }

    /**
     * Ensure that {@link DataLinkHost} is serializable.
     */
    @Test
    public void testSerialize() {
        for (DataLinkAddress dladdr: createDataLinkAddresses()) {
            DataLinkHost dlhost = new TestDataLinkHost(dladdr);
            serializeTest(dlhost);
        }
    }
}
