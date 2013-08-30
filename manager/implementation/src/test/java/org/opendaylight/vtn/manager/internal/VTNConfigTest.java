/**
 * Copyright (c) 2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */
package org.opendaylight.vtn.manager.internal;

import static org.junit.Assert.*;

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;

import org.junit.After;
import org.junit.AfterClass;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;

/**
 * test for {@code VTNConfig}
 */
public class VTNConfigTest extends TestBase {
    static String containerName = "unittest";

    @AfterClass
    public static void afterclass() {
        cleanup(true);
    }

    @Before
    public void before() {
        cleanup(false);
    }

    @After
    public void after() {
        cleanup(false);
    }


    /**
     * Test case for
     * {@link VTNConfig#VTNConfig(java.lang.String, java.lang.String)},
     * {@link VTNConfig#getNodeEdgeWait()}.
     */
    @Test
    public void testVTNConfig() {
        String[] values = {null, "empty", "", "0", "-1",  "1", "600000", "600001", "val"};
        String dir = "./";
        String filename = "vtnmanager-" + containerName + ".ini";
        String gfilename = "vtnmanager.ini";

        for (String gval : values) {
            // setup global .ini file.
            FileWriter gwriter;
            File ginifile = new File(dir, gfilename);
            if (gval != null && gval.equals("empty")) {
                // if null, create a empty file.
                try {
                    ginifile.createNewFile();
                } catch (IOException e) {
                    unexpected(e);
                }
            } else if (gval != null){
                String prop = "nodeEdgeWait=" + gval;
                try {
                    gwriter = new FileWriter(ginifile);
                    gwriter.write(prop);
                    gwriter.close();
                } catch (IOException e) {
                    unexpected(e);
                }
            }

            int defaultval = 3000;
            Integer v = null;
            if (gval != null && !gval.equals("empty")) {
                try {
                    v = Integer.decode(gval);
                } catch (NumberFormatException e) {
                }
            }
            if (v != null && v.intValue() >= 0 && v.intValue() <= 600000) {
                defaultval = v.intValue();
            }

            for (String cval : values) {
                String emsg = "global=" + gval + "," + "container=" + cval;

                // setup container .ini file
                FileWriter writer;
                File inifile = new File(dir, filename);
                VTNConfig conf;

                if (cval != null && cval.equals("empty")) {
                    try {
                        writer = new FileWriter(inifile);
                        writer.close();
                    } catch (IOException e) {
                        unexpected(e);
                    }
                    conf = new VTNConfig(dir, containerName);
                } else if (cval != null){
                    String prop = "nodeEdgeWait=" + cval;
                    try {
                        writer = new FileWriter(inifile);
                        writer.write(prop);
                        writer.close();
                    } catch (IOException e) {
                        unexpected(e);
                    }
                    conf = new VTNConfig(dir, containerName);
                } else { // cval == null
                    conf = new VTNConfig(dir, "null");
                }

                Integer cv = null;
                if (cval != null && !cval.equals("empty")) {
                    try {
                        cv = Integer.decode(cval);
                        if (cv.intValue() < 0 || cv.intValue() > 600000) {
                            cv = Integer.valueOf(3000);
                        }
                    } catch (NumberFormatException e) {
                        cv = Integer.valueOf(3000);
                    }
                }
                if (cv == null || cval.equals("empty")) {
                    assertEquals(emsg, defaultval, conf.getNodeEdgeWait());
                } else {
                    assertEquals(emsg, cv.intValue(), conf.getNodeEdgeWait());
                }
                conf = null;

                if (inifile.exists()) {
                    inifile.delete();
                }
            }
            if (ginifile.exists()) {
                ginifile.delete();
            }
        }
    }

    /**
     * delete .ini files.
     * @param isExit
     */
    public static void cleanup(boolean isExit) {
        String dir = "./";
        String filename = "vtnmanager-" + containerName + ".ini";
        String gfilename = "vtnmanager.ini";

        File ginifile = new File(dir, gfilename);
        if (ginifile.exists()) {
            if (isExit) {
                ginifile.deleteOnExit();
            } else {
                ginifile.delete();
            }
        }
        File inifile = new File(dir, filename);
        if (inifile.exists()) {
            if (isExit) {
                inifile.deleteOnExit();
            } else {
                inifile.delete();
            }
        }
    }
}
