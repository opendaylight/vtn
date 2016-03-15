/*
 * Copyright (c) 2014, 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util;

import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.verifyNoMoreInteractions;
import static org.mockito.Mockito.when;

import java.math.BigInteger;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;
import java.util.Comparator;
import java.util.EnumSet;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Random;
import java.util.Set;
import java.util.TreeMap;

import com.google.common.collect.ImmutableList;

import org.junit.Test;

import org.opendaylight.vtn.manager.VTNException;
import org.opendaylight.vtn.manager.util.EtherAddress;
import org.opendaylight.vtn.manager.util.NumberUtils;

import org.opendaylight.vtn.manager.internal.util.rpc.RpcErrorTag;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;

import org.opendaylight.vtn.manager.internal.TestBase;
import org.opendaylight.vtn.manager.internal.TestMacAddress;
import org.opendaylight.vtn.manager.internal.TestVlanId;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeName;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeState;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnErrorTag;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateType;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnVlanIdField;

import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.NodeConnectorId;

import org.opendaylight.yang.gen.v1.urn.opendaylight.l2.types.rev130827.VlanId;

import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.inet.types.rev100924.Uri;
import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.yang.types.rev100924.Counter32;
import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.yang.types.rev100924.Counter64;
import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.yang.types.rev100924.MacAddress;

/**
 * JUnit test for {@link MiscUtils}.
 */
public class MiscUtilsTest extends TestBase {
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
     * Test case for {@link MiscUtils#checkPresent(String, VnodeName)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testCheckPresent() throws Exception {
        VnodeName vname = new VnodeName("node");
        String desc = "Virtual node";
        MiscUtils.checkPresent(desc, vname);

        vname = null;
        String msg = desc + " name cannot be null";
        try {
            MiscUtils.checkPresent(desc, vname);
            unexpected();
        } catch (RpcException e) {
            checkException(e, msg);
            assertEquals(RpcErrorTag.MISSING_ELEMENT, e.getErrorTag());
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
     * Test case for {@link MiscUtils#toLowerCase(VtnUpdateType)}.
     */
    @Test
    public void testToLowerCaseVtnUpdateType() {
        assertEquals("created", MiscUtils.toLowerCase(VtnUpdateType.CREATED));
        assertEquals("removed", MiscUtils.toLowerCase(VtnUpdateType.REMOVED));
        assertEquals("changed", MiscUtils.toLowerCase(VtnUpdateType.CHANGED));
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
     * Test case for {@link MiscUtils#equalsAsSet(Collection, Collection)}.
     */
    @Test
    public void testEqualsAsSet() {
        List<Integer> nullList = null;
        List<Integer> emptyList = Collections.<Integer>emptyList();
        assertTrue(MiscUtils.equalsAsSet(nullList, nullList));
        assertTrue(MiscUtils.equalsAsSet(nullList, emptyList));
        assertTrue(MiscUtils.equalsAsSet(emptyList, nullList));
        assertTrue(MiscUtils.equalsAsSet(emptyList, emptyList));

        List<Integer> list1 = ImmutableList.of(0);
        assertFalse(MiscUtils.equalsAsSet(nullList, list1));
        assertFalse(MiscUtils.equalsAsSet(emptyList, list1));
        assertFalse(MiscUtils.equalsAsSet(list1, nullList));
        assertFalse(MiscUtils.equalsAsSet(list1, emptyList));

        List<Integer> list2 = ImmutableList.of(0);
        assertTrue(MiscUtils.equalsAsSet(list1, list2));
        assertTrue(MiscUtils.equalsAsSet(list2, list1));

        list2 = ImmutableList.of(0, 0);
        assertTrue(MiscUtils.equalsAsSet(list1, list2));
        assertTrue(MiscUtils.equalsAsSet(list2, list1));

        list2 = ImmutableList.of(0, 1, 2);
        assertFalse(MiscUtils.equalsAsSet(list1, list2));
        assertFalse(MiscUtils.equalsAsSet(list2, list1));

        list1 = ImmutableList.of(2, 0, 1, 2, 0);
        assertTrue(MiscUtils.equalsAsSet(list1, list2));
        assertTrue(MiscUtils.equalsAsSet(list2, list1));

        list2 = ImmutableList.of(0, 1, 2, 3, 4, 5);
        assertFalse(MiscUtils.equalsAsSet(list1, list2));
        assertFalse(MiscUtils.equalsAsSet(list2, list1));

        list1 = ImmutableList.of(5, 3, 1, 0, 2, 4, 1, 2, 3, 4, 5);
        assertTrue(MiscUtils.equalsAsSet(list1, list2));
        assertTrue(MiscUtils.equalsAsSet(list2, list1));
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
     * Test case for {@link MiscUtils#getValue(VnodeName)}.
     */
    @Test
    public void testGetValueVnodeName() {
        assertEquals(null, MiscUtils.getValue((VnodeName)null));

        String[] strings = {
            "a",
            "vtn_1",
            "vbridge_2",
            "vif_3",
        };

        for (String s: strings) {
            assertEquals(s, MiscUtils.getValue(new VnodeName(s)));
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
     * Test case for {@link MiscUtils#getVlanId(VtnVlanIdField)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testGetVlanId() throws Exception {
        VtnVlanIdField field = mock(VtnVlanIdField.class);
        when(field.getVlanId()).thenReturn((VlanId)null);
        VlanId vlanId = MiscUtils.getVlanId(field);
        assertEquals(0, vlanId.getValue().intValue());
        verify(field).getVlanId();
        verifyNoMoreInteractions(field);

        Integer[] vids = {
            0, 1, 2, 88, 567, 890, 1234, 3456, 4094, 4095,
        };
        for (Integer vid: vids) {
            vlanId = new VlanId(vid);
            field = mock(VtnVlanIdField.class);
            when(field.getVlanId()).thenReturn(vlanId);
            assertEquals(vlanId, MiscUtils.getVlanId(field));
            verify(field).getVlanId();
            verifyNoMoreInteractions(field);
        }

        // VLAN ID is missing.
        RpcErrorTag etag = RpcErrorTag.MISSING_ELEMENT;
        VtnErrorTag vtag = VtnErrorTag.BADREQUEST;
        String msg = "vlan-id cannot be null";
        vlanId = new TestVlanId();
        field = mock(VtnVlanIdField.class);
        when(field.getVlanId()).thenReturn(vlanId);

        try {
            MiscUtils.getVlanId(field);
            unexpected();
        } catch (RpcException e) {
            assertEquals(etag, e.getErrorTag());
            assertEquals(vtag, e.getVtnErrorTag());
            assertEquals(msg, e.getMessage());
        }
        verify(field).getVlanId();
        verifyNoMoreInteractions(field);

        // Invalid VLAN ID.
        etag = RpcErrorTag.BAD_ELEMENT;
        Integer[] badVids = {
            Integer.MIN_VALUE, -2, -1, 4096, 4097, Integer.MAX_VALUE,
        };
        for (Integer vid: badVids) {
            msg = "Invalid VLAN ID: " + vid;
            vlanId = new TestVlanId(vid);
            field = mock(VtnVlanIdField.class);
            when(field.getVlanId()).thenReturn(vlanId);

            try {
                MiscUtils.getVlanId(field);
                unexpected();
            } catch (RpcException e) {
                assertEquals(etag, e.getErrorTag());
                assertEquals(vtag, e.getVtnErrorTag());
                assertEquals(msg, e.getMessage());
            }
            verify(field).getVlanId();
            verifyNoMoreInteractions(field);
        }
    }

    /**
     * Test case for {@link MiscUtils#isBadRequest(Throwable)}.
     */
    @Test
    public void testIsBadRequest() {
        Throwable[] throwables = {
            new NullPointerException(),
            new IllegalArgumentException(),
            new IllegalStateException(),
        };
        for (Throwable t: throwables) {
            assertEquals(false, MiscUtils.isBadRequest(t));
        }

        Set<VtnErrorTag> badTags = EnumSet.of(
            VtnErrorTag.BADREQUEST, VtnErrorTag.NOTFOUND,
            VtnErrorTag.CONFLICT);
        for (VtnErrorTag vtag: VtnErrorTag.values()) {
            String msg = "Message";
            VTNException ve = new VTNException(vtag, msg);
            boolean bad = badTags.contains(vtag);
            assertEquals(bad, MiscUtils.isBadRequest(ve));

            RpcException re = new RpcException(
                RpcErrorTag.BAD_ELEMENT, vtag, msg);
            assertEquals(bad, MiscUtils.isBadRequest(ve));
        }
    }

    /**
     * Test case for {@link MiscUtils#toValueList(Map)}.
     */
    @Test
    public void testToValueList() {
        Map<Integer, String> map = null;
        assertEquals(null, MiscUtils.toValueList(map));

        map = new TreeMap<>();
        assertEquals(null, MiscUtils.toValueList(map));

        map.put(1, "a");
        map.put(10, "b");
        map.put(-1, "c");
        map.put(9, "d");
        map.put(3, "e");
        map.put(4, "f");
        map.put(5, "g");

        List<String> expected = Arrays.
            asList("c", "a", "e", "f", "g", "d", "b");
        assertEquals(expected, MiscUtils.toValueList(map));
    }

    /**
     * Test case for {@link MiscUtils#isEmpty(Collection)}.
     */
    @Test
    public void testIsEmpty() {
        List<Integer> list = null;
        Set<Integer> set = null;
        assertEquals(true, MiscUtils.isEmpty(list));
        assertEquals(true, MiscUtils.isEmpty(set));

        list = new ArrayList<>();
        assertEquals(true, MiscUtils.isEmpty(list));
        set = new HashSet<>();
        assertEquals(true, MiscUtils.isEmpty(set));

        for (int i = 0; i < 10; i++) {
            list.add(i);
            assertEquals(false, MiscUtils.isEmpty(list));
            set.add(i);
            assertEquals(false, MiscUtils.isEmpty(set));
        }
    }

    /**
     * Test case for {@link MiscUtils#toVnodeState(boolean)}.
     */
    @Test
    public void testToVnodeState() {
        assertEquals(VnodeState.UP, MiscUtils.toVnodeState(true));
        assertEquals(VnodeState.DOWN, MiscUtils.toVnodeState(false));
    }

    /**
     * Test case for {@link MiscUtils#verify(MacAddress)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testVerifyMacAddress() throws Exception {
        Random rand = new Random(0x11112222333L);
        for (int i = 0; i < 30; i++) {
            EtherAddress eaddr = new EtherAddress(rand.nextLong());
            MacAddress mac = eaddr.getMacAddress();
            assertSame(mac, MiscUtils.verify(mac));
        }

        // Null MAC address.
        RpcErrorTag etag = RpcErrorTag.MISSING_ELEMENT;
        VtnErrorTag vtag = VtnErrorTag.BADREQUEST;
        String msg = "MAC address cannot be null";
        try {
            MiscUtils.verify((MacAddress)null);
            unexpected();
        } catch (RpcException e) {
            assertEquals(etag, e.getErrorTag());
            assertEquals(vtag, e.getVtnErrorTag());
            assertEquals(msg, e.getMessage());
        }

        // Invalid MAC address.
        etag = RpcErrorTag.BAD_ELEMENT;
        String[] invAddrs = {
            null,
            "00:11:22",
            "00:11:22:33:44:55:66",
            "aa:bb:cc:dd:ee:gg",
            "Bad address",
        };
        for (String addr: invAddrs) {
            TestMacAddress mac = new TestMacAddress(addr);
            msg = "Invalid MAC address: " + mac;
            try {
                MiscUtils.verify(mac);
                unexpected();
            } catch (RpcException e) {
                assertEquals(etag, e.getErrorTag());
                assertEquals(vtag, e.getVtnErrorTag());
                assertEquals(msg, e.getMessage());
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
