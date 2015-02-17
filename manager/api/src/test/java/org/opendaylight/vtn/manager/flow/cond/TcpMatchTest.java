/*/*
 * Copyright (c) 2014-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.flow.cond;

import java.util.HashSet;
import java.util.List;

import org.junit.Test;

import org.opendaylight.vtn.manager.TestBase;

/**
 * JUnit test for {@link TcpMatch}.
 */
public class TcpMatchTest extends TestBase {
    /**
     * Test case for getter methods.
     */
    @Test
    public void testGetter() {
        for (PortMatch src: createPortMatches()) {
            for (PortMatch dst: createPortMatches()) {
                TcpMatch tm = new TcpMatch(src, dst);
                assertEquals(src, tm.getSourcePort());
                assertEquals(dst, tm.getDestinationPort());
            }
        }

        Integer[] srcs = {
            null, Integer.valueOf(0), Integer.valueOf(12345),
            Integer.valueOf(65355),
        };
        Integer[] dsts = {
            null, Integer.valueOf(0), Integer.valueOf(9988),
            Integer.valueOf(65354),
        };
        for (Integer src: srcs) {
            for (Integer dst: dsts) {
                TcpMatch tm = new TcpMatch(src, dst);
                assertEquals((src == null) ? null : new PortMatch(src),
                             tm.getSourcePort());
                assertEquals((dst == null) ? null : new PortMatch(dst),
                             tm.getDestinationPort());

                Short ss = (src == null)
                    ? null : Short.valueOf(src.shortValue());
                Short sd = (dst == null)
                    ? null : Short.valueOf(dst.shortValue());
                tm = new TcpMatch(ss, sd);
                assertEquals((ss == null) ? null : new PortMatch(src),
                             tm.getSourcePort());
                assertEquals((sd == null) ? null : new PortMatch(dst),
                             tm.getDestinationPort());
            }
        }
    }

    /**
     * Test case for {@link TcpMatch#equals(Object)} and
     * {@link TcpMatch#hashCode()}.
     */
    @Test
    public void testEquals() {
        HashSet<Object> set = new HashSet<Object>();
        List<PortMatch> srcs = createPortMatches();
        List<PortMatch> dsts = createPortMatches();
        for (PortMatch src: srcs) {
            for (PortMatch dst: dsts) {
                TcpMatch tm1 = new TcpMatch(src, dst);
                TcpMatch tm2 = new TcpMatch(copy(src), copy(dst));
                testEquals(set, tm1, tm2);

                UdpMatch um = new UdpMatch(src, dst);
                assertFalse(tm1.equals(um));
            }
        }

        int required = srcs.size() * dsts.size();
        assertEquals(required, set.size());
    }

    /**
     * Test case for {@link TcpMatch#toString()}.
     */
    @Test
    public void testToString() {
        String prefix = "TcpMatch[";
        String suffix = "]";
        for (PortMatch src: createPortMatches()) {
            for (PortMatch dst: createPortMatches()) {
                TcpMatch tm = new TcpMatch(src, dst);
                String s = (src == null) ? null : "src=" + src;
                String d = (dst == null) ? null : "dst=" + dst;
                String required = joinStrings(prefix, suffix, ",", s, d);
                assertEquals(required, tm.toString());
            }
        }
    }

    /**
     * Ensure that {@link TcpMatch} is serializable.
     */
    @Test
    public void testSerialize() {
        for (PortMatch src: createPortMatches()) {
            for (PortMatch dst: createPortMatches()) {
                TcpMatch tm = new TcpMatch(src, dst);
                serializeTest(tm);
            }
        }
    }

    /**
     * Ensure that {@link TcpMatch} is mapped to XML root element.
     */
    @Test
    public void testJAXB() {
        for (PortMatch src: createPortMatches()) {
            for (PortMatch dst: createPortMatches()) {
                TcpMatch tm = new TcpMatch(src, dst);
                jaxbTest(tm, "tcpmatch");
            }
        }
    }

    /**
     * Ensure that {@link TcpMatch} is mapped to JSON object.
     */
    @Test
    public void testJSON() {
        for (PortMatch src: createPortMatches()) {
            for (PortMatch dst: createPortMatches()) {
                TcpMatch tm = new TcpMatch(src, dst);
                jsonTest(tm, TcpMatch.class);
            }
        }
    }
}
