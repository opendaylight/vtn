/*
 * Copyright (c) 2013-2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal;

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;

import org.junit.After;
import org.junit.AfterClass;
import org.junit.Before;
import org.junit.Test;

/**
 * JUnit test for {@link VTNConfig}.
 */
public class VTNConfigTest extends TestBase {
    // Default value, min value, max value of nodeEdgeWait.
    private static final int  DEFAULT_NODE_EDGE_WAIT = 3000;
    private static final int  MIN_NODE_EDGE_WAIT = 0;
    private static final int  MAX_NODE_EDGE_WAIT = 600000;

    // Default value, min value, max value of l2FlowPriority
    private static final int  DEFAULT_L2FLOW_PRIORITY = 10;
    private static final int  MIN_L2FLOW_PRIORITY = 1;
    private static final int  MAX_L2FLOW_PRIORITY = 999;

    // Default value, min value, max value value of flowModTimeout
    private static final int  DEFAULT_FLOWMOD_TIMEOUT = 3000;
    private static final int  MIN_FLOWMOD_TIMEOUT = 100;
    private static final int  MAX_FLOWMOD_TIMEOUT = 60000;

    // Default value, min value, max value of remoteFlowModTimeout
    private static final int  DEFAULT_REMOTE_FLOWMOD_TIMEOUT = 5000;
    private static final int  MIN_REMOTE_FLOWMOD_TIMEOUT = 1000;
    private static final int  MAX_REMOTE_FLOWMOD_TIMEOUT = 60000;

    // Default value, min value, max value of remoteBulkFlowModTimeout
    private static final int  DEFAULT_REMOTE_BULK_FLOWMOD_TIMEOUT = 15000;
    private static final int  MIN_REMOTE_BULK_FLOWMOD_TIMEOUT = 3000;
    private static final int  MAX_REMOTE_BULK_FLOWMOD_TIMEOUT = 600000;

    // Default value, min value, max value of cacheInitTimeout
    private static final int  DEFAULT_CACHE_INIT_TIMEOUT = 3000;
    private static final int  MIN_CACHE_INIT_TIMEOUT = 100;
    private static final int  MAX_CACHE_INIT_TIMEOUT = 600000;

    // Default value, min value, max value of cacheTransactionTimeout
    private static final int  DEFAULT_CACHE_TRANSACTION_TIMEOUT = 10000;
    private static final int  MIN_CACHE_TRANSACTION_TIMEOUT = 100;
    private static final int  MAX_CACHE_TRANSACTION_TIMEOUT = 600000;

    // Separaters between key and value.
    private static final String[] SEPARATORS = new String[] {"=", ":"};

    // Configuration file name.
    private static final String CONTAINER_NAME = "unittest";
    private static final String CONTAINER_FILE_NAME =
        "vtnmanager-" + CONTAINER_NAME + ".ini";
    private static final String GLOBAL_FILE_NAME = "vtnmanager.ini";

    // The working directory used for test.
    private static final String WORK_DIR = "work";

    @AfterClass
    public static void afterclass() {
        cleanup(true);
    }

    @Before
    public void before() {
        File file = new File(WORK_DIR);
        if (!file.exists()) {
            file.mkdir();
        }

        cleanup(false);
    }

    @After
    public void after() {
        cleanup(false);

        File file = new File(WORK_DIR);
        if (file.exists()) {
            file.delete();
        }
    }

    /**
     * Test case for
     * {@link VTNConfig#VTNConfig(String, String)},
     * {@link VTNConfig#getNodeEdgeWait()}.
     *
     * This method tests {@code nodeEdgeWait} parameter.
     */
    @Test
    public void testVTNConfigNodeEdgeWait() {
        String[] values = {
            null, "empty", "",
            String.valueOf(MIN_NODE_EDGE_WAIT),
            String.valueOf(MIN_NODE_EDGE_WAIT - 1),
            String.valueOf(MAX_NODE_EDGE_WAIT),
            String.valueOf(MAX_NODE_EDGE_WAIT + 1),
            "val"
        };

        testVTNConfig("nodeEdgeWait", DEFAULT_NODE_EDGE_WAIT,
                      MIN_NODE_EDGE_WAIT, MAX_NODE_EDGE_WAIT, values);
    }

    /**
     * Test case for
     * {@link VTNConfig#VTNConfig(String, String)},
     * {@link VTNConfig#getL2FlowPriority()}.
     *
     * This method tests {@code l2FlowPriority} parameter.
     */
    @Test
    public void testVTNConfigL2FlowPriority() {
        String[] values = {
            null, "empty", "",
            String.valueOf(MIN_L2FLOW_PRIORITY),
            String.valueOf(MIN_L2FLOW_PRIORITY - 1),
            String.valueOf(MAX_L2FLOW_PRIORITY),
            String.valueOf(MAX_L2FLOW_PRIORITY + 1),
            "val"
        };

        testVTNConfig("l2FlowPriority", DEFAULT_L2FLOW_PRIORITY,
                      MIN_L2FLOW_PRIORITY, MAX_L2FLOW_PRIORITY, values);
    }

    /**
     * Test case for
     * {@link VTNConfig#VTNConfig(String, String)},
     * {@link VTNConfig#getFlowModTimeout()}.
     *
     * This method tests {@code flowModTimeout} parameter.
     */
    @Test
    public void testVTNConfigFlowModTimeout() {
        String[] values = {
            null, "empty", "",
            String.valueOf(MIN_FLOWMOD_TIMEOUT),
            String.valueOf(MIN_FLOWMOD_TIMEOUT - 1),
            String.valueOf(MAX_FLOWMOD_TIMEOUT),
            String.valueOf(MAX_FLOWMOD_TIMEOUT + 1),
            "val"
        };

        testVTNConfig("flowModTimeout", DEFAULT_FLOWMOD_TIMEOUT,
                      MIN_FLOWMOD_TIMEOUT, MAX_FLOWMOD_TIMEOUT, values);
    }

    /**
     * Test case for
     * {@link VTNConfig#VTNConfig(String, String)},
     * {@link VTNConfig#getRemoteFlowModTimeout()}.
     *
     * This method tests {@code remoteFlowModTimeout} parameter.
     */
    @Test
    public void testVTNConfigRemoteFlowModTimeout() {
        String[] values = {
            null, "empty", "",
            String.valueOf(MIN_REMOTE_FLOWMOD_TIMEOUT),
            String.valueOf(MIN_REMOTE_FLOWMOD_TIMEOUT - 1),
            String.valueOf(MAX_REMOTE_FLOWMOD_TIMEOUT),
            String.valueOf(MAX_REMOTE_FLOWMOD_TIMEOUT + 1),
            "val"
        };

        testVTNConfig("remoteFlowModTimeout", DEFAULT_REMOTE_FLOWMOD_TIMEOUT,
                      MIN_REMOTE_FLOWMOD_TIMEOUT, MAX_REMOTE_FLOWMOD_TIMEOUT,
                      values);
    }

    /**
     * Test case for
     * {@link VTNConfig#VTNConfig(String, String)},
     * {@link VTNConfig#getRemoteBulkFlowModTimeout()}.
     *
     * This method tests {@code remoteBulkFlowModTimeout} parameter.
     */
    @Test
    public void testVTNConigNodeRemoteBulkFlowModTimeout() {
        String[] values = {
            null, "empty", "",
            String.valueOf(MIN_REMOTE_BULK_FLOWMOD_TIMEOUT),
            String.valueOf(MIN_REMOTE_BULK_FLOWMOD_TIMEOUT - 1),
            String.valueOf(MAX_REMOTE_BULK_FLOWMOD_TIMEOUT),
            String.valueOf(MAX_REMOTE_BULK_FLOWMOD_TIMEOUT + 1),
            "val"
        };

        testVTNConfig("remoteBulkFlowModTimeout",
                      DEFAULT_REMOTE_BULK_FLOWMOD_TIMEOUT,
                      MIN_REMOTE_BULK_FLOWMOD_TIMEOUT,
                      MAX_REMOTE_BULK_FLOWMOD_TIMEOUT, values);
    }

    /**
     * Test case for
     * {@link VTNConfig#VTNConfig(String, String)},
     * {@link VTNConfig#getCacheInitTimeout()}.
     *
     * This method tests {@code cacheInitTimeout} parameter.
     */
    @Test
    public void testVTNConigCacheInitTimeout() {
        String[] values = {
            null, "empty", "",
            String.valueOf(MIN_CACHE_INIT_TIMEOUT),
            String.valueOf(MIN_CACHE_INIT_TIMEOUT - 1),
            String.valueOf(MAX_CACHE_INIT_TIMEOUT),
            String.valueOf(MAX_CACHE_INIT_TIMEOUT + 1),
            "val"
        };

        testVTNConfig("cacheInitTimeout",
                      DEFAULT_CACHE_INIT_TIMEOUT,
                      MIN_CACHE_INIT_TIMEOUT,
                      MAX_CACHE_INIT_TIMEOUT, values);
    }

    /**
     * Test case for
     * {@link VTNConfig#VTNConfig(String, String)},
     * {@link VTNConfig#getCacheTransactionTimeout()}.
     *
     * This method tests {@code cacheTransactionTimeout} parameter.
     */
    @Test
    public void testVTNConigCacheTransactionTimeout() {
        String[] values = {
            null, "empty", "",
            String.valueOf(MIN_CACHE_TRANSACTION_TIMEOUT),
            String.valueOf(MIN_CACHE_TRANSACTION_TIMEOUT - 1),
            String.valueOf(MAX_CACHE_TRANSACTION_TIMEOUT),
            String.valueOf(MAX_CACHE_TRANSACTION_TIMEOUT + 1),
            "val"
        };

        testVTNConfig("cacheTransactionTimeout",
                      DEFAULT_CACHE_TRANSACTION_TIMEOUT,
                      MIN_CACHE_TRANSACTION_TIMEOUT,
                      MAX_CACHE_TRANSACTION_TIMEOUT, values);
    }

    /**
     * Common routine for test cases of {@link VTNConfig}.
     *
     * @param parameterString   A key String of parameter.
     * @param defaultVal        A default value.
     * @param minValue          A minimum value.
     * @param maxValue          A maximum value.
     * @param values            Values which is tested as parameter.
     */
    private void testVTNConfig(String parameterString, int defaultVal,
                               int minValue, int maxValue, String[] values) {
        cleanup(false);

        for (String separater : SEPARATORS) {
            for (String gval : values) {
                // setup global .ini file.
                FileWriter gWriter;
                File gIniFile = new File(WORK_DIR, GLOBAL_FILE_NAME);
                if (gval != null && gval.equals("empty")) {
                    // if null, create a empty file.
                    try {
                        assertTrue(gIniFile.createNewFile());
                    } catch (IOException e) {
                        unexpected(e);
                    }
                } else if (gval != null) {
                    String prop = parameterString + separater + gval;
                    int ave = (minValue + maxValue) / 2;
                    String propComment = "#" + parameterString + separater + ave;
                    try {
                        gWriter = new FileWriter(gIniFile);
                        gWriter.write(prop);
                        gWriter.write("\n");
                        gWriter.write(propComment);
                        gWriter.close();
                    } catch (IOException e) {
                        unexpected(e);
                    }
                } else {
                    // in case of null delete configuration file.
                    if (gIniFile.exists()) {
                        assertTrue(gIniFile.delete());
                    }
                }

                int realDefaultVal = defaultVal;
                Integer v = null;
                if (gval != null && !gval.equals("empty")) {
                    try {
                        v = Integer.decode(gval);
                    } catch (NumberFormatException e) {
                    }
                }
                if (v != null && v.intValue() >= minValue
                        && v.intValue() <= maxValue) {
                    realDefaultVal = v.intValue();
                }

                for (String cval : values) {
                    String emsg = "global=" + gval + "," + "container=" + cval;

                    // setup container .ini file
                    FileWriter cWriter;
                    File contIniFile = new File(WORK_DIR, CONTAINER_FILE_NAME);
                    VTNConfig conf;

                    if (cval != null && cval.equals("empty")) {
                        try {
                            cWriter = new FileWriter(contIniFile);
                            cWriter.close();
                        } catch (IOException e) {
                            unexpected(e);
                        }
                        conf = new VTNConfig(WORK_DIR, CONTAINER_NAME);
                    } else if (cval != null) {
                        String prop = parameterString + separater + cval;
                        int ave = (minValue + maxValue) / 4;
                        String propComment = "#" + parameterString + separater + ave;
                        try {
                            cWriter = new FileWriter(contIniFile);
                            cWriter.write(prop);
                            cWriter.write("\n");
                            cWriter.write(propComment);
                            cWriter.close();
                        } catch (IOException e) {
                            unexpected(e);
                        }
                        conf = new VTNConfig(WORK_DIR, CONTAINER_NAME);
                    } else {
                        // in case of null delete configuration file.
                        if (contIniFile.exists()) {
                            assertTrue(contIniFile.delete());
                        }
                        conf = new VTNConfig(WORK_DIR, "null");
                    }

                    Integer cv = null;
                    if (cval != null && !cval.equals("empty")) {
                        try {
                            cv = Integer.decode(cval);
                            if (cv.intValue() < minValue
                                    || cv.intValue() > maxValue) {
                                cv = Integer.valueOf(defaultVal);
                            }
                        } catch (NumberFormatException e) {
                            cv = Integer.valueOf(defaultVal);
                        }
                    }

                    int confValue = -100;
                    if (parameterString.equals("nodeEdgeWait")) {
                        confValue = conf.getNodeEdgeWait();
                    } else if (parameterString.equals("l2FlowPriority")) {
                        confValue = conf.getL2FlowPriority();
                    } else if (parameterString.equals("flowModTimeout")) {
                        confValue = conf.getFlowModTimeout();
                    } else if (parameterString.equals("remoteFlowModTimeout")) {
                        confValue = conf.getRemoteFlowModTimeout();
                    } else if (parameterString.equals("remoteBulkFlowModTimeout")) {
                        confValue = conf.getRemoteBulkFlowModTimeout();
                    } else if (parameterString.equals("cacheInitTimeout")) {
                        confValue = conf.getCacheInitTimeout();
                    } else if (parameterString.
                               equals("cacheTransactionTimeout")) {
                        confValue = conf.getCacheTransactionTimeout();
                    } else {
                        fail("not supported test case.");
                    }

                    if (cv == null || cval.equals("empty")) {
                        assertEquals(emsg, realDefaultVal, confValue);
                    } else {
                        assertEquals(emsg, cv.intValue(), confValue);
                    }
                    conf = null;

                    if (contIniFile.exists()) {
                        contIniFile.delete();
                    }
                }
                if (gIniFile.exists()) {
                    gIniFile.delete();
                }
            }
        }
    }

    /**
     * test cases for {@link VTNConfig}.
     *
     * This tests with Configuration file which include some properties.
     */
    @Test
    public void testVTNConfigAll() {
        String[] parameterStrings = new String[] {
            "nodeEdgeWait",
            "l2FlowPriority",
            "flowModTimeout",
            "remoteFlowModTimeout",
            "remoteBulkFlowModTimeout",
            "cacheInitTimeout",
            "cacheTransactionTimeout"
        };

        cleanup(false);

        for (String separater : SEPARATORS) {
            File gIniFile = new File(WORK_DIR, GLOBAL_FILE_NAME);
            if (gIniFile.exists()) {
                assertTrue(gIniFile.delete());
            }

            File cIniFile = new File(WORK_DIR, CONTAINER_FILE_NAME);
            if (cIniFile.exists()) {
                assertTrue(cIniFile.delete());
            }

            VTNConfig conf = new VTNConfig(WORK_DIR, CONTAINER_NAME);

            assertEquals(DEFAULT_NODE_EDGE_WAIT, conf.getNodeEdgeWait());
            assertEquals(DEFAULT_L2FLOW_PRIORITY, conf.getL2FlowPriority());
            assertEquals(DEFAULT_FLOWMOD_TIMEOUT, conf.getFlowModTimeout());
            assertEquals(DEFAULT_REMOTE_FLOWMOD_TIMEOUT,
                         conf.getRemoteFlowModTimeout());
            assertEquals(DEFAULT_REMOTE_BULK_FLOWMOD_TIMEOUT,
                         conf.getRemoteBulkFlowModTimeout());
            assertEquals(DEFAULT_CACHE_INIT_TIMEOUT,
                         conf.getCacheInitTimeout());
            assertEquals(DEFAULT_CACHE_TRANSACTION_TIMEOUT,
                         conf.getCacheTransactionTimeout());

            // setup global .ini file.
            FileWriter gWriter = null;
            gIniFile = new File(WORK_DIR, GLOBAL_FILE_NAME);
            try {
                gWriter = new FileWriter(gIniFile);
            } catch (IOException e) {
                unexpected(e);
            }

            for (String parameterString : parameterStrings) {
                StringBuilder prop = new StringBuilder(parameterString + separater);

                if (parameterString.equals("nodeEdgeWait")) {
                    prop.append(MIN_NODE_EDGE_WAIT);
                } else if (parameterString.equals("l2FlowPriority")) {
                    prop.append(MIN_L2FLOW_PRIORITY);
                } else if (parameterString.equals("flowModTimeout")) {
                    prop.append(MIN_FLOWMOD_TIMEOUT);
                } else if (parameterString.equals("remoteFlowModTimeout")) {
                    prop.append(MIN_REMOTE_FLOWMOD_TIMEOUT);
                } else if (parameterString.equals("remoteBulkFlowModTimeout")) {
                    prop.append(MIN_REMOTE_BULK_FLOWMOD_TIMEOUT);
                } else if (parameterString.equals("cacheInitTimeout")) {
                    prop.append(MIN_CACHE_INIT_TIMEOUT);
                } else if (parameterString.equals("cacheTransactionTimeout")) {
                    prop.append(MIN_CACHE_TRANSACTION_TIMEOUT);
                } else {
                    fail("not supported test case.");
                }
                prop.append("\n");

                try {
                    gWriter.write(prop.toString());
                } catch (IOException e) {
                    unexpected(e);
                }
            }

            try {
                gWriter.close();
            } catch (IOException e) {
                unexpected(e);
            }

            // setup container file
            conf = new VTNConfig(WORK_DIR, CONTAINER_NAME);

            assertEquals(MIN_NODE_EDGE_WAIT, conf.getNodeEdgeWait());
            assertEquals(MIN_L2FLOW_PRIORITY, conf.getL2FlowPriority());
            assertEquals(MIN_FLOWMOD_TIMEOUT, conf.getFlowModTimeout());
            assertEquals(MIN_REMOTE_FLOWMOD_TIMEOUT,
                         conf.getRemoteFlowModTimeout());
            assertEquals(MIN_REMOTE_BULK_FLOWMOD_TIMEOUT,
                         conf.getRemoteBulkFlowModTimeout());
            assertEquals(MIN_CACHE_INIT_TIMEOUT, conf.getCacheInitTimeout());
            assertEquals(MIN_CACHE_TRANSACTION_TIMEOUT,
                         conf.getCacheTransactionTimeout());

            // setup container .ini file.
            cIniFile = new File(WORK_DIR, CONTAINER_FILE_NAME);
            FileWriter cwriter = null;
            try {
                cwriter = new FileWriter(cIniFile);
            } catch (IOException e) {
                unexpected(e);
            }
            for (String parameterString : parameterStrings) {
                StringBuilder prop = new StringBuilder(parameterString + separater);

                if (parameterString.equals("nodeEdgeWait")) {
                    prop.append(MAX_NODE_EDGE_WAIT);
                } else if (parameterString.equals("l2FlowPriority")) {
                    prop.append(MAX_L2FLOW_PRIORITY);
                } else if (parameterString.equals("flowModTimeout")) {
                    prop.append(MAX_FLOWMOD_TIMEOUT);
                } else if (parameterString.equals("remoteFlowModTimeout")) {
                    prop.append(MAX_REMOTE_FLOWMOD_TIMEOUT);
                } else if (parameterString.equals("remoteBulkFlowModTimeout")) {
                    prop.append(MAX_REMOTE_BULK_FLOWMOD_TIMEOUT);
                } else if (parameterString.equals("cacheInitTimeout")) {
                    prop.append(MAX_CACHE_INIT_TIMEOUT);
                } else if (parameterString.equals("cacheTransactionTimeout")) {
                    prop.append(MAX_CACHE_TRANSACTION_TIMEOUT);
                } else {
                    fail("not supported test case.");
                }
                prop.append("\n");

                try {
                    cwriter.write(prop.toString());
                } catch (IOException e) {
                    unexpected(e);
                }
            }

            try {
                cwriter.close();
            } catch (IOException e) {
                unexpected(e);
            }

            conf = new VTNConfig(WORK_DIR, CONTAINER_NAME);

            assertEquals(MAX_NODE_EDGE_WAIT, conf.getNodeEdgeWait());
            assertEquals(MAX_L2FLOW_PRIORITY, conf.getL2FlowPriority());
            assertEquals(MAX_FLOWMOD_TIMEOUT, conf.getFlowModTimeout());
            assertEquals(MAX_REMOTE_FLOWMOD_TIMEOUT,
                         conf.getRemoteFlowModTimeout());
            assertEquals(MAX_REMOTE_BULK_FLOWMOD_TIMEOUT,
                         conf.getRemoteBulkFlowModTimeout());
            assertEquals(MAX_CACHE_INIT_TIMEOUT, conf.getCacheInitTimeout());
            assertEquals(MAX_CACHE_TRANSACTION_TIMEOUT,
                         conf.getCacheTransactionTimeout());

            gIniFile.delete();
            cIniFile.delete();
        }
    }

    /**
     * delete .ini files.
     *
     * @param isExit    If this is {@code true}, {@code deleteOnExit()} called
     *                  instead of {@code delete()}.
     */
    public static void cleanup(boolean isExit) {
        File ginifile = new File(WORK_DIR, GLOBAL_FILE_NAME);
        if (ginifile.exists()) {
            if (isExit) {
                ginifile.deleteOnExit();
            } else {
                ginifile.delete();
            }
        }

        File inifile = new File(WORK_DIR, CONTAINER_FILE_NAME);
        if (inifile.exists()) {
            if (isExit) {
                inifile.deleteOnExit();
            } else {
                inifile.delete();
            }
        }
    }
}
