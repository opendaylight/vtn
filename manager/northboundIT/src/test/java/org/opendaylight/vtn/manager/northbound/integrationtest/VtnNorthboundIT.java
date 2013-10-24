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
import java.util.HashSet;
import java.util.Iterator;
import java.util.Set;

import javax.inject.Inject;

import org.apache.commons.codec.binary.Base64;
import org.codehaus.jettison.json.JSONArray;
import org.codehaus.jettison.json.JSONException;
import org.codehaus.jettison.json.JSONObject;
import org.codehaus.jettison.json.JSONTokener;
import org.junit.AfterClass;
import org.junit.Assert;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.opendaylight.controller.sal.core.Node;
import org.opendaylight.controller.sal.core.NodeConnector;
import org.opendaylight.controller.sal.packet.ARP;
import org.opendaylight.controller.sal.packet.IListenDataPacket;
import org.opendaylight.controller.sal.packet.RawPacket;
import org.opendaylight.controller.sal.utils.GlobalConstants;
import org.opendaylight.controller.sal.utils.HexEncode;
import org.opendaylight.controller.sal.utils.NodeConnectorCreator;
import org.opendaylight.controller.sal.utils.NodeCreator;
import org.opendaylight.controller.usermanager.IUserManager;
import org.opendaylight.vtn.manager.IVTNManager;
import org.opendaylight.vtn.manager.VBridgePath;
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
public class VtnNorthboundIT extends TestBase {
    private static final Logger log = LoggerFactory.
        getLogger(VtnNorthboundIT.class);
    private static final String  BUNDLE_VTN_MANAGER_IMPL
        = "org.opendaylight.vtn.manager.implementation";
    private static final String VTN_BASE_URL
        = "http://127.0.0.1:8080/controller/nb/v2/vtn/";

    // get the OSGI bundle context
    @Inject
    private BundleContext bc;
    private IUserManager userManager = null;
    private IVTNManager vtnManager = null;
    private IListenDataPacket listenDataPacket = null;
    private Boolean debugMsg = false;
    private Bundle  implBundle;

    /**
     * method called once before this test class start.
     */
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

    /**
     * method called once after this test class finish.
     */
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

    /**
     * Convert Bundle state to String.
     * @param state     A state of Bundle.
     * @return  A string representation of state.
     */
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

    /**
     * check test setup is ready or not before each test method start.
     */
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

        r = bc.getServiceReference(IVTNManager.class.getName());
        if (r != null) {
            this.vtnManager = (IVTNManager) bc.getService(r);
            this.listenDataPacket = (IListenDataPacket) this.vtnManager;
        }

        // If UserManager is null, cannot login to run tests.
        assertNotNull(this.userManager);
        assertNotNull(this.vtnManager);
    }

    // static variable to pass response code from getJsonResult()
    private static Integer httpResponseCode = null;

    private String  httpLocation;


    /**
     * Send request and get result
     *
     * @param restUrl   A target URL.
     * @return  A returned result for request.
     */
    private String getJsonResult(String restUrl) {
        return getJsonResult(restUrl, "GET", null);
    }

    /**
     * Send request and get result
     *
     * @param restUrl   A request URL.
     * @param method    A request method.
     * @return  A returned result for request.
     */
    private String getJsonResult(String restUrl, String method) {
        return getJsonResult(restUrl, method, null);
    }

    /**
     * Send request and get result
     *
     * @param restUrl   A request URL.
     * @param method    A request method.
     * @param body      A request body send with request.
     * @return  A returned result for request.
     */
    private String getJsonResult(String restUrl, String method, String body) {
        return getJsonResult(restUrl, method, body, "application/json");
    }

    /**
     * Send request and get result
     *
     * @param restUrl   A request URL.
     * @param method    A request method.
     * @param body      A request body send with request.
     * @param contentType A contentType of request.
     * @return  A returned result for request.
     */
    private String getJsonResult(String restUrl, String method, String body,
                                 String contentType) {
        return getJsonResult(restUrl, method, body, contentType, true);
    }

    /**
     * Send request and get result
     *
     * @param restUrl   A request URL.
     * @param method    A request method.
     * @param body      A request body send with request.
     * @param contentType A contentType of request.
     * @param auth      if {@true} authorization succeed,
     *                  else if {@false} authorization fails.
     * @return  A returned result for request.
     */
    private String getJsonResult(String restUrl, String method, String body,
                                 String contentType, boolean auth) {

        // Initialize response code to indicate error
        httpResponseCode = 400;
        httpLocation = null;

        if (debugMsg) {
            System.out.println("HTTP method: " + method + " url: " + restUrl.toString());
            if (body != null)
                System.out.println("body" + body);
        }

        try {
            URL url = new URL(restUrl);
            this.userManager.getAuthorizationList();
            String authString = null;
            if (auth) {
                this.userManager.authenticate("admin", "admin");
                authString = "admin:admin";
            } else {
                this.userManager.authenticate("admin", "bad");
                authString = "admin:hoge";
            }
            byte[] authEncBytes = Base64.encodeBase64(authString.getBytes());
            String authStringEnc = new String(authEncBytes);

            HttpURLConnection connection = (HttpURLConnection) url.openConnection();
            connection.setRequestMethod(method);
            connection.setRequestProperty("Authorization", "Basic " + authStringEnc);
            connection.setRequestProperty("Content-Type", contentType);
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
            if (httpResponseCode > 299) {
                return httpResponseCode.toString();
            }

            if (httpResponseCode == HttpURLConnection.HTTP_CREATED) {
                // Determine location.
                for (int i = 0; true; i++) {
                    String field = connection.getHeaderFieldKey(i);
                    if (field == null) {
                        if (i == 0) {
                            continue;
                        }
                        break;
                    }

                    if (field.equalsIgnoreCase("location")) {
                        httpLocation = connection.getHeaderField(i);
                        break;
                    }
                }
            }

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

    /**
     *  a class to construct query parameter for HTTP request
     */
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

    /**
     * Test case for VTN APIs.
     *
     * This calls {@code testVBridgeAPI}.
     */
    @Test
    public void testVTNAPI() throws JSONException {
        System.out.println("Starting VTN JAXB client.");
        String baseURL = VTN_BASE_URL;

        String tname1 = "testVtn1";
        String tname2 = "testVtn_2";
        String tname3 = "testVtn3";
        String tname4 = "4";
        String tname5 = "testVtnf";
        String tname6 = "testVtn6";
        String tname7 = "testVtn007testVtn007testVtn0007";
        String tname8 = "testVtn8";

        String tname = "testVtn";
        String tname32 = "testVtn032testVtn032testVtn032te";

        String desc1 = "testDescription1";
        String desc2 = "2";
        String desc3 = String.format("%01000d", 1);

        String itimeout1 = "100";
        String itimeout2 = "250";
        String htimeout1 = "200";
        String htimeout2 = "400";

        String timeout0 = "0";
        String timeout_negative = "-10";
        String timeout_max = "65535";
        String timeout_over = "65540";

        String cont_dummy = "cont_dummy";

        // Test GET vtn in default container, expecting no results
        String result = getJsonResult(baseURL + "default/vtns");
        Assert.assertEquals(200, httpResponseCode.intValue());
        JSONTokener jt = new JSONTokener(result);
        JSONObject json = new JSONObject(jt);
        JSONArray vtnArray = json.getJSONArray("vtn");
        Assert.assertEquals(0, vtnArray.length());

        // Test GET vtn in dummy container, expecting 404
        result = getJsonResult(baseURL + cont_dummy + "/vtns");
//        Assert.assertEquals(404, httpResponseCode.intValue());

        // Test POST vtn1
        String requestBody = "{}";
        String requestUri = baseURL + "default/vtns/" + tname1;
        result = getJsonResult(requestUri, "POST", requestBody);
        Assert.assertEquals(201, httpResponseCode.intValue());
        Assert.assertEquals(requestUri, httpLocation);

        // Test POST vtn1, expecting 409
        requestUri = baseURL + "default/vtns/" + tname1;
        result = getJsonResult(requestUri, "POST", requestBody);
        Assert.assertEquals(409, httpResponseCode.intValue());

        // Test GET vtn in default container, expecting one result
        result = getJsonResult(baseURL + "default/vtns");
        Assert.assertEquals(200, httpResponseCode.intValue());
        jt = new JSONTokener(result);
        json = new JSONObject(jt);
        vtnArray = json.getJSONArray("vtn");
        Assert.assertEquals(1, vtnArray.length());

        // Test POST vtn2, setting "_" to vBridgeName
        requestBody = "{\"description\":\"" + desc1 + "\", \"idleTimeout\":\"" + itimeout1 + "\", \"hardTimeout\":\""
                + htimeout1 + "\"}";
        requestUri = baseURL + "default/vtns/" + tname2;
        result = getJsonResult(requestUri, "POST", requestBody);
        Assert.assertEquals(201, httpResponseCode.intValue());
        Assert.assertEquals(requestUri, httpLocation);

        // Test GET all vtns in default container
        result = getJsonResult(baseURL + "default/vtns");
        jt = new JSONTokener(result);
        json = new JSONObject(jt);
        vtnArray = json.getJSONArray("vtn");
        JSONObject vtn;

        Assert.assertEquals(200, httpResponseCode.intValue());
        Assert.assertEquals(2, vtnArray.length());

        for (int i = 0; i < vtnArray.length(); i++) {
            vtn = vtnArray.getJSONObject(i);
            if (vtn.getString("name").equals(tname1)) {
                Assert.assertFalse(vtn.has("description"));
                Assert.assertEquals("300", vtn.getString("idleTimeout"));
                Assert.assertEquals("0", vtn.getString("hardTimeout"));
            } else if (vtn.getString("name").equals(tname2)) {
                Assert.assertEquals(desc1, vtn.getString("description"));
                Assert.assertEquals(itimeout1, vtn.getString("idleTimeout"));
                Assert.assertEquals(htimeout1, vtn.getString("hardTimeout"));
            } else {
                // Unexpected vtn name
                Assert.assertTrue(false);
            }
        }

        // Test POST vtn3. testing long description
        requestBody = "{\"idleTimeout\":\"" + itimeout1 + "\", \"hardTimeout\":\"" + htimeout1 + "\"}";
        requestUri = baseURL + "default/vtns/" + tname3;
        result = getJsonResult(requestUri, "POST", requestBody);
        Assert.assertEquals(201, httpResponseCode.intValue());
        Assert.assertEquals(requestUri, httpLocation);

        // Test POST vtn4, setting idle timeout of negative value and one character numeric of vtn name
        requestBody = "{\"description\":\"" + desc3 + "\", \"idleTimeout\":\"" + timeout_negative + "\"}";
        requestUri = baseURL + "default/vtns/" + tname4;
        result = getJsonResult(requestUri, "POST", requestBody);
        Assert.assertEquals(201, httpResponseCode.intValue());
        Assert.assertEquals(requestUri, httpLocation);

        // Test GET vtn
        result = getJsonResult(baseURL + "default/vtns/" + tname4);
        jt = new JSONTokener(result);
        json = new JSONObject(jt);

        Assert.assertEquals("300", json.getString("idleTimeout"));
        Assert.assertEquals(timeout0, json.getString("hardTimeout"));

        // Test POST vtn5, testing hard timeout of negative value
        requestBody = "{\"description\":\"" + desc1 + "\",  \"hardTimeout\":\"" + timeout_negative + "\"}";
        requestUri = baseURL + "default/vtns/" + tname5;
        result = getJsonResult(requestUri, "POST", requestBody);
        Assert.assertEquals(201, httpResponseCode.intValue());
        Assert.assertEquals(requestUri, httpLocation);

        // Test GET vtn
        result = getJsonResult(baseURL + "default/vtns/" + tname5);
        jt = new JSONTokener(result);
        json = new JSONObject(jt);

        Assert.assertEquals("300", json.getString("idleTimeout"));
        Assert.assertEquals(timeout0, json.getString("hardTimeout"));

        // Test POST vtn6, setting idle timeout of 0 and hard timeout of 65535
        requestBody = "{\"description\":\"" + desc1 + "\", \"idleTimeout\":\"" + timeout_max + "\", \"hardTimeout\":\""
                + timeout0 + "\"}";
        requestUri = baseURL + "default/vtns/" + tname6;
        result = getJsonResult(requestUri, "POST", requestBody);
        Assert.assertEquals(201, httpResponseCode.intValue());
        Assert.assertEquals(requestUri, httpLocation);

        // Test POST vtn7, setting idle timeout of 65535, hard timeout of 0 and vtn name of 31 characters
        requestBody = "{\"description\":\"" + desc2 + "\", \"idleTimeout\":\"" + timeout0 + "\", \"hardTimeout\":\""
                + timeout_max + "\"}";
        requestUri = baseURL + "default/vtns/" + tname7;
        result = getJsonResult(requestUri, "POST", requestBody);
        Assert.assertEquals(201, httpResponseCode.intValue());
        Assert.assertEquals(requestUri, httpLocation);

        // Test POST vtn8, setting invalid value after description to requestBody
        requestBody = "{\"description\":\"" + desc1 + "\", \"Timeout\":\"" + timeout0 + "\", \"hard\":\"" + timeout0 + "\"}";
        requestUri = baseURL + "default/vtns/" + tname8;
        result = getJsonResult(requestUri, "POST", requestBody);
        Assert.assertEquals(201, httpResponseCode.intValue());
        Assert.assertEquals(requestUri, httpLocation);

        // Test GET vtn
        result = getJsonResult(baseURL + "default/vtns/" + tname8);
        jt = new JSONTokener(result);
        json = new JSONObject(jt);

        Assert.assertEquals(desc1, json.getString("description"));
        Assert.assertEquals("300", json.getString("idleTimeout"));
        Assert.assertEquals(timeout0, json.getString("hardTimeout"));

        // Test POST vtn, expecting 400
        requestBody = "{\"enabled\":\"true\"" + "\"description\":\"" + desc1 + "\"}";
        result = getJsonResult(baseURL + "default/vtns/" + tname, "POST", requestBody);
        Assert.assertEquals(400, httpResponseCode.intValue());

        // Test POST vtn, expecting 400, setting invalid value to idle timeout
        requestBody = "{\"description\":\"" + desc1 + "\", \"idleTimeout\":\"idletimeout\", \"hardTimeout\":\""+ htimeout1 + "\"}";
        result = getJsonResult(baseURL + "default/vtns/" + tname, "POST", requestBody);
        Assert.assertEquals(400, httpResponseCode.intValue());

        // Test POST vtn, expecting 400, setting invalid value to hard timeout
        requestBody = "{\"description\":\"" + desc1 + "\", \"idleTimeout\":\"" + itimeout1 + "\", \"hardTimeout\":\"hardtimeout\"}";
        result = getJsonResult(baseURL + "default/vtns/" + tname, "POST", requestBody);
        Assert.assertEquals(400, httpResponseCode.intValue());

        // Test POST vtn, expecting 400, setting invalid value to requestBody
        requestBody = "{\"description\":\"" + desc3 + "\", \"didleTimeout\":\"rdTimeout\":\"}";
        result = getJsonResult(baseURL + "default/vtns/" + tname, "POST", requestBody);
        Assert.assertEquals(400, httpResponseCode.intValue());


        // Test POST vtn expecting 105, test when vtn name is ""
        requestBody = "{}";
        result = getJsonResult(baseURL + "default/vtns/" + "", "POST", requestBody);
        Assert.assertEquals(405, httpResponseCode.intValue());

        // Test POST vtn expecting 415, setting the vtn that don't exist
        requestUri = baseURL + "default/vtns/" + desc3;
        result = getJsonResult(requestUri, "POST", requestBody);
        Assert.assertEquals(415, httpResponseCode.intValue());

        // Test POST vtn expecting 415, setting "_" to  first letter of vBridgeName
        requestUri = baseURL + "default/vtns/" + "_testVtn";
        result = getJsonResult(requestUri, "POST", requestBody);
        Assert.assertEquals(415, httpResponseCode.intValue());

        // Test POST vtn expecting 415, setting vtn name including symbol "@"
        requestUri = baseURL + "default/vtns/" + "test@Vtn";
        result = getJsonResult(requestUri, "POST", requestBody);
        Assert.assertEquals(415, httpResponseCode.intValue());

        // Test POST vtn expecting 415, testing idle timeout of 65536
        requestBody =  "{\"description\":\"" + desc1 + "\", \"idleTimeout\":\"65536\", \"hardTimeout\":\""
                        + htimeout1 + "\"}";
        requestUri = baseURL + "default/vtns/" + tname;
        result = getJsonResult(requestUri, "POST", requestBody);
        Assert.assertEquals(415, httpResponseCode.intValue());

        // Test POST vtn expecting 415, testing hard timeout of 65536
        requestBody =  "{\"description\":\"" + desc1 + "\", \"idleTimeout\":\"" + itimeout1 + "\", \"hardTimeout\":\"65536\"}";
        requestUri = baseURL + "default/vtns/" + tname;
        result = getJsonResult(requestUri, "POST", requestBody);
        Assert.assertEquals(415, httpResponseCode.intValue());

        // Test POST vtn expecting 415, Setting the value idle timeout  is greater than hard timeout
        requestBody =  "{\"description\":\"" + desc1 + "\", \"idleTimeout\":\"" + itimeout2 + "\", \"hardTimeout\":\""+ htimeout1 +"\"}";
        requestUri = baseURL + "default/vtns/" + tname;
        result = getJsonResult(requestUri, "POST", requestBody);
        Assert.assertEquals(415, httpResponseCode.intValue());

        // Test POST vtn expecting 415, setting vBridgeName of 32 characters
        requestBody =  "{}";
        requestUri = baseURL + "default/vtns/" + tname32;
        result = getJsonResult(requestUri, "POST", requestBody);
        Assert.assertEquals(415, httpResponseCode.intValue());

        // Test POST vtn expecting 404, setting dummy container
        requestUri = baseURL + cont_dummy +"/vtns/" + tname;
        result = getJsonResult(requestUri, "POST", requestBody);
//        Assert.assertEquals(404, httpResponseCode.intValue());

        testVBridgeAPI(tname1, tname2);


        // Test GET vtn expecting 404, setting the vtn that don't exist
        result = getJsonResult(baseURL + "default/vtns/" + tname);
        Assert.assertEquals(404, httpResponseCode.intValue());

        // Test GET vtn expecting 404, setting dummy container
        result = getJsonResult(baseURL + cont_dummy +"/vtns/" + tname1);
//        Assert.assertEquals(404, httpResponseCode.intValue());

        // Test GET vtn
        result = getJsonResult(baseURL + "default/vtns/" + tname2);
        jt = new JSONTokener(result);
        json = new JSONObject(jt);

        Assert.assertEquals(200, httpResponseCode.intValue());
        Assert.assertEquals(tname2, json.getString("name"));
        Assert.assertEquals(desc1, json.getString("description"));
        Assert.assertEquals(itimeout1, json.getString("idleTimeout"));
        Assert.assertEquals(htimeout1, json.getString("hardTimeout"));

        // Test PUT vtn1, expecting all elements change
        requestBody = "{\"description\":\"" + desc3 + "\", \"idleTimeout\":\"" + timeout0 + "\", \"hardTimeout\":\""
                + timeout0 + "\"}";
        String queryParameter = new QueryParameter("all", "true").getString();

        result = getJsonResult(baseURL + "default/vtns/" + tname1 + queryParameter, "PUT", requestBody);
        Assert.assertEquals(200, httpResponseCode.intValue());

        // Test GET vtn
        result = getJsonResult(baseURL + "default/vtns/" + tname1);
        jt = new JSONTokener(result);
        json = new JSONObject(jt);

        Assert.assertEquals(desc3, json.getString("description"));
        Assert.assertEquals(timeout0, json.getString("idleTimeout"));
        Assert.assertEquals(timeout0, json.getString("hardTimeout"));

        // Test PUT vtn1,  abbreviate idle timeout and hard timeout
        requestBody = "{\"description\":\"" + desc2 + "\"}";
        result = getJsonResult(baseURL + "default/vtns/" + tname1 + queryParameter, "PUT", requestBody);
        Assert.assertEquals(200, httpResponseCode.intValue());

        // Test GET vtn
        result = getJsonResult(baseURL + "default/vtns/" + tname1);
        jt = new JSONTokener(result);
        json = new JSONObject(jt);

        Assert.assertEquals(desc2, json.getString("description"));
        Assert.assertEquals("300", json.getString("idleTimeout"));
        Assert.assertEquals("0", json.getString("hardTimeout"));

        // Test PUT vtn1, abbreviate description, testing idle timeout of 65535 and hard timeout of 0
        requestBody = "{\"idleTimeout\":\"" + timeout_max + "\", \"hardTimeout\":\"" + timeout0 + "\"}";
        result = getJsonResult(baseURL + "default/vtns/" + tname1 + queryParameter, "PUT", requestBody);
        Assert.assertEquals(200, httpResponseCode.intValue());

        // Test GET vtn
        result = getJsonResult(baseURL + "default/vtns/" + tname1);
        jt = new JSONTokener(result);
        json = new JSONObject(jt);
        try {
            Assert.assertEquals("", json.getString("description"));
        } catch (JSONException expected) {
            assertThat(expected.getMessage(), is("JSONObject[\"description\"] not found."));
        }
        Assert.assertEquals(timeout_max, json.getString("idleTimeout"));
        Assert.assertEquals(timeout0, json.getString("hardTimeout"));

        // Test PUT vtn1, abbreviate description and idle timeout, testing hard timeout of 65535
        requestBody = "{\"hardTimeout\":\"" + timeout_max + "\"}";
        result = getJsonResult(baseURL + "default/vtns/" + tname1 + queryParameter, "PUT", requestBody);
        Assert.assertEquals(200, httpResponseCode.intValue());

        // Test GET vtn
        result = getJsonResult(baseURL + "default/vtns/" + tname1);
        jt = new JSONTokener(result);
        json = new JSONObject(jt);
        Assert.assertFalse(json.has("description"));
        Assert.assertEquals("300", json.getString("idleTimeout"));
        Assert.assertEquals(timeout_max, json.getString("hardTimeout"));

        // Test PUT vtn1, abbreviate all elements
        requestBody = "{}";
        result = getJsonResult(baseURL + "default/vtns/" + tname1 + queryParameter, "PUT", requestBody);
        Assert.assertEquals(200, httpResponseCode.intValue());

        // Test GET vtn
        result = getJsonResult(baseURL + "default/vtns/" + tname1);
        jt = new JSONTokener(result);
        json = new JSONObject(jt);
        Assert.assertFalse(json.has("description"));
        Assert.assertEquals("300", json.getString("idleTimeout"));
        Assert.assertEquals("0", json.getString("hardTimeout"));

        // Test PUT vtn2, expecting all elements not change
        queryParameter = new QueryParameter("all", "false").getString();
        result = getJsonResult(baseURL + "default/vtns/" + tname2 + queryParameter, "PUT", requestBody);
        Assert.assertEquals(200, httpResponseCode.intValue());

        // Test GET vtn
        result = getJsonResult(baseURL + "default/vtns/" + tname2);
        jt = new JSONTokener(result);
        json = new JSONObject(jt);

        Assert.assertEquals(desc1, json.getString("description"));
        Assert.assertEquals(itimeout1, json.getString("idleTimeout"));
        Assert.assertEquals(htimeout1, json.getString("hardTimeout"));

        // Test PUT vtn2, setting 0 to idle timoeut and hard timeout
        requestBody = "{\"idleTimeout\":\"" + timeout0 + "\", \"hardTimeout\":\"" + timeout0 + "\"}";
        result = getJsonResult(baseURL + "default/vtns/" + tname2 + queryParameter, "PUT", requestBody);
        Assert.assertEquals(200, httpResponseCode.intValue());

        // Test GET vtn
        result = getJsonResult(baseURL + "default/vtns/" + tname2);
        jt = new JSONTokener(result);
        json = new JSONObject(jt);

        Assert.assertEquals(desc1, json.getString("description"));
        Assert.assertEquals(timeout0, json.getString("idleTimeout"));
        Assert.assertEquals(timeout0, json.getString("hardTimeout"));

        // Test PUT vtn2, expecting all elements change
        requestBody = "{\"description\":\"" + desc2 + "\", \"idleTimeout\":\"" + itimeout1 + "\", \"hardTimeout\":\""
                + htimeout1 + "\"}";
        result = getJsonResult(baseURL + "default/vtns/" + tname2 + queryParameter, "PUT", requestBody);
        Assert.assertEquals(200, httpResponseCode.intValue());

        // Test GET vtn
        result = getJsonResult(baseURL + "default/vtns/" + tname2);
        jt = new JSONTokener(result);
        json = new JSONObject(jt);
        Assert.assertEquals(desc2, json.getString("description"));
        Assert.assertEquals(itimeout1, json.getString("idleTimeout"));
        Assert.assertEquals(htimeout1, json.getString("hardTimeout"));

        // Test PUT vtn2, expecting all elements not change
        requestBody = "{\"idleTimeout\":\"" + timeout_negative + "\", \"hardTimeout\":\"" + timeout_negative + "\"}";
        result = getJsonResult(baseURL + "default/vtns/" + tname2 + queryParameter, "PUT", requestBody);
        Assert.assertEquals(200, httpResponseCode.intValue());

        // Test GET vtn
        result = getJsonResult(baseURL + "default/vtns/" + tname2);
        jt = new JSONTokener(result);
        json = new JSONObject(jt);

        Assert.assertEquals(desc2, json.getString("description"));
        Assert.assertEquals(itimeout1, json.getString("idleTimeout"));
        Assert.assertEquals(htimeout1, json.getString("hardTimeout"));

        // Test PUT vtn8, setting invalid value after description to requestBody
        requestBody = "{\"description\":\"" + desc2 + "\", \"Timeout\":\"" + itimeout1 + "\", \"hard\":\"" + htimeout2 + "\"}";
        result = getJsonResult(baseURL + "default/vtns/" + tname8 + queryParameter, "PUT", requestBody);
        Assert.assertEquals(200, httpResponseCode.intValue());

        // Test GET vtn
        result = getJsonResult(baseURL + "default/vtns/" + tname8);
        jt = new JSONTokener(result);
        json = new JSONObject(jt);

        Assert.assertEquals(desc2, json.getString("description"));
        Assert.assertEquals("300", json.getString("idleTimeout"));
        Assert.assertEquals(timeout0, json.getString("hardTimeout"));


        // Test PUT vtn2, description not change
        queryParameter = new QueryParameter("all", "true").getString();
        requestBody = "{\"idleTimeout\":\"" + timeout_negative + "\", \"hardTimeout\":\"" + timeout_negative + "\"}";
        result = getJsonResult(baseURL + "default/vtns/" + tname2 + queryParameter, "PUT", requestBody);
        Assert.assertEquals(200, httpResponseCode.intValue());

        // Test GET vtn
        result = getJsonResult(baseURL + "default/vtns/" + tname2);
        jt = new JSONTokener(result);
        json = new JSONObject(jt);

        Assert.assertFalse(json.has("description"));
        Assert.assertEquals("300", json.getString("idleTimeout"));
        Assert.assertEquals(timeout0, json.getString("hardTimeout"));

        // Test PUT vtn, expecting 400, setting the invalid value to idle timeout
        requestBody = "{\"idleTimeout\":\"" + desc1 + "\", \"hardTimeout\":\"" + htimeout1 + "\"}";
        result = getJsonResult(baseURL + "default/vtns/" + tname1 + queryParameter, "PUT", requestBody);
        Assert.assertEquals(400, httpResponseCode.intValue());

        // Test PUT vtn, expecting 400, setting the invalid value to hard timeout
        requestBody = "{\"idleTimeout\":\"" + itimeout1 + "\", \"hardTimeout\":\"" + desc1 + "\"}";
        result = getJsonResult(baseURL + "default/vtns/" + tname1 + queryParameter, "PUT", requestBody);
        Assert.assertEquals(400, httpResponseCode.intValue());

        // Test PUT vtn, expecting 400, setting invalid value to requestBody
        requestBody = "{\"description\":\"" + desc3 + "\", \"didleTimeout\":\"rdTimeout\":\"}";
        result = getJsonResult(baseURL + "default/vtns/" + tname1 + queryParameter, "PUT", requestBody);
        Assert.assertEquals(400, httpResponseCode.intValue());


        // Test PUT vtn expecting 404, setting the vtn that don't exist
        requestBody = "{\"description\":\"" + desc2 + "\", \"idleTimeout\":\"" + itimeout1 + "\", \"hardTimeout\":\""
                + htimeout1 + "\"}";
        result = getJsonResult(baseURL + "default/vtns/" + tname + queryParameter, "PUT", requestBody);
        Assert.assertEquals(404, httpResponseCode.intValue());

        // Test PUT vtn expecting 404, setting dummy container
        result = getJsonResult(baseURL + cont_dummy +"/vtns/" + tname1 + queryParameter, "PUT", requestBody);
//        Assert.assertEquals(404, httpResponseCode.intValue());

        // Test PUT vtn expecting 400, setting invalid value to requestBody
        requestBody = "{\"Test\"}";
        result = getJsonResult(baseURL + "default/vtns/" + tname1 + queryParameter, "PUT", requestBody);
        Assert.assertEquals(400, httpResponseCode.intValue());

        // Test PUT vtn expecting 415, setting idletimeout of 65540
        requestBody = "{\"description\":\"" + desc2 + "\", \"idleTimeout\":\"" + timeout_over + "\", \"hardTimeout\":\""
                + htimeout2 + "\"}";
        result = getJsonResult(baseURL + "default/vtns/" + tname1 + queryParameter, "PUT", requestBody);
        Assert.assertEquals(415, httpResponseCode.intValue());

        // Test PUT vtn expecting 415, setting hardtimeout of 65540
        requestBody = "{\"description\":\"" + desc2 + "\", \"idleTimeout\":\"" + itimeout1 + "\", \"hardTimeout\":\""
                + timeout_over + "\"}";
        result = getJsonResult(baseURL + "default/vtns/" + tname1 + queryParameter, "PUT", requestBody);
        Assert.assertEquals(415, httpResponseCode.intValue());

        // Test POST vtn expecting 415, Setting the value idle timeout  is greater than hard timeout
        requestBody =  "{\"description\":\"" + desc1 + "\", \"idleTimeout\":\"" + itimeout2 + "\", \"hardTimeout\":\""+ htimeout1 +"\"}";
        result = getJsonResult(baseURL + "default/vtns/" + tname1 + queryParameter, "PUT", requestBody);
        Assert.assertEquals(415, httpResponseCode.intValue());

        // Test PUT vtn
        requestBody = "{\"description\":\"" + desc2 + "\", \"idleTimeout\":\"" + itimeout2 + "\", \"hardTimeout\":\""
                + timeout_negative + "\"}";
        result = getJsonResult(baseURL + "default/vtns/" + tname1 + queryParameter, "PUT", requestBody);
        Assert.assertEquals(200, httpResponseCode.intValue());

        // Test GET all vtns in default container
        result = getJsonResult(baseURL + "default/vtns");
        jt = new JSONTokener(result);
        json = new JSONObject(jt);
        vtnArray = json.getJSONArray("vtn");

        Assert.assertEquals(200, httpResponseCode.intValue());
        Assert.assertEquals(8, vtnArray.length());

        // Test DELETE vtn expecting 404
        result = getJsonResult(baseURL + "default/vtns/" + tname, "DELETE");
        Assert.assertEquals(404, httpResponseCode.intValue());

        // set not supported "Content-type". expect to return 415.
        requestBody = "{}";
        result = getJsonResult(baseURL + "default/vtns/" + tname, "POST", requestBody,
                               "text/plain");
        Assert.assertEquals(415, httpResponseCode.intValue());

        requestBody = "{\"description\":\"desc\"}";
        result = getJsonResult(baseURL + "default/vtns/" + tname1 + queryParameter,
                               "PUT", requestBody, "text/plain");
        Assert.assertEquals(415, httpResponseCode.intValue());

        // auth failed.
        result = getJsonResult(baseURL + "default/vtns", "GET", null, "application/json", false);
        Assert.assertEquals(401, httpResponseCode.intValue());

        requestBody = "{}";
        result = getJsonResult(baseURL + "default/vtns/" + tname, "POST", requestBody,
                               "application/json", false);
        Assert.assertEquals(401, httpResponseCode.intValue());

        result = getJsonResult(baseURL + "default/vtns/" + tname1, "GET", null,
                               "application/json", false);
        Assert.assertEquals(401, httpResponseCode.intValue());

        requestBody = "{}";
        result = getJsonResult(baseURL + "default/vtns/" + tname1 + queryParameter,
                               "PUT", requestBody, "application/json", false);
        Assert.assertEquals(401, httpResponseCode.intValue());

        result = getJsonResult(baseURL + "default/vtns/" + tname1, "DELETE", null,
                               "application/json", false);
        Assert.assertEquals(401, httpResponseCode.intValue());

        // Test DELETE vtn expecting 404, setting dummy container
        result = getJsonResult(baseURL + cont_dummy + "/vtns/" + tname1, "DELETE");
//        Assert.assertEquals(404, httpResponseCode.intValue());

        // Test DELETE vtn
        result = getJsonResult(baseURL + "default/vtns/" + tname1, "DELETE");
        Assert.assertEquals(200, httpResponseCode.intValue());

        // Test DELETE vtn2
        result = getJsonResult(baseURL + "default/vtns/" + tname2, "DELETE");
        Assert.assertEquals(200, httpResponseCode.intValue());

        // Test DELETE vtn3
        result = getJsonResult(baseURL + "default/vtns/" + tname3, "DELETE");
        Assert.assertEquals(200, httpResponseCode.intValue());

        // Test DELETE vtn4
        result = getJsonResult(baseURL + "default/vtns/" + tname4, "DELETE");
        Assert.assertEquals(200, httpResponseCode.intValue());

        // Test DELETE vtn5
        result = getJsonResult(baseURL + "default/vtns/" + tname5, "DELETE");
        Assert.assertEquals(200, httpResponseCode.intValue());

        // Test DELETE vtn6
        result = getJsonResult(baseURL + "default/vtns/" + tname6, "DELETE");
        Assert.assertEquals(200, httpResponseCode.intValue());

        // Test DELETE vtn7
        result = getJsonResult(baseURL + "default/vtns/" + tname7, "DELETE");
        Assert.assertEquals(200, httpResponseCode.intValue());

        // Test DELETE vtn8
        result = getJsonResult(baseURL + "default/vtns/" + tname8, "DELETE");
        Assert.assertEquals(200, httpResponseCode.intValue());


        // Test DELETE vtn expecting 404
        result = getJsonResult(baseURL + "default/vtns/" + tname1, "DELETE");
        Assert.assertEquals(404, httpResponseCode.intValue());

        // Test GET vtn in default container, expecting no results
        result = getJsonResult(baseURL + "default/vtns");
        Assert.assertEquals(200, httpResponseCode.intValue());
        jt = new JSONTokener(result);
        json = new JSONObject(jt);
        vtnArray = json.getJSONArray("vtn");
        Assert.assertEquals(0, vtnArray.length());

        testVtnGlobal(baseURL);
    }

    /**
     * test case for VBridge APIs.
     *
     * This method is called by {@code testVTNAPI}.
     * This calls {@code testVLANMappingAPI},
     * {@code testVBridgeInterfaceAPIs},
     * {@code testVBridgeInterfaceDeleteAPI},
     * {@code testVLANMappingDeleteAPI},
     * {@code testMacAddressAPI}.
     */
    private void testVBridgeAPI(String tname1, String tname2) throws JSONException {
        System.out.println("Starting vBridge JAXB client.");
        String url = VTN_BASE_URL;
        StringBuilder baseURL = new StringBuilder();
        baseURL.append(url);
        baseURL.append("default/vtns/");

        String tname_dummy = "tenant_dummy";
        String cont_dummy = "cont_dummy";
        String desc1 = "testDescription1";
        String desc2 = "d";
        String desc3 = String.format("%01000d", 1);

        String bname1 = "n";
        String bname2 = "vbridge_Name2";
        String bname3 = "33333";
        String bname4 = "vbridge_name4vbridge_name4vbrid";
        String ebname = "vbridge_for_error";
        String bname32 = "vbridge_namevbridge_namevbridge_";

        String ageinter0 = "0";
        String ageinter1 = "10";
        String ageinter2 = "100";
        String ageinter3 = "-100";
        String ageinter4 = "1000000";
        String ageinter_over = "1000001";
        String fault = "";


        // Test GET vBridges in dummy container expecting 404
        String result = getJsonResult(url + cont_dummy + "/vtns/" + tname1 + "/vbridges");
//        Assert.assertEquals(404, httpResponseCode.intValue());

        // Test GET vBridges in default container expecting 404, setting dummy tenant
        result = getJsonResult(baseURL + tname_dummy + "/vbridges");
        Assert.assertEquals(404, httpResponseCode.intValue());

        // Test GET vBridges in default container, expecting no results
        result = getJsonResult(baseURL + tname1 + "/vbridges");
        Assert.assertEquals(200, httpResponseCode.intValue());
        JSONTokener jt = new JSONTokener(result);
        JSONObject json = new JSONObject(jt);
        JSONArray vBridgeArray = json.getJSONArray("vbridge");
        Assert.assertEquals(0, vBridgeArray.length());

        // Test POST vBridge1 expecting 404, setting dummy container
        String requestBody = "{}";
        result = getJsonResult(url + cont_dummy + "/vtns/" + tname1 + "/vbridges" + bname1, "POST", requestBody);
//        Assert.assertEquals(404, httpResponseCode.intValue());

        // Test POST vBridge1 expecting 404, setting dummy tenant
        result = getJsonResult(baseURL + tname_dummy + "/vbridges/" + bname1, "POST" , requestBody);
        Assert.assertEquals(404, httpResponseCode.intValue());

        // Test POST vBridge1 expecting 415
        requestBody = "{\"description\":\"" + desc1 + "\", \"ageInterval\":\"" + ageinter0 + "\"}";
        result = getJsonResult(baseURL + tname1 + "/vbridges/" + bname1 , "POST", requestBody);
        Assert.assertEquals(415, httpResponseCode.intValue());

        // Test POST vBridge1
        requestBody = "{}";
        String requestUri = baseURL + tname1 + "/vbridges/" + bname1;
        result = getJsonResult(requestUri, "POST", requestBody);
        Assert.assertEquals(201, httpResponseCode.intValue());
        Assert.assertEquals(requestUri, httpLocation);

        // Test GET vBridges in default container, expecting one result
        result = getJsonResult(baseURL + tname1 + "/vbridges");
        Assert.assertEquals(200, httpResponseCode.intValue());
        jt = new JSONTokener(result);
        json = new JSONObject(jt);
        vBridgeArray = json.getJSONArray("vbridge");
        Assert.assertEquals(1, vBridgeArray.length());

        // Test POST vBridge1 expecting 409
        requestBody = "{}";
        result = getJsonResult(baseURL + tname1 + "/vbridges/" + bname1, "POST", requestBody);
        Assert.assertEquals(409, httpResponseCode.intValue());

        // Test POST vBridge2
        requestBody = "{\"description\":\"" + desc2 + "\", \"ageInterval\":\"" + ageinter1 + "\"}";
        requestUri = baseURL + tname1 + "/vbridges/" + bname2;
        result = getJsonResult(requestUri, "POST", requestBody);
        Assert.assertEquals(201, httpResponseCode.intValue());
        Assert.assertEquals(requestUri, httpLocation);

        // Test POST vBridge2 for other tenant
        requestBody = "{\"ageInterval\":\"" + ageinter2 + "\"}";
        requestUri = baseURL + tname2 + "/vbridges/" + bname2;
        result = getJsonResult(requestUri, "POST", requestBody);
        Assert.assertEquals(201, httpResponseCode.intValue());
        Assert.assertEquals(requestUri, httpLocation);

        // Test GET all vBridges in default container
        result = getJsonResult(baseURL + tname1 + "/vbridges");
        jt = new JSONTokener(result);
        json = new JSONObject(jt);
        vBridgeArray = json.getJSONArray("vbridge");
        JSONObject vBridge;

        Assert.assertEquals(200, httpResponseCode.intValue());
        Assert.assertEquals(2, vBridgeArray.length());

        for (int i = 0; i < vBridgeArray.length(); i++) {
            vBridge = vBridgeArray.getJSONObject(i);
            if (vBridge.getString("name").equals(bname1)) {
                Assert.assertFalse(vBridge.has("description"));

                try {
                    Assert.assertEquals("600", json.getString("ageInterval"));
                } catch (JSONException expected) {
                    assertThat(expected.getMessage(), is("JSONObject[\"ageInterval\"] not found."));
                }

            } else if (vBridge.getString("name").equals(bname2)) {
                Assert.assertEquals(desc2, vBridge.getString("description"));
                Assert.assertEquals(ageinter1, vBridge.getString("ageInterval"));
                fault = vBridge.getString("faults");
            } else {
                // Unexpected vBridge name
                Assert.assertTrue(false);
            }
        }

        testVLANMappingAPI(tname1, bname1);
        testVBridgeInterfaceAPI(tname1, bname1, bname2);

        // Test POST vBridge3
        requestBody = "{\"description\":\"" + desc3 + "\", \"ageInterval\":\"" + ageinter3 + "\"}";
        requestUri = baseURL + tname1 + "/vbridges/" + bname3;
        result = getJsonResult(requestUri, "POST", requestBody);
        Assert.assertEquals(201, httpResponseCode.intValue());
        Assert.assertEquals(requestUri, httpLocation);

        // Test GET vBridge3
        result = getJsonResult(baseURL + tname1 + "/vbridges/" + bname3);
        Assert.assertEquals(200, httpResponseCode.intValue());
        jt = new JSONTokener(result);
        json = new JSONObject(jt);

        Assert.assertEquals(desc3,json.getString("description"));
        Assert.assertEquals("600", json.getString("ageInterval"));

        // Test POST vBridge4
        requestBody = "{\"description\":\"" + desc1 + "\", \"ageInterval\":\"" + ageinter4 + "\"}";
        requestUri = baseURL + tname1 + "/vbridges/" + bname4;
        result = getJsonResult(requestUri, "POST", requestBody);
        Assert.assertEquals(201, httpResponseCode.intValue());
        Assert.assertEquals(requestUri, httpLocation);

        // Test POST vBridge expecting 400
        requestBody = "{\"description\":\"" + desc1 + "\", \"ageInterval\":\"" + "ageInterval" + "\"}";
        requestUri = baseURL + tname1 + "/vbridges/" + ebname;
        result = getJsonResult(requestUri, "POST", requestBody);
        Assert.assertEquals(400, httpResponseCode.intValue());

        // Test POST vBridge expecting 415, setting vBridgeName of 32 characters
        requestBody = "{}";
        requestUri = baseURL + tname1 + "/vbridges/" + bname32;
        result = getJsonResult(requestUri, "POST", requestBody);
        Assert.assertEquals(415, httpResponseCode.intValue());

        // Test POST vBridge expecting 405, setting "" to vBridge name
        result = getJsonResult(baseURL + tname1 + "/vbridges/" + "", "POST", requestBody);
        Assert.assertEquals(405, httpResponseCode.intValue());

        // Test POST vBridge expecting 415, setting "_" to  first letter of vBridgeName
        result = getJsonResult(baseURL + tname1 + "/vbridges/" + "_vbridgename", "POST", requestBody);
        Assert.assertEquals(415, httpResponseCode.intValue());

        // Test POST vBridge expecting 415, setting vBridge Name including symbol "@"
        result = getJsonResult(baseURL + tname1 + "/vbridges/" + "vbridge@name", "POST", requestBody);
        Assert.assertEquals(415, httpResponseCode.intValue());

        // Test POST vBridge expecting 415, setting to ageInterval value greater than 1000000
        requestBody = "{\"ageInterval\":\"" + ageinter_over + "\"}";
        result = getJsonResult(baseURL + tname1 + "/vbridges/" + ebname, "POST", requestBody);
        Assert.assertEquals(415, httpResponseCode.intValue());

        // Test GET vBridges in default container, expecting 4 results
        result = getJsonResult(baseURL + tname1 + "/vbridges");
        Assert.assertEquals(200, httpResponseCode.intValue());
        jt = new JSONTokener(result);
        json = new JSONObject(jt);
        vBridgeArray = json.getJSONArray("vbridge");
        Assert.assertEquals(4, vBridgeArray.length());

        // Test GET vBridges in default container, expecting 1 result
        result = getJsonResult(baseURL + tname2 + "/vbridges");
        Assert.assertEquals(200, httpResponseCode.intValue());
        jt = new JSONTokener(result);
        json = new JSONObject(jt);
        vBridgeArray = json.getJSONArray("vbridge");
        Assert.assertEquals(1, vBridgeArray.length());

        // Test GET vBridge expecting 404 setting dummy container
        result = getJsonResult(url + cont_dummy + "/vtns/" + tname1 + "/vbridges/" + bname1);
//        Assert.assertEquals(404, httpResponseCode.intValue());

        // Test GET vBridge expecting 404 setting dummy vtn
        result = getJsonResult(baseURL + tname_dummy + "/vbridges/" + bname1);
        Assert.assertEquals(404, httpResponseCode.intValue());

        // Test GET vBridge expecting 404 setting dummy vBridge
        result = getJsonResult(baseURL + tname1 + "/vbridges/" + ebname);
        Assert.assertEquals(404, httpResponseCode.intValue());

        // Test GET vBridge
        result = getJsonResult(baseURL + tname1 + "/vbridges/" + bname2);
        jt = new JSONTokener(result);
        json = new JSONObject(jt);

        Assert.assertEquals(200, httpResponseCode.intValue());
        Assert.assertEquals(bname2, json.getString("name"));
        Assert.assertEquals(desc2, json.getString("description"));
        Assert.assertEquals(ageinter1, json.getString("ageInterval"));
        Assert.assertEquals(fault, json.getString("faults"));
        Assert.assertEquals("1", json.getString("state"));

        // Test GET vBridge, get from other tenant
        result = getJsonResult(baseURL + tname2 + "/vbridges/" + bname2);
        Assert.assertEquals(200, httpResponseCode.intValue());

        // Test PUT vBridge1, setting only description (queryparameter is true)
        String queryParameter = new QueryParameter("all", "true").getString();
        requestBody = "{\"description\":\"" + desc1 + "\"}";
        result = getJsonResult(baseURL + tname1 + "/vbridges/" + bname1 + queryParameter, "PUT", requestBody);
        Assert.assertEquals(200, httpResponseCode.intValue());

        // Test GET vBridge1
        result = getJsonResult(baseURL + tname1 + "/vbridges/" + bname1);
        Assert.assertEquals(200, httpResponseCode.intValue());
        jt = new JSONTokener(result);
        json = new JSONObject(jt);

        Assert.assertEquals(desc1,json.getString("description"));
        Assert.assertEquals("600", json.getString("ageInterval"));

        // Test PUT vBridge1, setting only ageInter (queryparameter is true)
        requestBody = "{\"ageInterval\":\"" + ageinter1 + "\"}";
        result = getJsonResult(baseURL + tname1 + "/vbridges/" + bname1 + queryParameter, "PUT", requestBody);
        Assert.assertEquals(200, httpResponseCode.intValue());

        // Test GET vBridge1
        result = getJsonResult(baseURL + tname1 + "/vbridges/" + bname1);
        Assert.assertEquals(200, httpResponseCode.intValue());
        jt = new JSONTokener(result);
        json = new JSONObject(jt);

        Assert.assertFalse(json.has("description"));
        Assert.assertEquals(ageinter1, json.getString("ageInterval"));

        // Test PUT vBridge1, setting description and ageInter (queryparameter is true)
        requestBody = "{\"description\":\"" + desc2 + "\", \"ageInterval\":\"" + ageinter4 + "\"}";
        result = getJsonResult(baseURL + tname1 + "/vbridges/" + bname1 + queryParameter, "PUT", requestBody);
        Assert.assertEquals(200, httpResponseCode.intValue());

        // Test GET vBridge1
        result = getJsonResult(baseURL + tname1 + "/vbridges/" + bname1);
        Assert.assertEquals(200, httpResponseCode.intValue());
        jt = new JSONTokener(result);
        json = new JSONObject(jt);

        Assert.assertEquals(desc2,json.getString("description"));
        Assert.assertEquals(ageinter4, json.getString("ageInterval"));

        // Test PUT vBridge1, setting {} (queryparameter is true)
        requestBody = "{}";
        result = getJsonResult(baseURL + tname1 + "/vbridges/" + bname1 + queryParameter, "PUT", requestBody);
        Assert.assertEquals(200, httpResponseCode.intValue());

        // Test GET vBridge1
        result = getJsonResult(baseURL + tname1 + "/vbridges/" + bname1);
        Assert.assertEquals(200, httpResponseCode.intValue());
        jt = new JSONTokener(result);
        json = new JSONObject(jt);

        Assert.assertFalse(json.has("description"));
        Assert.assertEquals("600", json.getString("ageInterval"));

        // Test PUT vBridge2 expecting not change (query parameter is false and requestBody is {})
        queryParameter = new QueryParameter("all", "false").getString();
        result = getJsonResult(baseURL + tname1 + "/vbridges/" + bname2 + queryParameter, "PUT", requestBody);
        Assert.assertEquals(200, httpResponseCode.intValue());

        // Test GET vBridge2
        result = getJsonResult(baseURL + tname1 + "/vbridges/" + bname2);
        Assert.assertEquals(200, httpResponseCode.intValue());
        jt = new JSONTokener(result);
        json = new JSONObject(jt);

        Assert.assertEquals(desc2,json.getString("description"));
        Assert.assertEquals(ageinter1, json.getString("ageInterval"));

        // Test PUT vBridge2, setting description (query parameter is false)
        requestBody = "{\"description\":\"" + desc1 + "\"}";
        result = getJsonResult(baseURL + tname1 + "/vbridges/" + bname2 + queryParameter, "PUT", requestBody);
        Assert.assertEquals(200, httpResponseCode.intValue());

        // Test GET vBridge2
        result = getJsonResult(baseURL + tname1 + "/vbridges/" + bname2);
        Assert.assertEquals(200, httpResponseCode.intValue());
        jt = new JSONTokener(result);
        json = new JSONObject(jt);

        Assert.assertEquals(desc1,json.getString("description"));
        Assert.assertEquals(ageinter1, json.getString("ageInterval"));

        // Test PUT vBridge2, setting ageInter (query parameter is false)
        requestBody = "{\"ageInterval\":\"" + ageinter2 + "\"}";
        result = getJsonResult(baseURL + tname1 + "/vbridges/" + bname2 + queryParameter, "PUT", requestBody);
        Assert.assertEquals(200, httpResponseCode.intValue());

        // Test GET vBridge2
        result = getJsonResult(baseURL + tname1 + "/vbridges/" + bname2);
        Assert.assertEquals(200, httpResponseCode.intValue());
        jt = new JSONTokener(result);
        json = new JSONObject(jt);

        Assert.assertEquals(desc1,json.getString("description"));
        Assert.assertEquals(ageinter2, json.getString("ageInterval"));

        // Test PUT vBridge2, setting description and ageInter (query parameter is false)
        requestBody = "{\"description\":\"" + desc3 + "\", \"ageInterval\":\"" + ageinter3 + "\"}";
        result = getJsonResult(baseURL + tname1 + "/vbridges/" + bname2 + queryParameter, "PUT", requestBody);
        Assert.assertEquals(200, httpResponseCode.intValue());

        // Test GET vBridge2
        result = getJsonResult(baseURL + tname1 + "/vbridges/" + bname2);
        Assert.assertEquals(200, httpResponseCode.intValue());
        jt = new JSONTokener(result);
        json = new JSONObject(jt);

        Assert.assertEquals(desc3,json.getString("description"));
        Assert.assertEquals("100", json.getString("ageInterval"));

        // Test PUT vBridge2, setting description and ageInter (query parameter is true)
        queryParameter = new QueryParameter("all", "true").getString();
        requestBody = "{\"description\":\"" + desc3 + "\", \"ageInterval\":\"" + ageinter3 + "\"}";
        result = getJsonResult(baseURL + tname1 + "/vbridges/" + bname2 + queryParameter, "PUT", requestBody);
        Assert.assertEquals(200, httpResponseCode.intValue());

        // Test GET vBridge2
        result = getJsonResult(baseURL + tname1 + "/vbridges/" + bname2);
        Assert.assertEquals(200, httpResponseCode.intValue());
        jt = new JSONTokener(result);
        json = new JSONObject(jt);

        Assert.assertEquals(desc3,json.getString("description"));
        Assert.assertEquals("600", json.getString("ageInterval"));

        // Test PUT vBridge1 expecting 400
        requestBody = "{\"ageInterval\":\"" + "ageinter" + "\"}";
        result = getJsonResult(baseURL +  tname1 + "/vbridges/" + bname1 + queryParameter, "PUT", requestBody);
        Assert.assertEquals(400, httpResponseCode.intValue());


        // Test PUT vBridge1 expecting 404, setting dummy_container
        requestBody = "{}";
        result = getJsonResult(url + cont_dummy + "/vtns/" + tname1 + "/vbridges/" + bname1 + queryParameter, "PUT", requestBody);
//        Assert.assertEquals(404, httpResponseCode.intValue());

        // Test PUT vBridge1 expecting 404, setting dummy vtn
        result = getJsonResult(baseURL +  tname_dummy + "/vbridges/" + bname1 + queryParameter, "PUT", requestBody);
        Assert.assertEquals(404, httpResponseCode.intValue());

        // Test PUT vBridge1 expecting 404, setting dummy vbridge
        result = getJsonResult(baseURL +  tname2 + "/vbridges/" + bname1 + queryParameter, "PUT", requestBody);
        Assert.assertEquals(404, httpResponseCode.intValue());

        // Test PUT vBridge1 expecting 415, setting ageinterval value to 0 (queryparameter is false)
        queryParameter = new QueryParameter("all", "false").getString();
        requestBody = "{\"description\":\"" + desc2 + "\", \"ageInterval\":\"" + ageinter0 + "\"}";
        result = getJsonResult(baseURL + tname1 + "/vbridges/" + bname1 + queryParameter, "PUT", requestBody);
        Assert.assertEquals(415, httpResponseCode.intValue());

        // Test PUT vBridge1 expecting 415, setting ageinterval value to 0 (queryparameter is true)
        queryParameter = new QueryParameter("all", "true").getString();
        requestBody = "{\"description\":\"" + desc2 + "\", \"ageInterval\":\"" + ageinter0 + "\"}";
        result = getJsonResult(baseURL + tname1 + "/vbridges/" + bname1 + queryParameter, "PUT", requestBody);
        Assert.assertEquals(415, httpResponseCode.intValue());

        // Test PUT vBridge1 expecting 415, setting ageinterval value to 1000000 (queryparameter is true)
        requestBody = "{\"description\":\"" + desc1 + "\", \"ageInterval\":\"" + ageinter_over + "\"}";
        result = getJsonResult(baseURL + tname1 + "/vbridges/" + bname1 + queryParameter, "PUT", requestBody);
        Assert.assertEquals(415, httpResponseCode.intValue());

        testVLANMappingDeleteAPI(tname1, bname1);
        testMacAddressAPI(tname1, bname1);
        testVBridgeInterfaceDeleteAPI(tname1, bname1);


        // Test DELETE vBridge expecting 404, setting dummy container
        result = getJsonResult(url + cont_dummy + "/vtns/" + tname1 + "/vbridges/" + bname1, "DELETE");
//        Assert.assertEquals(404, httpResponseCode.intValue());

        // Test DELETE vBridge expecting 404, setting dummy tenant
        result = getJsonResult(baseURL + tname_dummy + "/vbridges/" + bname1, "DELETE");
        Assert.assertEquals(404, httpResponseCode.intValue());

        // Test DELETE vBridge1
        result = getJsonResult(baseURL + tname1 + "/vbridges/" + bname1, "DELETE");
        Assert.assertEquals(200, httpResponseCode.intValue());

        // specify not supported Content-Type
        requestBody = "{}";
        result = getJsonResult(baseURL + tname1 + "/vbridges/" + bname1, "POST",
                               requestBody, "text/plain");
        Assert.assertEquals(415, httpResponseCode.intValue());

        requestBody = "{\"description\":\"test\"}";
        result = getJsonResult(baseURL + tname1 + "/vbridges/" + bname2 + queryParameter,
                               "PUT", requestBody, "text/plain");
        Assert.assertEquals(415, httpResponseCode.intValue());

        // auth fail
        requestBody = "{}";
        result = getJsonResult(baseURL + tname1 + "/vbridges/" + bname1, "POST",
                               requestBody, "application/json", false);
        Assert.assertEquals(401, httpResponseCode.intValue());

        requestBody = "{\"description\":\"test\"}";
        result = getJsonResult(baseURL + tname1 + "/vbridges/" + bname2 + queryParameter,
                               "PUT", requestBody, "application/json", false);
        Assert.assertEquals(401, httpResponseCode.intValue());

        result = getJsonResult(baseURL + tname2 + "/vbridges", "GET", null, "application/json",
                               false);
        Assert.assertEquals(401, httpResponseCode.intValue());

        result = getJsonResult(baseURL + tname2 + "/vbridges/" + bname2, "GET", null,
                               "application/json", false);
        Assert.assertEquals(401, httpResponseCode.intValue());

        result = getJsonResult(baseURL + tname1 + "/vbridges/" + bname2, "DELETE", null,
                               "application/json", false);
        Assert.assertEquals(401, httpResponseCode.intValue());

        // Test DELETE vBridge2
        result = getJsonResult(baseURL + tname1 + "/vbridges/" + bname2, "DELETE");
        Assert.assertEquals(200, httpResponseCode.intValue());

        // Test DELETE vBridge3
        result = getJsonResult(baseURL + tname1 + "/vbridges/" + bname3, "DELETE");
        Assert.assertEquals(200, httpResponseCode.intValue());

        // Test DELETE vBridge4
        result = getJsonResult(baseURL + tname1 + "/vbridges/" + bname4, "DELETE");
        Assert.assertEquals(200, httpResponseCode.intValue());

        // Test DELETE vBridge2 on other tenant
        result = getJsonResult(baseURL + tname2 + "/vbridges/" + bname2, "DELETE");
        Assert.assertEquals(200, httpResponseCode.intValue());

        // Test DELETE vBridge expecting 404
        result = getJsonResult(baseURL + tname1 + "/vbridges/" + bname1, "DELETE");
        Assert.assertEquals(404, httpResponseCode.intValue());

        // Test DELETE vBridge expecting 404, setting dummy tenant
        result = getJsonResult(baseURL + tname1 + "/vbridges/" + ebname, "DELETE");
        Assert.assertEquals(404, httpResponseCode.intValue());

        // Test GET vBridges in default container, expecting no results (tname1)
        result = getJsonResult(baseURL + tname1 + "/vbridges");
        Assert.assertEquals(200, httpResponseCode.intValue());
        jt = new JSONTokener(result);
        json = new JSONObject(jt);
        vBridgeArray = json.getJSONArray("vbridge");
        Assert.assertEquals(0, vBridgeArray.length());

        // Test GET vBridges in default container, expecting no results (tname2)
        result = getJsonResult(baseURL + tname2 + "/vbridges");
        Assert.assertEquals(200, httpResponseCode.intValue());
        jt = new JSONTokener(result);
        json = new JSONObject(jt);
        vBridgeArray = json.getJSONArray("vbridge");
        Assert.assertEquals(0, vBridgeArray.length());
    }

    /**
     * Test case for VBridgeInterface APIs.
     *
     * This method called by {@code testVBridge}.
     * This calls {@code testPortMappingAPI}.
     */
    private void testVBridgeInterfaceAPI(String tname1, String bname1, String bname2) throws JSONException {
        System.out.println("Starting vBridge Intergace JAXB client.");
        String url = VTN_BASE_URL;
        StringBuilder baseURL = new StringBuilder();
        baseURL.append(url);
        baseURL.append("default/vtns/");
        baseURL.append(tname1);
        baseURL.append("/vbridges/");

        StringBuilder baseURL2 = new StringBuilder();
        baseURL2.append(url);
        baseURL2.append("default/vtns/");
        baseURL2.append("testVtn_2");
        baseURL2.append("/vbridges/");

        String ifname = "testInterface0";
        String ifname1 = "testInterface";
        String ifname2 = "test_Interface2";
        String ifname3 = "testInterface3";
        String ifname4 = "4";
        String ifname5 = "testInterface5testinterface5tes";
        String ifname32 = "testInterface5testinterface5test";

        String bname_dummy = "bname_dummy";
        String tname_dummy = "tenant_dummy";
        String cont_dummy = "cont_dummy";

        String desc1 = "testDescription1";
        String desc2 = "t";
        String desc3 = String.format("%01000d", 1);


        // Test GET vBridge Interfaces in default container, expecting no results
        String result = getJsonResult(baseURL + bname1 + "/interfaces");
        Assert.assertEquals(200, httpResponseCode.intValue());
        JSONTokener jt = new JSONTokener(result);
        JSONObject json = new JSONObject(jt);
        JSONArray vbridgeifArray = json.getJSONArray("interface");
        Assert.assertEquals(0, vbridgeifArray.length());

        // Test POST vBridge Interface1 expecting 404
        String requestBody = "{}";
        result = getJsonResult(baseURL + bname_dummy + "/interfaces/" + ifname1, "POST" , requestBody);
        Assert.assertEquals(404, httpResponseCode.intValue());

        // Test POST vbridge interface expecting 201
        // setting vbridge Interface1
        requestBody = "{}";
        String requestUri = baseURL + bname1 + "/interfaces/" + ifname1;
        result = getJsonResult(requestUri, "POST", requestBody);
        Assert.assertEquals(201, httpResponseCode.intValue());
        Assert.assertEquals(requestUri, httpLocation);

        // Test POST vbridge interface expecting 409
        // setting vbridge Interface1
        result = getJsonResult(baseURL + bname1 + "/interfaces/" + ifname1, "POST", requestBody);
        Assert.assertEquals(409, httpResponseCode.intValue());

        // Test GET vbridge interface expecitng 404
        // setting dummy container
        result = getJsonResult(url + cont_dummy + "/vtns/" + tname1 + "/vbridges/" + bname1 + "/interfaces");
//        Assert.assertEquals(404, httpResponseCode.intValue());

        // setting dummy vtn
        result = getJsonResult(url + "default/vtns/" + tname_dummy + "/vbridges/" + bname1 + "/interfaces");
        Assert.assertEquals(404, httpResponseCode.intValue());

        // setting dummy vbridge
        result = getJsonResult(baseURL + bname_dummy +"/interfaces");
        Assert.assertEquals(404, httpResponseCode.intValue());

        // Test GET vBridge Interfaces in default container, expecting one result
        result = getJsonResult(baseURL + bname1 + "/interfaces");
        Assert.assertEquals(200, httpResponseCode.intValue());
        jt = new JSONTokener(result);
        json = new JSONObject(jt);
        vbridgeifArray = json.getJSONArray("interface");
        Assert.assertEquals(1, vbridgeifArray.length());

        // Test POST vbridge interface expecting 200
        // setting vbridge Interface2
        requestBody = "{\"description\":\"" + desc1 + "\", \"enabled\":true}";
        requestUri = baseURL + bname1 + "/interfaces/" + ifname2;
        result = getJsonResult(requestUri, "POST", requestBody);
        Assert.assertEquals(201, httpResponseCode.intValue());
        Assert.assertEquals(requestUri, httpLocation);

        // setting vbridge Interface3
        requestBody = "{\"description\":\"" + desc2 + "\", \"enabled\":true}";
        requestUri = baseURL + bname2 + "/interfaces/" + ifname3;
        result = getJsonResult(requestUri, "POST", requestBody);
        Assert.assertEquals(201, httpResponseCode.intValue());
        Assert.assertEquals(requestUri, httpLocation);

        testPortMappingAPI(tname1, bname1, bname2, ifname2, ifname3);

        // Test POST vBridge Interface2, for other tenant
        requestBody = "{\"description\":\"" + desc1 + "\", \"enabled\":true}";
        requestUri = baseURL2 + bname2 + "/interfaces/" + ifname2;
        result = getJsonResult(requestUri, "POST", requestBody);
        Assert.assertEquals(201, httpResponseCode.intValue());
        Assert.assertEquals(requestUri, httpLocation);

        // Test GET all vBridge Interfaces in default container
        result = getJsonResult(baseURL + bname1 + "/interfaces");
        jt = new JSONTokener(result);
        json = new JSONObject(jt);
        vbridgeifArray = json.getJSONArray("interface");
        JSONObject vbridgeif;

        Assert.assertEquals(200, httpResponseCode.intValue());
        Assert.assertEquals(2, vbridgeifArray.length());

        for (int i = 0; i < vbridgeifArray.length(); i++) {
            vbridgeif = vbridgeifArray.getJSONObject(i);
            if (vbridgeif.getString("name").equals(ifname1)) {
                Assert.assertFalse(vbridgeif.has("description"));

                try {
                    Assert.assertEquals("true", vbridgeif.getString("enabled"));
                } catch (JSONException expected) {
                    assertThat(expected.getMessage(), is("JSONObject[\"enabled\"] not found."));
                }
                Assert.assertEquals("-1", vbridgeif.getString("state"));
                Assert.assertEquals("-1", vbridgeif.getString("entityState"));
            } else if (vbridgeif.getString("name").equals(ifname2)) {
                Assert.assertEquals(desc1, vbridgeif.getString("description"));
                Assert.assertEquals("true", vbridgeif.getString("enabled"));
                Assert.assertEquals("0", vbridgeif.getString("state"));
                Assert.assertEquals("-1", vbridgeif.getString("entityState"));
            } else if (vbridgeif.getString("name").equals(ifname3)) {
                Assert.assertEquals(desc2, vbridgeif.getString("description"));
                Assert.assertEquals("true", vbridgeif.getString("enabled"));
                Assert.assertEquals("-1", vbridgeif.getString("state"));
                Assert.assertEquals("-1", vbridgeif.getString("entityState"));
            }
            else {
                // Unexpected vBridge Interface name
                Assert.assertTrue(false);
            }
        }

        // Test POST vBridge Interface4
        requestBody = "{\"enabled\":false}";
        requestUri = baseURL + bname1 + "/interfaces/" + ifname4;
        result = getJsonResult(requestUri, "POST", requestBody);
        Assert.assertEquals(201, httpResponseCode.intValue());
        Assert.assertEquals(requestUri, httpLocation);

        // Test GET vBridge Interface4
        result = getJsonResult(baseURL + bname1 + "/interfaces/" + ifname4);
        Assert.assertEquals(200, httpResponseCode.intValue());

        jt = new JSONTokener(result);
        json = new JSONObject(jt);

        Assert.assertFalse(json.has("description"));
        Assert.assertEquals("false", json.getString("enabled"));

        // Test POST vBridge Interface5
        requestBody = "{\"description\":\"" + desc3 + "\"}";
        requestUri = baseURL + bname1 + "/interfaces/" + ifname5;
        result = getJsonResult(requestUri, "POST", requestBody);
        Assert.assertEquals(201, httpResponseCode.intValue());
        Assert.assertEquals(requestUri, httpLocation);

        // Test POST vBridge Interface expecting 400, setting invalid value for ageInterval
        requestBody = "{\"description\":\"" + desc2 + "\", \"enabled\":enabled}";
        requestUri = baseURL + bname1 + "/interfaces/" + ifname;
        result = getJsonResult(requestUri, "POST", requestBody);
        Assert.assertEquals(400, httpResponseCode.intValue());


        // Test POST vBridge Interface expecting 404, setting dummy container
        result = getJsonResult(url + cont_dummy + "/vtns/" + tname1 + "/vbridges/" + bname1 + "/interfaces/" + ifname,
                "POST", requestBody);
//        Assert.assertEquals(404, httpResponseCode.intValue());

        // Test POST vBridge Interface expecting 404, setting dummy tenant
        requestBody = "{}";
        result = getJsonResult(url + "default/vtns/" + tname_dummy + "/vbridges/"
                + bname1 + "/interfaces/" + ifname, "POST", requestBody);
        Assert.assertEquals(404, httpResponseCode.intValue());

        // Test POST vBridge Interface expecting 404, setting vbridge that don't exist
        result = getJsonResult(baseURL + bname_dummy + "/interfaces/" + ifname, "POST", requestBody);
        Assert.assertEquals(404, httpResponseCode.intValue());

        // Test POST vBridge Interface expecting 405, setting "" to vbridgeIF name
        result = getJsonResult(baseURL + bname_dummy + "/interfaces/" + "", "POST", requestBody);
        Assert.assertEquals(405, httpResponseCode.intValue());

        // Test POST vBridge Interface expecting 415, setting 32 characters to vbridgeIF name
        result = getJsonResult(baseURL + bname1 + "/interfaces/" + ifname32, "POST", requestBody);
        Assert.assertEquals(415, httpResponseCode.intValue());

        // Test POST vBridge Interface expecting 415, setting "_" to first letter to vbridgeIF name
        result = getJsonResult(baseURL + bname1 + "/interfaces/" + "_ifname", "POST", requestBody);
        Assert.assertEquals(415, httpResponseCode.intValue());

        // Test POST vBridge Interface expecting 415, setting to vbridgeIF name including "@"
        result = getJsonResult(baseURL + bname1 + "/interfaces/" + "if@name", "POST", requestBody);
        Assert.assertEquals(415, httpResponseCode.intValue());

        // Test GET vBridge Interface expecting 404, setting vtn that don't exist
        result = getJsonResult(url + "default/vtns/" + tname_dummy + "/vbridges/" + bname1 + "/interfaces/" + ifname1);
        Assert.assertEquals(404, httpResponseCode.intValue());

        // Test GET vBridge Interface expecting 404, setting vbridge that don't exits
        result = getJsonResult(baseURL + bname_dummy + "/interfaces/" + ifname1);
        Assert.assertEquals(404, httpResponseCode.intValue());

        // Test GET vBridge Interface expecting 404, setting vbridgeIF that don't exits
        result = getJsonResult(baseURL + bname_dummy + "/interfaces/" + ifname);
        Assert.assertEquals(404, httpResponseCode.intValue());

        // Test GET vBridge Interface2
        result = getJsonResult(baseURL + bname1 + "/interfaces/" + ifname2);
        jt = new JSONTokener(result);
        json = new JSONObject(jt);

        Assert.assertEquals(200, httpResponseCode.intValue());
        Assert.assertEquals(ifname2, json.getString("name"));
        Assert.assertEquals(desc1, json.getString("description"));
        Assert.assertEquals("true", json.getString("enabled"));
        Assert.assertEquals("0", json.getString("state"));
        Assert.assertEquals("-1", json.getString("entityState"));

        // Test PUT vBridge interface1
        requestBody = "{\"description\":\"" + desc1 + "\", \"enabled\":\"true\"}";
        String queryParameter = new QueryParameter("all", "true").getString();
        result = getJsonResult(baseURL + bname1 + "/interfaces/" + ifname1 + queryParameter, "PUT", requestBody);
        Assert.assertEquals(200, httpResponseCode.intValue());

        // Test GET vBridge Interface1
        result = getJsonResult(baseURL + bname1 + "/interfaces/" + ifname1);
        jt = new JSONTokener(result);
        json = new JSONObject(jt);

        Assert.assertEquals(200, httpResponseCode.intValue());
        Assert.assertEquals(desc1, json.getString("description"));
        Assert.assertEquals("true", json.getString("enabled"));

        // Test PUT vBridge interface1, setting description (queryparameter is true)
        requestBody = "{\"description\":\"" + desc2 + "\"}";
        result = getJsonResult(baseURL + bname1 + "/interfaces/" + ifname1 + queryParameter, "PUT", requestBody);
        Assert.assertEquals(200, httpResponseCode.intValue());

        // Test GET vBridge Interface1
        result = getJsonResult(baseURL + bname1 + "/interfaces/" + ifname1);
        jt = new JSONTokener(result);
        json = new JSONObject(jt);

        Assert.assertEquals(200, httpResponseCode.intValue());
        Assert.assertEquals(desc2, json.getString("description"));
        Assert.assertEquals("true", json.getString("enabled"));

        // Test PUT vBridge interface1, setting enabled (queryparameter is true)
        requestBody = "{\"enabled\":\"false\"}";
        result = getJsonResult(baseURL + bname1 + "/interfaces/" + ifname1 + queryParameter, "PUT", requestBody);
        Assert.assertEquals(200, httpResponseCode.intValue());

        // Test GET vBridge Interface1
        result = getJsonResult(baseURL + bname1 + "/interfaces/" + ifname1);
        jt = new JSONTokener(result);
        json = new JSONObject(jt);

        Assert.assertEquals(200, httpResponseCode.intValue());
        Assert.assertFalse(json.has("description"));
        Assert.assertEquals("false", json.getString("enabled"));

        // Test PUT vBridge interface1, setting description and enabled
        requestBody = "{\"description\":\"" + desc3 + "\", \"enabled\":\"false\"}";
        result = getJsonResult(baseURL + bname1 + "/interfaces/" + ifname1 + queryParameter, "PUT", requestBody);
        Assert.assertEquals(200, httpResponseCode.intValue());

        // Test GET vBridge Interface1
        result = getJsonResult(baseURL + bname1 + "/interfaces/" + ifname1);
        jt = new JSONTokener(result);
        json = new JSONObject(jt);

        Assert.assertEquals(200, httpResponseCode.intValue());
        Assert.assertEquals(desc3, json.getString("description"));
        Assert.assertEquals("false", json.getString("enabled"));

        // Test PUT vBridge interface1, setting {}
        requestBody = "{}";
        result = getJsonResult(baseURL + bname1 + "/interfaces/" + ifname1 + queryParameter, "PUT", requestBody);
        Assert.assertEquals(200, httpResponseCode.intValue());

        // Test PUT vBridge Interface2 expecting not change
        queryParameter = new QueryParameter("all", "false").getString();
        result = getJsonResult(baseURL + bname1 +"/interfaces/" + ifname2 + queryParameter, "PUT", requestBody);
        Assert.assertEquals(200, httpResponseCode.intValue());

        // Test GET vBridge Interface2
        result = getJsonResult(baseURL + bname1 + "/interfaces/" + ifname2);
        jt = new JSONTokener(result);
        json = new JSONObject(jt);

        Assert.assertEquals(200, httpResponseCode.intValue());
        Assert.assertEquals(desc1, json.getString("description"));
        Assert.assertEquals("true", json.getString("enabled"));

        // Test PUT vBridge Interface2, setting enabled
        requestBody = "{\"enabled\":false}";
        result = getJsonResult(baseURL + bname1 +"/interfaces/" + ifname2 + queryParameter, "PUT", requestBody);
        Assert.assertEquals(200, httpResponseCode.intValue());

        // Test GET vBridge Interface2
        result = getJsonResult(baseURL + bname1 + "/interfaces/" + ifname2);
        jt = new JSONTokener(result);
        json = new JSONObject(jt);

        Assert.assertEquals(200, httpResponseCode.intValue());
        Assert.assertEquals(desc1, json.getString("description"));
        Assert.assertEquals("false", json.getString("enabled"));

        // Test PUT vBridge Interface2, setting enabled
        requestBody = "{\"description\":\"" + desc3 + "\"}";
        result = getJsonResult(baseURL + bname1 +"/interfaces/" + ifname2 + queryParameter, "PUT", requestBody);
        Assert.assertEquals(200, httpResponseCode.intValue());

        // Test GET vBridge Interface2
        result = getJsonResult(baseURL + bname1 + "/interfaces/" + ifname2);
        jt = new JSONTokener(result);
        json = new JSONObject(jt);

        Assert.assertEquals(200, httpResponseCode.intValue());
        Assert.assertEquals(desc3, json.getString("description"));
        Assert.assertEquals("false", json.getString("enabled"));

        // Test PUT vBridge Interface2, setting enabled
        requestBody = "{\"description\":\"" + desc1 + "\", \"enabled\":\"true\"}";
        result = getJsonResult(baseURL + bname1 +"/interfaces/" + ifname2 + queryParameter, "PUT", requestBody);
        Assert.assertEquals(200, httpResponseCode.intValue());

        // Test GET vBridge Interface2
        result = getJsonResult(baseURL + bname1 + "/interfaces/" + ifname2);
        jt = new JSONTokener(result);
        json = new JSONObject(jt);

        Assert.assertEquals(200, httpResponseCode.intValue());
        Assert.assertEquals(desc1, json.getString("description"));
        Assert.assertEquals("true", json.getString("enabled"));


        // Test PUT vBridge Interface expecting 404
        // setting dummy vtn
        result = getJsonResult(url + "default/vtns/" + tname_dummy + "/vbridges/" + bname1 + "/interfaces/"
                + ifname1 + queryParameter, "PUT", requestBody);
        Assert.assertEquals(404, httpResponseCode.intValue());

        // setting dummy vbridge
        result = getJsonResult(baseURL +  bname_dummy + "/interfaces/"
                + ifname1 + queryParameter, "PUT", requestBody);
        Assert.assertEquals(404, httpResponseCode.intValue());

        // setting  dummy vbridgeIF
        result = getJsonResult(baseURL +  bname1 + "/interfaces/" + ifname + queryParameter,
                "PUT", requestBody);
        Assert.assertEquals(404, httpResponseCode.intValue());


        // Test GET all vBridge Interfaces in default container
        result = getJsonResult(baseURL + bname1 + "/interfaces");
        jt = new JSONTokener(result);
        json = new JSONObject(jt);
        vbridgeifArray = json.getJSONArray("interface");

        Assert.assertEquals(200, httpResponseCode.intValue());
        Assert.assertEquals(4, vbridgeifArray.length());


        // Test DELETE expecting 200
        // delete vBridge Interface2 on other tenant
        result = getJsonResult(baseURL2 + bname2 + "/interfaces/" + ifname2, "DELETE");
        Assert.assertEquals(200, httpResponseCode.intValue());

        // specify not supported Content-Type
        requestBody = "{}";
        result = getJsonResult(baseURL + bname2 + "/interfaces/" + ifname2,
                               "POST", requestBody, "text/plain");
        Assert.assertEquals(415, httpResponseCode.intValue());

        requestBody = "{}";
        result = getJsonResult(baseURL + bname2 + "/interfaces/" + ifname3 + queryParameter,
                               "PUT", requestBody, "text/plain");
        Assert.assertEquals(415, httpResponseCode.intValue());

        // auth fail
        result = getJsonResult(baseURL + bname1 + "/interfaces", "GET", null,
                               "application/json", false);
        Assert.assertEquals(401, httpResponseCode.intValue());

        result = getJsonResult(baseURL + bname2 + "/interfaces/" + ifname3 , "GET", null,
                               "application/json", false);
        Assert.assertEquals(401, httpResponseCode.intValue());

        requestBody = "{}";
        result = getJsonResult(baseURL + bname2 + "/interfaces/" + ifname2,
                               "POST", requestBody, "application/json", false);
        Assert.assertEquals(401, httpResponseCode.intValue());

        requestBody = "{}";
        result = getJsonResult(baseURL + bname2 + "/interfaces/" + ifname3 + queryParameter,
                               "PUT", requestBody, "application/json", false);
        Assert.assertEquals(401, httpResponseCode.intValue());

        result = getJsonResult(baseURL + bname2 + "/interfaces/" + ifname3, "DELETE", null,
                               "application/json", false);
        Assert.assertEquals(401, httpResponseCode.intValue());


        // delete  vBridge Interface3
        result = getJsonResult(baseURL + bname2 + "/interfaces/" + ifname3, "DELETE");
        Assert.assertEquals(200, httpResponseCode.intValue());

        // delete vBridge Interface4
        result = getJsonResult(baseURL + bname1 + "/interfaces/" + ifname4, "DELETE");
        Assert.assertEquals(200, httpResponseCode.intValue());

        // delete vBridge Interface5
        result = getJsonResult(baseURL + bname1 + "/interfaces/" + ifname5, "DELETE");
        Assert.assertEquals(200, httpResponseCode.intValue());
    }

    /**
     * Test case for VBridgeInterface delete APIs.
     *
     * This method is called by {@code testVBridge}.
     */
    private void testVBridgeInterfaceDeleteAPI(String tname, String bname) throws JSONException {
        System.out.println("Starting delete vBridge Intergace JAXB client.");
        String url = VTN_BASE_URL;
        StringBuilder baseURL = new StringBuilder();
        baseURL.append(url);
        baseURL.append("default/vtns/");
        baseURL.append(tname);
        baseURL.append("/vbridges/");

        String ifname_dummy = "fname_dummy";
        String tname_dummy = "tname_dummy";
        String ifname1 = "testInterface";
        String ifname2 = "test_Interface2";
        String bname_dummy = "bname_dummy";

        testPortMappingDeleteAPI(tname, bname, ifname1);

        // Test DELETE vbridge interface expecting 404
        // setting dummy vtn
        String result = getJsonResult(url + "default/vtns/" + tname_dummy + "/vbridges/" + bname + "/interfaces/" + ifname1, "DELETE");
        Assert.assertEquals(404, httpResponseCode.intValue());

        // setting dummy vbridge
        result = getJsonResult(baseURL + bname_dummy + "/interfaces/" + ifname1, "DELETE");
        Assert.assertEquals(404, httpResponseCode.intValue());

        // setting dummy vbridge interface
        result = getJsonResult(baseURL + bname + "/interfaces/" + ifname_dummy, "DELETE");
        Assert.assertEquals(404, httpResponseCode.intValue());


        // Test DELETE vbridge interface expecting 404
        // setting vBridge Interface1
        result = getJsonResult(baseURL + bname + "/interfaces/" + ifname1, "DELETE");
        Assert.assertEquals(200, httpResponseCode.intValue());

        // setting vBridge Interface2
        result = getJsonResult(baseURL + bname + "/interfaces/" + ifname2, "DELETE");
        Assert.assertEquals(200, httpResponseCode.intValue());


        // Test DELETE vbridge interface expecting 404
        // setting deleted vbridge interface1
        result = getJsonResult(baseURL + bname + "/interfaces/" + ifname1, "DELETE");
        Assert.assertEquals(404, httpResponseCode.intValue());


        // Test GET all vBridge Interfaces in default container
        result = getJsonResult(baseURL + bname + "/interfaces");
        JSONTokener jt = new JSONTokener(result);
        JSONObject json = new JSONObject(jt);
        JSONArray vbridgeifArray = json.getJSONArray("interface");

        Assert.assertEquals(200, httpResponseCode.intValue());
        Assert.assertEquals(0, vbridgeifArray.length());

    }

    /**
     * Test case for Port Mapping APIs.
     *
     * This method is called by {@code testVBridgeInterfaceAPI.}
     */
    private void testPortMappingAPI(String tname, String bname, String bname2, String ifname, String ifname2) throws JSONException {
        System.out.println("Starting Port Mapping JAXB client.");
        String url = VTN_BASE_URL;
        StringBuilder baseURL = new StringBuilder();
        baseURL.append(url);
        baseURL.append("default/vtns/");
        baseURL.append(tname);
        baseURL.append("/vbridges/");
        baseURL.append(bname);
        baseURL.append("/interfaces/");

        String vlan0 = "0";
        String vlan1 = "100";
        String vlan2 = "4095";
        String vlan_over = "4096";
        String vlan_negative = "-10";

        String pname = "testPortname";

        String ifname_dummy = "ifname_dummy";
        String tenant_dummy = "tenant_dummy";
        String bname_dummy = "bname_dummy";

        String nodeid = "00:00:00:00:00:00:00:01";
        String nodeType = "OF";
        String portnum = "1";

        String test = "ERROR_TEST";

        // Tset GET PortMapping expecting 204
        // Test GET PortMapping in default container, expecting no results
        String result = getJsonResult(baseURL + ifname + "/portmap");
        Assert.assertEquals(204, httpResponseCode.intValue());
        Assert.assertEquals("", result);


        // Test PUT PortMapping expecting 400
        // setting invalid value to requestBody
        String requestBody = "{\"description\":\", \"enabled\":\"true\"}";
        result = getJsonResult(baseURL + ifname + "/portmap/", "PUT", requestBody);
        Assert.assertEquals(400, httpResponseCode.intValue());


        // Test PUT PortMapping expecting 415
        // setting over 4095 to vlan
        requestBody = "{\"vlan\":" + vlan_over + ", \"node\":{\"type\":\""+ nodeType + "\", \"id\":\""
                + nodeid + "\"}, \"port\":{\"name\":\"" + pname
                + "\", \"type\":\"" + nodeType + "\", \"id\":\"" + portnum + "\"}}";
        result = getJsonResult(baseURL + ifname + "/portmap/", "PUT", requestBody);
        Assert.assertEquals(415, httpResponseCode.intValue());

        // setting negative value to vlan
        requestBody = "{\"vlan\":" + vlan_negative + ", \"node\":{\"type\":\""+ nodeType + "\", \"id\":\""
                + nodeid + "\"}, \"port\":{\"name\":\"" + pname
                + "\", \"type\":\"" + nodeType + "\", \"id\":\"" + portnum + "\"}}";
        result = getJsonResult(baseURL + ifname + "/portmap/", "PUT", requestBody);
        Assert.assertEquals(415, httpResponseCode.intValue());

        // setting invalid type to node type
        requestBody = "{\"vlan\":" + vlan1 + ", \"node\":{\"type\":\""+ test + "\", \"id\":\""
                + nodeid + "\"}, \"port\":{\"name\":\"" + pname
                + "\", \"type\":\"" + nodeType + "\", \"id\":\"" + portnum + "\"}}";
        result = getJsonResult(baseURL + ifname + "/portmap/", "PUT", requestBody);
        Assert.assertEquals(415, httpResponseCode.intValue());

        // setting requestBody without all node's elements
        requestBody = "{\"vlan\":" + vlan1 + ", \"port\":{\"name\":\"" + pname
                + "\", \"type\":\"" + nodeType + "\", \"id\":\"" + portnum + "\"}}";
        result = getJsonResult(baseURL + ifname + "/portmap/", "PUT", requestBody);
        Assert.assertEquals(415, httpResponseCode.intValue());

        // setting requestBody without all port's elements
        requestBody = "{\"vlan\":" + vlan1 + ", \"node\":{\"type\":\""+ nodeType + "\", \"id\":\""
                + nodeid + "\"}}";
        result = getJsonResult(baseURL + ifname + "/portmap/", "PUT", requestBody);
        Assert.assertEquals(415, httpResponseCode.intValue());

        // setting port elements without name and type
        requestBody = "{\"vlan\":" + vlan1 + ", \"node\":{\"type\":\""+ nodeType + "\", \"id\":\""
                + nodeid + "\"}, \"port\":{\"id\":\"" + portnum + "\"}}";
        result = getJsonResult(baseURL + ifname + "/portmap/", "PUT", requestBody);
        Assert.assertEquals(415, httpResponseCode.intValue());

        // setting port elements without name and id
        requestBody = "{\"vlan\":" + vlan1 + ", \"node\":{\"type\":\""+ nodeType + "\", \"id\":\""
                + nodeid + "\"}, \"port\":{\"type\":\"" + nodeType + "\"}}";
        result = getJsonResult(baseURL + ifname + "/portmap/", "PUT", requestBody);
        Assert.assertEquals(415, httpResponseCode.intValue());

        // setting invalid type to port name
        requestBody = "{\"vlan\":" + vlan1 + ", \"node\":{\"type\":\""+ nodeType + "\", \"id\":\""
                + nodeid + "\"}, \"port\":{\"name\":\"" + ""
                + "\", \"type\":\"" + nodeType + "\", \"id\":\"" + portnum + "\"}}";
        result = getJsonResult(baseURL + ifname + "/portmap/", "PUT", requestBody);
        Assert.assertEquals(415, httpResponseCode.intValue());

        // Test PUT PortMapping for other vbridge and except node type
        requestBody = "{\"vlan\":" + vlan0 + ", \"node\":{\"id\":\""
                + nodeid + "\"}, \"port\":{\"name\":\"" + pname
                + "\", \"type\":\"" + nodeType + "\", \"id\":\"" + portnum + "\"}}";
        result = getJsonResult(url + "default/vtns/" + tname +"/vbridges/" + bname2 + "/interfaces/"
                + ifname2 + "/portmap/", "PUT", requestBody);
        Assert.assertEquals(415, httpResponseCode.intValue());

        // Test PUT PortMapping for other vbridge and except node id
        requestBody = "{\"vlan\":" + vlan1 + ", \"node\":{\"type\":\"" + nodeType + "\"}, \"port\":{\"name\":\"" + pname
                + "\", \"type\":\"" + nodeType + "\", \"id\":\"" + portnum + "\"}}";
        result = getJsonResult(url + "default/vtns/" + tname +"/vbridges/" + bname2 + "/interfaces/"
                + ifname2 + "/portmap/", "PUT", requestBody);
        Assert.assertEquals(415, httpResponseCode.intValue());

        // Test PUT PortMapping for other vbridge and except port type
        requestBody = "{\"vlan\":" + vlan1 + ", \"node\":{\"type\":\"" + nodeType + "\", \"id\":\""
                + nodeid + "\"}, \"port\":{\"name\":\"" + pname + "\", \"id\":\"" + portnum + "\"}}";
        result = getJsonResult(url + "default/vtns/" + tname +"/vbridges/" + bname2 + "/interfaces/"
                + ifname2 + "/portmap/", "PUT", requestBody);
        Assert.assertEquals(415, httpResponseCode.intValue());

        // Test PUT PortMapping for other vbridge and except port id
        requestBody = "{\"vlan\":" + vlan1 + ", \"node\":{\"type\":\"" + nodeType + "\", \"id\":\""
                + nodeid + "\"}, \"port\":{\"name\":\"" + pname
                + "\", \"type\":\"" + nodeType + "\"}}";
        result = getJsonResult(url + "default/vtns/" + tname +"/vbridges/" + bname2 + "/interfaces/"
                + ifname2 + "/portmap/", "PUT", requestBody);
        Assert.assertEquals(415, httpResponseCode.intValue());


        // Test PUT PortMapping 400
        // setting invalid value to node id
        requestBody = "{\"vlan\":" + vlan1 + ", \"node\":{\"type\":\""+ nodeType + "\", \"id\":\""
                + test + "\"}, \"port\":{\"name\":\"" + pname
                + "\", \"type\":\"" + nodeType + "\", \"id\":\"" + portnum + "\"}}";
        result = getJsonResult(baseURL + ifname + "/portmap/", "PUT", requestBody);
        Assert.assertEquals(400, httpResponseCode.intValue());

        // Test PUT PortMappint expecting 404
        // setting dummy vtn
        requestBody = "{\"vlan\":" + vlan1 + ", \"node\":{\"type\":\"" + nodeType + "\", \"id\":\""
                + nodeid + "\"}, \"port\":{\"name\":\"" + pname
                + "\", \"type\":\"" + nodeType + "\", \"id\":\"" + portnum + "\"}}";
        result = getJsonResult(url + "default/vtns/" + tenant_dummy + "/vbridges/" + bname + "/interfaces/"
                + ifname + "/portmap/", "PUT", requestBody);
        Assert.assertEquals(404, httpResponseCode.intValue());

        // setting dummy vBridge
        requestBody = "{\"vlan\":" + vlan1 + ", \"node\":{\"type\":\"" + nodeType + "\", \"id\":\""
                + nodeid + "\"}, \"port\":{\"name\":\"" + pname
                + "\", \"type\":\"" + nodeType + "\", \"id\":\"" + portnum + "\"}}";
        result = getJsonResult(url + "default/vtns/" + tenant_dummy + "/vbridges/"
                + bname_dummy + "/interfaces/" + ifname + "/portmap/", "PUT", requestBody);
        Assert.assertEquals(404, httpResponseCode.intValue());

        // setting dummy vBridge interface
        requestBody = "{\"vlan\":" + vlan1 + ", \"node\":{\"type\":\"" + nodeType + "\", \"id\":\""
                + nodeid + "\"}, \"port\":{\"name\":\"" + pname
                + "\", \"type\":\"" + nodeType + "\", \"id\":\"" + portnum + "\"}}";
        result = getJsonResult(baseURL + ifname_dummy + "/portmap/", "PUT", requestBody);
        Assert.assertEquals(404, httpResponseCode.intValue());

        // specfiy not supported Content-Type
        result = getJsonResult(baseURL + ifname + "/portmap/", "PUT", requestBody, "text/plain");
        Assert.assertEquals(415, httpResponseCode.intValue());

        // auth fail
        result = getJsonResult(baseURL + ifname + "/portmap/", "PUT", requestBody,
                               "application/json", false);
        Assert.assertEquals(401, httpResponseCode.intValue());

        result = getJsonResult(baseURL + ifname + "/portmap", "GET", null,
                               "application/json", false);
        Assert.assertEquals(401, httpResponseCode.intValue());


        // Test PUT PortMapping expecting 200
        // Test PUT PortMapping
        result = getJsonResult(baseURL + ifname + "/portmap/", "PUT", requestBody);
        Assert.assertEquals(200, httpResponseCode.intValue());

        // setting port element without port id and  port type
        requestBody = "{\"vlan\":" + vlan1 + ", \"node\":{\"type\":\""+ nodeType + "\", \"id\":\""
                + nodeid + "\"}, \"port\":{\"name\":\"" + pname + "\"}}";
        result = getJsonResult(baseURL + ifname + "/portmap/", "PUT", requestBody);
        Assert.assertEquals(200, httpResponseCode.intValue());

        // Test PUT PortMapping, change vlan value
        requestBody = "{\"vlan\":" + vlan2 + ", \"node\":{\"type\":\"" + nodeType + "\", \"id\":\""
                + nodeid + "\"}, \"port\":{\"name\":\"" + pname
                + "\", \"type\":\"" + nodeType + "\", \"id\":\"" + portnum + "\"}}";
        result = getJsonResult(baseURL + ifname + "/portmap/", "PUT", requestBody);
        Assert.assertEquals(200, httpResponseCode.intValue());


        // Test PUT PortMapping expecting 409
        // Test PUT PortMapping for other vbridge
//        requestBody = "{\"vlan\":" + vlan2 + ", \"node\":{\"type\":\"" + nodeType + "\", \"id\":\""
//                + nodeid + "\"}, \"port\":{\"name\":\"" + pname
//                + "\", \"type\":\"" + nodeType + "\", \"id\":\"" + portnum + "\"}}";
//        result = getJsonResult(url + "default/vtns/" + tname +"/vbridges/" + bname2 + "/interfaces/"
//                + ifname2 + "/portmap/", "PUT", requestBody);
//        Assert.assertEquals(409, httpResponseCode.intValue());

        // Test PUT PortMapping expecting 200
        // Test PUT PortMapping for other vbridge and except port name
        requestBody = "{\"vlan\":" + vlan1 + ", \"node\":{\"type\":\"" + nodeType + "\", \"id\":\""
                + nodeid + "\"}, \"port\":{\"type\":\"" + nodeType + "\", \"id\":\"" + portnum + "\"}}";
        result = getJsonResult(url + "default/vtns/" + tname +"/vbridges/" + bname2 + "/interfaces/"
                + ifname2 + "/portmap/", "PUT", requestBody);
        Assert.assertEquals(200, httpResponseCode.intValue());

        // Test PUT PortMapping, except vlan
        requestBody = "{\"node\":{\"type\":\"" + nodeType + "\", \"id\":\""
                + nodeid + "\"}, \"port\":{\"name\":\"" + pname
                + "\", \"type\":\"" + nodeType + "\", \"id\":\"" + portnum + "\"}}";
        result = getJsonResult(url + "default/vtns/" + tname +"/vbridges/" + bname2 + "/interfaces/"
                + ifname2 + "/portmap/", "PUT", requestBody);
        Assert.assertEquals(200, httpResponseCode.intValue());

        // Tset GET PortMapping from bname2
        result = getJsonResult(url + "default/vtns/" + tname +"/vbridges/" + bname2 + "/interfaces/"
                + ifname2 + "/portmap");
        Assert.assertEquals(200, httpResponseCode.intValue());
        JSONTokener jt = new JSONTokener(result);
        JSONObject json = new JSONObject(jt);

        Assert.assertEquals(vlan0, json.getString("vlan"));


        // Test GET PortMapping expecting 404
        // setting dummy vtn
        result = getJsonResult(url + "default/vtns/" + tenant_dummy + "/vbridges/" + bname + "/interfaces/"
                + ifname + "/portmap");
        Assert.assertEquals(404, httpResponseCode.intValue());

        // setting dummy vbridge
        result = getJsonResult(url + "default/vtns/" + tenant_dummy + "/vbridges/"
                + bname_dummy + "/interfaces/" + ifname + "/portmap");
        Assert.assertEquals(404, httpResponseCode.intValue());

        // setting dummy vbridge interface
        result = getJsonResult(baseURL + ifname_dummy + "/portmap");
        Assert.assertEquals(404, httpResponseCode.intValue());

        // Test GET PortMapping expecting 200
        // Test GET PortMapping
        result = getJsonResult(baseURL + ifname + "/portmap");
        Assert.assertEquals(200, httpResponseCode.intValue());
        jt = new JSONTokener(result);
        json = new JSONObject(jt);
        JSONObject nodeinfo = json.getJSONObject("node");
        JSONObject portinfo = json.getJSONObject("port");

        Assert.assertEquals(vlan2, json.getString("vlan"));
        Assert.assertEquals(nodeType, nodeinfo.getString("type"));
        Assert.assertEquals(nodeid, nodeinfo.getString("id"));
        Assert.assertEquals(pname, portinfo.getString("name"));
        Assert.assertEquals(nodeType, portinfo.getString("type"));
        Assert.assertEquals(portnum, portinfo.getString("id"));

        if (!portinfo.getString("type").equals(nodeType)) {
            JSONObject mapinfo = json.getJSONObject("mapped");
            Assert.assertEquals(nodeType, mapinfo.getString("type"));
            Assert.assertEquals(portnum, mapinfo.getString("id"));
        }
    }

    /**
     * Test case for Port Mapping delete APIs.
     *
     * This method is called by {@code testVBridgeInterfaceDeleteAPI}.
     */
    private void testPortMappingDeleteAPI(String tname, String bname, String ifname) throws JSONException {
        System.out.println("Starting delete Port Mapping JAXB client.");
        String url = VTN_BASE_URL;
        StringBuilder baseURL = new StringBuilder();
        baseURL.append(url);
        baseURL.append("default/vtns/");
        baseURL.append(tname);
        baseURL.append("/vbridges/");
        baseURL.append(bname);
        baseURL.append("/interfaces/");

        String tname_dummy = "tname_dummy";
        String bname_dummy = "bname_dummy";
        String ifname_dummy = "ifname_dummy";

        // Test DELETE PortMapping expecting 404
        // setting dummy vtn interface
        String result = getJsonResult(url + "default/vtns/" + tname_dummy + "/vbridges/"
                + bname +  "/interfaces/" + ifname_dummy + "/portmap", "DELETE");
        Assert.assertEquals(404, httpResponseCode.intValue());

        // setting dummy vbridge
        result = getJsonResult(url + "default/vtns/" + tname + "/vbridges/"
                + bname_dummy +  "/interfaces/" + ifname_dummy + "/portmap", "DELETE");
        Assert.assertEquals(404, httpResponseCode.intValue());

        // setting dummy vbridge interface
        result = getJsonResult(baseURL + ifname_dummy + "/portmap", "DELETE");
        Assert.assertEquals(404, httpResponseCode.intValue());

        // auth fail
        result = getJsonResult(baseURL + ifname + "/portmap", "DELETE", null,
                               "application/json", false);
        Assert.assertEquals(401, httpResponseCode.intValue());

        // Test DELETE PortMapping
        result = getJsonResult(baseURL + ifname + "/portmap", "DELETE");
        Assert.assertEquals(200, httpResponseCode.intValue());

        // Test DELETE PortMapping, setting deleted portMapping
        result = getJsonResult(baseURL + ifname + "/portmap", "DELETE");
        Assert.assertEquals(200, httpResponseCode.intValue());
    }

    /**
     * Test case for VLAN Mapping APIs.
     *
     * This method is called by {@code testVBridgeAPI}.
     */
    private void testVLANMappingAPI(String tname, String bname) throws JSONException {
        System.out.println("Starting VLAN Mapping JAXB client.");
        String url = VTN_BASE_URL;
        StringBuilder baseURL = new StringBuilder();
        baseURL.append(url);
        baseURL.append("default/vtns/");
        baseURL.append(tname);
        baseURL.append("/vbridges/");

        String nodeid1 = "00:00:00:00:00:00:00:01";
        String nodeid2 = "00:00:00:00:00:00:00:02";

        String vlan0 = "0";
        String vlan1 = "1000";
        String vlan2 = "4095";
        String vlan3 = "2000";
        String vlan = "10";

        String vlan_negative = "-100";
        String vlan_over = "4096";

        String nodeType = "OF";

        String tname_dummy = "tname_dummy";
        String bname_dummy = "bname_dummy";

        String bname2 = "vbridge_Name2";

        // Test GET VLAN Mapping
        String result = getJsonResult(baseURL + bname + "/vlanmaps");
        Assert.assertEquals(200, httpResponseCode.intValue());
        JSONTokener jt = new JSONTokener(result);
        JSONObject json = new JSONObject(jt);
        JSONArray vLANMapArray = json.getJSONArray("vlanmap");
        Assert.assertEquals(0, vLANMapArray.length());

        // Test GET VLAN Mapping expecting 404, setting dummy vtn
        result = getJsonResult(url + "default/vtns/" + tname_dummy + "/vbridges/" + bname + "/vlanmaps");
        Assert.assertEquals(404, httpResponseCode.intValue());

        // Test GET VLAN Mapping expecting 404, setting dummy vbridge
        result = getJsonResult(baseURL + bname_dummy + "/vlanmaps");
        Assert.assertEquals(404, httpResponseCode.intValue());


        // Test POST VLAN Mapping expecting 404, setting dummy vbridge
        String requestBody = "{\"vlan\":\"" + vlan1 +"\",\"node\":{\"type\":\""+ nodeType +"\",\"id\":\""
                + nodeid1 + "\"}}";
        result = getJsonResult(url + "default/vtns/" + tname_dummy + "/vbridges/"
                + bname + "/vlanmaps", "POST", requestBody);
        Assert.assertEquals(404, httpResponseCode.intValue());

        // Test POST VLAN Mapping expecting 404, setting dummy vbridge
        result = getJsonResult(baseURL + bname_dummy + "/vlanmaps", "POST", requestBody);
        Assert.assertEquals(404, httpResponseCode.intValue());

        // Test POST VLAN Mapping expecting 415
        // setting negative value to vlan
        requestBody = "{\"vlan\":\"" + vlan_negative +"\",\"node\":{\"type\":\""+ nodeType +"\",\"id\":\""
                + nodeid1 + "\"}}";
        result = getJsonResult(baseURL + bname + "/vlanmaps", "POST", requestBody);
        Assert.assertEquals(415, httpResponseCode.intValue());

        // setting 4096 to vlan
        requestBody = "{\"vlan\":\"" + vlan_over +"\",\"node\":{\"type\":\""+ nodeType +"\",\"id\":\""
                + nodeid1 + "\"}}";
        result = getJsonResult(baseURL + bname + "/vlanmaps", "POST", requestBody);
        Assert.assertEquals(415, httpResponseCode.intValue());

        // setting invalid type to node type
        requestBody = "{\"vlan\":\"" + vlan +"\",\"node\":{\"type\":\""+ "ERROR_TEST" +"\",\"id\":\""
                + nodeid1 + "\"}}";
        result = getJsonResult(baseURL + bname + "/vlanmaps", "POST", requestBody);
        Assert.assertEquals(415, httpResponseCode.intValue());

        // Test POST VLAN Mapping
        requestBody = "{\"vlan\":\"" + vlan1 +"\",\"node\":{\"type\":\""+ nodeType +"\",\"id\":\""
                + nodeid1 + "\"}}";
        String requestUri = baseURL + bname + "/vlanmaps";
        result = getJsonResult(requestUri, "POST", requestBody);
        Assert.assertEquals(201, httpResponseCode.intValue());
        String loc = requestUri + "/" + nodeType + "-" + nodeid1 + "." + vlan1;
        Assert.assertEquals(loc, httpLocation);

        // Test POST VLAN Mapping expecting 409
        requestBody = "{\"vlan\":\"" + vlan1 +"\",\"node\":{\"type\":\""+ nodeType +"\",\"id\":\""
                + nodeid1 + "\"}}";
        result = getJsonResult(baseURL + bname + "/vlanmaps", "POST", requestBody);
        Assert.assertEquals(409, httpResponseCode.intValue());

        // Test GET VLAN Mapping
        result = getJsonResult(baseURL + bname + "/vlanmaps");
        Assert.assertEquals(200, httpResponseCode.intValue());
        jt = new JSONTokener(result);
        json = new JSONObject(jt);
        vLANMapArray = json.getJSONArray("vlanmap");
        Assert.assertEquals(1, vLANMapArray.length());

        // Test POST VLAN Mapping
        requestBody = "{\"vlan\":\"" + vlan2 +"\",\"node\":{\"type\":\""+ nodeType +"\",\"id\":\""
                + nodeid2 + "\"}}";
        result = getJsonResult(baseURL + bname + "/vlanmaps", "POST", requestBody);
        Assert.assertEquals(201, httpResponseCode.intValue());
        loc = requestUri + "/" + nodeType + "-" + nodeid2 + "." + vlan2;
        Assert.assertEquals(loc, httpLocation);

        // Test GET all  VLAN Mapping in default container
        result = getJsonResult(baseURL + bname + "/vlanmaps");
        jt = new JSONTokener(result);
        json = new JSONObject(jt);
        vLANMapArray = json.getJSONArray("vlanmap");
        JSONObject vLANMap;

        Assert.assertEquals(200, httpResponseCode.intValue());
        Assert.assertEquals(2, vLANMapArray.length());

        for (int i = 0; i < vLANMapArray.length(); i++) {
            vLANMap = vLANMapArray.getJSONObject(i);
            JSONObject nodeinfo = vLANMap.getJSONObject("node");
            if (vLANMap.getString("id").equals(nodeType + "-" + nodeid1 + "." + vlan1)) {
                Assert.assertEquals(vlan1, vLANMap.getString("vlan"));
                Assert.assertEquals(nodeType, nodeinfo.getString("type"));
                Assert.assertEquals(nodeid1, nodeinfo.getString("id"));

            } else if (vLANMap.getString("id").equals(nodeType + "-" + nodeid2 + "." + vlan2)) {
                Assert.assertEquals(vlan2, vLANMap.getString("vlan"));
                Assert.assertEquals(nodeType, nodeinfo.getString("type"));
                Assert.assertEquals(nodeid2, nodeinfo.getString("id"));
            }
            else {
                // Unexpected VLAN Mapping
                Assert.assertTrue(false);
            }
        }

        // Test POST VLAN Mapping, setting 0 to vlan
        requestBody = "{\"vlan\":\"" + vlan0 + "\",\"node\":{\"type\":\"" + nodeType + "\",\"id\":\""
                        + nodeid1 + "\"}}";
        requestUri = baseURL + bname2 + "/vlanmaps";
        result = getJsonResult(requestUri, "POST", requestBody);
        Assert.assertEquals(201, httpResponseCode.intValue());
        loc = requestUri + "/" + nodeType + "-" + nodeid1 + "." + vlan0;
        Assert.assertEquals(loc, httpLocation);

        // Test POST VLAN Mapping, except vlan
        requestBody = "{\"node\":{\"type\":\""+ nodeType +"\",\"id\":\"" + nodeid1 + "\"}}";
        requestUri = baseURL + bname + "/vlanmaps";
        result = getJsonResult(requestUri, "POST", requestBody);
        Assert.assertEquals(409, httpResponseCode.intValue());

        // auth fail
        requestBody = "{\"vlan\":\"" + vlan3 + "\",\"node\":{\"type\":\"" + nodeType
                + "\",\"id\":\"" + nodeid1 + "\"}}";
        requestUri = baseURL + bname + "/vlanmaps";
        result = getJsonResult(requestUri, "POST", requestBody, "application/json", false);
        Assert.assertEquals(401, httpResponseCode.intValue());

        result = getJsonResult(baseURL + bname + "/vlanmaps", "GET", null,
                               "application/json", false);
        Assert.assertEquals(401, httpResponseCode.intValue());

        result = getJsonResult(baseURL + bname2 + "/vlanmaps/" + nodeType + "-" + nodeid1 + "." + vlan0,
                               "GET", null, "application/json", false);
        Assert.assertEquals(401, httpResponseCode.intValue());

        result = getJsonResult(baseURL + bname2 + "/vlanmaps/" + nodeType + "-" + nodeid1 + "." + vlan0,
                               "DELETE", null, "application/json", false);
        Assert.assertEquals(401, httpResponseCode.intValue());


        // Test DELETE VLAN Mapping
        result = getJsonResult(baseURL + bname2 + "/vlanmaps/" + nodeType + "-" + nodeid1 + "." + vlan0, "DELETE");
        Assert.assertEquals(200, httpResponseCode.intValue());

        // Test not supported Content-Type
        requestBody = "{\"vlan\":\"" + vlan0 + "\",\"node\":{\"type\":\"" + nodeType
                        + "\",\"id\":\"" + nodeid1 + "\"}}";
        requestUri = baseURL + bname + "/vlanmaps";
        result = getJsonResult(requestUri, "POST", requestBody, "text/plain");
        Assert.assertEquals(415, httpResponseCode.intValue());


        // Test POST VLAN Mapping, except vlan
        requestBody = "{\"node\":{\"type\":\""+ nodeType +"\",\"id\":\"" + nodeid1 + "\"}}";
        requestUri = baseURL + bname + "/vlanmaps";
        result = getJsonResult(requestUri, "POST", requestBody);
        Assert.assertEquals(201, httpResponseCode.intValue());
        loc = requestUri + "/" + nodeType + "-" + nodeid1 + "." + vlan0;
        Assert.assertEquals(loc, httpLocation);

        // Test POST VLAN Mapping, setting requestBody without node's id
        requestBody = "{\"vlan\":\"" + vlan +"\",\"node\":{\"type\":\""+ nodeType +"\"}}";
        requestUri = baseURL + bname + "/vlanmaps";
        result = getJsonResult(requestUri, "POST", requestBody);
        Assert.assertEquals(415, httpResponseCode.intValue());

        // Test POST VLAN Mapping, setting requestBody without node's type
        requestBody = "{\"vlan\":\"" + vlan +"\",\"node\":{\"id\":\"" + nodeid1 + "\"}}";
        requestUri = baseURL + bname2 + "/vlanmaps";
        result = getJsonResult(requestUri, "POST", requestBody);
        Assert.assertEquals(415, httpResponseCode.intValue());

        // Test POST VLAN Mapping, setting requestBody without node elements
        requestBody = "{\"vlan\":\"" + vlan3 +"\"}";
        requestUri = baseURL + bname2 + "/vlanmaps";
        result = getJsonResult(requestUri, "POST", requestBody);
        Assert.assertEquals(201, httpResponseCode.intValue());
        loc = requestUri + "/" + "ANY" + "." + vlan3;
        Assert.assertEquals(loc, httpLocation);

        // Test POST VLAN Mapping expecting 400
        // setting invalid value to node id
        requestBody = "{\"vlan\":\"" + vlan +"\",\"node\":{\"type\":\""+ nodeType +"\",\"id\":\""
                + "ERROR_TEST" + "\"}}";
        requestUri = baseURL + bname2 + "/vlanmaps";
        result = getJsonResult(requestUri, "POST", requestBody);
        Assert.assertEquals(400, httpResponseCode.intValue());

        // Test GET VLAN Mapping expecting 404
        // setting dummy vtn
        result = getJsonResult(url + "default/vtns/" + tname_dummy + "/vbridges/" + bname + "/vlanmaps"
                + nodeType + "-" + nodeid1 + "." + vlan1);
        Assert.assertEquals(404, httpResponseCode.intValue());

        // setting dummy vbridge
        result = getJsonResult(baseURL + bname_dummy + "/vlanmaps/" + nodeType + "-" + nodeid1 + "." + vlan1);
        Assert.assertEquals(404, httpResponseCode.intValue());

        // setting dummy vlan
        result = getJsonResult(baseURL + bname + "/vlanmaps/" + nodeType + "-" + nodeid1 + "." + vlan);
        Assert.assertEquals(404, httpResponseCode.intValue());

        // Test GET VLAN Mapping
        result = getJsonResult(baseURL + bname + "/vlanmaps/" + nodeType + "-" + nodeid1 + "." + vlan1);
        jt = new JSONTokener(result);
        json = new JSONObject(jt);
        Assert.assertEquals(200, httpResponseCode.intValue());

        Assert.assertEquals(nodeType + "-" + nodeid1 + "." + vlan1, json.getString("id"));
        Assert.assertEquals(vlan1, json.getString("vlan"));
        JSONObject nodeinfo = json.getJSONObject("node");
        Assert.assertEquals(nodeType, nodeinfo.getString("type"));
        Assert.assertEquals(nodeid1, nodeinfo.getString("id"));

        // Test GET all  VLAN Mapping in default container
        result = getJsonResult(baseURL + bname + "/vlanmaps");
        jt = new JSONTokener(result);
        json = new JSONObject(jt);
        vLANMapArray = json.getJSONArray("vlanmap");

        Assert.assertEquals(200, httpResponseCode.intValue());
        Assert.assertEquals(3, vLANMapArray.length());
    }

    /**
     * Test case for VLAN Mapping delete APIs.
     *
     * This method is called by {@code testVBridgeAPI}.
     */
    private void testVLANMappingDeleteAPI(String tname, String bname) throws JSONException {
        System.out.println("Starting delete Port Mapping JAXB client.");
        String url = VTN_BASE_URL;
        StringBuilder baseURL = new StringBuilder();
        baseURL.append(url);
        baseURL.append("default/vtns/");
        baseURL.append(tname);
        baseURL.append("/vbridges/");

        String nodeid1 = "00:00:00:00:00:00:00:01";
        String vlan1 = "1000";
        String nodeType = "OF";

        String tname_dummy = "tname_dummy";
        String bname_dummy = "bname_dummy";
        String vlan_dummy = "1234";


        // Test DELETE VLAN Mapping
        String result = getJsonResult(baseURL + bname + "/vlanmaps/" + nodeType + "-" + nodeid1 + "." + vlan1, "DELETE");
        Assert.assertEquals(200, httpResponseCode.intValue());

        // Test DELETE VLAN Mapping expecting 404
        // setting deleted vlan mapping
        result = getJsonResult(baseURL + bname + "/vlanmaps/" + nodeType + "-" + nodeid1 + "." + vlan1, "DELETE");
        Assert.assertEquals(404, httpResponseCode.intValue());

        // setting dummy tenant
        result = getJsonResult(url + "default/vtns/" + tname_dummy + "/vbridges/"
                + bname + "/vlanmaps/" + nodeType + "-" + nodeid1 + "." + vlan1, "DELETE");
        Assert.assertEquals(404, httpResponseCode.intValue());

        // setting dummy vbridge mapping
        result = getJsonResult(baseURL + bname_dummy + "/vlanmaps/" + nodeType + "-" + nodeid1 + "." + vlan1, "DELETE");
        Assert.assertEquals(404, httpResponseCode.intValue());

        // setting dummy vlan mapping
        result = getJsonResult(baseURL + bname + "/vlanmaps/" + nodeType + "-" + nodeid1 + "." + vlan_dummy, "DELETE");
        Assert.assertEquals(404, httpResponseCode.intValue());

        // Test GET VLAN Mapping expecting 404
        result = getJsonResult(baseURL + bname + "/vlanmaps" + nodeType + "-" + nodeid1 + "." + vlan1);
        Assert.assertEquals(404, httpResponseCode.intValue());


        // Test GET all  VLAN Mapping in default container
        result = getJsonResult(baseURL + bname + "/vlanmaps");
        JSONTokener jt = new JSONTokener(result);
        JSONObject json = new JSONObject(jt);
        JSONArray vLANMapArray = json.getJSONArray("vlanmap");

        Assert.assertEquals(200, httpResponseCode.intValue());
        Assert.assertEquals(2, vLANMapArray.length());

        // delete VLAN mappings set for test.
        for (int i = 0; i < vLANMapArray.length(); i++) {
            JSONObject vLANMap = vLANMapArray.getJSONObject(i);
            result = getJsonResult(baseURL + bname + "/vlanmaps/" + vLANMap.getString("id"), "DELETE");
            Assert.assertEquals(200, httpResponseCode.intValue());
        }
    }

    /**
     * Test case for MAC Address APIs.
     *
     * This method is called by {@code testVBrdigeInterface}.
     */
    private void testMacAddressAPI(String tname, String bname) throws JSONException {
        System.out.println("Starting MAC address JAXB client.");
        String url = VTN_BASE_URL;

        StringBuilder baseURL = new StringBuilder();
        baseURL.append(url);
        baseURL.append("default/vtns/");
        baseURL.append(tname);
        baseURL.append("/vbridges/");

        String dummy = "dummy";
        String macaddr = "00:00:00:00:00:01";

        // Test GET all MAC address
        String result = getJsonResult(baseURL + bname + "/mac");
        Assert.assertEquals(200, httpResponseCode.intValue());
        JSONTokener jt = new JSONTokener(result);
        JSONObject json = new JSONObject(jt);
        JSONArray macArray = json.getJSONArray("macentry");
        Assert.assertEquals(0, macArray.length());

        // Test GET all MAC address expecting 404
        // setting dummy vtn
        result = getJsonResult(url + "default/vtns/" + dummy + "/vbridges/" + bname + "/mac");
        Assert.assertEquals(404, httpResponseCode.intValue());

        // setting dummy vbridge
        result = getJsonResult(baseURL + dummy + "/mac");
        Assert.assertEquals(404, httpResponseCode.intValue());

        // Test DELETE all MAC address
        result = getJsonResult(baseURL + bname + "/mac", "DELETE");
        Assert.assertEquals(200, httpResponseCode.intValue());

        // Test DELETE all MAC address expecting 404
        // setting dummy vtn
        result = getJsonResult(url + "default/vtns/" + dummy + "/vbridges/"
                + bname + "/mac", "DELETE");
        Assert.assertEquals(404, httpResponseCode.intValue());

        // setting dummy vbridge
        result = getJsonResult(baseURL + dummy + "/mac", "DELETE");
        Assert.assertEquals(404, httpResponseCode.intValue());

        // Test GET MAC address expecting 404
        // setting MAC address that don't exist
        result = getJsonResult(baseURL + bname + "/mac/" + macaddr);
        Assert.assertEquals(404, httpResponseCode.intValue());

        // Test DELETE MAC address expecting 404
        // setting MAC address that don't exist
        result = getJsonResult(baseURL + bname + "/mac/" + macaddr, "DELETE");
        Assert.assertEquals(404, httpResponseCode.intValue());


        // add MAC Address Entry.
        VBridgePath bpath = new VBridgePath(tname, bname);
        Node node = NodeCreator.createOFNode(0L);
        NodeConnector nc
            = NodeConnectorCreator.createOFNodeConnector(Short.valueOf((short) 0), node);

        // test with MAC Table entry existing.
        // set VLAN Map and Mac Table Entry before test.
        short vlan = 0;
        String requestBody = "{\"vlan\":\"" + vlan + "\"}";
        result = getJsonResult(baseURL + bname + "/vlanmaps", "POST", requestBody);
        Assert.assertEquals(201, httpResponseCode.intValue());

        byte[] srcMac1 = new byte[] {0x00, 0x00, 0x00, 0x00, 0x00, 0x01};
        byte[] srcMac2 = new byte[] {0x00, 0x00, 0x00, 0x00, 0x00, 0x02};
        byte[] sender1 = new byte[] {(byte)192, (byte)168, (byte)0, (byte) 1};
        byte[] sender2 = new byte[] {(byte)192, (byte)168, (byte)0, (byte) 2};

        putMacTableEntry(listenDataPacket, bpath, srcMac1, sender1, nc);
        putMacTableEntry(listenDataPacket, bpath, srcMac2, sender2, nc);

        Set<String> expectedMacSet = new HashSet<String>();
        expectedMacSet.add(HexEncode.bytesToHexStringFormat(srcMac1));
        expectedMacSet.add(HexEncode.bytesToHexStringFormat(srcMac2));

        // GET a list of MAC entry in vBridge.
        checkMacTableEntry(baseURL + bname, 2, expectedMacSet, vlan);

        // Test GET all MAC address expecting 404
        // setting dummy vtn
        result = getJsonResult(url + "default/vtns/" + dummy + "/vbridges/" + bname + "/mac");
        Assert.assertEquals(404, httpResponseCode.intValue());

        // setting dummy vbridge
        result = getJsonResult(baseURL + dummy + "/mac");
        Assert.assertEquals(404, httpResponseCode.intValue());


        // Test DELETE MAC address expecting 404
        // setting dummy vtn
        Iterator<String> itMac = expectedMacSet.iterator();
        String expectedMac = itMac.next();
        result = getJsonResult(url + "default/vtns/" + dummy + "/vbridges/" + bname + "/mac" + expectedMac,
                               "DELETE");
        Assert.assertEquals(404, httpResponseCode.intValue());

        // setting dummy vbridge
        result = getJsonResult(baseURL + dummy + "/mac/" + expectedMac, "DELETE");
        Assert.assertEquals(404, httpResponseCode.intValue());

        // setting correct URI.
        result = getJsonResult(baseURL + bname + "/mac/" + expectedMac, "DELETE");
        Assert.assertEquals(200, httpResponseCode.intValue());

        expectedMacSet.remove(expectedMac);
        checkMacTableEntry(baseURL + bname, 1, expectedMacSet, vlan);

        // delete another entry.
        expectedMac = expectedMacSet.iterator().next();
        result = getJsonResult(baseURL + bname + "/mac/" + expectedMac, "DELETE");
        Assert.assertEquals(200, httpResponseCode.intValue());

        expectedMacSet.clear();
        checkMacTableEntry(baseURL + bname, 0, expectedMacSet, vlan);

        // Add 10 Mac Entries.
        int numEntry = 10;
        for (int i = 0; i < numEntry; i++) {
            byte[] src = new byte[] {0x00, 0x00, 0x00, 0x00, 0x01, (byte) (i + 1)};
            byte[] sender = new byte[] {(byte)192, (byte)168, (byte)1, (byte) (i + 1)};

            putMacTableEntry(listenDataPacket, bpath, src, sender, nc);
            expectedMacSet.add(HexEncode.bytesToHexStringFormat(src));
        }

        // GET a list of MAC entry in vBridge
        checkMacTableEntry(baseURL + bname, numEntry, expectedMacSet, vlan);

        // auth faile
        expectedMac = expectedMacSet.iterator().next();
        result = getJsonResult(baseURL + bname + "/mac", "GET", null, "application/json", false);
        Assert.assertEquals(401, httpResponseCode.intValue());

        result = getJsonResult(baseURL + bname + "/mac/" + expectedMac, "GET", null,
                               "application/json", false);
        Assert.assertEquals(401, httpResponseCode.intValue());

        result = getJsonResult(baseURL + bname + "/mac/" + expectedMac, "DELETE", null,
                               "application/json", false);
        Assert.assertEquals(401, httpResponseCode.intValue());

        result = getJsonResult(baseURL + bname + "/mac", "DELETE", null,
                               "application/json", false);
        Assert.assertEquals(401, httpResponseCode.intValue());

        // flush mac address table.
        result = getJsonResult(baseURL + bname + "/mac", "DELETE", null, "applicatin/json",
                               false);
        Assert.assertEquals(401, httpResponseCode.intValue());

    }

    /**
     * Check Mac Table Entry.
     *
     * @param url               A request URL.
     * @param numEntry          A number of MAC Address Entries.
     * @param expectedMacSet    A set of Mac Address which is expected to be added.
     * @param vlan              VLAN ID.
     * @throws JSONException
     */
    private void checkMacTableEntry(String url, int numEntry, Set<String> expectedMacSet,
                                    short vlan) throws JSONException {
        String result = getJsonResult(url + "/mac");
        Assert.assertEquals(200, httpResponseCode.intValue());
        JSONTokener jt = new JSONTokener(result);
        JSONObject json = new JSONObject(jt);
        JSONArray macArray = json.getJSONArray("macentry");
        Assert.assertEquals(numEntry, macArray.length());

        for (int i = 0; i < macArray.length(); i++) {
            JSONObject macEntry = macArray.getJSONObject(i);
            String regMac = macEntry.getString("address");
            String regVlan = macEntry.getString("vlan");

            Assert.assertTrue(expectedMacSet.contains(regMac));
            Assert.assertEquals(Short.toString(vlan), regVlan);
        }
    }

    /**
     * put a Mac Address Table Entry to Mac Address Table of specified bridge.
     *
     * @param mgr   VTNManagerImpl Service.
     * @param src   A source MAC Address.
     * @param sender A sender IP addesss.
     * @param bpath A VBridgePath.
     * @param nc    A NodeConnector.
     */
    private void putMacTableEntry(IListenDataPacket listenData, VBridgePath bpath,
                                  byte[] src, byte[] sender, NodeConnector nc) {

        byte[] dst = new byte[] {(byte)0xFF, (byte)0xFF, (byte)0xFF,
                                 (byte)0xFF, (byte)0xFF, (byte)0xFF};
        byte[] target = new byte[] {(byte)192, (byte)168, (byte)0, (byte)250};

        RawPacket inPkt = createARPRawPacket (src, dst, sender, target, (short)-1, nc, ARP.REQUEST);

        listenData.receiveDataPacket(inPkt);
    }

    /**
     * Test case for getting version information APIs.
     *
     * This method is called by {@core testVTNAPI}.
     */
    private void testVtnGlobal(String base) throws JSONException {
        System.out.println("Starting IVTNGlobal JAXB client.");

        // Get version information of the VTN Manager.
        String result = getJsonResult(base + "version");
        Assert.assertEquals(200, httpResponseCode.intValue());

        JSONTokener jt = new JSONTokener(result);
        JSONObject json = new JSONObject(jt);
        int api = json.getInt("api");
        JSONObject bv = json.getJSONObject("bundle");
        Assert.assertTrue("API version: " + api, api > 0);

        // Determine actual bundle version of manager.implementation.
        Assert.assertNotNull(implBundle);
        Version ver = implBundle.getVersion();
        Assert.assertNotNull(ver);

        Assert.assertEquals(ver.getMajor(), bv.getInt("major"));
        Assert.assertEquals(ver.getMinor(), bv.getInt("minor"));
        Assert.assertEquals(ver.getMicro(), bv.getInt("micro"));

        String qf = ver.getQualifier();
        String qfkey  ="qualifier";
        if (qf == null || qf.isEmpty()) {
            Assert.assertFalse(bv.has(qfkey));
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
