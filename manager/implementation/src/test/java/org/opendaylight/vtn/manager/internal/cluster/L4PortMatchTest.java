/*
 * Copyright (c) 2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.cluster;

import java.io.ByteArrayInputStream;
import java.util.HashSet;

import javax.xml.bind.JAXB;

import org.junit.Test;

import org.opendaylight.vtn.manager.VTNException;
import org.opendaylight.vtn.manager.flow.cond.PortMatch;

import org.opendaylight.vtn.manager.internal.TestBase;

import org.opendaylight.controller.sal.utils.Status;
import org.opendaylight.controller.sal.utils.StatusCode;

/**
 * JUnit test for {@link L4PortMatch}.
 */
public class L4PortMatchTest extends TestBase {
    /**
     * Test for getter methods.
     *
     * @throws VTNException  An error occurred.
     */
    @Test
    public void testGetter() throws VTNException {
        int[] ports = {
            0, 1, 10, 150, 789, 12345, 33333, 56789, 65535,
        };
        String xml =
            "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>";

        int nranges = 3;
        for (int port: ports) {
            Integer from = Integer.valueOf(port);
            PortMatch pm = new PortMatch(from);
            L4PortMatch lpm = new L4PortMatch(pm);
            assertEquals(port, lpm.getPortFrom());
            assertEquals(port, lpm.getPortTo());

            // Construct PortMatch that has no portTo field.
            StringBuilder sb = new StringBuilder(xml);
            sb.append("<portmatch from=\"").append(port).append("\" />");
            ByteArrayInputStream in =
                new ByteArrayInputStream(sb.toString().getBytes());
            pm = JAXB.unmarshal(in, PortMatch.class);
            assertEquals(null, pm.getPortTo());
            lpm = new L4PortMatch(pm);
            assertEquals(port, lpm.getPortFrom());
            assertEquals(port, lpm.getPortTo());

            if (port != 65535) {
                // Specify the range of ports.
                for (int to = port; to <= port + nranges; to++) {
                    pm = new PortMatch(from, Integer.valueOf(to));
                    lpm = new L4PortMatch(pm);
                    assertEquals(port, lpm.getPortFrom());
                    assertEquals(to, lpm.getPortTo());
                }
            }
            if (port != 0) {
                // Specifying invalid port range.
                for (int to = port - nranges; to <= port; to++) {
                    if (from > to) {
                        continue;
                    }
                    pm = new PortMatch(from, Integer.valueOf(to));
                    try {
                        new L4PortMatch(pm);
                    } catch (VTNException e) {
                        Status st = e.getStatus();
                        assertEquals(StatusCode.BADREQUEST, st.getCode());
                        assertEquals("Invalid port range: from=" + port +
                                     ", to=" + to, st.getDescription());
                    }
                }
            }
        }

        // Specifying empty PortMatch.
        PortMatch pm = new PortMatch((Integer)null);
        try {
            new L4PortMatch(pm);
            unexpected();
        } catch (VTNException e) {
            Status st = e.getStatus();
            assertEquals(StatusCode.BADREQUEST, st.getCode());
            assertEquals("\"from\" is not specified.", st.getDescription());
        }

        // Specifying invalid port.
        int[] badPorts = {
            Integer.MIN_VALUE, -3, -2, -1,
            0x10000, 0x10001, 0x400000, Integer.MAX_VALUE,
        };
        for (int port: badPorts) {
            Integer bad = Integer.valueOf(port);
            pm = new PortMatch(bad);
            try {
                new L4PortMatch(pm);
                unexpected();
            } catch (VTNException e) {
                Status st = e.getStatus();
                assertEquals(StatusCode.BADREQUEST, st.getCode());
                assertEquals("from: Invalid port number: " + port,
                             st.getDescription());
            }

            pm = new PortMatch(Integer.valueOf(0), bad);
            try {
                new L4PortMatch(pm);
                unexpected();
            } catch (VTNException e) {
                Status st = e.getStatus();
                assertEquals(StatusCode.BADREQUEST, st.getCode());
                assertEquals("to: Invalid port number: " + port,
                             st.getDescription());
            }
        }
    }

    /**
     * Test case for {@link L4PortMatch#equals(Object)} and
     * {@link L4PortMatch#hashCode()}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testEquals() throws Exception {
        HashSet<Object> set = new HashSet<Object>();
        int[] ports = {
            0, 1, 10, 150, 789, 12345, 33333, 56789, 65535,
        };

        int nranges = 3;
        for (int port: ports) {
            Integer from = Integer.valueOf(port);
            PortMatch pm1 = new PortMatch(from);
            PortMatch pm2 = new PortMatch(from);
            L4PortMatch lpm1 = new L4PortMatch(pm1);
            L4PortMatch lpm2 = new L4PortMatch(pm2);
            testEquals(set, lpm1, lpm2);

            if (port != 65535) {
                // Specify the range of ports.
                for (int to = port + 1; to <= port + nranges; to++) {
                    pm1 = new PortMatch(from, Integer.valueOf(to));
                    pm2 = new PortMatch(from, Integer.valueOf(to));
                    lpm1 = new L4PortMatch(pm1);
                    lpm2 = new L4PortMatch(pm2);
                    testEquals(set, lpm1, lpm2);
                }
            }
        }

        int requires = ports.length + ((ports.length - 1) * nranges);
        assertEquals(requires, set.size());
    }

    /**
     * Test case for {@link L4PortMatch#toString()}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testToString() throws Exception {
        int[] ports = {
            0, 1, 10, 150, 789, 12345, 33333, 56789, 65535,
        };

        int nranges = 3;
        for (int port: ports) {
            Integer from = Integer.valueOf(port);
            PortMatch pm = new PortMatch(from);
            L4PortMatch lpm = new L4PortMatch(pm);
            assertEquals(from.toString(), lpm.toString());

            if (port != 65535) {
                // Specify the range of ports.
                for (int to = port + 1; to <= port + nranges; to++) {
                    pm = new PortMatch(from, Integer.valueOf(to));
                    lpm = new L4PortMatch(pm);
                    assertEquals(from + "-" + to, lpm.toString());
                }
            }
        }
    }

    /**
     * Ensure that {@link L4PortMatch} is serializable.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testSerialize() throws Exception {
        int[] ports = {
            0, 1, 10, 150, 789, 12345, 33333, 56789, 65535,
        };

        int nranges = 3;
        for (int port: ports) {
            Integer from = Integer.valueOf(port);
            PortMatch pm = new PortMatch(from);
            L4PortMatch lpm = new L4PortMatch(pm);
            serializeTest(lpm);

            if (port != 65535) {
                // Specify the range of ports.
                for (int to = port + 1; to <= port + nranges; to++) {
                    pm = new PortMatch(from, Integer.valueOf(to));
                    lpm = new L4PortMatch(pm);
                    serializeTest(lpm);
                }
            }
        }
    }

    /**
     * Test case for {@link L4PortMatch#match(int)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testMatch() throws Exception {
        int[] ports = {
            0, 1, 10, 150, 789, 12345, 33333, 56789, 65535,
        };
        int[] badPorts = {
            Integer.MIN_VALUE, -3, -2, -1,
            0x10000, 0x10001, 0x400000, Integer.MAX_VALUE,
        };

        int nranges = 3;
        for (int port: ports) {
            Integer from = Integer.valueOf(port);
            PortMatch pm = new PortMatch(from);
            L4PortMatch lpm = new L4PortMatch(pm);
            assertEquals(true, lpm.match(port));
            for (int i = 1; i <= 10; i++) {
                assertEquals(false, lpm.match(port + i));
                assertEquals(false, lpm.match(port - i));
            }
            for (int p: badPorts) {
                assertEquals(false, lpm.match(p));
            }

            if (port != 65535) {
                // Specify the range of ports.
                for (int to = port + 1; to <= port + nranges; to++) {
                    pm = new PortMatch(from, Integer.valueOf(to));
                    lpm = new L4PortMatch(pm);
                    for (int p = port; p <= to; p++) {
                        assertEquals(true, lpm.match(port));
                    }
                    for (int p: badPorts) {
                        assertEquals(false, lpm.match(p));
                    }
                    for (int i = 1; i <= 10; i++) {
                        assertEquals(false, lpm.match(to + i));
                        assertEquals(false, lpm.match(port - i));
                    }
                }
            }
        }
    }
}
