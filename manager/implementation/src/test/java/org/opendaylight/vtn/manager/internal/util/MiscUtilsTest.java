/*
 * Copyright (c) 2014, 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util;

import java.math.BigInteger;
import java.net.Inet4Address;
import java.net.InetAddress;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.Comparator;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Random;
import java.util.Set;

import org.slf4j.Logger;

import org.junit.Test;

import org.mockito.Mockito;

import org.opendaylight.vtn.manager.VTNException;
import org.opendaylight.vtn.manager.util.EtherAddress;
import org.opendaylight.vtn.manager.util.NumberUtils;

import org.opendaylight.vtn.manager.internal.util.rpc.RpcErrorTag;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;

import org.opendaylight.vtn.manager.internal.TestBase;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeName;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnErrorTag;

import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.inet.types.rev100924.Uri;
import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.yang.types.rev100924.Counter32;
import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.yang.types.rev100924.Counter64;
import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.yang.types.rev100924.MacAddress;

import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.NodeConnectorId;

/**
 * JUnit test for {@link MiscUtils}.
 */
public class MiscUtilsTest extends TestBase {
    /**
     * Test case for {@link MiscUtils#formatMacAddress(long mac)}.
     */
    @Test
    public void testFormatMacAddress() {
        assertEquals("00:00:00:00:00:00", MiscUtils.formatMacAddress(0L));
        assertEquals("00:00:00:00:00:01", MiscUtils.formatMacAddress(1L));
        assertEquals("00:00:00:00:00:ff", MiscUtils.formatMacAddress(0xffL));
        assertEquals("00:00:00:00:a0:ff", MiscUtils.formatMacAddress(0xa0ffL));
        assertEquals("00:00:00:12:34:56",
                     MiscUtils.formatMacAddress(0x123456L));
        assertEquals("aa:bb:cc:dd:ee:ff",
                     MiscUtils.formatMacAddress(0xaabbccddeeffL));
    }

    /**
     * Test case for {@link MiscUtils#toEtherAddress(MacAddress)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testToEtherAddress() throws Exception {
        MacAddress mac = new MacAddress("00:00:00:00:00:00");
        assertEquals(new EtherAddress(0L), MiscUtils.toEtherAddress(mac));
        mac = new MacAddress("00:00:00:00:00:01");
        assertEquals(new EtherAddress(1L), MiscUtils.toEtherAddress(mac));
        mac = new MacAddress("00:00:00:00:00:ff");
        assertEquals(new EtherAddress(0xffL), MiscUtils.toEtherAddress(mac));
        mac = new MacAddress("12:34:56:78:9a:bc");
        assertEquals(new EtherAddress(0x123456789abcL),
                     MiscUtils.toEtherAddress(mac));
        mac = new MacAddress("aa:BB:cC:Dd:eE:ff");
        assertEquals(new EtherAddress(0xaabbccddeeffL),
                     MiscUtils.toEtherAddress(mac));
        mac = new MacAddress("ff:ff:ff:ff:ff:ff");
        assertEquals(new EtherAddress(0xffffffffffffL),
                     MiscUtils.toEtherAddress(mac));

        assertEquals(null, MiscUtils.toEtherAddress((MacAddress)null));
    }

    /**
     * Test case for {@link MiscUtils#checkName(String, String)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testCheckName() throws Exception {
        String[] descriptions = {"desc-1", "desc-2", "desc-3"};

        String[] good = {
            "a",
            "1",
            "12",
            "name",
            "NaMe_",
            "GooD_NamE1",
            "1234567890123456789012345678901",
        };

        Map<String, String> badRequests = new HashMap<String, String>();
        badRequests.put(null, "%s name cannot be null");
        badRequests.put("", "%s name cannot be empty");
        badRequests.put("12345678901234567890123456789012",
                        "%s name is invalid");
        badRequests.put("_name", "%s name is invalid");

        List<Character> badCharacters = new ArrayList<Character>();
        for (char c = 0; c < 0x80; c++) {
            if (c == '_') {
                continue;
            }

            int type = Character.getType(c);
            if (type != Character.DECIMAL_DIGIT_NUMBER &&
                type != Character.UPPERCASE_LETTER &&
                type != Character.LOWERCASE_LETTER) {
                badCharacters.add(c);
            }
        }

        // Unicode fullwidth digit zero.
        badCharacters.add((char)0xff10);

        // Unicode hiragana letter A.
        badCharacters.add((char)0x3042);

        List<String> badNames = new ArrayList<String>();
        for (char bad: badCharacters) {
            StringBuilder builder = new StringBuilder();
            badNames.add(builder.append(bad).append("name").toString());

            builder = new StringBuilder("na");
            badNames.add(builder.append(bad).append("me").toString());

            builder = new StringBuilder("name");
            badNames.add(builder.append(bad).toString());
        }

        for (String desc: descriptions) {
            for (String name: good) {
                VnodeName vname = MiscUtils.checkName(desc, name);
                assertEquals(name, vname.getValue());
            }

            for (Map.Entry<String, String> entry: badRequests.entrySet()) {
                String name = entry.getKey();
                String msg = String.format(entry.getValue(), desc);

                try {
                    MiscUtils.checkName(desc, name);
                    unexpected();
                } catch (RpcException e) {
                    checkException(e, msg);
                    RpcErrorTag tag = (name == null)
                        ? RpcErrorTag.MISSING_ELEMENT
                        : RpcErrorTag.BAD_ELEMENT;
                    assertEquals(tag, e.getErrorTag());
                }
            }

            String msg = desc + " name is invalid";
            for (String name: badNames) {
                try {
                    MiscUtils.checkName(desc, name);
                    unexpected();
                } catch (RpcException e) {
                    checkException(e, msg);
                    assertEquals(RpcErrorTag.BAD_ELEMENT, e.getErrorTag());
                }
            }
        }
    }

    /**
     * Test case for {@link MiscUtils#toInetAddress(int)} and
     * {@link MiscUtils#toInteger(InetAddress)}.
     */
    @Test
    public void testToInetAddress() {
        Random rand = new Random();
        for (int i = 0; i < 100; i++) {
            int v = rand.nextInt();
            InetAddress iaddr = MiscUtils.toInetAddress(v);
            assertTrue(iaddr instanceof Inet4Address);

            byte[] raw = iaddr.getAddress();
            assertEquals(4, raw.length);
            for (int j = 0; j < raw.length; j++) {
                byte b = (byte)(v >>> ((3 - j) * Byte.SIZE));
                assertEquals(b, raw[j]);
            }

            assertEquals(v, MiscUtils.toInteger(iaddr));
        }

        List<InetAddress> invalid = new ArrayList<InetAddress>();
        invalid.add(null);
        try {
            invalid.add(InetAddress.getByName("::1"));
        } catch (Exception e) {
            unexpected(e);
        }

        for (InetAddress iaddr: invalid) {
            try {
                MiscUtils.toInteger(iaddr);
                unexpected();
            } catch (IllegalStateException e) {
            }
        }
    }

    /**
     * Test case for {@link MiscUtils#checkNotNull(Object, Logger, String)}.
     */
    @Test
    public void testCheckNotNull() {
        Random r = new Random();
        Logger logger = Mockito.mock(Logger.class);
        String msg = "Error message";
        assertSame(r, MiscUtils.checkNotNull(r, logger, msg));
        Mockito.verify(logger, Mockito.never()).
            error(Mockito.anyString(), Mockito.any(Throwable.class));

        r = null;
        try {
            MiscUtils.checkNotNull(r, logger, msg);
            unexpected();
        } catch (IllegalStateException e) {
            assertEquals(msg, e.getMessage());
            Mockito.verify(logger, Mockito.times(1)).error(msg, e);
        }
    }

    /**
     * Test case for string join methods.
     *
     * <ul>
     *   <li>{@link MiscUtils#joinColon(Object[])}</li>
     *   <li>{@link MiscUtils#join(String, Object[])}</li>
     *   <li>{@link MiscUtils#join(String, Collection)}</li>
     * </ul>
     */
    @Test
    public void testJoin() {
        assertEquals("", MiscUtils.joinColon());
        String s = "Test string";
        assertEquals(s, MiscUtils.joinColon(s));
        String expected = "This is a test: 1";
        assertEquals(expected, MiscUtils.joinColon("This is a test", 1));
        expected = "This is a test: 1: 2: 3";
        assertEquals(expected, MiscUtils.joinColon("This is a test", 1, 2L,
                                                   Short.valueOf((short)3)));

        String sep = ",";
        assertEquals("", MiscUtils.join(sep));
        assertEquals(s, MiscUtils.join(sep, s));
        expected = "This is a test,1";
        assertEquals(expected, MiscUtils.join(sep, "This is a test", 1));
        expected = "This is a test,1,2,3";
        assertEquals(expected, MiscUtils.join(sep, "This is a test", 1, 2L,
                                              Short.valueOf((short)3)));

        assertEquals("", MiscUtils.join(sep, (Collection<?>)null));
        assertEquals("", MiscUtils.join(sep, Collections.emptyList()));
        assertEquals(s, MiscUtils.join(sep, Collections.singleton(s)));
        List<Object> list = new ArrayList<>();
        Collections.addAll(list, "This is a test", Integer.valueOf(1));
        expected = "This is a test,1";
        assertEquals(expected, MiscUtils.join(sep, list));
        Collections.addAll(list, Integer.valueOf(2), Integer.valueOf(3));
        expected = "This is a test,1,2,3";
        assertEquals(expected, MiscUtils.join(sep, "This is a test", 1, 2L,
                                              Short.valueOf((short)3)));
    }

    /**
     * Test case for {@link MiscUtils#cast(Class,Object)} and
     * {@link MiscUtils#checkedCast(Class,Object)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testCast() throws Exception {
        Object num = Integer.valueOf(0);
        List<Class<?>> good = new ArrayList<>();
        Collections.addAll(good, Integer.class, Number.class);
        for (Class<?> cls: good) {
            assertEquals(num, MiscUtils.cast(cls, num));
            assertEquals(num, MiscUtils.checkedCast(cls, num));
        }

        List<Class<?>> bad = new ArrayList<>();
        Collections.addAll(bad, Byte.class, Short.class, Long.class,
                           Float.class, Double.class, MiscUtils.class);
        for (Class<?> cls: bad) {
            assertEquals(null, MiscUtils.cast(cls, num));
            try {
                MiscUtils.checkedCast(cls, num);
                unexpected();
            } catch (DataTypeMismatchException e) {
                assertEquals(cls, e.getTargetType());
                assertSame(num, e.getObject());
            }
        }
    }

    /**
     * Test case for {@link MiscUtils#toLowerCase(String)}.
     */
    @Test
    public void testToLowerCase() {
        Map<String, String> cases = new HashMap<>();
        cases.put("", "");
        cases.put("This is a test.", "this is a test.");
        cases.put("ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789;:'\"-_=+\\|`~",
                  "abcdefghijklmnopqrstuvwxyz0123456789;:'\"-_=+\\|`~");
        for (Map.Entry<String, String> entry: cases.entrySet()) {
            String str = entry.getKey();
            String expected = entry.getValue();
            assertEquals(expected, MiscUtils.toLowerCase(str));
        }
    }

    /**
     * Test case for {@link MiscUtils#unexpected()}.
     */
    @Test
    public void testUnexpected() {
        String msg = "Should never be called.";
        for (int i = 0; i < 10; i++) {
            IllegalStateException ise = MiscUtils.unexpected();
            assertEquals(IllegalStateException.class, ise.getClass());
            assertEquals(msg, ise.getMessage());
        }
    }

    /**
     * Test case for the following methods.
     *
     * <ul>
     *   <li>{@link MiscUtils#longValue(Counter32)}</li>
     *   <li>{@link MiscUtils#longValue(Counter64)}</li>
     *   <li>{@link MiscUtils#doubleValue(Counter32)}</li>
     *   <li>{@link MiscUtils#doubleValue(Counter64)}</li>
     * </ul>
     */
    @Test
    public void testCounterValue() {
        double delta = 0.000001;
        assertEquals(0, MiscUtils.longValue((Counter32)null));
        assertEquals(0, MiscUtils.longValue((Counter64)null));
        assertEquals(0d, MiscUtils.doubleValue((Counter32)null), delta);
        assertEquals(0d, MiscUtils.doubleValue((Counter64)null), delta);

        Random rand = new Random();
        for (int i = 0; i < 100; i++) {
            long v = rand.nextLong();
            long v32 = v & 0xffffffffL;
            Counter32 c32 = new Counter32(Long.valueOf(v32));
            assertEquals(v32, MiscUtils.longValue(c32));
            assertEquals((double)v32, MiscUtils.doubleValue(c32), delta);

            BigInteger bi = NumberUtils.getUnsigned(v);
            Counter64 c64 = new Counter64(bi);
            assertEquals(v, MiscUtils.longValue(c64));
            assertEquals(bi.doubleValue(), MiscUtils.doubleValue(c64), delta);
        }
    }

    /**
     * Test case for {@link MiscUtils#sortedCopy(List,Comparator)}.
     */
    @Test
    public void testSortedCopy() {
        Comparator<Number> comp = new Comparator<Number>() {
            @Override
            public int compare(Number n1, Number n2) {
                return Integer.compare(n1.intValue(), n2.intValue());
            }
        };

        Random rand = new Random();
        List<Integer> list = new ArrayList<>();
        Set<Integer> numbers = new HashSet<>();
        do {
            Integer i = rand.nextInt();
            if (numbers.add(i)) {
                list.add(i);
            }
        } while (numbers.size() < 100);

        List<Integer> sorted = MiscUtils.sortedCopy(list, comp);
        assertNotSame(list, sorted);
        assertEquals(list.size(), sorted.size());

        Integer prev = null;
        for (Integer i: sorted) {
            assertEquals(true, numbers.remove(i));
            if (prev != null) {
                assertTrue(prev.intValue() < i.intValue());
            }
            prev = i;
        }

        assertEquals(true, numbers.isEmpty());
    }

    /**
     * Test case for {@link MiscUtils#equalsUri(Uri,Uri)}.
     */
    @Test
    public void testEqualsUri() {
        Uri nullUri = null;
        String[] strings = {
            "uri:1",
            "uri:2",
            "uri:2.1",
            "uri:3",
        };

        assertEquals(true, MiscUtils.equalsUri(nullUri, nullUri));

        for (String s1: strings) {
            Uri u1 = new Uri(new String(s1));
            assertEquals(false, MiscUtils.equalsUri(u1, nullUri));
            assertEquals(false, MiscUtils.equalsUri(nullUri, u1));

            for (String s2: strings) {
                Uri u2 = new Uri(new String(s2));
                assertEquals(false, MiscUtils.equalsUri(u2, nullUri));
                assertEquals(false, MiscUtils.equalsUri(nullUri, u2));

                boolean expected = s1.equals(s2);
                assertEquals(expected, MiscUtils.equalsUri(u1, u2));
                assertEquals(expected, MiscUtils.equalsUri(u2, u1));
            }
        }
    }

    /**
     * Test case for {@link MiscUtils#getValue(Uri)}.
     */
    @Test
    public void testGetValueUri() {
        assertEquals(null, MiscUtils.getValue((Uri)null));
        assertEquals(null, MiscUtils.getValue((NodeConnectorId)null));

        String[] strings = {
            "uri:1",
            "uri:2",
            "uri:2.1",
            "uri:3",
        };

        for (String s: strings) {
            assertEquals(s, MiscUtils.getValue(new Uri(s)));
            assertEquals(s, MiscUtils.getValue(new NodeConnectorId(s)));
        }
    }

    /**
     * Test case for {@link MiscUtils#unmodifiableList(List)}.
     */
    @Test
    public void testUnmodifiableList() {
        List<Integer> src = null;
        List<Integer> list = MiscUtils.unmodifiableList(src);
        assertTrue(list.isEmpty());
        try {
            list.add(1);
            unexpected();
        } catch (UnsupportedOperationException e) {
        }

        src = new ArrayList<>();
        list = MiscUtils.unmodifiableList(src);
        assertTrue(list.isEmpty());
        try {
            list.add(1);
            unexpected();
        } catch (UnsupportedOperationException e) {
        }

        for (int i = 0; i <= 10; i++) {
            assertTrue(src.add(i));
            list = MiscUtils.unmodifiableList(src);
            assertEquals(src, list);
            try {
                list.add(1);
                unexpected();
            } catch (UnsupportedOperationException e) {
            }
        }
    }

    /**
     * Test case for {@link MiscUtils#unmodifiableSet(Set)}.
     */
    @Test
    public void testUnmodifiableSet() {
        Set<Integer> src = null;
        Set<Integer> set = MiscUtils.unmodifiableSet(src);
        assertTrue(set.isEmpty());
        try {
            set.add(1);
            unexpected();
        } catch (UnsupportedOperationException e) {
        }

        src = new HashSet<>();
        set = MiscUtils.unmodifiableSet(src);
        assertTrue(set.isEmpty());
        try {
            set.add(1);
            unexpected();
        } catch (UnsupportedOperationException e) {
        }

        for (int i = 0; i <= 10; i++) {
            assertTrue(src.add(i));
            set = MiscUtils.unmodifiableSet(src);
            assertEquals(src, set);
            try {
                set.add(1);
                unexpected();
            } catch (UnsupportedOperationException e) {
            }
        }
    }

    /**
     * Test case for {@link MiscUtils#unmodifiableKeySet(Map)}.
     */
    @Test
    public void testUnmodifiableKeySet() {
        Map<Integer, Boolean> src = null;
        Set<Integer> set = MiscUtils.unmodifiableKeySet(src);
        assertTrue(set.isEmpty());
        try {
            set.add(1);
            unexpected();
        } catch (UnsupportedOperationException e) {
        }

        src = new HashMap<>();
        set = MiscUtils.unmodifiableKeySet(src);
        assertTrue(set.isEmpty());
        try {
            set.add(1);
            unexpected();
        } catch (UnsupportedOperationException e) {
        }

        for (int i = 0; i <= 10; i++) {
            assertEquals(null, src.put(i, Boolean.TRUE));
            set = MiscUtils.unmodifiableKeySet(src);
            assertEquals(src.keySet(), set);
            try {
                set.add(1);
                unexpected();
            } catch (UnsupportedOperationException e) {
            }
        }
    }

    /**
     * Verify the contents of the given {@link VTNException}.
     *
     * <p>
     *   This method expects that {@link VtnErrorTag#BADREQUEST} is configured
     *   in the given exception.
     * </p>
     *
     * @param e     A {@link VTNException} to be tested.
     * @param desc  An expected description.
     */
    private void checkException(VTNException e, String desc) {
        checkException(e, desc, VtnErrorTag.BADREQUEST);
    }

    /**
     * Verify the contents of the given {@link VTNException}.
     *
     * @param e     A {@link VTNException} to be tested.
     * @param desc  An expected description.
     * @param vtag  An expected error tag.
     */
    private void checkException(VTNException e, String desc, VtnErrorTag vtag) {
        assertEquals(desc, e.getMessage());
        assertEquals(vtag, e.getVtnErrorTag());
    }
}
