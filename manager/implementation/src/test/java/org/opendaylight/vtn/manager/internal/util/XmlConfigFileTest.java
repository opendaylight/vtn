/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util;

import java.io.File;
import java.io.FileOutputStream;
import java.io.PrintStream;
import java.util.HashMap;
import java.util.Map;
import java.util.Set;
import java.util.HashSet;

import org.junit.Test;

import org.opendaylight.vtn.manager.VTenantConfig;
import org.opendaylight.vtn.manager.util.EtherAddress;

import org.opendaylight.vtn.manager.internal.config.VTNConfigImpl;

import org.opendaylight.vtn.manager.internal.TestBase;
import org.opendaylight.vtn.manager.internal.config.ConfigType;

/**
 * JUnit test for {@link XmlConfigFile}.
 */
public class XmlConfigFileTest extends TestBase {
    /**
     * The number of test keys.
     */
    private static final int  NUM_KEYS = 100;

    /**
     * Test case for {@link XmlConfigFile#init()}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testInit() throws Exception {
        File dir = getConfigDir();
        assertFalse(dir.exists());
        XmlConfigFile.init();
        assertTrue(dir.exists());
        XmlConfigFile.Type[] types = XmlConfigFile.Type.values();
        for (XmlConfigFile.Type type: types) {
            File d = new File(dir, type.toString());
            assertTrue(d.toString(), d.isDirectory());
            assertEquals(0, d.list().length);
        }
        assertEquals(types.length, dir.list().length);

        // Initialize again.
        XmlConfigFile.init();
        assertTrue(dir.exists());
        for (XmlConfigFile.Type type: types) {
            File d = new File(dir, type.toString());
            assertTrue(d.toString(), d.isDirectory());
            assertEquals(0, d.list().length);
        }
        assertEquals(types.length, dir.list().length);

        deleteStartUpHome();

        // Put unexpected regular files.
        assertTrue(dir.mkdirs());
        for (XmlConfigFile.Type type: types) {
            File f = new File(dir, type.toString());
            assertTrue(f.createNewFile());
        }

        String[] files = {
            "aaa",
            "bbb",
            "ccc",
            "ddd",
        };

        // Unexpected regular files should be deleted.
        XmlConfigFile.init();
        assertTrue(dir.isDirectory());
        for (XmlConfigFile.Type type: types) {
            File d = new File(dir, type.toString());
            assertTrue(d.isDirectory());
            assertEquals(0, d.list().length);

            // Create files under config directories.
            for (String file: files) {
                File f = new File(d, file);
                f.createNewFile();
            }
            assertEquals(files.length, d.list().length);
        }
        assertEquals(types.length, dir.list().length);
    }

    /**
     * Ensure that the configuration file can be saved and loaded.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testSaveLoad() throws Exception {
        Object obj = new Object();
        File baseDir = getConfigDir();
        Class<VTenantConfig> cls = VTenantConfig.class;

        // Try to access configuration file before initialization.
        String badKey = "badkey";
        XmlConfigFile.Type[] types = XmlConfigFile.Type.values();
        VTenantConfig dummy = new VTenantConfig("tenant 1", 100, 200);
        for (XmlConfigFile.Type type: types) {
            assertFalse(XmlConfigFile.save(type, badKey, dummy));
            assertEquals(null, XmlConfigFile.load(type, badKey, cls));
            assertFalse(XmlConfigFile.delete(type, badKey));
        }

        XmlConfigFile.init();

        String[] ignored = {
            "aaa",
            ".conf",
            ".xml",
            "___test___",
        };
        String[] badDirs = {
            "aaa",
            "bbb",
            "foo",
        };
        String[] broken = {
            "broken_1",
            "broken_2",
            "broken_3",
        };
        Map<XmlConfigFile.Type, Map<String, VTenantConfig>> saved =
            new HashMap<>();
        int idle = 0;
        int hard = 10;
        for (XmlConfigFile.Type type: types) {
            Map<String, VTenantConfig> map = new HashMap<>();
            assertEquals(null, saved.put(type, map));
            assertEquals(0, XmlConfigFile.getKeys(type).size());

            File dir = new File(baseDir, type.toString());
            for (String fname: ignored) {
                File f = new File(dir, fname);
                assertTrue(f.createNewFile());
            }
            for (String dname: badDirs) {
                File f = new File(dir, dname + ".xml");
                assertTrue(f.mkdirs());
                assertEquals(null, XmlConfigFile.load(type, dname, cls));
            }
            for (String key: broken) {
                String fname = key + ".xml";
                File f = new File(dir, fname);
                assertTrue(f.createNewFile());
                assertEquals(null, map.put(key, null));
            }

            assertEquals(broken.length, XmlConfigFile.getKeys(type).size());

            // Construct test data and save.
            for (int i = 0; i < NUM_KEYS; i++) {
                String key = "key_" + i;

                // Try to save non-JAXB object.
                assertFalse(XmlConfigFile.save(type, key, obj));

                String desc = key + ": " + idle;
                VTenantConfig tconf = new VTenantConfig(desc, idle, hard);
                assertEquals(null, map.put(key, tconf));
                assertTrue(XmlConfigFile.save(type, key, tconf));

                Set<String> names = new HashSet<>(XmlConfigFile.getKeys(type));
                assertEquals(map.keySet(), names);
                idle++;
                hard++;
            }
            assertEquals(NUM_KEYS + broken.length, map.size());
        }
        assertEquals(types.length, saved.size());

        // Load data from configuration files.
        Map<XmlConfigFile.Type, Map<String, VTenantConfig>> loaded =
            new HashMap<>();
        for (XmlConfigFile.Type type: types) {
            Map<String, VTenantConfig> map = new HashMap<>();
            assertEquals(null, loaded.put(type, map));

            for (String key: XmlConfigFile.getKeys(type)) {
                // Note that this will remove broken files.
                VTenantConfig tconf = XmlConfigFile.load(type, key, cls);
                if (key.startsWith("broken")) {
                    assertEquals(null, tconf);
                } else {
                    assertNotNull(tconf);
                }
                map.put(key, tconf);
            }
        }
        assertEquals(saved, loaded);

        // Delete configuration files.
        for (XmlConfigFile.Type type: types) {
            Map<String, VTenantConfig> map = loaded.get(type);
            Set<String> names = new HashSet<>(XmlConfigFile.getKeys(type));
            for (Map.Entry<String, VTenantConfig> entry: map.entrySet()) {
                String key = entry.getKey();
                VTenantConfig tconf = entry.getValue();
                if (key.startsWith("broken")) {
                    assertEquals(null, XmlConfigFile.load(type, key, cls));
                    assertFalse(XmlConfigFile.delete(type, key));
                } else {
                    assertEquals(tconf, XmlConfigFile.load(type, key, cls));
                    assertTrue(XmlConfigFile.delete(type, key));
                    assertTrue(names.remove(key));
                }
                assertEquals(null, XmlConfigFile.load(type, key, cls));
                Set<String> keys = new HashSet<>(XmlConfigFile.getKeys(type));
                assertEquals(names, keys);
            }
        }
    }

    /**
     * Test case for {@link XmlConfigFile#deleteAll(org.opendaylight.vtn.manager.internal.util.XmlConfigFile.Type, Set)}.
     */
    @Test
    public void testDeleteAll() {
        File baseDir = getConfigDir();

        // Try to access configuration file before initialization.
        XmlConfigFile.Type[] types = XmlConfigFile.Type.values();
        for (XmlConfigFile.Type type: types) {
            assertFalse(XmlConfigFile.deleteAll(type, null));
        }

        XmlConfigFile.init();

        for (XmlConfigFile.Type type: types) {
            assertTrue(XmlConfigFile.deleteAll(type, null));
        }

        // Construct test data and save.
        for (XmlConfigFile.Type type: types) {
            int idle = 0;
            int hard = 10;
            Set<String> retain = new HashSet<>();
            Map<String, VTenantConfig> saved = new HashMap<>();
            for (int i = 0; i < NUM_KEYS; i++) {
                String key = "key_" + i;
                String desc = key + ": " + idle;
                VTenantConfig tconf = new VTenantConfig(desc, idle, hard);
                assertEquals(null, saved.put(key, tconf));
                assertTrue(XmlConfigFile.save(type, key, tconf));
                if ((i % 4) == 0) {
                    assertTrue(retain.add(key));
                }
                idle++;
                hard++;
            }
            assertFalse(retain.isEmpty());

            Set<String> names = new HashSet<>(XmlConfigFile.getKeys(type));
            assertEquals(saved.keySet(), names);

            // Remove all files except for files in "retain".
            assertTrue(XmlConfigFile.deleteAll(type, retain));
            names = new HashSet<>(XmlConfigFile.getKeys(type));
            assertEquals(retain, names);

            // Remove all files.
            assertTrue(XmlConfigFile.deleteAll(type, null));
            assertEquals(0, XmlConfigFile.getKeys(type).size());
        }
    }

    /**
     * Ensure that {@link XmlConfigFile#load(org.opendaylight.vtn.manager.internal.util.XmlConfigFile.Type, String, Class)}
     * can detect an error in JAXB annotated method.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testLoad() throws Exception {
        XmlConfigFile.init();
        XmlConfigFile.Type type = XmlConfigFile.Type.CONFIG;

        // Write configuration with the minimum values.
        String minimum = "minimum";
        Boolean minBool = Boolean.FALSE;
        EtherAddress minMac = new EtherAddress(0L);
        File baseDir = getConfigDir();
        File dir = new File(baseDir, type.toString());
        File file = new File(dir, minimum + ".xml");
        PrintStream ps = new PrintStream(new FileOutputStream(file));
        ps.printf("<vtn-config>\n");
        ConfigType[] configTypes = ConfigType.values();
        for (ConfigType ctype: configTypes) {
            Object min = ctype.getMinimumValue();
            if (min == null) {
                min = (ctype.isBoolean()) ? minBool : minMac;
            }
            ps.println(ctype.getXmlElement(min));
        }
        ps.printf("</vtn-config>\n");
        ps.close();

        // Write configuration with the maximum values.
        String maximum = "maximum";
        Boolean maxBool = Boolean.TRUE;
        EtherAddress maxMac = new EtherAddress(0xffffffffffffL);
        file = new File(dir, maximum + ".xml");
        ps = new PrintStream(new FileOutputStream(file));
        ps.printf("<vtn-config>\n");
        for (ConfigType ctype: configTypes) {
            Object max = ctype.getMaximumValue();
            if (max == null) {
                max = (ctype.isBoolean()) ? maxBool : maxMac;
            }
            ps.println(ctype.getXmlElement(max));
        }
        ps.printf("</vtn-config>\n");
        ps.close();

        Class<VTNConfigImpl> cls = VTNConfigImpl.class;
        for (int i = 0; i < 2; i++) {
            VTNConfigImpl min = XmlConfigFile.load(type, minimum, cls);
            assertNotNull(min);
            for (ConfigType ctype: configTypes) {
                Object value = ctype.get(min);
                Object expected = ctype.getMinimumValue();
                if (expected == null) {
                    expected = (ctype.isBoolean()) ? minBool : minMac;
                }
                assertEquals(expected, value);
            }

            VTNConfigImpl max = XmlConfigFile.load(type, maximum, cls);
            assertNotNull(max);
            for (ConfigType ctype: configTypes) {
                Object value = ctype.get(max);
                Object expected = ctype.getMaximumValue();
                if (expected == null) {
                    expected = (ctype.isBoolean()) ? maxBool : maxMac;
                }
                assertEquals(expected, value);
            }
        }

        // Test with broken value.
        String bad = "bad";
        for (ConfigType ctype: configTypes) {
            if (!ctype.isBoolean()) {
                file = new File(dir, bad + ".xml");
                ps = new PrintStream(new FileOutputStream(file));
                ps.printf("<vtn-config>\n");
                ps.println(ctype.getXmlElement("bad value"));
                ps.printf("</vtn-config>\n");
                ps.close();
                assertEquals(true, file.isFile());
                assertEquals(ctype.toString(), null,
                             XmlConfigFile.load(type, bad, cls));
                assertEquals(false, file.isFile());
            }
        }

        // Test with too small values.
        for (ConfigType ctype: configTypes) {
            Object o = ctype.getMinimumValue();
            if (o == null) {
                continue;
            }
            assertTrue(o instanceof Integer);
            Integer min = (Integer)o;
            Integer value = Integer.valueOf(min.intValue() - 1);

            file = new File(dir, bad + ".xml");
            ps = new PrintStream(new FileOutputStream(file));
            ps.printf("<vtn-config>\n");
            ps.println(ctype.getXmlElement(value));
            ps.printf("</vtn-config>\n");
            ps.close();
            assertEquals(true, file.isFile());
            assertEquals(String.format("%s -> %s", ctype, value),
                         null, XmlConfigFile.load(type, bad, cls));
            assertEquals(false, file.isFile());
        }

        // Test with too large values.
        for (ConfigType ctype: configTypes) {
            Object o = ctype.getMaximumValue();
            if (o == null) {
                continue;
            }
            assertTrue(o instanceof Integer);
            Integer max = (Integer)o;
            Integer value = Integer.valueOf(max.intValue() + 1);

            file = new File(dir, bad + ".xml");
            ps = new PrintStream(new FileOutputStream(file));
            ps.printf("<vtn-config>\n");
            ps.println(ctype.getXmlElement(value));
            ps.printf("</vtn-config>\n");
            ps.close();
            assertEquals(true, file.isFile());
            assertEquals(String.format("%s -> %s", ctype, value),
                         null, XmlConfigFile.load(type, bad, cls));
            assertEquals(false, file.isFile());
        }
    }
}
