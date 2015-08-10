/*
 * Copyright (c) 2014, 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal;

import java.io.File;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Map;

import org.junit.Test;

import org.opendaylight.controller.sal.utils.Status;

/**
 * JUnit test for {@link ContainerConfig}.
 */
public class ContainerConfigTest extends TestBase {
    /**
     * The number of test keys.
     */
    private static final int  NUM_KEYS = 100;

    /**
     * Test case for {@link ContainerConfig#init()} and
     * {@link ContainerConfig#cleanUp()}.
     */
    @Test
    public void testInit() {
        String[] containers = {
            "default",
            "container1",
            "container2",
        };

        ContainerConfig.Type[] types = ContainerConfig.Type.values();
        ContainerConfig[] configs = new ContainerConfig[containers.length];
        File[] dirs = new File[containers.length];
        for (int i = 0; i < containers.length; i++) {
            String container = containers[i];
            configs[i] = new ContainerConfig(container);
            dirs[i] = getConfigDir(container);
            assertFalse(dirs[i].exists());
            configs[i].init();
            assertTrue(dirs[i].isDirectory());
            for (ContainerConfig.Type type: types) {
                File d = new File(dirs[i], type.toString());
                assertTrue(d.toString(), d.isDirectory());
                assertEquals(0, d.list().length);
            }
            assertEquals(types.length, dirs[i].list().length);
        }

        // Initialize again.
        for (int i = 0; i < containers.length; i++) {
            String container = containers[i];
            configs[i].init();
            assertTrue(dirs[i].isDirectory());
            for (ContainerConfig.Type type: types) {
                File d = new File(dirs[i], type.toString());
                assertTrue(d.isDirectory());
                assertEquals(0, d.list().length);
            }
            assertEquals(types.length, dirs[i].list().length);
        }

        deleteStartUpHome();

        // Put unexpected regular files.
        for (int i = 0; i < containers.length; i++) {
            String container = containers[i];
            assertTrue(dirs[i].mkdirs());
            for (ContainerConfig.Type type: types) {
                File f = new File(dirs[i], type.toString());
                try {
                    assertTrue(f.createNewFile());
                } catch (Exception e) {
                    unexpected(e);
                }
            }
        }

        String[] files = {
            "aaa",
            "bbb",
            "ccc",
            "ddd",
        };

        // Unexpected regular files should be deleted.
        for (int i = 0; i < containers.length; i++) {
            String container = containers[i];
            configs[i].init();
            assertTrue(dirs[i].isDirectory());
            for (ContainerConfig.Type type: types) {
                File d = new File(dirs[i], type.toString());
                assertTrue(d.isDirectory());
                assertEquals(0, d.list().length);

                // Create files under config directories.
                for (String file: files) {
                    File f = new File(d, file);
                    try {
                        f.createNewFile();
                    } catch (Exception e) {
                        unexpected(e);
                    }
                }
                assertEquals(files.length, d.list().length);
            }
            assertEquals(types.length, dirs[i].list().length);

        }

        // Clean up config directories.
        for (int i = 0; i < containers.length; i++) {
            assertTrue(dirs[i].exists());
            configs[i].cleanUp();
            assertFalse(dirs[i].exists());
            for (int j = i + 1; j < containers.length; j++) {
                assertTrue(dirs[j].exists());
            }
        }
    }

    /**
     * Ensure that the configuration file can be saved and loaded.
     */
    @Test
    public void testSaveLoad() {
        String container = "default";
        File baseDir = getConfigDir(container);
        ContainerConfig cfg = new ContainerConfig(container);

        // Try to access configuration file before initialization.
        String badKey = "badkey";
        Status status = cfg.save(ContainerConfig.Type.TENANT, badKey,
                                 Integer.valueOf(1));
        assertFalse(status.isSuccess());
        assertEquals(null, cfg.load(ContainerConfig.Type.TENANT, badKey));
        assertFalse(cfg.delete(ContainerConfig.Type.TENANT, badKey));

        cfg.init();

        String[] broken = {
            "aaa",
            ".conf",
            "___test___",
        };
        long nano = System.nanoTime();
        HashMap<ContainerConfig.Type, Map<String, Long>> saved =
            new HashMap<>();
        ContainerConfig.Type[] types = ContainerConfig.Type.values();
        for (ContainerConfig.Type type: types) {
            HashMap<String, Long> map = new HashMap<>();
            assertNull(saved.put(type, map));
            assertEquals(0, cfg.getKeys(type).size());

            File dir = new File(baseDir, type.toString());
            for (String file: broken) {
                File f = new File(dir, file);
                try {
                    assertTrue(f.createNewFile());
                } catch (Exception e) {
                    unexpected(e);
                }
            }
            assertEquals(0, cfg.getKeys(type).size());

            // Construct test data and save.
            for (int i = 0; i < NUM_KEYS; i++) {
                String key = "key_" + i;
                assertNull(map.put(key, nano));

                status = cfg.save(type, key, nano);
                assertTrue(status.toString(), status.isSuccess());

                HashSet<String> names = new HashSet<String>(cfg.getKeys(type));
                assertEquals(map.keySet(), names);
                nano++;
            }
            assertEquals(NUM_KEYS, map.size());
        }
        assertEquals(types.length, saved.size());

        // Load data from configuration files.
        HashMap<ContainerConfig.Type, Map<String, Long>> loaded =
            new HashMap<>();
        for (ContainerConfig.Type type: types) {
            HashMap<String, Long> map = new HashMap<>();
            assertNull(loaded.put(type, map));

            for (String key: cfg.getKeys(type)) {
                Object obj = cfg.load(type, key);
                assertTrue(obj instanceof Long);
                map.put(key, (Long)obj);
            }
        }
        assertEquals(saved, loaded);

        // Delete configuration files.
        for (ContainerConfig.Type type: types) {
            Map<String, Long> map = loaded.get(type);
            HashSet<String> names = new HashSet<String>(cfg.getKeys(type));
            for (Map.Entry<String, Long> entry: map.entrySet()) {
                String key = entry.getKey();
                Long value = entry.getValue();
                assertEquals(value, cfg.load(type, key));
                assertTrue(cfg.delete(type, key));
                assertEquals(null, cfg.load(type, key));
                assertTrue(names.remove(key));
                HashSet<String> keys = new HashSet<String>(cfg.getKeys(type));
                assertEquals(names, keys);
            }
        }
    }

    /**
     * Test case for {@link ContainerConfig#deleteAll(org.opendaylight.vtn.manager.internal.ContainerConfig.Type, Set)}.
     */
    @Test
    public void testDeleteAll() {
        String container = "default";
        File baseDir = getConfigDir(container);
        ContainerConfig cfg = new ContainerConfig(container);

        // Try to access configuration file before initialization.
        assertFalse(cfg.deleteAll(ContainerConfig.Type.TENANT, null));
        cfg.init();
        assertTrue(cfg.deleteAll(ContainerConfig.Type.TENANT, null));

        // Construct test data and save.
        long nano = System.nanoTime();
        HashSet<String> retain = new HashSet<>();
        HashMap<String, Long> saved = new HashMap<>();
        ContainerConfig.Type type = ContainerConfig.Type.TENANT;
        for (int i = 0; i < NUM_KEYS; i++) {
            String key = "key_" + i;
            assertNull(saved.put(key, nano));
            Status status = cfg.save(type, key, nano);
            assertTrue(status.toString(), status.isSuccess());
            if ((i % 4) == 0) {
                assertTrue(retain.add(key));
            }
            nano++;
        }
        assertFalse(retain.isEmpty());
        assertEquals(saved.keySet(), new HashSet<String>(cfg.getKeys(type)));

        // Remove all files except for files in "retain".
        assertTrue(cfg.deleteAll(ContainerConfig.Type.TENANT, retain));
        assertEquals(retain, new HashSet<String>(cfg.getKeys(type)));

        // Remove all files.
        assertTrue(cfg.deleteAll(ContainerConfig.Type.TENANT, null));
        assertEquals(0, cfg.getKeys(type).size());
    }
}
