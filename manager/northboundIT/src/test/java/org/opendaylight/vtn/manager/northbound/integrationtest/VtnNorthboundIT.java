/**
 * Copyright (c) 2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */
package org.opendaylight.vtn.manager.northbound.integrationtest;

import static org.hamcrest.CoreMatchers.is;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertThat;
import static org.ops4j.pax.exam.CoreOptions.junitBundles;
import static org.ops4j.pax.exam.CoreOptions.mavenBundle;
import static org.ops4j.pax.exam.CoreOptions.options;
import static org.ops4j.pax.exam.CoreOptions.systemPackages;
import static org.ops4j.pax.exam.CoreOptions.systemProperty;

import java.io.BufferedReader;
import java.io.File;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;
import java.net.HttpURLConnection;
import java.net.URL;
import java.nio.charset.Charset;

import javax.inject.Inject;

import org.apache.commons.codec.binary.Base64;
import org.codehaus.jettison.json.JSONArray;
import org.codehaus.jettison.json.JSONException;
import org.codehaus.jettison.json.JSONObject;
import org.codehaus.jettison.json.JSONTokener;
import org.junit.After;
import org.junit.AfterClass;
import org.junit.Assert;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.opendaylight.controller.sal.utils.GlobalConstants;
import org.opendaylight.controller.usermanager.IUserManager;
import org.ops4j.pax.exam.Configuration;
import org.ops4j.pax.exam.Option;
import org.ops4j.pax.exam.junit.PaxExam;
import org.ops4j.pax.exam.util.PathUtils;
import org.osgi.framework.Bundle;
import org.osgi.framework.BundleContext;
import org.osgi.framework.ServiceReference;
import org.osgi.framework.Version;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

@RunWith(PaxExam.class)
public class VtnNorthboundIT {
    private static final Logger log = LoggerFactory.
        getLogger(VtnNorthboundIT.class);
    private static final String  BUNDLE_VTN_MANAGER_IMPL =
        "org.opendaylight.vtn.manager.implementation";

    // get the OSGI bundle context
    @Inject
    private BundleContext bc;
    private IUserManager userManager = null;
    private Boolean debugMsg = false;

    private Bundle  implBundle;

    @BeforeClass
    public static void beforeClass() {
        File confdir = new File(GlobalConstants.STARTUPHOME.toString());
        boolean result = confdir.exists();
        if (!result) {
            result = confdir.mkdirs();
        } else {
            File[] list = confdir.listFiles();
            for (File f : list) {
                f.delete();
            }
        }
    }

    @AfterClass
    public static void afterClass() {
        String currdir = new File(".").getAbsoluteFile().getParent();
        File confdir = new File(GlobalConstants.STARTUPHOME.toString());

        if (confdir.exists()) {
            File[] list = confdir.listFiles();
            for (File f : list) {
                f.delete();
            }

            while (confdir != null && confdir.getAbsolutePath() != currdir) {
                confdir.delete();
                String pname = confdir.getParent();
                if (pname == null) {
                    break;
                }
                confdir = new File(pname);
            }
        }
    }


    private String stateToString(int state) {
        switch (state) {
        case Bundle.ACTIVE:
            return "ACTIVE";
        case Bundle.INSTALLED:
            return "INSTALLED";
        case Bundle.RESOLVED:
            return "RESOLVED";
        case Bundle.UNINSTALLED:
            return "UNINSTALLED";
        default:
            return "Not CONVERTED";
        }
    }

    @Before
    public void areWeReady() {
        assertNotNull(bc);
        boolean debugit = false;
        for (Bundle b: bc.getBundles()) {
            String name = b.getSymbolicName();
            int state = b.getState();
            if (state != Bundle.ACTIVE && state != Bundle.RESOLVED) {
                log.debug("Bundle:" + name + " state:" + stateToString(state));
                debugit = true;
            } else if (BUNDLE_VTN_MANAGER_IMPL.equals(name)) {
                implBundle = b;
            }
        }
        if (debugit) {
            log.debug("Do some debugging because some bundle is " + "unresolved");
        }
        // Assert if true, if false we are good to go!
        assertFalse(debugit);

        ServiceReference r = bc.getServiceReference(IUserManager.class.getName());
        if (r != null) {
            this.userManager = (IUserManager) bc.getService(r);
        }
        // If UserManager is null, cannot login to run tests.
        assertNotNull(this.userManager);
    }

    // static variable to pass response code from getJsonResult()
    private static Integer httpResponseCode = null;

    private String getJsonResult(String restUrl) {
        return getJsonResult(restUrl, "GET", null);
    }

    private String getJsonResult(String restUrl, String method) {
        return getJsonResult(restUrl, method, null);
    }

    private String getJsonResult(String restUrl, String method, String body) {
        // Initialize response code to indicate error
        httpResponseCode = 400;

        if (debugMsg) {
            System.out.println("HTTP method: " + method + " url: " + restUrl.toString());
            if (body != null)
                System.out.println("body" + body);
        }

        try {
            URL url = new URL(restUrl);
            this.userManager.getAuthorizationList();
            this.userManager.authenticate("admin", "admin");
            String authString = "admin:admin";
            byte[] authEncBytes = Base64.encodeBase64(authString.getBytes());
            String authStringEnc = new String(authEncBytes);

            HttpURLConnection connection = (HttpURLConnection) url.openConnection();
            connection.setRequestMethod(method);
            connection.setRequestProperty("Authorization", "Basic " + authStringEnc);
            connection.setRequestProperty("Content-Type", "application/json");
            connection.setRequestProperty("Accept", "application/json");

            if (body != null) {
                connection.setDoOutput(true);
                OutputStreamWriter wr = new OutputStreamWriter(connection.getOutputStream());
                wr.write(body);
                wr.flush();
            }
            connection.connect();
            connection.getContentType();

            // Response code for success should be 2xx
            httpResponseCode = connection.getResponseCode();
            if (httpResponseCode > 299)
                return httpResponseCode.toString();

            if (debugMsg) {
                System.out.println("HTTP response code: " + connection.getResponseCode());
                System.out.println("HTTP response message: " + connection.getResponseMessage());
            }

            InputStream is = connection.getInputStream();
            BufferedReader rd = new BufferedReader(new InputStreamReader(is, Charset.forName("UTF-8")));
            StringBuilder sb = new StringBuilder();
            int cp;
            while ((cp = rd.read()) != -1) {
                sb.append((char) cp);
            }
            is.close();
            connection.disconnect();
            return sb.toString();
        } catch (Exception e) {
            return null;
        }
    }

    // a class to construct query parameter for HTTP request
    private class QueryParameter {
        StringBuilder queryString = null;

        // constructor
        QueryParameter(String key, String value) {
            queryString = new StringBuilder();
            queryString.append("?").append(key).append("=").append(value);
        }

        // method to get the query parameter string
        String getString() {
            return this.queryString.toString();
        }
    }

    @Test
    public void testVTNAPI() throws JSONException {
        System.out.println("Starting VTN JAXB client.");
        String baseURL = "http://127.0.0.1:8080/controller/nb/v2/vtn/";

        String tname1 = "testVtn1";
        String tname2 = "testVtn2";
        String tname3 = "testVtn3";
        String desc1 = "testDiscription1";
        String desc2 = "testDiscription2";
        String itimeout1 = "100";
        String itimeout2 = "200";
        String itimeout3 = "65540";
        String htimeout1 = "200";
        String htimeout2 = "400";
        String htimeout3 = "-10";

        // Test GET vtn in default container, expecting no results
        String result = getJsonResult(baseURL + "default/vtns");
        Assert.assertEquals(200, httpResponseCode.intValue());
        Assert.assertEquals("{}", result);

        // Test POST vtn1
        String requestBody = "{}";
        result = getJsonResult(baseURL + "default/vtns/" + tname1, "POST", requestBody);
        Assert.assertEquals(201, httpResponseCode.intValue());

        // Test POST vtn2
        requestBody = "{\"@description\":\"" + desc1 + "\", \"@idleTimeout\":\"" + itimeout1 + "\", \"@hardTimeout\":\""
                + htimeout1 + "\"}";
        result = getJsonResult(baseURL + "default/vtns/" + tname2, "POST", requestBody);
        Assert.assertEquals(201, httpResponseCode.intValue());

        // Test GET all vtns in default container
        result = getJsonResult(baseURL + "default/vtns");
        JSONTokener jt = new JSONTokener(result);
        JSONObject json = new JSONObject(jt);
        JSONArray vtnArray = json.getJSONArray("vtn");
        JSONObject vtn;

        Assert.assertEquals(200, httpResponseCode.intValue());
        Assert.assertEquals(2, vtnArray.length());

        for (int i = 0; i < vtnArray.length(); i++) {
            vtn = vtnArray.getJSONObject(i);
            if (vtn.getString("@name").equals(tname1)) {
                try {
                    Assert.assertEquals("", vtn.getString("@description"));
                } catch (JSONException expected) {
                    assertThat(expected.getMessage(), is("JSONObject[\"@description\"] not found."));
                }
                Assert.assertEquals("300", vtn.getString("@idleTimeout"));
                Assert.assertEquals("0", vtn.getString("@hardTimeout"));
            } else if (vtn.getString("@name").equals(tname2)) {
                Assert.assertEquals(desc1, vtn.getString("@description"));
                Assert.assertEquals(itimeout1, vtn.getString("@idleTimeout"));
                Assert.assertEquals(htimeout1, vtn.getString("@hardTimeout"));
            } else {
                // Unexpected vtn name
                Assert.assertTrue(false);
            }
        }

        testVBridgeAPI(baseURL, tname1, tname2);

        // Test GET vtn expecting 404
        result = getJsonResult(baseURL + "default/vtns/" + tname3);
        Assert.assertEquals(404, httpResponseCode.intValue());

        // Test GET vtn
        result = getJsonResult(baseURL + "default/vtns/" + tname2);
        jt = new JSONTokener(result);
        json = new JSONObject(jt);

        Assert.assertEquals(200, httpResponseCode.intValue());
        Assert.assertEquals(tname2, json.getString("@name"));
        Assert.assertEquals(desc1, json.getString("@description"));
        Assert.assertEquals(itimeout1, json.getString("@idleTimeout"));
        Assert.assertEquals(htimeout1, json.getString("@hardTimeout"));

        // Test PUT vtn1
        requestBody = "{\"@description\":\"" + desc2 + "\", \"@idleTimeout\":\"" + itimeout2 + "\", \"@hardTimeout\":\""
                + htimeout2 + "\"}";
        String queryParameter = new QueryParameter("all", "true").getString();

        result = getJsonResult(baseURL + "default/vtns/" + tname1 + queryParameter, "PUT", requestBody);
        Assert.assertEquals(200, httpResponseCode.intValue());

        // Test PUT vtn2 expecting not change
        queryParameter = new QueryParameter("all", "false").getString();
        result = getJsonResult(baseURL + "default/vtns/" + tname2 + queryParameter, "PUT", requestBody);
        Assert.assertEquals(200, httpResponseCode.intValue());

        // Test PUT vtn expecting 404
        queryParameter = new QueryParameter("all", "true").getString();
        result = getJsonResult(baseURL + "default/vtns/" + tname3 + queryParameter, "PUT", requestBody);
        Assert.assertEquals(404, httpResponseCode.intValue());

        // Test PUT vtn expecting 415
        requestBody = "{\"@description\":\"" + desc2 + "\", \"@idleTimeout\":\"" + itimeout3 + "\", \"@hardTimeout\":\""
                + htimeout2 + "\"}";
        result = getJsonResult(baseURL + "default/vtns/" + tname1 + queryParameter, "PUT", requestBody);
        Assert.assertEquals(415, httpResponseCode.intValue());

        // Test PUT vtn
        requestBody = "{\"@description\":\"" + desc2 + "\", \"@idleTimeout\":\"" + itimeout2 + "\", \"@hardTimeout\":\""
                + htimeout3 + "\"}";
        result = getJsonResult(baseURL + "default/vtns/" + tname1 + queryParameter, "PUT", requestBody);
        Assert.assertEquals(200, httpResponseCode.intValue());

        // Test DELETE vtn
        result = getJsonResult(baseURL + "default/vtns/" + tname1, "DELETE");
        Assert.assertEquals(200, httpResponseCode.intValue());

        // Test DELETE vtn2
        result = getJsonResult(baseURL + "default/vtns/" + tname2, "DELETE");
        Assert.assertEquals(200, httpResponseCode.intValue());

        // Test DELETE vtn expecting 404
        result = getJsonResult(baseURL + "default/vtns/" + tname1, "DELETE");
        Assert.assertEquals(404, httpResponseCode.intValue());

        testVtnGlobal(baseURL);
    }

    private void testVBridgeAPI(String url, String tname1, String tname2) throws JSONException {
        System.out.println("Starting vBridge JAXB client.");
        StringBuilder baseURL = new StringBuilder();
        baseURL.append(url);
        baseURL.append("default/vtns/");

        String tname3 = "testVtn3";
        String desc1 = "testDiscription1";
        String desc2 = "testDiscription2";
        String bname1 = "vbridgeName1";
        String bname2 = "vbridgeName2";
        String ageinter1 = "10";
        String ageinter2 = "100";
        String ageinter3 = "0";
        String fault = "";

        // Test GET vBridges in default container, expecting no results
        String result = getJsonResult(baseURL + tname1 + "/vbridges");
        Assert.assertEquals(200, httpResponseCode.intValue());
        Assert.assertEquals("{}", result);

        // Test POST vBridge1 expecting 404
        String requestBody = "{}";
        result = getJsonResult(baseURL + tname3 + "/vbridges/" + bname1, "POST" , requestBody);
        Assert.assertEquals(404, httpResponseCode.intValue());

        // Test POST vBridge1 expecting 415
        requestBody = "{\"@description\":\"" + desc1 + "\", \"@ageInterval\":\"" + ageinter3 + "\"}";
        result = getJsonResult(baseURL + tname1 + "/vbridges/" + bname1 , "POST", requestBody);
        Assert.assertEquals(415, httpResponseCode.intValue());

        // Test POST vBridge1
        requestBody = "{}";
        result = getJsonResult(baseURL + tname1 + "/vbridges/" + bname1, "POST", requestBody);
        Assert.assertEquals(201, httpResponseCode.intValue());

        // Test POST vBridge1 expecting 409
        requestBody = "{}";
        result = getJsonResult(baseURL + tname1 + "/vbridges/" + bname1, "POST", requestBody);
        Assert.assertEquals(409, httpResponseCode.intValue());

        // Test POST vBridge2
        requestBody = "{\"@description\":\"" + desc2 + "\", \"@ageInterval\":\"" + ageinter1 + "\"}";
        result = getJsonResult(baseURL + tname1 + "/vbridges/" + bname2 , "POST", requestBody);
        Assert.assertEquals(201, httpResponseCode.intValue());

        // Test GET all vBridges in default container
        result = getJsonResult(baseURL + tname1 + "/vbridges");
        JSONTokener jt = new JSONTokener(result);
        JSONObject json = new JSONObject(jt);
        JSONArray vBridgeArray = json.getJSONArray("vbridge");
        JSONObject vBridge;

        Assert.assertEquals(200, httpResponseCode.intValue());
        Assert.assertEquals(2, vBridgeArray.length());

        for (int i = 0; i < vBridgeArray.length(); i++) {
            vBridge = vBridgeArray.getJSONObject(i);
            if (vBridge.getString("@name").equals(bname1)) {

                try {
                    Assert.assertEquals("", vBridge.getString("@description"));
                } catch (JSONException expected) {
                    assertThat(expected.getMessage(), is("JSONObject[\"@description\"] not found."));
                }

                try {
                    Assert.assertEquals("600", json.getString("@ageInterval"));
                } catch (JSONException expected) {
                    assertThat(expected.getMessage(), is("JSONObject[\"@ageInterval\"] not found."));
                }

            } else if (vBridge.getString("@name").equals(bname2)) {
                Assert.assertEquals(desc2, vBridge.getString("@description"));
                Assert.assertEquals(ageinter1, vBridge.getString("@ageInterval"));
                fault = vBridge.getString("@faults");
            } else {
                // Unexpected vBridge name
                Assert.assertTrue(false);
            }
        }

        testVLANMappingAPI(baseURL, tname1, bname1);
        testVBridgeInterfaceAPI(baseURL, tname1, bname1, bname2);
        testMacAddressAPI(baseURL, tname1, bname1);

        // Test GET vBridge expecting 404
        result = getJsonResult(baseURL + tname3 + "/vbridges/" + bname1);
        Assert.assertEquals(404, httpResponseCode.intValue());

        // Test GET vBridge
        result = getJsonResult(baseURL + tname1 + "/vbridges/" + bname2);
        jt = new JSONTokener(result);
        json = new JSONObject(jt);

        Assert.assertEquals(200, httpResponseCode.intValue());
        Assert.assertEquals(bname2, json.getString("@name"));
        Assert.assertEquals(desc2, json.getString("@description"));
        Assert.assertEquals(ageinter1, json.getString("@ageInterval"));
        Assert.assertEquals(fault, json.getString("@faults"));
        Assert.assertEquals("0", json.getString("@state"));

        // Test PUT vBridge1
        requestBody = "{\"@description\":\"" + desc2 + "\", \"@ageInterval\":\"" + ageinter2 + "\"}";
        String queryParameter = new QueryParameter("all", "true").getString();
        result = getJsonResult(baseURL + tname1 + "/vbridges/" + bname1 + queryParameter, "PUT", requestBody);
        Assert.assertEquals(200, httpResponseCode.intValue());

        // Test PUT vBridge1
        requestBody = "{}";
        result = getJsonResult(baseURL + tname1 + "/vbridges/" + bname1 + queryParameter, "PUT", requestBody);
        Assert.assertEquals(200, httpResponseCode.intValue());

        // Test PUT vBridge2 expecting not change
        queryParameter = new QueryParameter("all", "false").getString();
        result = getJsonResult(baseURL + tname1 + "/vbridges/" + bname2 + queryParameter, "PUT", requestBody);
        Assert.assertEquals(200, httpResponseCode.intValue());

        // Test PUT vBridge1 expecting 404
        queryParameter = new QueryParameter("all", "true").getString();
        result = getJsonResult(baseURL +  tname3 + "/vbridges/" + bname1 + queryParameter, "PUT", requestBody);
        Assert.assertEquals(404, httpResponseCode.intValue());

        // Test PUT vBridge1 expecting 415
        requestBody = "{\"@description\":\"" + desc2 + "\", \"@ageInterval\":\"" + ageinter3 + "\"}";
        result = getJsonResult(baseURL + tname1 + "/vbridges/" + bname1 + queryParameter, "PUT", requestBody);
        Assert.assertEquals(415, httpResponseCode.intValue());

        testVLANMappingDeleteAPI(baseURL, tname1, bname1);
        testVBridgeInterfaceDeleteAPI(baseURL, tname1, bname1);

        // Test DELETE vBridge
        result = getJsonResult(baseURL + tname1 + "/vbridges/" + bname1, "DELETE");
        Assert.assertEquals(200, httpResponseCode.intValue());

        // Test DELETE vBridge2
        result = getJsonResult(baseURL + tname1 + "/vbridges/" + bname2, "DELETE");
        Assert.assertEquals(200, httpResponseCode.intValue());

        // Test DELETE vBridge expecting 404
        result = getJsonResult(baseURL + tname1 + "/vbridges/" + bname1, "DELETE");
        Assert.assertEquals(404, httpResponseCode.intValue());

    }

    private void testVBridgeInterfaceAPI(StringBuilder url, String tname1, String bname1, String bname2) throws JSONException {
        System.out.println("Starting vBridge Intergace JAXB client.");
        StringBuilder baseURL = new StringBuilder();
        baseURL.append(url);
        baseURL.append(tname1);
        baseURL.append("/vbridges/");

        String ifname1 = "testInterface1";
        String ifname2 = "testInterface2";
        String ifname3 = "testInterface3";
        String bname_dammy = "testdammy";
        String desc1 = "testDiscription1";
        String desc2 = "testDiscription2";

        // Test GET vBridge Interfaces in default container, expecting no results
        String result = getJsonResult(baseURL + bname1 + "/interfaces");
        Assert.assertEquals(200, httpResponseCode.intValue());
        Assert.assertEquals("{}", result);

        // Test POST vBridge Interface1 expecting 404
        String requestBody = "{}";
        result = getJsonResult(baseURL + bname_dammy + "/interfaces/" + ifname1, "POST" , requestBody);
        Assert.assertEquals(404, httpResponseCode.intValue());

        // Test POST vBridge Interface1 expecting 415
//        requestBody = "{\"@description\":\"" + desc1 + "\", \"@enabled\":\"enaled\"}";
//        result = getJsonResult(baseURL + bname1 + "/interfaces/" + ifname1, "POST", requestBody);
//        Assert.assertEquals(415, httpResponseCode.intValue());

        // Test POST vBridge Interface1
        requestBody = "{}";
        result = getJsonResult(baseURL + bname1 + "/interfaces/" + ifname1, "POST", requestBody);
        Assert.assertEquals(201, httpResponseCode.intValue());

        // Test POST vBridge Interface1 expecting 409
        result = getJsonResult(baseURL + bname1 + "/interfaces/" + ifname1, "POST", requestBody);
        Assert.assertEquals(409, httpResponseCode.intValue());

        // Test POST vBridge Interface2
        requestBody = "{\"@description\":\"" + desc1 + "\", \"@ageInterval\":true}";
        result = getJsonResult(baseURL + bname1 + "/interfaces/" + ifname2, "POST", requestBody);
        Assert.assertEquals(201, httpResponseCode.intValue());

        // Test POST vBridge Interface3
        requestBody = "{\"@description\":\"" + desc2 + "\", \"@ageInterval\":true}";
        result = getJsonResult(baseURL + bname2 + "/interfaces/" + ifname3, "POST", requestBody);
        Assert.assertEquals(201, httpResponseCode.intValue());

        testPortMappingAPI(baseURL, bname1, bname2, ifname2, ifname3);

        // Test GET all vBridge Interfaces in default container
        result = getJsonResult(baseURL + bname1 + "/interfaces");
        JSONTokener jt = new JSONTokener(result);
        JSONObject json = new JSONObject(jt);
        JSONArray vbridgeifArray = json.getJSONArray("interface");
        JSONObject vbridgeif;

        Assert.assertEquals(200, httpResponseCode.intValue());
        Assert.assertEquals(2, vbridgeifArray.length());

        for (int i = 0; i < vbridgeifArray.length(); i++) {
            vbridgeif = vbridgeifArray.getJSONObject(i);
            if (vbridgeif.getString("@name").equals(ifname1)) {
                try {
                    Assert.assertEquals(desc1, vbridgeif.getString("@description"));
                } catch (JSONException expected) {
                    assertThat(expected.getMessage(), is("JSONObject[\"@description\"] not found."));
                }

                try {
                    Assert.assertEquals("true", vbridgeif.getString("@enabled"));
                } catch (JSONException expected) {
                    assertThat(expected.getMessage(), is("JSONObject[\"@enabled\"] not found."));
                }
                Assert.assertEquals("-1", vbridgeif.getString("@state"));
                Assert.assertEquals("-1", vbridgeif.getString("@entityState"));
            } else if (vbridgeif.getString("@name").equals(ifname2)) {
                Assert.assertEquals(desc1, vbridgeif.getString("@description"));
                Assert.assertEquals("true", vbridgeif.getString("@enabled"));
                Assert.assertEquals("0", vbridgeif.getString("@state"));
                Assert.assertEquals("-1", vbridgeif.getString("@entityState"));
            } else if (vbridgeif.getString("@name").equals(ifname3)) {
                Assert.assertEquals(desc2, vbridgeif.getString("@description"));
                Assert.assertEquals("true", vbridgeif.getString("@enabled"));
                Assert.assertEquals("-1", vbridgeif.getString("@state"));
                Assert.assertEquals("-1", vbridgeif.getString("@entityState"));
            }
            else {
                // Unexpected vBridge Interface name
                Assert.assertTrue(false);
            }
        }

        // Test GET vBridge Interface expecting 404
        result = getJsonResult(baseURL + bname_dammy + "/interfaces/" + ifname1);
        Assert.assertEquals(404, httpResponseCode.intValue());

        // Test GET vBridge Interface2
        result = getJsonResult(baseURL + bname1 + "/interfaces/" + ifname2);
        jt = new JSONTokener(result);
        json = new JSONObject(jt);

        Assert.assertEquals(200, httpResponseCode.intValue());
        Assert.assertEquals(ifname2, json.getString("@name"));
        Assert.assertEquals(desc1, json.getString("@description"));
        Assert.assertEquals("true", json.getString("@enabled"));
        Assert.assertEquals("0", json.getString("@state"));
        Assert.assertEquals("-1", json.getString("@entityState"));


        // Test PUT vBridge interface1
        requestBody = "{\"@description\":\"" + desc1 + "\", \"@enabled\":\"ture\"}";
        String queryParameter = new QueryParameter("all", "true").getString();
        result = getJsonResult(baseURL + bname1 + "/interfaces/" + ifname1 + queryParameter, "PUT", requestBody);
        Assert.assertEquals(200, httpResponseCode.intValue());

        // Test PUT vBridge interface1
        requestBody = "{}";
        result = getJsonResult(baseURL + bname1 + "/interfaces/" + ifname1 + queryParameter, "PUT", requestBody);
        Assert.assertEquals(200, httpResponseCode.intValue());

        // Test PUT vBridge Interface2 expecting not change
        requestBody = "{\"@description\":\"" + desc2 + "\", \"@enabled\":\"false\"}";
        queryParameter = new QueryParameter("all", "false").getString();
        result = getJsonResult(baseURL + bname1 +"/interfaces/" + ifname2 + queryParameter, "PUT", requestBody);
        Assert.assertEquals(200, httpResponseCode.intValue());

        // Test PUT vBridge Interface1 expecting 404
        queryParameter = new QueryParameter("all", "true").getString();
        result = getJsonResult(baseURL +  bname_dammy + "/interfaces/"
                + ifname1 + queryParameter, "PUT", requestBody);
        Assert.assertEquals(404, httpResponseCode.intValue());

        // Test PUT vBridge Interface1 expecting 415
        requestBody = "{\"@description\":\"" + desc2 + "\", \"@enabled\":\"-\"}";
        //result = getJsonResult(baseURL + bname1 + "/interfaces/" + ifname1 +queryParameter, "PUT", requestBody);
        //Assert.assertEquals(415, httpResponseCode.intValue());
    }

    private void testVBridgeInterfaceDeleteAPI(StringBuilder url, String tname, String bname) throws JSONException {
        System.out.println("Starting delete vBridge Intergace JAXB client.");
        StringBuilder baseURL = new StringBuilder();
        baseURL.append(url);
        baseURL.append(tname);
        baseURL.append("/vbridges/");

        String ifname1 = "testInterface1";
        String ifname2 = "testInterface2";

        testPortMappingDeleteAPI(baseURL, bname, ifname1);

        // Test DELETE vBridge Interface1
        String result = getJsonResult(baseURL + bname + "/interfaces/" + ifname1, "DELETE");
        Assert.assertEquals(200, httpResponseCode.intValue());

        // Test DELETE vBridge Interface2
        result = getJsonResult(baseURL + bname + "/interfaces/" + ifname2, "DELETE");
        Assert.assertEquals(200, httpResponseCode.intValue());

        // Test DELETE vBridge Interface1 expecting 404
        result = getJsonResult(baseURL + bname + "/interfaces/" + ifname1, "DELETE");
        Assert.assertEquals(404, httpResponseCode.intValue());
    }

    private void testPortMappingAPI(StringBuilder url, String bname, String bname2, String ifname, String ifname2) throws JSONException {
        System.out.println("Starting Port Mapping JAXB client.");
        StringBuilder baseURL = new StringBuilder();
        baseURL.append(url);
        baseURL.append(bname);
        baseURL.append("/interfaces/");

        String ifname_dammy = "testifname_dammy";
        String vlan1 = "100";
        String vlan2 = "4096";
        String vlan3 = "2000";
        String pname = "testPortname";

        String nodeid = "00:00:00:00:00:00:00:01";
        String nodeType = "OF";
        String portnum = "1";

        // Test GET PortMapping in default container, expecting no results
        String result = getJsonResult(baseURL + ifname + "/portmap");
        Assert.assertEquals(204, httpResponseCode.intValue());
        Assert.assertEquals("", result);

        // Test PUT PortMapping expecting 400
        String requestBody = "{\"@description\":\", \"@enabled\":\"ture\"}";
        result = getJsonResult(baseURL + ifname + "/portmap/", "PUT", requestBody);
        Assert.assertEquals(400, httpResponseCode.intValue());

        // Test PUT PortMapping expecting 415
        requestBody = "{\"@vlan\":" + vlan2 + ", \"node\":{\"type\":\""+ nodeType + "\", \"id\":\""
                + nodeid + "\"}, \"port\":{\"@name\":\"" + pname
                + "\", \"@type\":\"" + nodeType + "\", \"@id\":\"" + portnum + "\"}}";
        result = getJsonResult(baseURL + ifname + "/portmap/", "PUT", requestBody);
        Assert.assertEquals(415, httpResponseCode.intValue());

        // Test PUT PortMapping expecting 404
        requestBody = "{\"@vlan\":" + vlan1 + ", \"node\":{\"type\":\"" + nodeType + "\", \"id\":\""
                + nodeid + "\"}, \"port\":{\"@name\":\"" + pname
                + "\", \"@type\":\"" + nodeType + "\", \"@id\":\"" + portnum + "\"}}";
        result = getJsonResult(baseURL + ifname_dammy + "/portmap/", "PUT", requestBody);
        Assert.assertEquals(404, httpResponseCode.intValue());

        // Test PUT PortMapping
        result = getJsonResult(baseURL + ifname + "/portmap/", "PUT", requestBody);
        Assert.assertEquals(200, httpResponseCode.intValue());

        // Test PUT PortMapping
        requestBody = "{\"@vlan\":" + vlan3 + ", \"node\":{\"type\":\"" + nodeType + "\", \"id\":\""
                + nodeid + "\"}, \"port\":{\"@name\":\"" + pname
                + "\", \"@type\":\"" + nodeType + "\", \"@id\":\"" + portnum + "\"}}";
        result = getJsonResult(url + bname2 + "/interfaces/" + ifname2 + "/portmap/", "PUT", requestBody);
        Assert.assertEquals(200, httpResponseCode.intValue());

        // Test GET PortMapping expecting 404
        result = getJsonResult(baseURL + ifname_dammy + "/portmap");
        Assert.assertEquals(404, httpResponseCode.intValue());

        // Test GET PortMapping
        result = getJsonResult(baseURL + ifname + "/portmap");
        Assert.assertEquals(200, httpResponseCode.intValue());
        JSONTokener jt = new JSONTokener(result);
        JSONObject json = new JSONObject(jt);
        JSONObject nodeinfo = json.getJSONObject("node");
        JSONObject portinfo = json.getJSONObject("port");


        Assert.assertEquals(vlan1, json.getString("@vlan"));
        Assert.assertEquals(nodeType, nodeinfo.getString("type"));
        Assert.assertEquals(nodeid, nodeinfo.getString("id"));
        Assert.assertEquals(pname, portinfo.getString("@name"));
        Assert.assertEquals(nodeType, portinfo.getString("@type"));
        Assert.assertEquals(portnum, portinfo.getString("@id"));

        if (!portinfo.getString("@type").equals(nodeType)) {
            JSONObject mapinfo = json.getJSONObject("mapped");
            Assert.assertEquals(nodeType, mapinfo.getString("@type"));
            Assert.assertEquals(portnum, mapinfo.getString("@id"));
        }
    }

    private void testPortMappingDeleteAPI(StringBuilder url, String bname, String ifname) throws JSONException {
        System.out.println("Starting delete Port Mapping JAXB client.");
        StringBuilder baseURL = new StringBuilder();
        baseURL.append(url);
        baseURL.append(bname);
        baseURL.append("/interfaces/");

        String ifname_dammy = "ifname_dammy";

        // Test DELETE PortMapping
        String result = getJsonResult(baseURL + ifname + "/portmap", "DELETE");
        Assert.assertEquals(200, httpResponseCode.intValue());

        // Test DELETE PortMapping expecting 404
        result = getJsonResult(baseURL + ifname_dammy + "/portmap", "DELETE");
        Assert.assertEquals(404, httpResponseCode.intValue());
    }

    private void testVLANMappingAPI(StringBuilder url, String tname, String bname) throws JSONException {
        System.out.println("Starting VLAN Mapping JAXB client.");
        StringBuilder baseURL = new StringBuilder();
        baseURL.append(url);
        baseURL.append(tname);
        baseURL.append("/vbridges/");

        String bname_dammy = "testVBridge_dammy";
        String nodeid1 = "00:00:00:00:00:00:00:01";
        String nodeid2 = "00:00:00:00:00:00:00:02";
        String vlan1 = "1000";
        String vlan2 = "2000";
        String vlan3 = "4097";
        String nodeType = "OF";

        // Test GET VLAN Mapping
        String result = getJsonResult(baseURL + bname + "/vlanmaps");
        Assert.assertEquals(200, httpResponseCode.intValue());
        Assert.assertEquals("{}", result);

        // Test POST VLAN Mapping expecting 404
        String requestBody = "{\"@vlan\":\"" + vlan1 +"\",\"node\":{\"type\":\""+ nodeType +"\",\"id\":\""
                + nodeid1 + "\"}}";
        result = getJsonResult(baseURL + bname_dammy + "/vlanmaps", "POST", requestBody);
        Assert.assertEquals(404, httpResponseCode.intValue());

        // Test POST VLAN Mapping expecting 415
        requestBody = "{\"@vlan\":\"" + vlan3 +"\",\"node\":{\"type\":\""+ nodeType +"\",\"id\":\""
                + nodeid1 + "\"}}";
        result = getJsonResult(baseURL + bname + "/vlanmaps", "POST", requestBody);
        Assert.assertEquals(415, httpResponseCode.intValue());

        // Test POST VLAN Mapping
        requestBody = "{\"@vlan\":\"" + vlan1 +"\",\"node\":{\"type\":\""+ nodeType +"\",\"id\":\""
                + nodeid1 + "\"}}";
        result = getJsonResult(baseURL + bname + "/vlanmaps", "POST", requestBody);
        Assert.assertEquals(201, httpResponseCode.intValue());

        // Test POST VLAN Mapping expecting 409
        requestBody = "{\"@vlan\":\"" + vlan1 +"\",\"node\":{\"type\":\""+ nodeType +"\",\"id\":\""
                + nodeid1 + "\"}}";
        result = getJsonResult(baseURL + bname + "/vlanmaps", "POST", requestBody);
        Assert.assertEquals(409, httpResponseCode.intValue());

        // Test POST VLAN Mapping
        requestBody = "{\"@vlan\":\"" + vlan2 +"\",\"node\":{\"type\":\""+ nodeType +"\",\"id\":\""
                + nodeid2 + "\"}}";
        result = getJsonResult(baseURL + bname + "/vlanmaps", "POST", requestBody);
        Assert.assertEquals(201, httpResponseCode.intValue());

        // Test GET all  VLAN Mapping in default container
        result = getJsonResult(baseURL + bname + "/vlanmaps");
        JSONTokener jt = new JSONTokener(result);
        JSONObject json = new JSONObject(jt);
        JSONArray vLANMapArray = json.getJSONArray("vlanmap");
        JSONObject vLANMap;

        Assert.assertEquals(200, httpResponseCode.intValue());
        Assert.assertEquals(2, vLANMapArray.length());

        for (int i = 0; i < vLANMapArray.length(); i++) {
            vLANMap = vLANMapArray.getJSONObject(i);
            JSONObject nodeinfo = vLANMap.getJSONObject("node");
            if (vLANMap.getString("@id").equals(nodeType + "-" + nodeid1 + "." + vlan1)) {
                Assert.assertEquals(vlan1, vLANMap.getString("@vlan"));
                Assert.assertEquals(nodeType, nodeinfo.getString("type"));
                Assert.assertEquals(nodeid1, nodeinfo.getString("id"));

            } else if (vLANMap.getString("@id").equals(nodeType + "-" + nodeid2 + "." + vlan2)) {
                Assert.assertEquals(vlan2, vLANMap.getString("@vlan"));
                Assert.assertEquals(nodeType, nodeinfo.getString("type"));
                Assert.assertEquals(nodeid2, nodeinfo.getString("id"));
            }
            else {
                // Unexpected VLAN Mapping
                Assert.assertTrue(false);
            }
        }

        // Test GET VLAN Mapping expecting 404
        result = getJsonResult(baseURL + bname_dammy + "/vlanmaps/" + nodeType + "-" + nodeid1 + "." + vlan1);
        Assert.assertEquals(404, httpResponseCode.intValue());

        // Test GET VLAN Mapping
        result = getJsonResult(baseURL + bname + "/vlanmaps/" + nodeType + "-" + nodeid1 + "." + vlan1);
        jt = new JSONTokener(result);
        json = new JSONObject(jt);
        Assert.assertEquals(200, httpResponseCode.intValue());

        Assert.assertEquals(nodeType + "-" + nodeid1 + "." + vlan1, json.getString("@id"));
        Assert.assertEquals(vlan1, json.getString("@vlan"));
        JSONObject nodeinfo = json.getJSONObject("node");
        Assert.assertEquals(nodeType, nodeinfo.getString("type"));
        Assert.assertEquals(nodeid1, nodeinfo.getString("id"));
    }

    private void testVLANMappingDeleteAPI(StringBuilder url, String tname, String bname) throws JSONException {
        System.out.println("Starting delete Port Mapping JAXB client.");
        StringBuilder baseURL = new StringBuilder();
        baseURL.append(url);
        baseURL.append(tname);
        baseURL.append("/vbridges/");

        String nodeid1 = "00:00:00:00:00:00:00:01";
        String vlan1 = "1000";
        String nodeType = "OF";

        String bname_dammy = "bname_dammy";

        // Test DELETE PortMapping
        String result = getJsonResult(baseURL + bname + "/vlanmaps/" + nodeType + "-" + nodeid1 + "." + vlan1, "DELETE");
        Assert.assertEquals(200, httpResponseCode.intValue());

        // Test DELETE PortMapping expecting 404
        result = getJsonResult(baseURL + bname_dammy + "/vlanmaps/" + nodeType + "-" + nodeid1 + "." + vlan1, "DELETE");
        Assert.assertEquals(404, httpResponseCode.intValue());
    }

    private void testMacAddressAPI(StringBuilder url, String tname, String bname) throws JSONException {
        System.out.println("Starting MAC address JAXB client.");
        StringBuilder baseURL = new StringBuilder();
        baseURL.append(url);
        baseURL.append(tname);
        baseURL.append("/vbridges/");

        String dammy = "dammy";
        String macaddr = "00:00:00:00:00:01";

        // Test GET all MAC address
        String result = getJsonResult(baseURL + bname + "/mac");
        Assert.assertEquals(200, httpResponseCode.intValue());
        Assert.assertEquals("{}", result);

        // Test DELETE all MAC address
        result = getJsonResult(baseURL + bname + "/mac", "DELETE");
        Assert.assertEquals(200, httpResponseCode.intValue());

        // Test DELETE all MAC address expecting 404
        result = getJsonResult(baseURL + dammy + "/mac", "DELETE");
        Assert.assertEquals(404, httpResponseCode.intValue());

        // Test GET MAC address expecting 404
        result = getJsonResult(baseURL + bname + "/mac/" + macaddr);
        Assert.assertEquals(404, httpResponseCode.intValue());

        // Test DELETE MAC address expecting 404
        result = getJsonResult(baseURL + bname + "/mac/" + macaddr, "DELETE");
        Assert.assertEquals(404, httpResponseCode.intValue());

    }

    private void testVtnGlobal(String base) throws JSONException {
        System.out.println("Starting IVTNGlobal JAXB client.");

        // Get version information of the VTN Manager.
        String result = getJsonResult(base + "version");
        Assert.assertEquals(200, httpResponseCode.intValue());

        JSONTokener jt = new JSONTokener(result);
        JSONObject json = new JSONObject(jt);
        int api = json.getInt("@api");
        JSONObject bv = json.getJSONObject("bundle");
        Assert.assertTrue("API version: " + api, api > 0);

        // Determine actual bundle version of manager.implementation.
        Assert.assertNotNull(implBundle);
        Version ver = implBundle.getVersion();
        Assert.assertNotNull(ver);

        Assert.assertEquals(ver.getMajor(), bv.getInt("@major"));
        Assert.assertEquals(ver.getMinor(), bv.getInt("@minor"));
        Assert.assertEquals(ver.getMicro(), bv.getInt("@micro"));

        String qf = ver.getQualifier();
        String qfkey  ="@qualifier";
        if (qf == null || qf.isEmpty()) {
            Assert.assertTrue(bv.isNull(qfkey));
        } else {
            Assert.assertEquals(qf, bv.getString(qfkey));
        }
    }

    // Configure the OSGi container
    @Configuration
    public Option[] config() {
        return options(
                //
                systemProperty("logback.configurationFile").value(
                        "file:" + PathUtils.getBaseDir() + "/src/test/resources/logback.xml"),
                // To start OSGi console for inspection remotely
                systemProperty("osgi.console").value("2401"),
                systemProperty("org.eclipse.gemini.web.tomcat.config.path").value(
                        PathUtils.getBaseDir() + "/src/test/resources/tomcat-server.xml"),

                // setting default level. Jersey bundles will need to be started
                // earlier.
                systemProperty("osgi.bundles.defaultStartLevel").value("4"),

                // Set the systemPackages (used by clustering)
                systemPackages("sun.reflect", "sun.reflect.misc", "sun.misc"),

                mavenBundle("javax.servlet", "servlet-api").versionAsInProject(),

                mavenBundle("org.slf4j", "jcl-over-slf4j").versionAsInProject(),
                mavenBundle("org.slf4j", "slf4j-api").versionAsInProject(),
                mavenBundle("org.slf4j", "log4j-over-slf4j").versionAsInProject(),
                mavenBundle("ch.qos.logback", "logback-core").versionAsInProject(),
                mavenBundle("ch.qos.logback", "logback-classic").versionAsInProject(),
                mavenBundle("org.apache.commons", "commons-lang3").versionAsInProject(),
                mavenBundle("org.apache.felix", "org.apache.felix.dependencymanager").versionAsInProject(),

                // the plugin stub to get data for the tests
                mavenBundle("org.opendaylight.controller", "protocol_plugins.stub").versionAsInProject(),

                // List all the opendaylight modules
                mavenBundle("org.opendaylight.controller", "configuration").versionAsInProject(),
                mavenBundle("org.opendaylight.controller", "configuration.implementation").versionAsInProject(),
                mavenBundle("org.opendaylight.controller", "containermanager").versionAsInProject(),
                mavenBundle("org.opendaylight.controller", "containermanager.it.implementation").versionAsInProject(),
                mavenBundle("org.opendaylight.controller", "clustering.services").versionAsInProject(),
                mavenBundle("org.opendaylight.controller", "clustering.services-implementation").versionAsInProject(),
                mavenBundle("org.opendaylight.controller", "security").versionAsInProject().noStart(),
                mavenBundle("org.opendaylight.controller", "sal").versionAsInProject(),
                mavenBundle("org.opendaylight.controller", "sal.implementation").versionAsInProject(),
                mavenBundle("org.opendaylight.controller", "sal.connection").versionAsInProject(),
                mavenBundle("org.opendaylight.controller", "sal.connection.implementation").versionAsInProject(),
                mavenBundle("org.opendaylight.controller", "sal.networkconfiguration").versionAsInProject(),
                mavenBundle("org.opendaylight.controller", "sal.networkconfiguration.implementation").versionAsInProject(),
                mavenBundle("org.opendaylight.controller", "connectionmanager").versionAsInProject(),
                mavenBundle("org.opendaylight.controller", "connectionmanager.implementation").versionAsInProject(),

                mavenBundle("org.opendaylight.controller", "switchmanager").versionAsInProject(),
                mavenBundle("org.opendaylight.controller", "switchmanager.implementation").versionAsInProject(),
                mavenBundle("org.opendaylight.controller", "forwardingrulesmanager").versionAsInProject(),
                mavenBundle("org.opendaylight.controller", "forwardingrulesmanager.implementation").versionAsInProject(),
                mavenBundle("org.opendaylight.controller", "statisticsmanager").versionAsInProject(),
                mavenBundle("org.opendaylight.controller", "statisticsmanager.implementation").versionAsInProject(),
                mavenBundle("org.opendaylight.controller", "hosttracker").versionAsInProject(),
                mavenBundle("org.opendaylight.controller", "hosttracker.implementation").versionAsInProject(),
                //mavenBundle("org.opendaylight.controller", "arphandler").versionAsInProject(),
                mavenBundle("org.opendaylight.controller", "routing.dijkstra_implementation").versionAsInProject(),
                mavenBundle("org.opendaylight.controller", "topologymanager").versionAsInProject(),
                mavenBundle("org.opendaylight.controller", "usermanager").versionAsInProject(),
                mavenBundle("org.opendaylight.controller", "usermanager.implementation").versionAsInProject(),

                mavenBundle("org.opendaylight.controller", "clustering.test").versionAsInProject(),
                mavenBundle("org.opendaylight.controller", "forwarding.staticrouting").versionAsInProject(),
                mavenBundle("org.opendaylight.controller", "bundlescanner").versionAsInProject(),
                mavenBundle("org.opendaylight.controller", "bundlescanner.implementation").versionAsInProject(),

                // Northbound bundles
                mavenBundle("org.opendaylight.controller", "commons.northbound").versionAsInProject(),
                mavenBundle("org.opendaylight.controller", "forwarding.staticrouting.northbound").versionAsInProject(),
                mavenBundle("org.opendaylight.controller", "statistics.northbound").versionAsInProject(),
                mavenBundle("org.opendaylight.controller", "topology.northbound").versionAsInProject(),
                mavenBundle("org.opendaylight.controller", "hosttracker.northbound").versionAsInProject(),
                mavenBundle("org.opendaylight.controller", "switchmanager.northbound").versionAsInProject(),
                mavenBundle("org.opendaylight.controller", "flowprogrammer.northbound").versionAsInProject(),
                mavenBundle("org.opendaylight.controller", "subnets.northbound").versionAsInProject(),

                // VTN Manager bundels
                mavenBundle("org.opendaylight.vtn", "manager").versionAsInProject(),
                mavenBundle("org.opendaylight.vtn", "manager.northbound").versionAsInProject(),
                mavenBundle("org.opendaylight.vtn", "manager.implementation").versionAsInProject(),

                mavenBundle("org.codehaus.jackson", "jackson-mapper-asl").versionAsInProject(),
                mavenBundle("org.codehaus.jackson", "jackson-core-asl").versionAsInProject(),
                mavenBundle("org.codehaus.jackson", "jackson-jaxrs").versionAsInProject(),
                mavenBundle("org.codehaus.jackson", "jackson-xc").versionAsInProject(),
                mavenBundle("org.codehaus.jettison", "jettison").versionAsInProject(),

                mavenBundle("commons-io", "commons-io").versionAsInProject(),

                mavenBundle("commons-fileupload", "commons-fileupload").versionAsInProject(),

                mavenBundle("equinoxSDK381", "javax.servlet").versionAsInProject(),
                mavenBundle("equinoxSDK381", "javax.servlet.jsp").versionAsInProject(),
                mavenBundle("equinoxSDK381", "org.eclipse.equinox.ds").versionAsInProject(),
                mavenBundle("orbit", "javax.xml.rpc").versionAsInProject(),
                mavenBundle("equinoxSDK381", "org.eclipse.equinox.util").versionAsInProject(),
                mavenBundle("equinoxSDK381", "org.eclipse.osgi.services").versionAsInProject(),
                mavenBundle("equinoxSDK381", "org.apache.felix.gogo.command").versionAsInProject(),
                mavenBundle("equinoxSDK381", "org.apache.felix.gogo.runtime").versionAsInProject(),
                mavenBundle("equinoxSDK381", "org.apache.felix.gogo.shell").versionAsInProject(),
                mavenBundle("equinoxSDK381", "org.eclipse.equinox.cm").versionAsInProject(),
                mavenBundle("equinoxSDK381", "org.eclipse.equinox.console").versionAsInProject(),
                mavenBundle("equinoxSDK381", "org.eclipse.equinox.launcher").versionAsInProject(),

                mavenBundle("geminiweb", "org.eclipse.gemini.web.core").versionAsInProject(),
                mavenBundle("geminiweb", "org.eclipse.gemini.web.extender").versionAsInProject(),
                mavenBundle("geminiweb", "org.eclipse.gemini.web.tomcat").versionAsInProject(),
                mavenBundle("geminiweb", "org.eclipse.virgo.kernel.equinox.extensions").versionAsInProject().noStart(),
                mavenBundle("geminiweb", "org.eclipse.virgo.util.common").versionAsInProject(),
                mavenBundle("geminiweb", "org.eclipse.virgo.util.io").versionAsInProject(),
                mavenBundle("geminiweb", "org.eclipse.virgo.util.math").versionAsInProject(),
                mavenBundle("geminiweb", "org.eclipse.virgo.util.osgi").versionAsInProject(),
                mavenBundle("geminiweb", "org.eclipse.virgo.util.osgi.manifest").versionAsInProject(),
                mavenBundle("geminiweb", "org.eclipse.virgo.util.parser.manifest").versionAsInProject(),

                mavenBundle("org.apache.felix", "org.apache.felix.dependencymanager").versionAsInProject(),
                mavenBundle("org.apache.felix", "org.apache.felix.dependencymanager.shell").versionAsInProject(),

                mavenBundle("com.google.code.gson", "gson").versionAsInProject(),
                mavenBundle("org.jboss.spec.javax.transaction", "jboss-transaction-api_1.1_spec").versionAsInProject(),
                mavenBundle("org.apache.felix", "org.apache.felix.fileinstall").versionAsInProject(),
                mavenBundle("org.apache.commons", "commons-lang3").versionAsInProject(),
                mavenBundle("commons-codec", "commons-codec"),
                mavenBundle("virgomirror", "org.eclipse.jdt.core.compiler.batch").versionAsInProject(),
                mavenBundle("eclipselink", "javax.persistence").versionAsInProject(),
                mavenBundle("eclipselink", "javax.resource").versionAsInProject(),

                mavenBundle("orbit", "javax.activation").versionAsInProject(),
                mavenBundle("orbit", "javax.annotation").versionAsInProject(),
                mavenBundle("orbit", "javax.ejb").versionAsInProject(),
                mavenBundle("orbit", "javax.el").versionAsInProject(),
                mavenBundle("orbit", "javax.mail.glassfish").versionAsInProject(),
                mavenBundle("orbit", "javax.xml.rpc").versionAsInProject(),
                mavenBundle("orbit", "org.apache.catalina").versionAsInProject(),
                // these are bundle fragments that can't be started on its own
                mavenBundle("orbit", "org.apache.catalina.ha").versionAsInProject().noStart(),
                mavenBundle("orbit", "org.apache.catalina.tribes").versionAsInProject().noStart(),
                mavenBundle("orbit", "org.apache.coyote").versionAsInProject().noStart(),
                mavenBundle("orbit", "org.apache.jasper").versionAsInProject().noStart(),

                mavenBundle("orbit", "org.apache.el").versionAsInProject(),
                mavenBundle("orbit", "org.apache.juli.extras").versionAsInProject(),
                mavenBundle("orbit", "org.apache.tomcat.api").versionAsInProject(),
                mavenBundle("orbit", "org.apache.tomcat.util").versionAsInProject().noStart(),
                mavenBundle("orbit", "javax.servlet.jsp.jstl").versionAsInProject(),
                mavenBundle("orbit", "javax.servlet.jsp.jstl.impl").versionAsInProject(),

                mavenBundle("org.ops4j.pax.exam", "pax-exam-container-native"),
                mavenBundle("org.ops4j.pax.exam", "pax-exam-junit4"),
                mavenBundle("org.ops4j.pax.exam", "pax-exam-link-mvn"),
                mavenBundle("org.ops4j.pax.url", "pax-url-aether"),

                mavenBundle("org.ow2.asm", "asm-all").versionAsInProject(),

                mavenBundle("org.springframework", "org.springframework.asm").versionAsInProject(),
                mavenBundle("org.springframework", "org.springframework.aop").versionAsInProject(),
                mavenBundle("org.springframework", "org.springframework.context").versionAsInProject(),
                mavenBundle("org.springframework", "org.springframework.context.support").versionAsInProject(),
                mavenBundle("org.springframework", "org.springframework.core").versionAsInProject(),
                mavenBundle("org.springframework", "org.springframework.beans").versionAsInProject(),
                mavenBundle("org.springframework", "org.springframework.expression").versionAsInProject(),
                mavenBundle("org.springframework", "org.springframework.web").versionAsInProject(),

                mavenBundle("org.aopalliance", "com.springsource.org.aopalliance").versionAsInProject(),
                mavenBundle("org.springframework", "org.springframework.web.servlet").versionAsInProject(),
                mavenBundle("org.springframework.security", "spring-security-config").versionAsInProject(),
                mavenBundle("org.springframework.security", "spring-security-core").versionAsInProject(),
                mavenBundle("org.springframework.security", "spring-security-web").versionAsInProject(),
                mavenBundle("org.springframework.security", "spring-security-taglibs").versionAsInProject(),
                mavenBundle("org.springframework", "org.springframework.transaction").versionAsInProject(),

                mavenBundle("org.ow2.chameleon.management", "chameleon-mbeans").versionAsInProject(),
                mavenBundle("org.opendaylight.controller.thirdparty", "net.sf.jung2").versionAsInProject(),
                mavenBundle("org.opendaylight.controller.thirdparty", "com.sun.jersey.jersey-servlet").versionAsInProject(),
                mavenBundle("org.opendaylight.controller.thirdparty", "org.apache.catalina.filters.CorsFilter").versionAsInProject().noStart(),

                // Jersey needs to be started before the northbound application
                // bundles, using a lower start level
                mavenBundle("com.sun.jersey", "jersey-client").versionAsInProject(),
                mavenBundle("com.sun.jersey", "jersey-server").versionAsInProject().startLevel(2),
                mavenBundle("com.sun.jersey", "jersey-core").versionAsInProject().startLevel(2),
                mavenBundle("com.sun.jersey", "jersey-json").versionAsInProject().startLevel(2),

                junitBundles());
    }
}
