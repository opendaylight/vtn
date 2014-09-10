/*
 * Copyright (c) 2013-2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.northbound.integrationtest;

import static java.net.HttpURLConnection.HTTP_BAD_METHOD;
import static java.net.HttpURLConnection.HTTP_BAD_REQUEST;
import static java.net.HttpURLConnection.HTTP_CONFLICT;
import static java.net.HttpURLConnection.HTTP_CREATED;
import static java.net.HttpURLConnection.HTTP_NOT_FOUND;
import static java.net.HttpURLConnection.HTTP_NO_CONTENT;
import static java.net.HttpURLConnection.HTTP_OK;
import static java.net.HttpURLConnection.HTTP_UNAUTHORIZED;
import static java.net.HttpURLConnection.HTTP_UNAVAILABLE;
import static java.net.HttpURLConnection.HTTP_UNSUPPORTED_TYPE;

import static org.hamcrest.CoreMatchers.is;
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
import java.util.ArrayList;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.TreeMap;

import javax.inject.Inject;

import org.apache.commons.codec.binary.Base64;
import org.codehaus.jettison.json.JSONArray;
import org.codehaus.jettison.json.JSONException;
import org.codehaus.jettison.json.JSONObject;
import org.codehaus.jettison.json.JSONTokener;
import org.junit.After;
import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.ops4j.pax.exam.Configuration;
import org.ops4j.pax.exam.Option;
import org.ops4j.pax.exam.junit.ExamReactorStrategy;
import org.ops4j.pax.exam.junit.PaxExam;
import org.ops4j.pax.exam.spi.reactors.PerClass;
import org.ops4j.pax.exam.util.PathUtils;
import org.osgi.framework.Bundle;
import org.osgi.framework.BundleContext;
import org.osgi.framework.BundleException;
import org.osgi.framework.ServiceReference;
import org.osgi.framework.Version;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import org.opendaylight.vtn.manager.IVTNManager;
import org.opendaylight.vtn.manager.VBridgeIfPath;
import org.opendaylight.vtn.manager.VBridgePath;
import org.opendaylight.vtn.manager.VInterfacePath;
import org.opendaylight.vtn.manager.VTerminalIfPath;

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

@RunWith(PaxExam.class)
@ExamReactorStrategy(PerClass.class)
public class VtnNorthboundIT extends TestBase {
    private static final Logger LOG = LoggerFactory.
        getLogger(VtnNorthboundIT.class);
    private static final String  BUNDLE_VTN_MANAGER_IMPL =
        "org.opendaylight.vtn.manager.implementation";
    private static final String VTN_BASE_URL =
        "http://127.0.0.1:8080/controller/nb/v2/vtn/";

    /**
     * Resource name for VTN APIs.
     */
    private static final String RES_VTNS = "vtns";

    /**
     * Resource name for vBridge APIs.
     */
    private static final String RES_VBRIDGES = "vbridges";

    /**
     * Resource name for virtual interface APIs.
     */
    private static final String RES_INTERFACES = "interfaces";

    /**
     * Resource name for vTerminal APIs.
     */
    private static final String RES_VTERMINALS = "vterminals";

    /**
     * Resource name for flow filter APIs.
     */
    private static final String  RES_FLOWFILTERS = "flowfilters";

    /**
     * Valid bits in flow filter index.
     */
    private static final int  MASK_FLOWFILTER_INDEX = 0xffff;

    /**
     * Valid bits in VLAN priority.
     */
    private static final int  MASK_VLAN_PCP = 0x7;

    /**
     * Valid bits in IP DSCP value.
     */
    private static final int  MASK_IP_DSCP = 0x3f;

    /**
     * Valid bits in ICMP type and code.
     */
    private static final int  MASK_ICMP = 0xff;

    // get the OSGI bundle context
    @Inject
    private BundleContext bc;
    private IUserManager userManager = null;
    private IVTNManager vtnManager = null;
    private IListenDataPacket listenDataPacket = null;
    private Bundle  implBundle;

    /**
     * Convert bundle's state to String.
     *
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
     * check each bundles are ready or not before start each test methods .
     */
    @Before
    public void areWeReady() {
        assertNotNull(bc);
        boolean debugit = false;

        for (Bundle b: bc.getBundles()) {
            String name = b.getSymbolicName();
            int state = b.getState();
            if (state != Bundle.ACTIVE && state != Bundle.RESOLVED) {
                LOG.debug("Bundle:" + name + " state:" + stateToString(state));
                debugit = true;
            } else if (BUNDLE_VTN_MANAGER_IMPL.equals(name)) {
                implBundle = b;
            }
        }

        if (debugit) {
            LOG.debug("Do some debugging because some bundle is " + "unresolved");
        }
        // Assert if true, if false we are good to go!
        assertFalse(debugit);
        assertNotNull(implBundle);

        // Start manager.implementation bundle if it is not active.
        if (implBundle.getState() != Bundle.ACTIVE) {
            startVTN();
            assertEquals(Bundle.ACTIVE, implBundle.getState());
        }

        ServiceReference r = bc.getServiceReference(IUserManager.class.getName());
        if (r != null) {
            this.userManager = (IUserManager)bc.getService(r);
        }

        r = bc.getServiceReference(IVTNManager.class.getName());
        if (r != null) {
            this.vtnManager = (IVTNManager)bc.getService(r);
            this.listenDataPacket = (IListenDataPacket)this.vtnManager;
        }

        // If UserManager or VTNManager is null, cannot login to run tests.
        assertNotNull(this.userManager);
        assertNotNull(this.vtnManager);
    }

    /**
     * Called when a test suite quits.
     */
    @After
    public void tearDown() {
        if (implBundle.getState() != Bundle.ACTIVE) {
            return;
        }

        try {
            // Remove all VTNs in the default container.
            String uri = VTN_BASE_URL + "default/vtns";
            String result = getJsonResult(uri);
            assertResponse(HTTP_OK);

            JSONTokener jt = new JSONTokener(result);
            JSONObject json = new JSONObject(jt);
            JSONArray vtnArray = json.getJSONArray("vtn");
            for (int i = 0; i < vtnArray.length(); i++) {
                JSONObject vtn = vtnArray.getJSONObject(i);
                String name = vtn.getString("name");
                LOG.debug("Clean up VTN: {}", name);
                result = getJsonResult(uri + "/" + name, "DELETE");
                assertResponse(HTTP_NOT_FOUND);
            }
        } catch (Exception e) {
            unexpected(e);
        }
    }

    // static variable to pass response code from getJsonResult()
    private static Integer httpResponseCode = null;

    private String  httpLocation;

    /**
     * Check HTTP response code.
     *
     * @param code  Expected response code.
     */
    private void assertResponse(int code) {
        assertEquals(code, httpResponseCode.intValue());
    }

    /**
     * Construct a URI from the given path components.
     *
     * @param components  A string array of path components.
     * @return  A URI.
     */
    private String createURI(String ... components) {
        return createRelativeURI(VTN_BASE_URL, components);
    }

    /**
     * Construct a URI relative to the given base URI.
     *
     * @param base        A base URI.
     * @param components  A string array of path components.
     * @return  A URI.
     */
    private String createRelativeURI(String base, String ... components) {
        StringBuilder builder = new StringBuilder(base);

        for (String comp: components) {
            if (comp != null) {
                if (builder.charAt(builder.length() - 1) != '/') {
                    builder.append('/');
                }
                builder.append(comp);
            }
        }

        return builder.toString();
    }

    /**
     * Create a {@link JSONObject} which represents a <strong>machost</strong>
     * element.
     *
     * @param addr  A string representation of MAC address.
     * @param vlan  A string representation of VLAN ID.
     * @return  A {@link JSONObject} instance.
     * @throws JSONException  An error occurred.
     */
    private JSONObject createMacHost(String addr, String vlan)
        throws JSONException {
        JSONObject json = new JSONObject();
        if (addr != null) {
            json.put("address", addr);
        }
        if (vlan != null) {
            json.put("vlan", Integer.valueOf(vlan));
        }

        return json;
    }

    /**
     * Create a {@link JSONObject} which represents a <strong>machosts</strong>
     * element.
     *
     * @param args  A list of MAC address and VLAN ID.
     *              The number of arguments must be a multiple of 2.
     * @return  A {@link JSONObject} instance.
     * @throws JSONException  An error occurred.
     */
    private JSONObject createMacHostSet(List<String> args)
        throws JSONException {
        return createMacHostSet(args.toArray(new String[args.size()]));
    }

    /**
     * Create a {@link JSONObject} which represents a <strong>machosts</strong>
     * element.
     *
     * @param args  Sequence of MAC address and VLAN ID.
     *              The number of arguments must be a multiple of 2.
     * @return  A {@link JSONObject} instance.
     * @throws JSONException  An error occurred.
     */
    private JSONObject createMacHostSet(String ... args) throws JSONException {
        JSONObject json = new JSONObject();
        JSONArray list = new JSONArray();
        int i = 0;
        while (i < args.length) {
            String addr = args[i++];
            String vlan = args[i++];
            list.put(createMacHost(addr, vlan));
        }

        json.put("machost", list);

        return json;
    }

    /**
     * Create JSON object which represents <strong>macmapconf</strong>.
     *
     * @param allowed  A list which contains hosts in allow list.
     * @param denied   A list which contains hosts in deny list.
     * @return  A {@link JSONObject} instance.
     * @throws JSONException  An error occurred.
     */
    private JSONObject createMacMapConfig(List<String> allowed,
                                          List<String> denied)
        throws JSONException {
        JSONObject allow = createMacHostSet(allowed);
        JSONObject deny = createMacHostSet(denied);
        JSONObject config = new JSONObject();
        config.put("allow", allow).put("deny", deny);

        return config;
    }

    /**
     * Complete omitted elements in <strong>machosts</strong>.
     *
     * @param json  A {@link JSONObject} which represents a
     *              <strong>machosts</strong> element.
     * @return  A {@link JSONObject} element.
     * @throws JSONException  An error occurred.
     */
    private JSONObject completeMacHostSet(JSONObject json)
        throws JSONException {
        JSONObject newjson = new JSONObject();
        JSONArray newlist = new JSONArray();

        String listKey = "machost";
        String addrKey = "address";
        String vlanKey = "vlan";
        String[] hostKeys = {addrKey, vlanKey};
        JSONArray list = json.getJSONArray(listKey);
        int len = list.length();
        for (int i = 0; i < len; i++) {
            JSONObject host = list.getJSONObject(i);
            JSONObject newhost = new JSONObject(host, hostKeys);
            if (newhost.isNull(vlanKey)) {
                newhost.put(vlanKey, Integer.valueOf(0));
            }
            newlist.put(newhost);
        }
        newjson.put(listKey, newlist);

        return newjson;
    }

    /**
     * Complete omitted elements in <strong>macmapconf</strong>.
     *
     * @param json  A {@link JSONObject} which represents a
     *              <strong>macmapconf</strong> element.
     * @return  A {@link JSONObject} element.
     * @throws JSONException  An error occurred.
     */
    private JSONObject completeMacMapConfig(JSONObject json)
        throws JSONException {
        JSONObject conf = new JSONObject();
        for (String acl: new String[]{"allow", "deny"}) {
            JSONObject set = json.optJSONObject(acl);
            if (set != null) {
                JSONArray list = set.optJSONArray("machost");
                if (list != null && list.length() != 0) {
                    set = completeMacHostSet(set);
                    conf.put(acl, set);
                }
            }
        }

        return conf;
    }

    /**
     * Verify that the given two JSON objects are identical.
     *
     * @param expected   An expected value.
     * @param json       A {@link JSONObject} to be tested.
     * @throws JSONException  An error occurred.
     */
    private void assertEquals(JSONObject expected, JSONObject json)
        throws JSONException {
        JSONArray keys = json.names();
        JSONArray exkeys = expected.names();
        if (keys == null) {
            Assert.assertEquals(null, exkeys);
            return;
        }
        assertEquals(exkeys, keys);

        int keyLen = keys.length();
        for (int i = 0; i < keyLen; i++) {
            String key = keys.getString(i);
            Object exv = expected.get(key);
            Object value = json.get(key);
            if (exv instanceof JSONArray) {
                assertEquals((JSONArray)exv, (JSONArray)value);
            } else if (exv instanceof JSONObject) {
                assertEquals((JSONObject)exv, (JSONObject)value);
            } else {
                assertEquals(exv, value);
            }
        }
    }

    /**
     * Verify that the given two JSON arrays are identical,
     *
     * <p>
     *   Note that this method ignores element order in the arrays.
     * </p>
     *
     * @param expected  An expected value.
     * @param jarray    A {@link JSONArray} to be tested.
     * @throws JSONException  An error occurred.
     */
    private void assertEquals(JSONArray expected, JSONArray jarray)
        throws JSONException {
        if (expected == null) {
            Assert.assertEquals(null, jarray);
            return;
        }
        if (jarray == null) {
            Assert.assertEquals(null, expected);
            return;
        }
        int len = jarray.length();
        assertEquals(expected.length(), len);

        // Convert expected array elements into strings.
        HashMap<String, Integer> exmap = new HashMap<String, Integer>();
        for (int i = 0; i < len; i++) {
            String value = expected.getString(i);
            Integer count = exmap.get(value);
            int newcnt = (count == null) ? 1 : count.intValue() + 1;
            exmap.put(value, Integer.valueOf(newcnt));
        }

        // Test the specified array.
        for (int i = 0; i < len; i++) {
            String value = jarray.getString(i);
            Integer count = exmap.get(value);
            assertNotNull(count);
            int cnt = count.intValue() - 1;
            if (cnt == 0) {
                exmap.remove(value);
            } else {
                exmap.put(value, Integer.valueOf(cnt));
            }
        }

        assertTrue(exmap.isEmpty());
    }

    /**
     * Send request and get result
     *
     * @param restUrl   A request URL.
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
     * @param auth      if {@code true} authorization succeed,
     *                  else if {@code false} authorization fails.
     * @return  A returned result for request.
     */
    private String getJsonResult(String restUrl, String method, String body,
                                 String contentType, boolean auth) {

        // Initialize response code to indicate error
        httpResponseCode = HTTP_BAD_REQUEST;
        httpLocation = null;

        LOG.debug("HTTP method: {}, uri: {}", method, restUrl);
        LOG.debug("Body: {}", body);

        try {
            URL url = new URL(restUrl);
            this.userManager.getAuthorizationList();
            String authString = null;
            if (auth) {
                this.userManager.authenticate("admin", "admin");
                authString = "admin:admin";
            } else {
                this.userManager.authenticate("admin", "bad");
                authString = "admin:bad";
            }
            byte[] authEncBytes = Base64.encodeBase64(authString.getBytes());
            String authStringEnc = new String(authEncBytes);

            HttpURLConnection connection =
                (HttpURLConnection)url.openConnection();
            connection.setRequestMethod(method);
            connection.setRequestProperty("Authorization", "Basic " +
                                          authStringEnc);
            connection.setRequestProperty("Content-Type", contentType);
            connection.setRequestProperty("Accept", "application/json");

            if (body != null) {
                connection.setDoOutput(true);
                OutputStreamWriter wr =
                    new OutputStreamWriter(connection.getOutputStream());
                wr.write(body);
                wr.flush();
            }
            connection.connect();
            connection.getContentType();

            // Response code for success should be 2xx
            httpResponseCode = connection.getResponseCode();
            LOG.debug("HTTP response code: {}", httpResponseCode);
            LOG.debug("HTTP response message: {}",
                      connection.getResponseMessage());

            if (httpResponseCode == HTTP_CREATED) {
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

            boolean error = (httpResponseCode > 299);
            InputStream is = (error)
                ? connection.getErrorStream()
                : connection.getInputStream();
            InputStreamReader in =
                new InputStreamReader(is, Charset.forName("UTF-8"));
            BufferedReader rd = new BufferedReader(in);
            StringBuilder sb = new StringBuilder();
            int cp;
            while ((cp = rd.read()) != -1) {
                sb.append((char)cp);
            }
            is.close();
            connection.disconnect();

            if (httpResponseCode > 299) {
                String msg = sb.toString();
                if (msg.length() != 0) {
                    LOG.debug("HTTP error response body: {}", msg);
                    return msg;
                }
                return httpResponseCode.toString();
            }

            if (httpResponseCode == HTTP_NO_CONTENT) {
                assertEquals(0, sb.length());
            }

            return sb.toString();
        } catch (Exception e) {
            LOG.error("Caught exception.", e);
            return null;
        }
    }

    /**
     * Send GET request and return result as a JSON object.
     *
     * @param uri  A request URI.
     * @return  A returned result for request.
     * @throws JSONException  An error occurred.
     */
    private JSONObject getJSONObject(String uri) throws JSONException {
        String res = getJsonResult(uri);
        assertResponse(HTTP_OK);
        JSONTokener jt = new JSONTokener(res);
        return new JSONObject(jt);
    }

    /**
     * Create a polymorphic {@link JSONObject} object.
     *
     * @param type  The name of the object type.
     * @param args  Sequence of key/value pairs.
     *              The number of arguments must be an even.
     * @return  A {@link JSONObject} instance.
     * @throws JSONException  An error occurred.
     */
    private JSONObject createJSONObject(String type, Object ... args)
        throws JSONException {
        JSONObject json = new JSONObject();
        int i = 0;
        while (i < args.length) {
            String key = args[i++].toString();
            Object value = args[i++];
            json.put(key, value);
        }

        return new JSONObject().put(type, json);
    }

    /**
     *  A class to construct query parameter for HTTP request
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
     * <p>
     *   This calls {@link #testVBridgeAPI(String, String)}.
     * </p>
     */
    @Test
    public void testVTNAPI() throws JSONException {
        LOG.info("Starting VTN JAX-RS client.");
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
        String timeoutNegative = "-10";
        String timeoutMax = "65535";
        String timeoutOver = "65540";

        // Test GET vtn in default container, expecting no results
        String result = getJsonResult(baseURL + "default/vtns");
        assertResponse(HTTP_OK);
        JSONTokener jt = new JSONTokener(result);
        JSONObject json = new JSONObject(jt);
        JSONArray vtnArray = json.getJSONArray("vtn");
        Assert.assertEquals(0, vtnArray.length());

        // Test POST vtn1
        String requestBody = "{}";
        String requestUri = baseURL + "default/vtns/" + tname1;
        result = getJsonResult(requestUri, "POST", requestBody);
        assertResponse(HTTP_CREATED);
        Assert.assertEquals(requestUri, httpLocation);

        // Test POST vtn1, expecting 409
        requestUri = baseURL + "default/vtns/" + tname1;
        result = getJsonResult(requestUri, "POST", requestBody);
        assertResponse(HTTP_CONFLICT);

        // Test GET vtn in default container, expecting one result
        result = getJsonResult(baseURL + "default/vtns");
        assertResponse(HTTP_OK);
        jt = new JSONTokener(result);
        json = new JSONObject(jt);
        vtnArray = json.getJSONArray("vtn");
        Assert.assertEquals(1, vtnArray.length());

        // Test POST vtn2, setting "_" to vBridgeName
        requestBody = "{\"description\":\"" + desc1 + "\", \"idleTimeout\":\"" + itimeout1 + "\", \"hardTimeout\":\""
                + htimeout1 + "\"}";
        requestUri = baseURL + "default/vtns/" + tname2;
        result = getJsonResult(requestUri, "POST", requestBody);
        assertResponse(HTTP_CREATED);
        Assert.assertEquals(requestUri, httpLocation);

        // Test GET all vtns in default container
        result = getJsonResult(baseURL + "default/vtns");
        jt = new JSONTokener(result);
        json = new JSONObject(jt);
        vtnArray = json.getJSONArray("vtn");
        JSONObject vtn;

        assertResponse(HTTP_OK);
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
        requestBody = "{\"idleTimeout\":\"" + itimeout1 +
            "\", \"hardTimeout\":\"" + htimeout1 + "\"}";
        requestUri = baseURL + "default/vtns/" + tname3;
        result = getJsonResult(requestUri, "POST", requestBody);
        assertResponse(HTTP_CREATED);
        Assert.assertEquals(requestUri, httpLocation);

        // Test POST vtn4, setting idle timeout of negative value and one character numeric of vtn name
        requestBody = "{\"description\":\"" + desc3 +
            "\", \"idleTimeout\":\"" + timeoutNegative + "\"}";
        requestUri = baseURL + "default/vtns/" + tname4;
        result = getJsonResult(requestUri, "POST", requestBody);
        assertResponse(HTTP_CREATED);
        Assert.assertEquals(requestUri, httpLocation);

        // Test GET vtn
        result = getJsonResult(baseURL + "default/vtns/" + tname4);
        jt = new JSONTokener(result);
        json = new JSONObject(jt);

        Assert.assertEquals("300", json.getString("idleTimeout"));
        Assert.assertEquals(timeout0, json.getString("hardTimeout"));

        // Test POST vtn5, testing hard timeout of negative value
        requestBody = "{\"description\":\"" + desc1 +
            "\",  \"hardTimeout\":\"" + timeoutNegative + "\"}";
        requestUri = baseURL + "default/vtns/" + tname5;
        result = getJsonResult(requestUri, "POST", requestBody);
        assertResponse(HTTP_CREATED);
        Assert.assertEquals(requestUri, httpLocation);

        // Test GET vtn
        result = getJsonResult(baseURL + "default/vtns/" + tname5);
        jt = new JSONTokener(result);
        json = new JSONObject(jt);

        Assert.assertEquals("300", json.getString("idleTimeout"));
        Assert.assertEquals(timeout0, json.getString("hardTimeout"));

        // Test POST vtn6, setting idle timeout of 0 and hard timeout of 65535
        requestBody = "{\"description\":\"" + desc1 +
            "\", \"idleTimeout\":\"" + timeoutMax + "\", \"hardTimeout\":\"" +
            timeout0 + "\"}";
        requestUri = baseURL + "default/vtns/" + tname6;
        result = getJsonResult(requestUri, "POST", requestBody);
        assertResponse(HTTP_CREATED);
        Assert.assertEquals(requestUri, httpLocation);

        // Test POST vtn7, setting idle timeout of 65535, hard timeout of 0
        // and vtn name of 31 characters
        requestBody = "{\"description\":\"" + desc2 +
            "\", \"idleTimeout\":\"" + timeout0 + "\", \"hardTimeout\":\"" +
            timeoutMax + "\"}";
        requestUri = baseURL + "default/vtns/" + tname7;
        result = getJsonResult(requestUri, "POST", requestBody);
        assertResponse(HTTP_CREATED);
        Assert.assertEquals(requestUri, httpLocation);

        // Test POST vtn8, setting invalid value after description to
        // requestBody
        requestBody = "{\"description\":\"" + desc1 +
            "\", \"Timeout\":\"" + timeout0 + "\", \"hard\":\"" + timeout0 +
            "\"}";
        requestUri = baseURL + "default/vtns/" + tname8;

        // Ensure that query parameters are eliminated from Location.
        result = getJsonResult(requestUri + "?param1=1&param2=2", "POST",
                               requestBody);
        assertResponse(HTTP_CREATED);
        Assert.assertEquals(requestUri, httpLocation);

        // Test GET vtn
        result = getJsonResult(baseURL + "default/vtns/" + tname8);
        jt = new JSONTokener(result);
        json = new JSONObject(jt);

        Assert.assertEquals(desc1, json.getString("description"));
        Assert.assertEquals("300", json.getString("idleTimeout"));
        Assert.assertEquals(timeout0, json.getString("hardTimeout"));

        // Test POST vtn, expecting 400
        requestBody = "{\"enabled\":\"true\"" + "\"description\":\"" + desc1 +
            "\"}";
        result = getJsonResult(baseURL + "default/vtns/" + tname, "POST",
                               requestBody);
        assertResponse(HTTP_BAD_REQUEST);

        // Test POST vtn, expecting 400, setting invalid value to idle timeout
        requestBody = "{\"description\":\"" + desc1 +
            "\", \"idleTimeout\":\"idletimeout\", \"hardTimeout\":\"" +
            htimeout1 + "\"}";
        result = getJsonResult(baseURL + "default/vtns/" + tname, "POST",
                               requestBody);
        assertResponse(HTTP_BAD_REQUEST);

        // Test POST vtn, expecting 400, setting invalid value to hard timeout
        requestBody = "{\"description\":\"" + desc1 +
            "\", \"idleTimeout\":\"" + itimeout1 +
            "\", \"hardTimeout\":\"hardtimeout\"}";
        result = getJsonResult(baseURL + "default/vtns/" + tname, "POST",
                               requestBody);
        assertResponse(HTTP_BAD_REQUEST);

        // Test POST vtn, expecting 400, setting invalid value to requestBody
        requestBody = "{\"description\":\"" + desc3 +
            "\", \"didleTimeout\":\"rdTimeout\":\"}";
        result = getJsonResult(baseURL + "default/vtns/" + tname, "POST",
                               requestBody);
        assertResponse(HTTP_BAD_REQUEST);


        // Test POST vtn expecting 105, test when vtn name is ""
        requestBody = "{}";
        result = getJsonResult(baseURL + "default/vtns/" + "", "POST",
                               requestBody);
        assertResponse(HTTP_BAD_METHOD);

        // Test POST vtn expecting 400, specifying invalid tenant name.
        requestUri = baseURL + "default/vtns/" + desc3;
        result = getJsonResult(requestUri, "POST", requestBody);
        assertResponse(HTTP_BAD_REQUEST);

        // Test POST vtn expecting 400, specifying invalid tenant name
        // which starts with "_".
        requestUri = baseURL + "default/vtns/" + "_testVtn";
        result = getJsonResult(requestUri, "POST", requestBody);
        assertResponse(HTTP_BAD_REQUEST);

        // Test POST vtn expecting 400, specifying invalid tenant name
        // including symbol "@".
        requestUri = baseURL + "default/vtns/" + "test@Vtn";
        result = getJsonResult(requestUri, "POST", requestBody);
        assertResponse(HTTP_BAD_REQUEST);

        // Test POST vtn expecting 400, specifying 65536 as idle timeout.
        requestBody =  "{\"description\":\"" + desc1 +
            "\", \"idleTimeout\":\"65536\", \"hardTimeout\":\"" +
            htimeout1 + "\"}";
        requestUri = baseURL + "default/vtns/" + tname;
        result = getJsonResult(requestUri, "POST", requestBody);
        assertResponse(HTTP_BAD_REQUEST);

        // Test POST vtn expecting 400, specifying 65536 as hard timeout.
        requestBody =  "{\"description\":\"" + desc1 +
            "\", \"idleTimeout\":\"" + itimeout1 +
            "\", \"hardTimeout\":\"65536\"}";
        requestUri = baseURL + "default/vtns/" + tname;
        result = getJsonResult(requestUri, "POST", requestBody);
        assertResponse(HTTP_BAD_REQUEST);

        // Test POST vtn expecting 400, specifying idle timeout value greater
        // than hard timeout.
        requestBody =  "{\"description\":\"" + desc1 +
            "\", \"idleTimeout\":\"" + itimeout2 +
            "\", \"hardTimeout\":\"" + htimeout1 + "\"}";
        requestUri = baseURL + "default/vtns/" + tname;
        result = getJsonResult(requestUri, "POST", requestBody);
        assertResponse(HTTP_BAD_REQUEST);

        // Test POST vtn expecting 400, specifying too long tenant name.
        requestBody =  "{}";
        requestUri = baseURL + "default/vtns/" + tname32;
        result = getJsonResult(requestUri, "POST", requestBody);
        assertResponse(HTTP_BAD_REQUEST);

        testVBridgeAPI(tname1, tname2);

        // Test GET vtn expecting 404, setting the vtn that don't exist
        result = getJsonResult(baseURL + "default/vtns/" + tname);
        assertResponse(HTTP_NOT_FOUND);

        // Test GET vtn
        result = getJsonResult(baseURL + "default/vtns/" + tname2);
        jt = new JSONTokener(result);
        json = new JSONObject(jt);

        assertResponse(HTTP_OK);
        Assert.assertEquals(tname2, json.getString("name"));
        Assert.assertEquals(desc1, json.getString("description"));
        Assert.assertEquals(itimeout1, json.getString("idleTimeout"));
        Assert.assertEquals(htimeout1, json.getString("hardTimeout"));

        // Test PUT vtn1, expecting all elements change
        requestBody = "{\"description\":\"" + desc3 +
            "\", \"idleTimeout\":\"" + timeout0 + "\", \"hardTimeout\":\"" +
            timeout0 + "\"}";
        String queryParameter = new QueryParameter("all", "true").getString();

        result = getJsonResult(baseURL + "default/vtns/" + tname1 + queryParameter, "PUT", requestBody);
        assertResponse(HTTP_OK);

        // Test GET vtn
        result = getJsonResult(baseURL + "default/vtns/" + tname1);
        jt = new JSONTokener(result);
        json = new JSONObject(jt);

        Assert.assertEquals(desc3, json.getString("description"));
        Assert.assertEquals(timeout0, json.getString("idleTimeout"));
        Assert.assertEquals(timeout0, json.getString("hardTimeout"));

        // Test PUT vtn1,  abbreviate idle timeout and hard timeout
        requestBody = "{\"description\":\"" + desc2 + "\"}";
        result = getJsonResult(baseURL + "default/vtns/" + tname1 +
                               queryParameter, "PUT", requestBody);
        assertResponse(HTTP_OK);

        // Test GET vtn
        result = getJsonResult(baseURL + "default/vtns/" + tname1);
        jt = new JSONTokener(result);
        json = new JSONObject(jt);

        Assert.assertEquals(desc2, json.getString("description"));
        Assert.assertEquals("300", json.getString("idleTimeout"));
        Assert.assertEquals("0", json.getString("hardTimeout"));

        // Test PUT vtn1, abbreviate description, testing idle timeout of
        // 65535 and hard timeout of 0
        requestBody = "{\"idleTimeout\":\"" + timeoutMax +
            "\", \"hardTimeout\":\"" + timeout0 + "\"}";
        result = getJsonResult(baseURL + "default/vtns/" + tname1 +
                               queryParameter, "PUT", requestBody);
        assertResponse(HTTP_OK);

        // Test GET vtn
        result = getJsonResult(baseURL + "default/vtns/" + tname1);
        jt = new JSONTokener(result);
        json = new JSONObject(jt);
        try {
            Assert.assertEquals("", json.getString("description"));
        } catch (JSONException expected) {
            assertThat(expected.getMessage(), is("JSONObject[\"description\"] not found."));
        }
        Assert.assertEquals(timeoutMax, json.getString("idleTimeout"));
        Assert.assertEquals(timeout0, json.getString("hardTimeout"));

        // Test PUT vtn1, abbreviate description and idle timeout, testing
        // hard timeout of 65535
        requestBody = "{\"hardTimeout\":\"" + timeoutMax + "\"}";
        result = getJsonResult(baseURL + "default/vtns/" + tname1 +
                               queryParameter, "PUT", requestBody);
        assertResponse(HTTP_OK);

        // Test GET vtn
        result = getJsonResult(baseURL + "default/vtns/" + tname1);
        jt = new JSONTokener(result);
        json = new JSONObject(jt);
        Assert.assertFalse(json.has("description"));
        Assert.assertEquals("300", json.getString("idleTimeout"));
        Assert.assertEquals(timeoutMax, json.getString("hardTimeout"));

        // Test PUT vtn1, abbreviate all elements
        requestBody = "{}";
        result = getJsonResult(baseURL + "default/vtns/" + tname1 +
                               queryParameter, "PUT", requestBody);
        assertResponse(HTTP_OK);

        // Test GET vtn
        result = getJsonResult(baseURL + "default/vtns/" + tname1);
        jt = new JSONTokener(result);
        json = new JSONObject(jt);
        Assert.assertFalse(json.has("description"));
        Assert.assertEquals("300", json.getString("idleTimeout"));
        Assert.assertEquals("0", json.getString("hardTimeout"));

        // Test PUT vtn2, expecting all elements not change
        queryParameter = new QueryParameter("all", "false").getString();
        result = getJsonResult(baseURL + "default/vtns/" + tname2 +
                               queryParameter, "PUT", requestBody);
        assertResponse(HTTP_OK);

        // Test GET vtn
        result = getJsonResult(baseURL + "default/vtns/" + tname2);
        jt = new JSONTokener(result);
        json = new JSONObject(jt);

        Assert.assertEquals(desc1, json.getString("description"));
        Assert.assertEquals(itimeout1, json.getString("idleTimeout"));
        Assert.assertEquals(htimeout1, json.getString("hardTimeout"));

        // Test PUT vtn2, setting 0 to idle timoeut and hard timeout
        requestBody = "{\"idleTimeout\":\"" + timeout0 +
            "\", \"hardTimeout\":\"" + timeout0 + "\"}";
        result = getJsonResult(baseURL + "default/vtns/" + tname2 +
                               queryParameter, "PUT", requestBody);
        assertResponse(HTTP_OK);

        // Test GET vtn
        result = getJsonResult(baseURL + "default/vtns/" + tname2);
        jt = new JSONTokener(result);
        json = new JSONObject(jt);

        Assert.assertEquals(desc1, json.getString("description"));
        Assert.assertEquals(timeout0, json.getString("idleTimeout"));
        Assert.assertEquals(timeout0, json.getString("hardTimeout"));

        // Test PUT vtn2, expecting all elements change
        requestBody = "{\"description\":\"" + desc2 +
            "\", \"idleTimeout\":\"" + itimeout1 + "\", \"hardTimeout\":\"" +
            htimeout1 + "\"}";
        result = getJsonResult(baseURL + "default/vtns/" + tname2 +
                               queryParameter, "PUT", requestBody);
        assertResponse(HTTP_OK);

        // Test GET vtn
        result = getJsonResult(baseURL + "default/vtns/" + tname2);
        jt = new JSONTokener(result);
        json = new JSONObject(jt);
        Assert.assertEquals(desc2, json.getString("description"));
        Assert.assertEquals(itimeout1, json.getString("idleTimeout"));
        Assert.assertEquals(htimeout1, json.getString("hardTimeout"));

        // Test PUT vtn2, expecting all elements not change
        requestBody = "{\"idleTimeout\":\"" + timeoutNegative +
            "\", \"hardTimeout\":\"" + timeoutNegative + "\"}";
        result = getJsonResult(baseURL + "default/vtns/" + tname2 +
                               queryParameter, "PUT", requestBody);
        assertResponse(HTTP_OK);

        // Test GET vtn
        result = getJsonResult(baseURL + "default/vtns/" + tname2);
        jt = new JSONTokener(result);
        json = new JSONObject(jt);

        Assert.assertEquals(desc2, json.getString("description"));
        Assert.assertEquals(itimeout1, json.getString("idleTimeout"));
        Assert.assertEquals(htimeout1, json.getString("hardTimeout"));

        // Test PUT vtn8, setting invalid value after description to requestBody
        requestBody = "{\"description\":\"" + desc2 + "\", \"Timeout\":\"" +
            itimeout1 + "\", \"hard\":\"" + htimeout2 + "\"}";
        result = getJsonResult(baseURL + "default/vtns/" + tname8 +
                               queryParameter, "PUT", requestBody);
        assertResponse(HTTP_OK);

        // Test GET vtn
        result = getJsonResult(baseURL + "default/vtns/" + tname8);
        jt = new JSONTokener(result);
        json = new JSONObject(jt);

        Assert.assertEquals(desc2, json.getString("description"));
        Assert.assertEquals("300", json.getString("idleTimeout"));
        Assert.assertEquals(timeout0, json.getString("hardTimeout"));


        // Test PUT vtn2, description not change
        queryParameter = new QueryParameter("all", "true").getString();
        requestBody = "{\"idleTimeout\":\"" + timeoutNegative +
            "\", \"hardTimeout\":\"" + timeoutNegative + "\"}";
        result = getJsonResult(baseURL + "default/vtns/" + tname2 +
                               queryParameter, "PUT", requestBody);
        assertResponse(HTTP_OK);

        // Test GET vtn
        result = getJsonResult(baseURL + "default/vtns/" + tname2);
        jt = new JSONTokener(result);
        json = new JSONObject(jt);

        Assert.assertFalse(json.has("description"));
        Assert.assertEquals("300", json.getString("idleTimeout"));
        Assert.assertEquals(timeout0, json.getString("hardTimeout"));

        // Test PUT vtn, expecting 400, setting the invalid value to idle timeout
        requestBody = "{\"idleTimeout\":\"" + desc1 +
            "\", \"hardTimeout\":\"" + htimeout1 + "\"}";
        result = getJsonResult(baseURL + "default/vtns/" + tname1 +
                               queryParameter, "PUT", requestBody);
        assertResponse(HTTP_BAD_REQUEST);

        // Test PUT vtn, expecting 400, setting the invalid value to hard timeout
        requestBody = "{\"idleTimeout\":\"" + itimeout1 +
            "\", \"hardTimeout\":\"" + desc1 + "\"}";
        result = getJsonResult(baseURL + "default/vtns/" + tname1 +
                               queryParameter, "PUT", requestBody);
        assertResponse(HTTP_BAD_REQUEST);

        // Test PUT vtn, expecting 400, setting invalid value to requestBody
        requestBody = "{\"description\":\"" + desc3 +
            "\", \"didleTimeout\":\"rdTimeout\":\"}";
        result = getJsonResult(baseURL + "default/vtns/" + tname1 +
                               queryParameter, "PUT", requestBody);
        assertResponse(HTTP_BAD_REQUEST);


        // Test PUT vtn expecting 404, setting the vtn that don't exist
        requestBody = "{\"description\":\"" + desc2 +
            "\", \"idleTimeout\":\"" + itimeout1 + "\", \"hardTimeout\":\"" +
            htimeout1 + "\"}";
        result = getJsonResult(baseURL + "default/vtns/" + tname +
                               queryParameter, "PUT", requestBody);
        assertResponse(HTTP_NOT_FOUND);

        // Test PUT vtn expecting 400, setting invalid value to requestBody
        requestBody = "{\"Test\"}";
        result = getJsonResult(baseURL + "default/vtns/" + tname1 +
                               queryParameter, "PUT", requestBody);
        assertResponse(HTTP_BAD_REQUEST);

        // Test PUT vtn expecting 400, specifying 65540 as idle timeout.
        requestBody = "{\"description\":\"" + desc2 +
            "\", \"idleTimeout\":\"" + timeoutOver +
            "\", \"hardTimeout\":\"" + htimeout2 + "\"}";
        result = getJsonResult(baseURL + "default/vtns/" + tname1 +
                               queryParameter, "PUT", requestBody);
        assertResponse(HTTP_BAD_REQUEST);

        // Test PUT vtn expecting 400, specifying 65540 as hard timeout.
        requestBody = "{\"description\":\"" + desc2 +
            "\", \"idleTimeout\":\"" + itimeout1 +
            "\", \"hardTimeout\":\"" + timeoutOver + "\"}";
        result = getJsonResult(baseURL + "default/vtns/" + tname1 +
                               queryParameter, "PUT", requestBody);
        assertResponse(HTTP_BAD_REQUEST);

        // Test PUT vtn expecting 400, specifying idle timeout value greater
        // than hard timeout.
        requestBody =  "{\"description\":\"" + desc1 +
            "\", \"idleTimeout\":\"" + itimeout2 +
            "\", \"hardTimeout\":\"" + htimeout1 + "\"}";
        result = getJsonResult(baseURL + "default/vtns/" + tname1 +
                               queryParameter, "PUT", requestBody);
        assertResponse(HTTP_BAD_REQUEST);

        // Test PUT vtn
        requestBody = "{\"description\":\"" + desc2 +
            "\", \"idleTimeout\":\"" + itimeout2 + "\", \"hardTimeout\":\"" +
            timeoutNegative + "\"}";
        result = getJsonResult(baseURL + "default/vtns/" + tname1 +
                               queryParameter, "PUT", requestBody);
        assertResponse(HTTP_OK);

        // Test GET all vtns in default container
        result = getJsonResult(baseURL + "default/vtns");
        jt = new JSONTokener(result);
        json = new JSONObject(jt);
        vtnArray = json.getJSONArray("vtn");

        assertResponse(HTTP_OK);
        Assert.assertEquals(8, vtnArray.length());

        // Test DELETE vtn expecting 404
        result = getJsonResult(baseURL + "default/vtns/" + tname, "DELETE");
        assertResponse(HTTP_NOT_FOUND);

        // set not supported "Content-type". expect to return 415.
        requestBody = "{}";
        result = getJsonResult(baseURL + "default/vtns/" + tname, "POST",
                               requestBody,
                               "text/plain");
        assertResponse(HTTP_UNSUPPORTED_TYPE);

        requestBody = "{\"description\":\"desc\"}";
        result = getJsonResult(baseURL + "default/vtns/" + tname1 +
                               queryParameter, "PUT", requestBody,
                               "text/plain");
        assertResponse(HTTP_UNSUPPORTED_TYPE);

        // Authentication failure.
        checkVTNError(GlobalConstants.DEFAULT.toString(), tname1, false,
                      HttpURLConnection.HTTP_UNAUTHORIZED);

        // Invalid container name.
        String[] invalid = {"bad_container", "version"};
        for (String inv: invalid) {
            checkVTNError(inv, tname1, true, HttpURLConnection.HTTP_NOT_FOUND);
            checkVTNError(inv, tname1, false,
                          HttpURLConnection.HTTP_UNAUTHORIZED);
        }


        // Test DELETE vtn
        result = getJsonResult(baseURL + "default/vtns/" + tname1, "DELETE");
        assertResponse(HTTP_OK);

        // Test DELETE vtn2
        result = getJsonResult(baseURL + "default/vtns/" + tname2, "DELETE");
        assertResponse(HTTP_OK);

        // Test DELETE vtn3
        result = getJsonResult(baseURL + "default/vtns/" + tname3, "DELETE");
        assertResponse(HTTP_OK);

        // Test DELETE vtn4
        result = getJsonResult(baseURL + "default/vtns/" + tname4, "DELETE");
        assertResponse(HTTP_OK);

        // Test DELETE vtn5
        result = getJsonResult(baseURL + "default/vtns/" + tname5, "DELETE");
        assertResponse(HTTP_OK);

        // Test DELETE vtn6
        result = getJsonResult(baseURL + "default/vtns/" + tname6, "DELETE");
        assertResponse(HTTP_OK);

        // Test DELETE vtn7
        result = getJsonResult(baseURL + "default/vtns/" + tname7, "DELETE");
        assertResponse(HTTP_OK);

        // Test DELETE vtn8
        result = getJsonResult(baseURL + "default/vtns/" + tname8, "DELETE");
        assertResponse(HTTP_OK);


        // Test DELETE vtn expecting 404
        result = getJsonResult(baseURL + "default/vtns/" + tname1, "DELETE");
        assertResponse(HTTP_NOT_FOUND);

        // Test GET vtn in default container, expecting no results
        result = getJsonResult(baseURL + "default/vtns");
        assertResponse(HTTP_OK);
        jt = new JSONTokener(result);
        json = new JSONObject(jt);
        vtnArray = json.getJSONArray("vtn");
        Assert.assertEquals(0, vtnArray.length());

        testVtnGlobal(baseURL);
    }

    /**
     * Ensure that the VTN APIs return correct error response.
     *
     * @param containerName   The container name.
     * @param tname           The tenant name for the test.
     * @param auth            If {@code true}, the client sends authenticated
     *                        request. Otherwise the client sends unauthorized
     *                        request.
     * @param expected        Expected HTTP response code.
     */
    private void checkVTNError(String containerName, String tname,
                               boolean auth, int expected) {
        String base = VTN_BASE_URL + containerName + "/vtns";
        String ct = "Application/Json";
        String body = "{}";
        getJsonResult(base, "GET", null, ct,  auth);
        assertResponse(expected);

        String uri = base + "/" + tname;
        getJsonResult(uri, "POST", body, ct, auth);
        assertResponse(expected);

        getJsonResult(uri, "GET", null, ct, auth);
        assertResponse(expected);

        String qp = new QueryParameter("all", "true").toString();
        getJsonResult(uri + qp, "PUT", body, ct, auth);
        assertResponse(expected);

        getJsonResult(uri, "DELETE", null, ct, auth);
        assertResponse(expected);
    }

    /**
     * test case for VBridge APIs.
     *
     * This method is called by {@link #testVTNAPI()}.
     *
     * This calls {@link #testVLANMappingAPI(String, String)},
     * {@link #testVBridgeInterfaceAPI(String, String, String)},
     * {@link #testVBridgeInterfaceDeleteAPI(String, String)},
     * {@link #testVLANMappingDeleteAPI(String, String)},
     * {@link #testMacMappingAPI(String, String, String)},
     * {@link #testMacAddressAPI(String, String)}.
     *
     * @param tname1    A tenant name.
     *                  Specified tenant is necessary to be configured
     *                  before method is called.
     * @param tname2    A tenant name.
     *                  This tenant is also necessary to be configured
     *                  before method is called.
     * @throws JSONException  An error occurred.
     */
    private void testVBridgeAPI(String tname1, String tname2)
        throws JSONException {
        LOG.info("Starting vBridge JAX-RS client.");
        String url = VTN_BASE_URL;
        StringBuilder baseURL = new StringBuilder();
        baseURL.append(url);
        baseURL.append("default/vtns/");

        String tnameDummy = "tenant_dummy";
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
        String ageinterOver = "1000001";
        String fault = "";

        // Test GET vBridges in default container expecting 404, setting dummy tenant
        String result = getJsonResult(baseURL + tnameDummy + "/vbridges");
        assertResponse(HTTP_NOT_FOUND);

        // Test GET vBridges in default container, expecting no results
        result = getJsonResult(baseURL + tname1 + "/vbridges");
        assertResponse(HTTP_OK);
        JSONTokener jt = new JSONTokener(result);
        JSONObject json = new JSONObject(jt);
        JSONArray vBridgeArray = json.getJSONArray("vbridge");
        Assert.assertEquals(0, vBridgeArray.length());


        // Test POST vBridge1 expecting 404, setting dummy tenant
        String requestBody = "{}";
        result = getJsonResult(baseURL + tnameDummy + "/vbridges/" + bname1,
                               "POST" , requestBody);
        assertResponse(HTTP_NOT_FOUND);

        // Test POST vBridge1 expecting 400, specifying too small ageInterval.
        requestBody = "{\"description\":\"" + desc1 +
            "\", \"ageInterval\":\"" + ageinter0 + "\"}";
        result = getJsonResult(baseURL + tname1 + "/vbridges/" + bname1 ,
                               "POST", requestBody);
        assertResponse(HTTP_BAD_REQUEST);

        // Test POST vBridge1
        requestBody = "{}";
        String requestUri = baseURL + tname1 + "/vbridges/" + bname1;
        result = getJsonResult(requestUri, "POST", requestBody);
        assertResponse(HTTP_CREATED);
        Assert.assertEquals(requestUri, httpLocation);

        // Test GET vBridges in default container, expecting one result
        result = getJsonResult(baseURL + tname1 + "/vbridges");
        assertResponse(HTTP_OK);
        jt = new JSONTokener(result);
        json = new JSONObject(jt);
        vBridgeArray = json.getJSONArray("vbridge");
        Assert.assertEquals(1, vBridgeArray.length());

        // Test POST vBridge1 expecting 409
        requestBody = "{}";
        result = getJsonResult(baseURL + tname1 + "/vbridges/" + bname1, "POST", requestBody);
        assertResponse(HTTP_CONFLICT);

        // Test POST vBridge2
        requestBody = "{\"description\":\"" + desc2 + "\", \"ageInterval\":\"" + ageinter1 + "\"}";
        requestUri = baseURL + tname1 + "/vbridges/" + bname2;
        result = getJsonResult(requestUri, "POST", requestBody);
        assertResponse(HTTP_CREATED);
        Assert.assertEquals(requestUri, httpLocation);

        // Test POST vBridge2 for other tenant
        requestBody = "{\"ageInterval\":\"" + ageinter2 + "\"}";
        requestUri = baseURL + tname2 + "/vbridges/" + bname2;
        result = getJsonResult(requestUri, "POST", requestBody);
        assertResponse(HTTP_CREATED);
        Assert.assertEquals(requestUri, httpLocation);

        // Test GET all vBridges in default container
        result = getJsonResult(baseURL + tname1 + "/vbridges");
        jt = new JSONTokener(result);
        json = new JSONObject(jt);
        vBridgeArray = json.getJSONArray("vbridge");
        JSONObject vBridge;

        assertResponse(HTTP_OK);
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
        testMacMappingAPI(tname1, bname1, bname2);
        testVBridgeInterfaceAPI(tname1, bname1, bname2);

        // Test POST vBridge3
        requestBody = "{\"description\":\"" + desc3 + "\", \"ageInterval\":\"" + ageinter3 + "\"}";
        requestUri = baseURL + tname1 + "/vbridges/" + bname3;
        result = getJsonResult(requestUri, "POST", requestBody);
        assertResponse(HTTP_CREATED);
        Assert.assertEquals(requestUri, httpLocation);

        // Test GET vBridge3
        result = getJsonResult(baseURL + tname1 + "/vbridges/" + bname3);
        assertResponse(HTTP_OK);
        jt = new JSONTokener(result);
        json = new JSONObject(jt);

        Assert.assertEquals(desc3, json.getString("description"));
        Assert.assertEquals("600", json.getString("ageInterval"));

        // Test POST vBridge4
        requestBody = "{\"description\":\"" + desc1 + "\", \"ageInterval\":\"" + ageinter4 + "\"}";
        requestUri = baseURL + tname1 + "/vbridges/" + bname4;

        // Ensure that query parameters are eliminated from Location.
        result = getJsonResult(requestUri + "?param1=1&param2=2", "POST",
                               requestBody);
        assertResponse(HTTP_CREATED);
        Assert.assertEquals(requestUri, httpLocation);

        // Test POST vBridge expecting 400
        requestBody = "{\"description\":\"" + desc1 + "\", \"ageInterval\":\"" + "ageInterval" + "\"}";
        requestUri = baseURL + tname1 + "/vbridges/" + ebname;
        result = getJsonResult(requestUri, "POST", requestBody);
        assertResponse(HTTP_BAD_REQUEST);

        // Test POST vBridge expecting 400, specifying too long vBridge name.
        requestBody = "{}";
        requestUri = baseURL + tname1 + "/vbridges/" + bname32;
        result = getJsonResult(requestUri, "POST", requestBody);
        assertResponse(HTTP_BAD_REQUEST);

        // Test POST vBridge expecting 405, setting "" to vBridge name
        result = getJsonResult(baseURL + tname1 + "/vbridges/" + "", "POST", requestBody);
        assertResponse(HTTP_BAD_METHOD);

        // Test POST vBridge expecting 400, specifying invalid vBridge name
        // which starts with "_".
        result = getJsonResult(baseURL + tname1 + "/vbridges/" +
                               "_vbridgename", "POST", requestBody);
        assertResponse(HTTP_BAD_REQUEST);

        // Test POST vBridge expecting 400, specifying invalid vBridge name
        // including symbol "@".
        result = getJsonResult(baseURL + tname1 + "/vbridges/" +
                               "vbridge@name", "POST", requestBody);
        assertResponse(HTTP_BAD_REQUEST);

        // Test POST vBridge expecting 400, specifying too large ageInterval.
        requestBody = "{\"ageInterval\":\"" + ageinterOver + "\"}";
        result = getJsonResult(baseURL + tname1 + "/vbridges/" + ebname,
                               "POST", requestBody);
        assertResponse(HTTP_BAD_REQUEST);

        // Test GET vBridges in default container, expecting 4 results
        result = getJsonResult(baseURL + tname1 + "/vbridges");
        assertResponse(HTTP_OK);
        jt = new JSONTokener(result);
        json = new JSONObject(jt);
        vBridgeArray = json.getJSONArray("vbridge");
        Assert.assertEquals(4, vBridgeArray.length());

        // Test GET vBridges in default container, expecting 1 result
        result = getJsonResult(baseURL + tname2 + "/vbridges");
        assertResponse(HTTP_OK);
        jt = new JSONTokener(result);
        json = new JSONObject(jt);
        vBridgeArray = json.getJSONArray("vbridge");
        Assert.assertEquals(1, vBridgeArray.length());

        // Test GET vBridge expecting 404 setting dummy vtn
        result = getJsonResult(baseURL + tnameDummy + "/vbridges/" + bname1);
        assertResponse(HTTP_NOT_FOUND);

        // Test GET vBridge expecting 404 setting dummy vBridge
        result = getJsonResult(baseURL + tname1 + "/vbridges/" + ebname);
        assertResponse(HTTP_NOT_FOUND);

        // Test GET vBridge
        result = getJsonResult(baseURL + tname1 + "/vbridges/" + bname2);
        jt = new JSONTokener(result);
        json = new JSONObject(jt);

        assertResponse(HTTP_OK);
        Assert.assertEquals(bname2, json.getString("name"));
        Assert.assertEquals(desc2, json.getString("description"));
        Assert.assertEquals(ageinter1, json.getString("ageInterval"));
        Assert.assertEquals(fault, json.getString("faults"));
        Assert.assertEquals("1", json.getString("state"));

        // Test GET vBridge, get from other tenant
        result = getJsonResult(baseURL + tname2 + "/vbridges/" + bname2);
        assertResponse(HTTP_OK);

        // Test PUT vBridge1, setting only description (queryparameter is true)
        String queryParameter = new QueryParameter("all", "true").getString();
        requestBody = "{\"description\":\"" + desc1 + "\"}";
        result = getJsonResult(baseURL + tname1 + "/vbridges/" + bname1 + queryParameter, "PUT", requestBody);
        assertResponse(HTTP_OK);

        // Test GET vBridge1
        result = getJsonResult(baseURL + tname1 + "/vbridges/" + bname1);
        assertResponse(HTTP_OK);
        jt = new JSONTokener(result);
        json = new JSONObject(jt);

        Assert.assertEquals(desc1, json.getString("description"));
        Assert.assertEquals("600", json.getString("ageInterval"));

        // Test PUT vBridge1, setting only ageInter (queryparameter is true)
        requestBody = "{\"ageInterval\":\"" + ageinter1 + "\"}";
        result = getJsonResult(baseURL + tname1 + "/vbridges/" + bname1 + queryParameter, "PUT", requestBody);
        assertResponse(HTTP_OK);

        // Test GET vBridge1
        result = getJsonResult(baseURL + tname1 + "/vbridges/" + bname1);
        assertResponse(HTTP_OK);
        jt = new JSONTokener(result);
        json = new JSONObject(jt);

        Assert.assertFalse(json.has("description"));
        Assert.assertEquals(ageinter1, json.getString("ageInterval"));

        // Test PUT vBridge1, setting description and ageInter (queryparameter is true)
        requestBody = "{\"description\":\"" + desc2 + "\", \"ageInterval\":\"" + ageinter4 + "\"}";
        result = getJsonResult(baseURL + tname1 + "/vbridges/" + bname1 + queryParameter, "PUT", requestBody);
        assertResponse(HTTP_OK);

        // Test GET vBridge1
        result = getJsonResult(baseURL + tname1 + "/vbridges/" + bname1);
        assertResponse(HTTP_OK);
        jt = new JSONTokener(result);
        json = new JSONObject(jt);

        Assert.assertEquals(desc2, json.getString("description"));
        Assert.assertEquals(ageinter4, json.getString("ageInterval"));

        // Test PUT vBridge1, setting {} (queryparameter is true)
        requestBody = "{}";
        result = getJsonResult(baseURL + tname1 + "/vbridges/" + bname1 + queryParameter, "PUT", requestBody);
        assertResponse(HTTP_OK);

        // Test GET vBridge1
        result = getJsonResult(baseURL + tname1 + "/vbridges/" + bname1);
        assertResponse(HTTP_OK);
        jt = new JSONTokener(result);
        json = new JSONObject(jt);

        Assert.assertFalse(json.has("description"));
        Assert.assertEquals("600", json.getString("ageInterval"));

        // Test PUT vBridge2 expecting not change (query parameter is false and requestBody is {})
        queryParameter = new QueryParameter("all", "false").getString();
        result = getJsonResult(baseURL + tname1 + "/vbridges/" + bname2 + queryParameter, "PUT", requestBody);
        assertResponse(HTTP_OK);

        // Test GET vBridge2
        result = getJsonResult(baseURL + tname1 + "/vbridges/" + bname2);
        assertResponse(HTTP_OK);
        jt = new JSONTokener(result);
        json = new JSONObject(jt);

        Assert.assertEquals(desc2, json.getString("description"));
        Assert.assertEquals(ageinter1, json.getString("ageInterval"));

        // Test PUT vBridge2, setting description (query parameter is false)
        requestBody = "{\"description\":\"" + desc1 + "\"}";
        result = getJsonResult(baseURL + tname1 + "/vbridges/" + bname2 + queryParameter, "PUT", requestBody);
        assertResponse(HTTP_OK);

        // Test GET vBridge2
        result = getJsonResult(baseURL + tname1 + "/vbridges/" + bname2);
        assertResponse(HTTP_OK);
        jt = new JSONTokener(result);
        json = new JSONObject(jt);

        Assert.assertEquals(desc1, json.getString("description"));
        Assert.assertEquals(ageinter1, json.getString("ageInterval"));

        // Test PUT vBridge2, setting ageInter (query parameter is false)
        requestBody = "{\"ageInterval\":\"" + ageinter2 + "\"}";
        result = getJsonResult(baseURL + tname1 + "/vbridges/" + bname2 + queryParameter, "PUT", requestBody);
        assertResponse(HTTP_OK);

        // Test GET vBridge2
        result = getJsonResult(baseURL + tname1 + "/vbridges/" + bname2);
        assertResponse(HTTP_OK);
        jt = new JSONTokener(result);
        json = new JSONObject(jt);

        Assert.assertEquals(desc1, json.getString("description"));
        Assert.assertEquals(ageinter2, json.getString("ageInterval"));

        // Test PUT vBridge2, setting description and ageInter (query parameter is false)
        requestBody = "{\"description\":\"" + desc3 + "\", \"ageInterval\":\"" + ageinter3 + "\"}";
        result = getJsonResult(baseURL + tname1 + "/vbridges/" + bname2 + queryParameter, "PUT", requestBody);
        assertResponse(HTTP_OK);

        // Test GET vBridge2
        result = getJsonResult(baseURL + tname1 + "/vbridges/" + bname2);
        assertResponse(HTTP_OK);
        jt = new JSONTokener(result);
        json = new JSONObject(jt);

        Assert.assertEquals(desc3, json.getString("description"));
        Assert.assertEquals("100", json.getString("ageInterval"));

        // Test PUT vBridge2, setting description and ageInter (query parameter is true)
        queryParameter = new QueryParameter("all", "true").getString();
        requestBody = "{\"description\":\"" + desc3 + "\", \"ageInterval\":\"" + ageinter3 + "\"}";
        result = getJsonResult(baseURL + tname1 + "/vbridges/" + bname2 + queryParameter, "PUT", requestBody);
        assertResponse(HTTP_OK);

        // Test GET vBridge2
        result = getJsonResult(baseURL + tname1 + "/vbridges/" + bname2);
        assertResponse(HTTP_OK);
        jt = new JSONTokener(result);
        json = new JSONObject(jt);

        Assert.assertEquals(desc3, json.getString("description"));
        Assert.assertEquals("600", json.getString("ageInterval"));

        // Test PUT vBridge1 expecting 400
        requestBody = "{\"ageInterval\":\"" + "ageinter" + "\"}";
        result = getJsonResult(baseURL +  tname1 + "/vbridges/" + bname1 +
                               queryParameter, "PUT", requestBody);
        assertResponse(HTTP_BAD_REQUEST);

        // Test PUT vBridge1 expecting 404, setting dummy vtn
        requestBody = "{}";
        result = getJsonResult(baseURL +  tnameDummy + "/vbridges/" + bname1 +
                               queryParameter, "PUT", requestBody);
        assertResponse(HTTP_NOT_FOUND);

        // Test PUT vBridge1 expecting 404, setting dummy vbridge
        result = getJsonResult(baseURL +  tname2 + "/vbridges/" + bname1 +
                               queryParameter, "PUT", requestBody);
        assertResponse(HTTP_NOT_FOUND);

        // Test PUT vBridge1 expecting 400, specifying too small ageInterval.
        queryParameter = new QueryParameter("all", "false").getString();
        requestBody = "{\"description\":\"" + desc2 +
            "\", \"ageInterval\":\"" + ageinter0 + "\"}";
        result = getJsonResult(baseURL + tname1 + "/vbridges/" + bname1 +
                               queryParameter, "PUT", requestBody);
        assertResponse(HTTP_BAD_REQUEST);

        // Test PUT vBridge1 expecting 400, specifying too small ageInterval
        // ("all=true" is specified as query paramter).
        queryParameter = new QueryParameter("all", "true").getString();
        requestBody = "{\"description\":\"" + desc2 +
            "\", \"ageInterval\":\"" + ageinter0 + "\"}";
        result = getJsonResult(baseURL + tname1 + "/vbridges/" + bname1 +
                               queryParameter, "PUT", requestBody);
        assertResponse(HTTP_BAD_REQUEST);

        // Test PUT vBridge1 expecting 400, specifying too large ageInterval
        // ("all=true" is specified as query paramter).
        requestBody = "{\"description\":\"" + desc1 +
            "\", \"ageInterval\":\"" + ageinterOver + "\"}";
        result = getJsonResult(baseURL + tname1 + "/vbridges/" + bname1 +
                               queryParameter, "PUT", requestBody);
        assertResponse(HTTP_BAD_REQUEST);

        testVLANMappingDeleteAPI(tname1, bname1);
        testMacAddressAPI(tname1, bname1);
        testVBridgeInterfaceDeleteAPI(tname1, bname1);


        // Test DELETE vBridge expecting 404, setting dummy tenant
        result = getJsonResult(baseURL + tnameDummy + "/vbridges/" + bname1,
                               "DELETE");
        assertResponse(HTTP_NOT_FOUND);

        // Test DELETE vBridge1
        result = getJsonResult(baseURL + tname1 + "/vbridges/" + bname1,
                               "DELETE");
        assertResponse(HTTP_OK);

        // specify not supported Content-Type
        requestBody = "{}";
        result = getJsonResult(baseURL + tname1 + "/vbridges/" + bname1, "POST",
                               requestBody, "text/plain");
        assertResponse(HTTP_UNSUPPORTED_TYPE);

        requestBody = "{\"description\":\"test\"}";
        result = getJsonResult(baseURL + tname1 + "/vbridges/" + bname2 +
                               queryParameter, "PUT", requestBody,
                               "text/plain");
        assertResponse(HTTP_UNSUPPORTED_TYPE);

        // Authentication failure.
        checkVBridgeError(GlobalConstants.DEFAULT.toString(), tname1, tname2,
                          bname1, bname2, false,
                          HttpURLConnection.HTTP_UNAUTHORIZED);

        // Invalid container name.
        String[] invalid = {"bad_container", "version"};
        for (String inv: invalid) {
            checkVBridgeError(inv, tname1, tname2, bname1, bname2, true,
                              HttpURLConnection.HTTP_NOT_FOUND);
            checkVBridgeError(inv, tname1, tname2, bname1, bname2, false,
                              HttpURLConnection.HTTP_UNAUTHORIZED);
        }

        // Test DELETE vBridge2
        result = getJsonResult(baseURL + tname1 + "/vbridges/" + bname2,
                               "DELETE");
        assertResponse(HTTP_OK);

        // Test DELETE vBridge3
        result = getJsonResult(baseURL + tname1 + "/vbridges/" + bname3,
                               "DELETE");
        assertResponse(HTTP_OK);

        // Test DELETE vBridge4
        result = getJsonResult(baseURL + tname1 + "/vbridges/" + bname4,
                               "DELETE");
        assertResponse(HTTP_OK);

        // Test DELETE vBridge2 on other tenant
        result = getJsonResult(baseURL + tname2 + "/vbridges/" + bname2,
                               "DELETE");
        assertResponse(HTTP_OK);

        // Test DELETE vBridge expecting 404
        result = getJsonResult(baseURL + tname1 + "/vbridges/" + bname1,
                               "DELETE");
        assertResponse(HTTP_NOT_FOUND);

        // Test DELETE vBridge expecting 404, setting dummy tenant
        result = getJsonResult(baseURL + tname1 + "/vbridges/" + ebname,
                               "DELETE");
        assertResponse(HTTP_NOT_FOUND);

        // Test GET vBridges in default container, expecting no results (tname1)
        result = getJsonResult(baseURL + tname1 + "/vbridges");
        assertResponse(HTTP_OK);
        jt = new JSONTokener(result);
        json = new JSONObject(jt);
        vBridgeArray = json.getJSONArray("vbridge");
        Assert.assertEquals(0, vBridgeArray.length());

        // Test GET vBridges in default container, expecting no results (tname2)
        result = getJsonResult(baseURL + tname2 + "/vbridges");
        assertResponse(HTTP_OK);
        jt = new JSONTokener(result);
        json = new JSONObject(jt);
        vBridgeArray = json.getJSONArray("vbridge");
        Assert.assertEquals(0, vBridgeArray.length());
    }

    /**
     * Ensure that the VBridge APIs return correct error response.
     *
     * @param containerName   The container name.
     * @param tname1          The tenant name for the test.
     * @param tname2          The tenant name for the test.
     * @param bname1          The bridge name for the test.
     * @param bname2          The bridge name for the test.
     * @param auth            If {@code true}, the client sends authenticated
     *                        request. Otherwise the client sends unauthorized
     *                        request.
     * @param expected        Expected HTTP response code.
     */
    private void checkVBridgeError(String containerName, String tname1,
                                   String tname2, String bname1, String bname2,
                                   boolean auth, int expected) {
        String base = VTN_BASE_URL + containerName + "/vtns/";
        String ct = "Application/Json";
        String body = "{}";
        getJsonResult(base + tname1 + "/vbridges/" + bname1, "POST", body, ct,
                      auth);
        assertResponse(expected);

        String qp = new QueryParameter("all", "true").getString();
        body = "{\"description\":\"test\"}";
        getJsonResult(base + tname1 + "/vbridges/" + bname2 + qp, "PUT", body,
                      ct, auth);
        assertResponse(expected);

        getJsonResult(base + tname2 + "/vbridges", "GET", null, ct, auth);
        assertResponse(expected);

        getJsonResult(base + tname2 + "/vbridges/" + bname2, "GET", null, ct,
                      auth);
        assertResponse(expected);

        getJsonResult(base + tname1 + "/vbridges/" + bname2, "DELETE", null,
                      ct, auth);
        assertResponse(expected);
    }

    /**
     * Test case for VBridgeInterface APIs.
     *
     * This method called by {@code testVBridge}.
     * This calls {@code testPortMappingAPI}.
     *
     * @param tname1    A tenant name.
     *                  Specified tenant is necessary to be configured
     *                  before method is called.
     * @param bname1    A vBridge name.
     *                  Specified vBridge is necessary to be configured
     *                  before method is called.
     * @param bname2    A vBridge name.
     *                  This vBridge is also necessary to be configured
     *                  before method is called.
     * @throws JSONException  An error occurred.
     */
    private void testVBridgeInterfaceAPI(String tname1, String bname1,
                                         String bname2) throws JSONException {
        LOG.info("Starting vBridge Intergace JAX-RS client.");
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

        String bnameDummy = "bname_dummy";
        String tnameDummy = "tenant_dummy";

        String desc1 = "testDescription1";
        String desc2 = "t";
        String desc3 = String.format("%01000d", 1);


        // Test GET vBridge Interfaces in default container, expecting no
        // results
        String result = getJsonResult(baseURL + bname1 + "/interfaces");
        assertResponse(HTTP_OK);
        JSONTokener jt = new JSONTokener(result);
        JSONObject json = new JSONObject(jt);
        JSONArray vbridgeifArray = json.getJSONArray("interface");
        Assert.assertEquals(0, vbridgeifArray.length());

        // Test POST vBridge Interface1 expecting 404
        String requestBody = "{}";
        result = getJsonResult(baseURL + bnameDummy + "/interfaces/" + ifname1,
                               "POST" , requestBody);
        assertResponse(HTTP_NOT_FOUND);

        // Test POST vbridge interface expecting 201
        // setting vbridge Interface1
        requestBody = "{}";
        String requestUri = baseURL + bname1 + "/interfaces/" + ifname1;
        result = getJsonResult(requestUri, "POST", requestBody);
        assertResponse(HTTP_CREATED);
        Assert.assertEquals(requestUri, httpLocation);

        // Test POST vbridge interface expecting 409
        // setting vbridge Interface1
        result = getJsonResult(baseURL + bname1 + "/interfaces/" + ifname1,
                               "POST", requestBody);
        assertResponse(HTTP_CONFLICT);

        // Test GET vbridge interface expecitng 404
        // setting dummy vtn
        result = getJsonResult(url + "default/vtns/" + tnameDummy +
                               "/vbridges/" + bname1 + "/interfaces");
        assertResponse(HTTP_NOT_FOUND);

        // setting dummy vbridge
        result = getJsonResult(baseURL + bnameDummy + "/interfaces");
        assertResponse(HTTP_NOT_FOUND);

        // Test GET vBridge Interfaces in default container, expecting one
        // result
        result = getJsonResult(baseURL + bname1 + "/interfaces");
        assertResponse(HTTP_OK);
        jt = new JSONTokener(result);
        json = new JSONObject(jt);
        vbridgeifArray = json.getJSONArray("interface");
        Assert.assertEquals(1, vbridgeifArray.length());

        // Test POST vbridge interface expecting 200
        // setting vbridge Interface2
        requestBody = "{\"description\":\"" + desc1 + "\", \"enabled\":true}";
        requestUri = baseURL + bname1 + "/interfaces/" + ifname2;
        result = getJsonResult(requestUri, "POST", requestBody);
        assertResponse(HTTP_CREATED);
        Assert.assertEquals(requestUri, httpLocation);

        // setting vbridge Interface3
        requestBody = "{\"description\":\"" + desc2 + "\", \"enabled\":true}";
        requestUri = baseURL + bname2 + "/interfaces/" + ifname3;
        result = getJsonResult(requestUri, "POST", requestBody);
        assertResponse(HTTP_CREATED);
        Assert.assertEquals(requestUri, httpLocation);

        testPortMappingAPI(tname1, bname1, bname2, ifname2, ifname3);

        // Test POST vBridge Interface2, for other tenant
        requestBody = "{\"description\":\"" + desc1 + "\", \"enabled\":true}";
        requestUri = baseURL2 + bname2 + "/interfaces/" + ifname2;
        result = getJsonResult(requestUri, "POST", requestBody);
        assertResponse(HTTP_CREATED);
        Assert.assertEquals(requestUri, httpLocation);

        // Test GET all vBridge Interfaces in default container
        result = getJsonResult(baseURL + bname1 + "/interfaces");
        jt = new JSONTokener(result);
        json = new JSONObject(jt);
        vbridgeifArray = json.getJSONArray("interface");
        JSONObject vbridgeif;

        assertResponse(HTTP_OK);
        Assert.assertEquals(2, vbridgeifArray.length());

        for (int i = 0; i < vbridgeifArray.length(); i++) {
            vbridgeif = vbridgeifArray.getJSONObject(i);
            if (vbridgeif.getString("name").equals(ifname1)) {
                Assert.assertFalse(vbridgeif.has("description"));

                try {
                    Assert.assertEquals("true", vbridgeif.getString("enabled"));
                } catch (JSONException expected) {
                    assertThat(expected.getMessage(),
                               is("JSONObject[\"enabled\"] not found."));
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
            } else {
                // Unexpected vBridge Interface name
                Assert.assertTrue(false);
            }
        }

        // Test POST vBridge Interface4
        requestBody = "{\"enabled\":false}";
        requestUri = baseURL + bname1 + "/interfaces/" + ifname4;
        result = getJsonResult(requestUri, "POST", requestBody);
        assertResponse(HTTP_CREATED);
        Assert.assertEquals(requestUri, httpLocation);

        // Test GET vBridge Interface4
        result = getJsonResult(baseURL + bname1 + "/interfaces/" + ifname4);
        assertResponse(HTTP_OK);

        jt = new JSONTokener(result);
        json = new JSONObject(jt);

        Assert.assertFalse(json.has("description"));
        Assert.assertEquals("false", json.getString("enabled"));

        // Test POST vBridge Interface5
        requestBody = "{\"description\":\"" + desc3 + "\"}";
        requestUri = baseURL + bname1 + "/interfaces/" + ifname5;

        // Ensure that query parameters are eliminated from Location.
        result = getJsonResult(requestUri + "?param1=1&param2=2", "POST",
                               requestBody);
        assertResponse(HTTP_CREATED);
        Assert.assertEquals(requestUri, httpLocation);

        // Test POST vBridge Interface expecting 400, setting invalid value
        // for ageInterval
        requestBody = "{\"description\":\"" + desc2 +
            "\", \"enabled\":enabled}";
        requestUri = baseURL + bname1 + "/interfaces/" + ifname;
        result = getJsonResult(requestUri, "POST", requestBody);
        assertResponse(HTTP_BAD_REQUEST);

        // Test POST vBridge Interface expecting 404, setting dummy tenant
        requestBody = "{}";
        result = getJsonResult(url + "default/vtns/" + tnameDummy +
                               "/vbridges/" + bname1 + "/interfaces/" + ifname,
                               "POST", requestBody);
        assertResponse(HTTP_NOT_FOUND);

        // Test POST vBridge Interface expecting 404, setting vbridge that
        // don't exist
        result = getJsonResult(baseURL + bnameDummy + "/interfaces/" + ifname,
                               "POST", requestBody);
        assertResponse(HTTP_NOT_FOUND);

        // Test POST vBridge Interface expecting 405, setting "" to vbridgeIF
        // name
        result = getJsonResult(baseURL + bnameDummy + "/interfaces/" + "",
                               "POST", requestBody);
        assertResponse(HTTP_BAD_METHOD);

        // Test POST vBridge Interface expecting 400, specifying too long
        // interface name.
        result = getJsonResult(baseURL + bname1 + "/interfaces/" + ifname32,
                               "POST", requestBody);
        assertResponse(HTTP_BAD_REQUEST);

        // Test POST vBridge Interface expecting 400, specifying invalid
        // interface name which starts with "_".
        result = getJsonResult(baseURL + bname1 + "/interfaces/" + "_ifname",
                               "POST", requestBody);
        assertResponse(HTTP_BAD_REQUEST);

        // Test POST vBridge Interface expecting 400, specifying invalid
        // interface name which includes "@".
        result = getJsonResult(baseURL + bname1 + "/interfaces/" + "if@name",
                               "POST", requestBody);
        assertResponse(HTTP_BAD_REQUEST);

        // Test GET vBridge Interface expecting 404, setting vtn that don't exist
        result = getJsonResult(url + "default/vtns/" + tnameDummy +
                               "/vbridges/" + bname1 + "/interfaces/" +
                               ifname1);
        assertResponse(HTTP_NOT_FOUND);

        // Test GET vBridge Interface expecting 404, setting vbridge that don't
        // exits
        result = getJsonResult(baseURL + bnameDummy + "/interfaces/" +
                               ifname1);
        assertResponse(HTTP_NOT_FOUND);

        // Test GET vBridge Interface expecting 404, setting vbridgeIF that
        // don't exits
        result = getJsonResult(baseURL + bnameDummy + "/interfaces/" + ifname);
        assertResponse(HTTP_NOT_FOUND);

        // Test GET vBridge Interface2
        result = getJsonResult(baseURL + bname1 + "/interfaces/" + ifname2);
        jt = new JSONTokener(result);
        json = new JSONObject(jt);

        assertResponse(HTTP_OK);
        Assert.assertEquals(ifname2, json.getString("name"));
        Assert.assertEquals(desc1, json.getString("description"));
        Assert.assertEquals("true", json.getString("enabled"));
        Assert.assertEquals("0", json.getString("state"));
        Assert.assertEquals("-1", json.getString("entityState"));

        // Test PUT vBridge interface1
        requestBody = "{\"description\":\"" + desc1 +
            "\", \"enabled\":\"true\"}";
        String queryParameter = new QueryParameter("all", "true").getString();
        result = getJsonResult(baseURL + bname1 + "/interfaces/" + ifname1 +
                               queryParameter, "PUT", requestBody);
        assertResponse(HTTP_OK);

        // Test GET vBridge Interface1
        result = getJsonResult(baseURL + bname1 + "/interfaces/" + ifname1);
        jt = new JSONTokener(result);
        json = new JSONObject(jt);

        assertResponse(HTTP_OK);
        Assert.assertEquals(desc1, json.getString("description"));
        Assert.assertEquals("true", json.getString("enabled"));

        // Test PUT vBridge interface1, setting description
        // (queryparameter is true)
        requestBody = "{\"description\":\"" + desc2 + "\"}";
        result = getJsonResult(baseURL + bname1 + "/interfaces/" + ifname1 +
                               queryParameter, "PUT", requestBody);
        assertResponse(HTTP_OK);

        // Test GET vBridge Interface1
        result = getJsonResult(baseURL + bname1 + "/interfaces/" + ifname1);
        jt = new JSONTokener(result);
        json = new JSONObject(jt);

        assertResponse(HTTP_OK);
        Assert.assertEquals(desc2, json.getString("description"));
        Assert.assertEquals("true", json.getString("enabled"));

        // Test PUT vBridge interface1, setting enabled (queryparameter is true)
        requestBody = "{\"enabled\":\"false\"}";
        result = getJsonResult(baseURL + bname1 + "/interfaces/" + ifname1 +
                               queryParameter, "PUT", requestBody);
        assertResponse(HTTP_OK);

        // Test GET vBridge Interface1
        result = getJsonResult(baseURL + bname1 + "/interfaces/" + ifname1);
        jt = new JSONTokener(result);
        json = new JSONObject(jt);

        assertResponse(HTTP_OK);
        Assert.assertFalse(json.has("description"));
        Assert.assertEquals("false", json.getString("enabled"));

        // Test PUT vBridge interface1, setting description and enabled
        requestBody = "{\"description\":\"" + desc3 +
            "\", \"enabled\":\"false\"}";
        result = getJsonResult(baseURL + bname1 + "/interfaces/" + ifname1 +
                               queryParameter, "PUT", requestBody);
        assertResponse(HTTP_OK);

        // Test GET vBridge Interface1
        result = getJsonResult(baseURL + bname1 + "/interfaces/" + ifname1);
        jt = new JSONTokener(result);
        json = new JSONObject(jt);

        assertResponse(HTTP_OK);
        Assert.assertEquals(desc3, json.getString("description"));
        Assert.assertEquals("false", json.getString("enabled"));

        // Test PUT vBridge interface1, setting {}
        requestBody = "{}";
        result = getJsonResult(baseURL + bname1 + "/interfaces/" + ifname1 +
                               queryParameter, "PUT", requestBody);
        assertResponse(HTTP_OK);

        // Test PUT vBridge Interface2 expecting not change
        queryParameter = new QueryParameter("all", "false").getString();
        result = getJsonResult(baseURL + bname1 + "/interfaces/" + ifname2 +
                               queryParameter, "PUT", requestBody);
        assertResponse(HTTP_OK);

        // Test GET vBridge Interface2
        result = getJsonResult(baseURL + bname1 + "/interfaces/" + ifname2);
        jt = new JSONTokener(result);
        json = new JSONObject(jt);

        assertResponse(HTTP_OK);
        Assert.assertEquals(desc1, json.getString("description"));
        Assert.assertEquals("true", json.getString("enabled"));

        // Test PUT vBridge Interface2, setting enabled
        requestBody = "{\"enabled\":false}";
        result = getJsonResult(baseURL + bname1 + "/interfaces/" + ifname2 +
                               queryParameter, "PUT", requestBody);
        assertResponse(HTTP_OK);

        // Test GET vBridge Interface2
        result = getJsonResult(baseURL + bname1 + "/interfaces/" + ifname2);
        jt = new JSONTokener(result);
        json = new JSONObject(jt);

        assertResponse(HTTP_OK);
        Assert.assertEquals(desc1, json.getString("description"));
        Assert.assertEquals("false", json.getString("enabled"));

        // Test PUT vBridge Interface2, setting enabled
        requestBody = "{\"description\":\"" + desc3 + "\"}";
        result = getJsonResult(baseURL + bname1 + "/interfaces/" + ifname2 +
                               queryParameter, "PUT", requestBody);
        assertResponse(HTTP_OK);

        // Test GET vBridge Interface2
        result = getJsonResult(baseURL + bname1 + "/interfaces/" + ifname2);
        jt = new JSONTokener(result);
        json = new JSONObject(jt);

        assertResponse(HTTP_OK);
        Assert.assertEquals(desc3, json.getString("description"));
        Assert.assertEquals("false", json.getString("enabled"));

        // Test PUT vBridge Interface2, setting enabled
        requestBody = "{\"description\":\"" + desc1 +
            "\", \"enabled\":\"true\"}";
        result = getJsonResult(baseURL + bname1 + "/interfaces/" + ifname2 +
                               queryParameter, "PUT", requestBody);
        assertResponse(HTTP_OK);

        // Test GET vBridge Interface2
        result = getJsonResult(baseURL + bname1 + "/interfaces/" + ifname2);
        jt = new JSONTokener(result);
        json = new JSONObject(jt);

        assertResponse(HTTP_OK);
        Assert.assertEquals(desc1, json.getString("description"));
        Assert.assertEquals("true", json.getString("enabled"));


        // Test PUT vBridge Interface expecting 404
        // setting dummy vtn
        result = getJsonResult(url + "default/vtns/" + tnameDummy +
                               "/vbridges/" + bname1 + "/interfaces/"
                + ifname1 + queryParameter, "PUT", requestBody);
        assertResponse(HTTP_NOT_FOUND);

        // setting dummy vbridge
        result = getJsonResult(baseURL +  bnameDummy + "/interfaces/"
                + ifname1 + queryParameter, "PUT", requestBody);
        assertResponse(HTTP_NOT_FOUND);

        // setting  dummy vbridgeIF
        result = getJsonResult(baseURL +  bname1 + "/interfaces/" + ifname +
                               queryParameter,
                "PUT", requestBody);
        assertResponse(HTTP_NOT_FOUND);


        // Test GET all vBridge Interfaces in default container
        result = getJsonResult(baseURL + bname1 + "/interfaces");
        jt = new JSONTokener(result);
        json = new JSONObject(jt);
        vbridgeifArray = json.getJSONArray("interface");

        assertResponse(HTTP_OK);
        Assert.assertEquals(4, vbridgeifArray.length());


        // Test DELETE expecting 200
        // delete vBridge Interface2 on other tenant
        result = getJsonResult(baseURL2 + bname2 + "/interfaces/" + ifname2,
                               "DELETE");
        assertResponse(HTTP_OK);

        // specify not supported Content-Type
        requestBody = "{}";
        result = getJsonResult(baseURL + bname2 + "/interfaces/" + ifname2,
                               "POST", requestBody, "text/plain");
        assertResponse(HTTP_UNSUPPORTED_TYPE);

        requestBody = "{}";
        result = getJsonResult(baseURL + bname2 + "/interfaces/" + ifname3 +
                               queryParameter, "PUT", requestBody,
                               "text/plain");
        assertResponse(HTTP_UNSUPPORTED_TYPE);

        // Authentication failure.
        checkVBridgeInterfaceError(GlobalConstants.DEFAULT.toString(), tname1,
                                   bname1, bname2, ifname3, ifname2, false,
                                   HttpURLConnection.HTTP_UNAUTHORIZED);

        // Invalid container name.
        String[] invalid = {"bad_container", "version"};
        for (String inv: invalid) {
            checkVBridgeInterfaceError(inv, tname1, bname1, bname2, ifname3,
                                       ifname2, true,
                                       HttpURLConnection.HTTP_NOT_FOUND);
            checkVBridgeInterfaceError(inv, tname1, bname1, bname2, ifname3,
                                       ifname2, false,
                                       HttpURLConnection.HTTP_UNAUTHORIZED);
        }

        // delete  vBridge Interface3
        result = getJsonResult(baseURL + bname2 + "/interfaces/" + ifname3,
                               "DELETE");
        assertResponse(HTTP_OK);

        // delete vBridge Interface4
        result = getJsonResult(baseURL + bname1 + "/interfaces/" + ifname4,
                               "DELETE");
        assertResponse(HTTP_OK);

        // delete vBridge Interface5
        result = getJsonResult(baseURL + bname1 + "/interfaces/" + ifname5,
                               "DELETE");
        assertResponse(HTTP_OK);
    }

    /**
     * Ensure that the VBridge interface APIs return correct error response.
     *
     * @param containerName   The container name.
     * @param tname           The tenant name for the test.
     * @param bname1          The bridge name for the test.
     * @param bname2          The bridge name for the test.
     * @param ifname1         The interface name for the test.
     * @param ifname2         The interface name for the test.
     * @param auth            If {@code true}, the client sends authenticated
     *                        request. Otherwise the client sends unauthorized
     *                        request.
     * @param expected        Expected HTTP response code.
     */
    private void checkVBridgeInterfaceError(String containerName,
                                            String tname, String bname1,
                                            String bname2, String ifname1,
                                            String ifname2, boolean auth,
                                            int expected) {
        String base = VTN_BASE_URL + containerName + "/vtns/" + tname +
            "/vbridges/";
        String ct = "Application/Json";
        getJsonResult(base + bname1 + "/interfaces", "GET", null, ct, auth);
        assertResponse(expected);

        getJsonResult(base + bname2 + "/interfaces/" + ifname1 , "GET", null,
                      ct, auth);
        assertResponse(expected);

        String body = "{}";
        getJsonResult(base + bname2 + "/interfaces/" + ifname2, "POST", body,
                      ct, auth);
        assertResponse(expected);

        String qp = new QueryParameter("all", "true").toString();
        getJsonResult(base + bname2 + "/interfaces/" + ifname1 + qp,
                      "PUT", body, ct, auth);
        assertResponse(expected);

        getJsonResult(base + bname2 + "/interfaces/" + ifname1, "DELETE", null,
                      ct, auth);
        assertResponse(expected);
    }

    /**
     * Test case for VBridgeInterface delete APIs.
     *
     * This method is called by {@code testVBridge}.
     *
     * @param tname     A tenant name.
     *                  Specified tenant is necessary to be configured
     *                  before method is called.
     * @param bname     A vBridge name.
     *                  Specified vBridge is necessary to be configured
     *                  before method is called.
     * @throws JSONException  An error occurred.
     */
    private void testVBridgeInterfaceDeleteAPI(String tname, String bname)
        throws JSONException {
        LOG.info("Starting delete vBridge Intergace JAX-RS client.");
        String url = VTN_BASE_URL;
        StringBuilder baseURL = new StringBuilder();
        baseURL.append(url);
        baseURL.append("default/vtns/");
        baseURL.append(tname);
        baseURL.append("/vbridges/");

        String ifnameDummy = "fname_dummy";
        String tnameDummy = "tnameDummy";
        String ifname1 = "testInterface";
        String ifname2 = "test_Interface2";
        String bnameDummy = "bname_dummy";

        testPortMappingDeleteAPI(tname, bname, ifname1);

        // Test DELETE vbridge interface expecting 404
        // setting dummy vtn
        String result = getJsonResult(url + "default/vtns/" + tnameDummy + "/vbridges/" + bname + "/interfaces/" + ifname1, "DELETE");
        assertResponse(HTTP_NOT_FOUND);

        // setting dummy vbridge
        result = getJsonResult(baseURL + bnameDummy + "/interfaces/" + ifname1, "DELETE");
        assertResponse(HTTP_NOT_FOUND);

        // setting dummy vbridge interface
        result = getJsonResult(baseURL + bname + "/interfaces/" + ifnameDummy, "DELETE");
        assertResponse(HTTP_NOT_FOUND);


        // Test DELETE vbridge interface expecting 404
        // setting vBridge Interface1
        result = getJsonResult(baseURL + bname + "/interfaces/" + ifname1, "DELETE");
        assertResponse(HTTP_OK);

        // setting vBridge Interface2
        result = getJsonResult(baseURL + bname + "/interfaces/" + ifname2, "DELETE");
        assertResponse(HTTP_OK);


        // Test DELETE vbridge interface expecting 404
        // setting deleted vbridge interface1
        result = getJsonResult(baseURL + bname + "/interfaces/" + ifname1, "DELETE");
        assertResponse(HTTP_NOT_FOUND);


        // Test GET all vBridge Interfaces in default container
        result = getJsonResult(baseURL + bname + "/interfaces");
        JSONTokener jt = new JSONTokener(result);
        JSONObject json = new JSONObject(jt);
        JSONArray vbridgeifArray = json.getJSONArray("interface");

        assertResponse(HTTP_OK);
        Assert.assertEquals(0, vbridgeifArray.length());
    }

    /**
     * Test case for Port Mapping APIs.
     *
     * This method is called by {@code testVBridgeInterfaceAPI.}
     *
     * @param tname     A tenant name.
     *                  Specified tenant is necessary to be configured
     *                  before method is called.
     * @param bname     A vBridge name.
     *                  Specified vBridge is necessary to be configured
     *                  before method is called.
     * @param ifname    A vBridgeInterface name.
     *                  Specified vBridgeInterface is necessary to be configured
     *                  before method is called.
     * @param ifname2   A vBridgeInterface name.
     *                  This interface is also necessary to be configured
     *                  before method is called.
     * @throws JSONException  An error occurred.
     */
    private void testPortMappingAPI(String tname, String bname, String bname2,
                                    String ifname, String ifname2)
        throws JSONException {
        LOG.info("Starting Port Mapping JAX-RS client.");
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
        String vlanOver = "4096";
        String vlanNegative = "-10";

        String pname = "testPortname";

        String ifnameDummy = "ifname_dummy";
        String tenantDummy = "tenant_dummy";
        String bnameDummy = "bname_dummy";

        String nodeid = "00:00:00:00:00:00:00:01";
        String nodeType = "OF";
        String portnum = "1";

        String test = "ERROR_TEST";

        // Tset GET PortMapping expecting 204
        // Test GET PortMapping in default container, expecting no results
        String result = getJsonResult(baseURL + ifname + "/portmap");
        assertResponse(HTTP_NO_CONTENT);

        // Test PUT PortMapping expecting 400
        // setting invalid value to requestBody
        String requestBody = "{\"description\":\", \"enabled\":\"true\"}";
        result = getJsonResult(baseURL + ifname + "/portmap/", "PUT", requestBody);
        assertResponse(HTTP_BAD_REQUEST);


        // Test PUT PortMapping expecting 400, specifying too large VLAN ID.
        requestBody = "{\"vlan\":" + vlanOver + ", \"node\":{\"type\":\"" +
            nodeType + "\", \"id\":\"" + nodeid +
            "\"}, \"port\":{\"name\":\"" + pname + "\", \"type\":\"" +
            nodeType + "\", \"id\":\"" + portnum + "\"}}";
        result = getJsonResult(baseURL + ifname + "/portmap/", "PUT",
                               requestBody);
        assertResponse(HTTP_BAD_REQUEST);

        // Specifying a negative VLAN ID.
        requestBody = "{\"vlan\":" + vlanNegative +
            ", \"node\":{\"type\":\"" + nodeType + "\", \"id\":\"" + nodeid +
            "\"}, \"port\":{\"name\":\"" + pname + "\", \"type\":\"" +
            nodeType + "\", \"id\":\"" + portnum + "\"}}";
        result = getJsonResult(baseURL + ifname + "/portmap/", "PUT",
                               requestBody);
        assertResponse(HTTP_BAD_REQUEST);

        // Specifying invalid node type.
        requestBody = "{\"vlan\":" + vlan1 + ", \"node\":{\"type\":\"" +
            test + "\", \"id\":\"" + nodeid + "\"}, \"port\":{\"name\":\"" +
            pname + "\", \"type\":\"" + nodeType + "\", \"id\":\"" + portnum +
            "\"}}";
        result = getJsonResult(baseURL + ifname + "/portmap/", "PUT",
                               requestBody);
        assertResponse(HTTP_BAD_REQUEST);

        // Without specifying node element.
        requestBody = "{\"vlan\":" + vlan1 + ", \"port\":{\"name\":\"" +
            pname + "\", \"type\":\"" + nodeType + "\", \"id\":\"" + portnum +
            "\"}}";
        result = getJsonResult(baseURL + ifname + "/portmap/", "PUT",
                               requestBody);
        assertResponse(HTTP_BAD_REQUEST);

        // Without specifying port element.
        requestBody = "{\"vlan\":" + vlan1 + ", \"node\":{\"type\":\"" +
            nodeType + "\", \"id\":\"" + nodeid + "\"}}";
        result = getJsonResult(baseURL + ifname + "/portmap/", "PUT",
                               requestBody);
        assertResponse(HTTP_BAD_REQUEST);

        // Specifying incomplete port element.
        //   - "id" is specified without "type".
        requestBody = "{\"vlan\":" + vlan1 + ", \"node\":{\"type\":\"" +
            nodeType + "\", \"id\":\"" + nodeid + "\"}, \"port\":{\"id\":\"" +
            portnum + "\"}}";
        result = getJsonResult(baseURL + ifname + "/portmap/", "PUT",
                               requestBody);
        assertResponse(HTTP_BAD_REQUEST);

        // Specifying incomplete port element.
        //   - "name" and "id" are specified without "type",
        requestBody = "{\"vlan\":" + vlan1 + ", \"node\":{\"type\":\"" +
            nodeType + "\", \"id\":\"" + nodeid +
            "\"}, \"port\":{\"type\":\"" + nodeType + "\"}}";
        result = getJsonResult(baseURL + ifname + "/portmap/", "PUT",
                               requestBody);
        assertResponse(HTTP_BAD_REQUEST);

        // Specifying invalid port element.
        //   - Invalid port type with specifying "name".
        requestBody = "{\"vlan\":" + vlan1 + ", \"node\":{\"type\":\"" +
            nodeType + "\", \"id\":\"" + nodeid +
            "\"}, \"port\":{\"name\":\"" + "" + "\", \"type\":\"" +
            nodeType + "\", \"id\":\"" + portnum + "\"}}";
        result = getJsonResult(baseURL + ifname + "/portmap/", "PUT",
                               requestBody);
        assertResponse(HTTP_BAD_REQUEST);

        // Specifying invalid node which does not contain node type.
        requestBody = "{\"vlan\":" + vlan0 + ", \"node\":{\"id\":\"" + nodeid +
            "\"}, \"port\":{\"name\":\"" + pname + "\", \"type\":\"" +
            nodeType + "\", \"id\":\"" + portnum + "\"}}";
        result = getJsonResult(url + "default/vtns/" + tname + "/vbridges/" +
                               bname2 + "/interfaces/" + ifname2 +
                               "/portmap/", "PUT", requestBody);
        assertResponse(HTTP_BAD_REQUEST);

        // Specifying invalid node which does not contain node ID.
        requestBody = "{\"vlan\":" + vlan1 + ", \"node\":{\"type\":\"" +
            nodeType + "\"}, \"port\":{\"name\":\"" + pname +
            "\", \"type\":\"" + nodeType + "\", \"id\":\"" + portnum +
            "\"}}";
        result = getJsonResult(url + "default/vtns/" + tname + "/vbridges/" +
                               bname2 + "/interfaces/" + ifname2 + "/portmap/",
                               "PUT", requestBody);
        assertResponse(HTTP_BAD_REQUEST);

        // Specifying invalid port which does not contain port type.
        requestBody = "{\"vlan\":" + vlan1 + ", \"node\":{\"type\":\"" +
            nodeType + "\", \"id\":\"" + nodeid +
            "\"}, \"port\":{\"name\":\"" + pname + "\", \"id\":\"" +
            portnum + "\"}}";
        result = getJsonResult(url + "default/vtns/" + tname + "/vbridges/" +
                               bname2 + "/interfaces/" + ifname2 + "/portmap/",
                               "PUT", requestBody);
        assertResponse(HTTP_BAD_REQUEST);

        // Specifying invalid port which does not contain port ID.
        requestBody = "{\"vlan\":" + vlan1 + ", \"node\":{\"type\":\"" +
            nodeType + "\", \"id\":\"" + nodeid +
            "\"}, \"port\":{\"name\":\"" + pname + "\", \"type\":\"" +
            nodeType + "\"}}";
        result = getJsonResult(url + "default/vtns/" + tname + "/vbridges/" +
                               bname2 + "/interfaces/" + ifname2 + "/portmap/",
                               "PUT", requestBody);
        assertResponse(HTTP_BAD_REQUEST);

        // Test PUT PortMapping 400
        // setting invalid value to node id
        requestBody = "{\"vlan\":" + vlan1 + ", \"node\":{\"type\":\"" +
            nodeType + "\", \"id\":\"" + test + "\"}, \"port\":{\"name\":\"" +
            pname + "\", \"type\":\"" + nodeType + "\", \"id\":\"" + portnum +
            "\"}}";
        result = getJsonResult(baseURL + ifname + "/portmap/", "PUT",
                               requestBody);
        assertResponse(HTTP_BAD_REQUEST);

        // Test PUT PortMappint expecting 404
        // setting dummy vtn
        requestBody = "{\"vlan\":" + vlan1 + ", \"node\":{\"type\":\"" +
            nodeType + "\", \"id\":\"" + nodeid +
            "\"}, \"port\":{\"name\":\"" + pname + "\", \"type\":\"" +
            nodeType + "\", \"id\":\"" + portnum + "\"}}";
        result = getJsonResult(url + "default/vtns/" + tenantDummy +
                               "/vbridges/" + bname + "/interfaces/" + ifname +
                               "/portmap/", "PUT", requestBody);
        assertResponse(HTTP_NOT_FOUND);

        // setting dummy vBridge
        requestBody = "{\"vlan\":" + vlan1 + ", \"node\":{\"type\":\"" +
            nodeType + "\", \"id\":\"" + nodeid +
            "\"}, \"port\":{\"name\":\"" + pname + "\", \"type\":\"" +
            nodeType + "\", \"id\":\"" + portnum + "\"}}";
        result = getJsonResult(url + "default/vtns/" + tenantDummy +
                               "/vbridges/" + bnameDummy + "/interfaces/" +
                               ifname + "/portmap/", "PUT", requestBody);
        assertResponse(HTTP_NOT_FOUND);

        // setting dummy vBridge interface
        requestBody = "{\"vlan\":" + vlan1 + ", \"node\":{\"type\":\"" +
            nodeType + "\", \"id\":\"" + nodeid +
            "\"}, \"port\":{\"name\":\"" + pname + "\", \"type\":\"" +
            nodeType + "\", \"id\":\"" + portnum + "\"}}";
        result = getJsonResult(baseURL + ifnameDummy + "/portmap/", "PUT",
                               requestBody);
        assertResponse(HTTP_NOT_FOUND);

        // specfiy not supported Content-Type
        result = getJsonResult(baseURL + ifname + "/portmap/", "PUT",
                               requestBody, "text/plain");
        assertResponse(HTTP_UNSUPPORTED_TYPE);

        // Authentication failure.
        checkPortMapError(GlobalConstants.DEFAULT.toString(), tname, bname,
                          ifname, false,
                          HttpURLConnection.HTTP_UNAUTHORIZED);

        // Invalid container name.
        String[] invalid = {"bad_container", "version"};
        for (String inv: invalid) {
            checkPortMapError(inv, tname, bname, ifname, true,
                              HttpURLConnection.HTTP_NOT_FOUND);
            checkPortMapError(inv, tname, bname, ifname, false,
                              HttpURLConnection.HTTP_UNAUTHORIZED);
        }

        // Test PUT PortMapping expecting 200
        // Test PUT PortMapping
        result = getJsonResult(baseURL + ifname + "/portmap/", "PUT",
                               requestBody);
        assertResponse(HTTP_OK);

        // setting port element without port id and  port type
        requestBody = "{\"vlan\":" + vlan1 + ", \"node\":{\"type\":\"" +
            nodeType + "\", \"id\":\"" + nodeid +
            "\"}, \"port\":{\"name\":\"" + pname + "\"}}";
        result = getJsonResult(baseURL + ifname + "/portmap/", "PUT",
                               requestBody);
        assertResponse(HTTP_OK);

        // Test PUT PortMapping, change vlan value
        requestBody = "{\"vlan\":" + vlan2 + ", \"node\":{\"type\":\"" +
            nodeType + "\", \"id\":\"" + nodeid +
            "\"}, \"port\":{\"name\":\"" + pname + "\", \"type\":\"" +
            nodeType + "\", \"id\":\"" + portnum + "\"}}";
        result = getJsonResult(baseURL + ifname + "/portmap/", "PUT",
                               requestBody);
        assertResponse(HTTP_OK);

        // Test PUT PortMapping expecting 409
        // Test PUT PortMapping for other vbridge
        requestBody = "{\"vlan\":" + vlan2 + ", \"node\":{\"type\":\"" +
            nodeType + "\", \"id\":\"" + nodeid +
            "\"}, \"port\":{\"name\":\"" + pname + "\", \"type\":\"" +
            nodeType + "\", \"id\":\"" + portnum + "\"}}";
        result = getJsonResult(url + "default/vtns/" + tname + "/vbridges/" +
                               bname2 + "/interfaces/" + ifname2 + "/portmap/",
                               "PUT", requestBody);

        // when physical port doesn't exist, conflict mapping succeeds.
        assertResponse(HTTP_OK);

        // Test PUT PortMapping expecting 200
        // Test PUT PortMapping for other vbridge and except port name
        requestBody = "{\"vlan\":" + vlan1 + ", \"node\":{\"type\":\"" +
            nodeType + "\", \"id\":\"" + nodeid +
            "\"}, \"port\":{\"type\":\"" + nodeType + "\", \"id\":\"" +
            portnum + "\"}}";
        result = getJsonResult(url + "default/vtns/" + tname + "/vbridges/" +
                               bname2 + "/interfaces/" + ifname2 + "/portmap/",
                               "PUT", requestBody);
        assertResponse(HTTP_OK);

        // Test PUT PortMapping, except vlan
        requestBody = "{\"node\":{\"type\":\"" + nodeType + "\", \"id\":\"" +
            nodeid + "\"}, \"port\":{\"name\":\"" + pname +
            "\", \"type\":\"" + nodeType + "\", \"id\":\"" + portnum + "\"}}";
        result = getJsonResult(url + "default/vtns/" + tname + "/vbridges/" +
                               bname2 + "/interfaces/" + ifname2 + "/portmap/",
                               "PUT", requestBody);
        assertResponse(HTTP_OK);

        // Test GET PortMapping from bname2
        result = getJsonResult(url + "default/vtns/" + tname + "/vbridges/" +
                               bname2 + "/interfaces/" + ifname2 + "/portmap");
        assertResponse(HTTP_OK);
        JSONTokener jt = new JSONTokener(result);
        JSONObject json = new JSONObject(jt);

        Assert.assertEquals(vlan0, json.getString("vlan"));


        // Test GET PortMapping expecting 404
        // setting dummy vtn
        result = getJsonResult(url + "default/vtns/" + tenantDummy +
                               "/vbridges/" + bname + "/interfaces/" + ifname +
                               "/portmap");
        assertResponse(HTTP_NOT_FOUND);

        // setting dummy vbridge
        result = getJsonResult(url + "default/vtns/" + tenantDummy +
                               "/vbridges/" + bnameDummy + "/interfaces/" +
                               ifname + "/portmap");
        assertResponse(HTTP_NOT_FOUND);

        // setting dummy vbridge interface
        result = getJsonResult(baseURL + ifnameDummy + "/portmap");
        assertResponse(HTTP_NOT_FOUND);

        // Test GET PortMapping expecting 200
        // Test GET PortMapping
        result = getJsonResult(baseURL + ifname + "/portmap");
        assertResponse(HTTP_OK);
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
     * Ensure that the port mapping APIs return correct error response.
     *
     * @param containerName   The container name.
     * @param tname           The tenant name for the test.
     * @param bname           The bridge name for the test.
     * @param ifname          The interface name for the test.
     * @param auth            If {@code true}, the client sends authenticated
     *                        request. Otherwise the client sends unauthorized
     *                        request.
     * @param expected        Expected HTTP response code.
     */
    private void checkPortMapError(String containerName, String tname,
                                   String bname, String ifname, boolean auth,
                                   int expected) {
        String uri = VTN_BASE_URL + containerName + "/vtns/" + tname +
            "/vbridges/" + bname + "/interfaces/" + ifname + "/portmap";
        String ct = "Application/Json";
        String body = "{\"vlan\": 0, " +
            "\"node\":{\"type\": \"OF\", \"id\": 1}, " +
            "\"port\":{\"name\": \"port-2\"}}";
        getJsonResult(uri, "PUT", body, ct, auth);
        assertResponse(expected);

        getJsonResult(uri, "GET", null, ct, auth);
        assertResponse(expected);

        getJsonResult(uri, "DELETE", null, ct, auth);
        assertResponse(expected);
    }

    /**
     * Test case for Port Mapping delete APIs.
     *
     * This method is called by {@code testVBridgeInterfaceDeleteAPI}.
     *
     * @param tname     A tenant name.
     *                  Specified tenant is necessary to be configured
     *                  before method is called.
     * @param bname     A vBridge name.
     *                  Specified vBridge is necessary to be configured
     *                  before method is called.
     * @param ifname    A vBridgeInterface name.
     *                  Specified vBridgeInterface is necessary to be
     *                  configured before method is called.
     * @throws JSONException  An error occurred.
     */
    private void testPortMappingDeleteAPI(String tname, String bname,
                                          String ifname) throws JSONException {
        LOG.info("Starting delete Port Mapping JAX-RS client.");
        String url = VTN_BASE_URL;
        StringBuilder baseURL = new StringBuilder();
        baseURL.append(url);
        baseURL.append("default/vtns/");
        baseURL.append(tname);
        baseURL.append("/vbridges/");
        baseURL.append(bname);
        baseURL.append("/interfaces/");

        String tnameDummy = "tnameDummy";
        String bnameDummy = "bname_dummy";
        String ifnameDummy = "ifname_dummy";

        // Test DELETE PortMapping expecting 404
        // setting dummy vtn interface
        String result = getJsonResult(url + "default/vtns/" + tnameDummy + "/vbridges/"
                + bname +  "/interfaces/" + ifnameDummy + "/portmap", "DELETE");
        assertResponse(HTTP_NOT_FOUND);

        // setting dummy vbridge
        result = getJsonResult(url + "default/vtns/" + tname + "/vbridges/"
                + bnameDummy +  "/interfaces/" + ifnameDummy + "/portmap", "DELETE");
        assertResponse(HTTP_NOT_FOUND);

        // setting dummy vbridge interface
        result = getJsonResult(baseURL + ifnameDummy + "/portmap", "DELETE");
        assertResponse(HTTP_NOT_FOUND);

        // Test DELETE PortMapping
        result = getJsonResult(baseURL + ifname + "/portmap", "DELETE");
        assertResponse(HTTP_OK);

        // Test DELETE PortMapping, setting deleted portMapping
        result = getJsonResult(baseURL + ifname + "/portmap", "DELETE");
        assertResponse(HTTP_OK);
    }

    /**
     * Test case for VLAN Mapping APIs.
     *
     * <p>
     *   This method is called by {@link #testVBridgeAPI(String, String)}.
     * </p>
     *
     * @param tname     A tenant name.
     *                  Specified tenant is necessary to be configured
     *                  before method is called.
     * @param bname     A vBridge name.
     *                  Specified vBridge is necessary to be configured
     *                  before method is called.
     * @throws JSONException  An error occurred.
     */
    private void testVLANMappingAPI(String tname, String bname)
        throws JSONException {
        LOG.info("Starting VLAN Mapping JAX-RS client.");
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

        String vlanNegative = "-100";
        String vlanOver = "4096";

        String nodeType = "OF";

        String tnameDummy = "tnameDummy";
        String bnameDummy = "bname_dummy";

        String bname2 = "vbridge_Name2";

        // Test GET VLAN Mapping
        String result = getJsonResult(baseURL + bname + "/vlanmaps");
        assertResponse(HTTP_OK);
        JSONTokener jt = new JSONTokener(result);
        JSONObject json = new JSONObject(jt);
        JSONArray vLANMapArray = json.getJSONArray("vlanmap");
        Assert.assertEquals(0, vLANMapArray.length());

        // Test GET VLAN Mapping expecting 404, setting dummy vtn
        String searchByConf = "/vlanmapsearch/byconf";
        String badTenant = url + "default/vtns/" + tnameDummy +
            "/vbridges/" + bname;
        result = getJsonResult(badTenant + "/vlanmaps");
        assertResponse(HTTP_NOT_FOUND);
        result = getJsonResult(badTenant + searchByConf);
        assertResponse(HTTP_NOT_FOUND);
        result = getJsonResult(badTenant + searchByConf + "?vlan=1");
        assertResponse(HTTP_NOT_FOUND);

        // Test GET VLAN Mapping expecting 404, setting dummy vbridge
        result = getJsonResult(baseURL + bnameDummy + "/vlanmaps");
        assertResponse(HTTP_NOT_FOUND);
        result = getJsonResult(baseURL + bnameDummy + searchByConf);
        assertResponse(HTTP_NOT_FOUND);
        result = getJsonResult(baseURL + bnameDummy + searchByConf +
                               "?vlan=1");
        assertResponse(HTTP_NOT_FOUND);

        // Specifying malformed VLAN ID and node.
        String searchUrl = baseURL + bname + searchByConf;
        result = getJsonResult(searchUrl + "?vlan=0x123");
        assertResponse(HTTP_BAD_REQUEST);
        result = getJsonResult(searchUrl + "?node=InvalidNode");
        assertResponse(HTTP_BAD_REQUEST);

        // Specifying VLAN ID and node which don't exist.
        result = getJsonResult(searchUrl + "?vlan=0");
        assertResponse(HTTP_NOT_FOUND);
        result = getJsonResult(searchUrl + "?node=OF|12345");
        assertResponse(HTTP_NOT_FOUND);
        result = getJsonResult(searchUrl + "?vlan=0&node=OF|1000");
        assertResponse(HTTP_NOT_FOUND);

        // Test POST VLAN Mapping expecting 404, setting dummy vbridge
        String requestBody = "{\"vlan\":\"" + vlan1 +
            "\",\"node\":{\"type\":\"" + nodeType + "\",\"id\":\"" + nodeid1 +
            "\"}}";
        result = getJsonResult(url + "default/vtns/" + tnameDummy +
                               "/vbridges/" + bname + "/vlanmaps", "POST",
                               requestBody);
        assertResponse(HTTP_NOT_FOUND);

        // Test POST VLAN Mapping expecting 404, setting dummy vbridge
        result = getJsonResult(baseURL + bnameDummy + "/vlanmaps", "POST",
                               requestBody);
        assertResponse(HTTP_NOT_FOUND);

        // Test POST VLAN Mapping expecting 400
        // Specifying a negative VLAN ID.
        requestBody = "{\"vlan\":\"" + vlanNegative +
            "\",\"node\":{\"type\":\"" + nodeType + "\",\"id\":\"" + nodeid1 +
            "\"}}";
        result = getJsonResult(baseURL + bname + "/vlanmaps", "POST",
                               requestBody);
        assertResponse(HTTP_BAD_REQUEST);

        // Specifyin too large VLAN ID.
        requestBody = "{\"vlan\":\"" + vlanOver + "\",\"node\":{\"type\":\"" +
            nodeType + "\",\"id\":\"" + nodeid1 + "\"}}";
        result = getJsonResult(baseURL + bname + "/vlanmaps", "POST",
                               requestBody);
        assertResponse(HTTP_BAD_REQUEST);

        // Specifyin invalid node type.
        requestBody = "{\"vlan\":\"" + vlan + "\",\"node\":{\"type\":\"" +
            "ERROR_TEST" + "\",\"id\":\"" + nodeid1 + "\"}}";
        result = getJsonResult(baseURL + bname + "/vlanmaps", "POST",
                               requestBody);
        assertResponse(HTTP_BAD_REQUEST);

        // Test POST VLAN Mapping
        requestBody = "{\"vlan\":\"" + vlan1 + "\",\"node\":{\"type\":\"" +
            nodeType + "\",\"id\":\"" + nodeid1 + "\"}}";
        String requestUri = baseURL + bname + "/vlanmaps";
        result = getJsonResult(requestUri, "POST", requestBody);
        assertResponse(HTTP_CREATED);
        String loc = requestUri + "/" + nodeType + "-" + nodeid1 + "." + vlan1;
        Assert.assertEquals(loc, httpLocation);

        // Test POST VLAN Mapping expecting 409
        requestBody = "{\"vlan\":\"" + vlan1 + "\",\"node\":{\"type\":\"" +
            nodeType + "\",\"id\":\"" + nodeid1 + "\"}}";
        result = getJsonResult(baseURL + bname + "/vlanmaps", "POST",
                               requestBody);
        assertResponse(HTTP_CONFLICT);

        // Test GET VLAN Mapping
        result = getJsonResult(baseURL + bname + "/vlanmaps");
        assertResponse(HTTP_OK);
        jt = new JSONTokener(result);
        json = new JSONObject(jt);
        vLANMapArray = json.getJSONArray("vlanmap");
        Assert.assertEquals(1, vLANMapArray.length());

        // Test POST VLAN Mapping
        requestBody = "{\"vlan\":\"" + vlan2 + "\",\"node\":{\"type\":\"" +
            nodeType + "\",\"id\":\"" + nodeid2 + "\"}}";
        result = getJsonResult(baseURL + bname + "/vlanmaps", "POST",
                               requestBody);
        assertResponse(HTTP_CREATED);
        loc = requestUri + "/" + nodeType + "-" + nodeid2 + "." + vlan2;
        Assert.assertEquals(loc, httpLocation);

        // Test GET all  VLAN Mapping in default container
        result = getJsonResult(baseURL + bname + "/vlanmaps");
        jt = new JSONTokener(result);
        json = new JSONObject(jt);
        vLANMapArray = json.getJSONArray("vlanmap");
        JSONObject vLANMap;

        assertResponse(HTTP_OK);
        Assert.assertEquals(2, vLANMapArray.length());

        for (int i = 0; i < vLANMapArray.length(); i++) {
            vLANMap = vLANMapArray.getJSONObject(i);
            JSONObject nodeinfo = vLANMap.getJSONObject("node");
            if (vLANMap.getString("id").equals(nodeType + "-" + nodeid1 + "." +
                                               vlan1)) {
                Assert.assertEquals(vlan1, vLANMap.getString("vlan"));
                Assert.assertEquals(nodeType, nodeinfo.getString("type"));
                Assert.assertEquals(nodeid1, nodeinfo.getString("id"));

            } else if (vLANMap.getString("id").equals(nodeType + "-" +
                                                      nodeid2 + "." + vlan2)) {
                Assert.assertEquals(vlan2, vLANMap.getString("vlan"));
                Assert.assertEquals(nodeType, nodeinfo.getString("type"));
                Assert.assertEquals(nodeid2, nodeinfo.getString("id"));
            } else {
                // Unexpected VLAN Mapping
                Assert.assertTrue(false);
            }
        }

        // Test POST VLAN Mapping, setting 0 to vlan
        requestBody = "{\"vlan\":\"" + vlan0 + "\",\"node\":{\"type\":\"" +
            nodeType + "\",\"id\":\"" + nodeid1 + "\"}}";
        requestUri = baseURL + bname2 + "/vlanmaps";
        result = getJsonResult(requestUri, "POST", requestBody);
        assertResponse(HTTP_CREATED);
        loc = requestUri + "/" + nodeType + "-" + nodeid1 + "." + vlan0;
        Assert.assertEquals(loc, httpLocation);

        // Test POST VLAN Mapping, except vlan
        requestBody = "{\"node\":{\"type\":\"" + nodeType + "\",\"id\":\"" +
            nodeid1 + "\"}}";
        requestUri = baseURL + bname + "/vlanmaps";
        result = getJsonResult(requestUri, "POST", requestBody);
        assertResponse(HTTP_CONFLICT);

        // Authentication failure.
        checkVlanMapError(GlobalConstants.DEFAULT.toString(), tname, bname,
                          bname2, false, HttpURLConnection.HTTP_UNAUTHORIZED);

        // Invalid container name.
        String[] invalid = {"bad_container", "version"};
        for (String inv: invalid) {
            checkVlanMapError(inv, tname, bname, bname2, true,
                              HttpURLConnection.HTTP_NOT_FOUND);
            checkVlanMapError(inv, tname, bname, bname2, false,
                              HttpURLConnection.HTTP_UNAUTHORIZED);
        }

        // Test DELETE VLAN Mapping
        result = getJsonResult(baseURL + bname2 + "/vlanmaps/" + nodeType +
                               "-" + nodeid1 + "." + vlan0, "DELETE");
        assertResponse(HTTP_OK);

        // Test not supported Content-Type
        requestBody = "{\"vlan\":\"" + vlan0 + "\",\"node\":{\"type\":\"" +
            nodeType + "\",\"id\":\"" + nodeid1 + "\"}}";
        requestUri = baseURL + bname + "/vlanmaps";
        result = getJsonResult(requestUri, "POST", requestBody, "text/plain");
        assertResponse(HTTP_UNSUPPORTED_TYPE);

        // Test POST VLAN Mapping, except vlan
        requestBody = "{\"node\":{\"type\":\"" + nodeType + "\",\"id\":\"" +
            nodeid1 + "\"}}";
        requestUri = baseURL + bname + "/vlanmaps";
        result = getJsonResult(requestUri, "POST", requestBody);
        assertResponse(HTTP_CREATED);
        loc = requestUri + "/" + nodeType + "-" + nodeid1 + "." + vlan0;
        Assert.assertEquals(loc, httpLocation);

        // Test POST VLAN Mapping, specifying invalid node which does not
        // contain node ID.
        requestBody = "{\"vlan\":\"" + vlan + "\",\"node\":{\"type\":\"" +
            nodeType + "\"}}";
        requestUri = baseURL + bname + "/vlanmaps";
        result = getJsonResult(requestUri, "POST", requestBody);
        assertResponse(HTTP_BAD_REQUEST);

        // Test POST VLAN Mapping, specifying invalid node which does not
        // contain node type.
        requestBody = "{\"vlan\":\"" + vlan + "\",\"node\":{\"id\":\"" +
            nodeid1 + "\"}}";
        requestUri = baseURL + bname2 + "/vlanmaps";
        result = getJsonResult(requestUri, "POST", requestBody);
        assertResponse(HTTP_BAD_REQUEST);

        // Test POST VLAN Mapping, setting requestBody without node elements
        requestBody = "{\"vlan\":\"" + vlan3 + "\"}";
        requestUri = baseURL + bname2 + "/vlanmaps";

        // Ensure that query parameters are eliminated from Location.
        result = getJsonResult(requestUri + "?param1=1&param2=2", "POST",
                               requestBody);
        assertResponse(HTTP_CREATED);
        loc = requestUri + "/" + "ANY" + "." + vlan3;
        Assert.assertEquals(loc, httpLocation);

        // Test POST VLAN Mapping expecting 400
        // setting invalid value to node id
        requestBody = "{\"vlan\":\"" + vlan + "\",\"node\":{\"type\":\"" +
            nodeType + "\",\"id\":\"" + "ERROR_TEST" + "\"}}";
        requestUri = baseURL + bname2 + "/vlanmaps";
        result = getJsonResult(requestUri, "POST", requestBody);
        assertResponse(HTTP_BAD_REQUEST);

        // Test GET VLAN Mapping expecting 404
        // setting dummy vtn
        result = getJsonResult(url + "default/vtns/" + tnameDummy +
                               "/vbridges/" + bname + "/vlanmaps"
                + nodeType + "-" + nodeid1 + "." + vlan1);
        assertResponse(HTTP_NOT_FOUND);

        // setting dummy vbridge
        result = getJsonResult(baseURL + bnameDummy + "/vlanmaps/" +
                               nodeType + "-" + nodeid1 + "." + vlan1);
        assertResponse(HTTP_NOT_FOUND);

        // setting dummy vlan
        result = getJsonResult(baseURL + bname + "/vlanmaps/" + nodeType +
                               "-" + nodeid1 + "." + vlan);
        assertResponse(HTTP_NOT_FOUND);

        // Test GET VLAN Mapping
        result = getJsonResult(baseURL + bname + "/vlanmaps/" + nodeType +
                               "-" + nodeid1 + "." + vlan1);
        jt = new JSONTokener(result);
        json = new JSONObject(jt);
        assertResponse(HTTP_OK);

        String mapId = nodeType + "-" + nodeid1 + "." + vlan1;
        Assert.assertEquals(mapId, json.getString("id"));
        Assert.assertEquals(vlan1, json.getString("vlan"));
        JSONObject nodeinfo = json.getJSONObject("node");
        Assert.assertEquals(nodeType, nodeinfo.getString("type"));
        Assert.assertEquals(nodeid1, nodeinfo.getString("id"));

        result = getJsonResult(baseURL + bname + searchByConf + "?vlan=" +
                               vlan1 + "&node=" + nodeType + "|" + nodeid1);
        assertResponse(HTTP_OK);
        jt = new JSONTokener(result);
        json = new JSONObject(jt);
        Assert.assertEquals(mapId, json.getString("id"));
        Assert.assertEquals(vlan1, json.getString("vlan"));
        nodeinfo = json.getJSONObject("node");
        Assert.assertEquals(nodeType, nodeinfo.getString("type"));
        Assert.assertEquals(nodeid1, nodeinfo.getString("id"));

        // Test GET all  VLAN Mapping in default container
        result = getJsonResult(baseURL + bname + "/vlanmaps");
        jt = new JSONTokener(result);
        json = new JSONObject(jt);
        vLANMapArray = json.getJSONArray("vlanmap");

        assertResponse(HTTP_OK);
        Assert.assertEquals(3, vLANMapArray.length());
    }

    /**
     * Ensure that the VLAN mapping APIs return correct error response.
     *
     * @param containerName   The container name.
     * @param tname           The tenant name for the test.
     * @param bname1          The bridge name for the test.
     * @param bname2          The bridge name for the test.
     * @param auth            If {@code true}, the client sends authenticated
     *                        request. Otherwise the client sends unauthorized
     *                        request.
     * @param expected        Expected HTTP response code.
     */
    private void checkVlanMapError(String containerName, String tname,
                                   String bname1, String bname2, boolean auth,
                                   int expected) {
        String base = VTN_BASE_URL + containerName + "/vtns/" + tname +
            "/vbridges/";
        String ct = "Application/Json";
        String body = "{\"vlan\": 0, \"node\":{\"type\":\"OF\",\"id\":1}}";
        String uri = base + bname1 + "/vlanmaps";
        getJsonResult(uri, "POST", body, ct, auth);
        assertResponse(expected);

        getJsonResult(uri, "GET", null, ct, auth);
        assertResponse(expected);

        uri = base + bname2 + "/vlanmaps/OF-1.0";
        getJsonResult(uri, "GET", null, ct, auth);
        assertResponse(expected);

        getJsonResult(uri, "DELETE", null, ct, auth);
        assertResponse(expected);
    }

    /**
     * Test case for VLAN Mapping delete APIs.
     *
     * <p>
     *   This method is called by {@link #testVBridgeAPI(String, String)}.
     * </p>
     *
     * @param tname     A tenant name.
     *                  Specified tenant is necessary to be configured
     *                  before method is called.
     * @param bname     A vBridge name.
     *                  Specified vBridge is necessary to be configured
     *                  before method is called.
     * @throws JSONException  An error occurred.
     */
    private void testVLANMappingDeleteAPI(String tname, String bname)
        throws JSONException {
        LOG.info("Starting delete VLAN Mapping JAX-RS client.");
        String url = VTN_BASE_URL;
        StringBuilder baseURL = new StringBuilder();
        baseURL.append(url);
        baseURL.append("default/vtns/");
        baseURL.append(tname);
        baseURL.append("/vbridges/");

        String nodeid1 = "00:00:00:00:00:00:00:01";
        String vlan1 = "1000";
        String nodeType = "OF";

        String tnameDummy = "tnameDummy";
        String bnameDummy = "bname_dummy";
        String vlanDummy = "1234";

        // Test DELETE VLAN Mapping
        String result = getJsonResult(baseURL + bname + "/vlanmaps/" +
                                      nodeType + "-" + nodeid1 + "." + vlan1,
                                      "DELETE");
        assertResponse(HTTP_OK);

        // Test DELETE VLAN Mapping expecting 404
        // setting deleted vlan mapping
        result = getJsonResult(baseURL + bname + "/vlanmaps/" + nodeType +
                               "-" + nodeid1 + "." + vlan1, "DELETE");
        assertResponse(HTTP_NOT_FOUND);

        // setting dummy tenant
        result = getJsonResult(url + "default/vtns/" + tnameDummy +
                               "/vbridges/" +  bname + "/vlanmaps/" +
                               nodeType + "-" + nodeid1 + "." + vlan1,
                               "DELETE");
        assertResponse(HTTP_NOT_FOUND);

        // setting dummy vbridge mapping
        result = getJsonResult(baseURL + bnameDummy + "/vlanmaps/" + nodeType +
                               "-" + nodeid1 + "." + vlan1, "DELETE");
        assertResponse(HTTP_NOT_FOUND);

        // setting dummy vlan mapping
        result = getJsonResult(baseURL + bname + "/vlanmaps/" + nodeType +
                               "-" + nodeid1 + "." + vlanDummy, "DELETE");
        assertResponse(HTTP_NOT_FOUND);

        // Test GET VLAN Mapping expecting 404
        result = getJsonResult(baseURL + bname + "/vlanmaps" + nodeType + "-"
                               + nodeid1 + "." + vlan1);
        assertResponse(HTTP_NOT_FOUND);


        // Test GET all  VLAN Mapping in default container
        result = getJsonResult(baseURL + bname + "/vlanmaps");
        JSONTokener jt = new JSONTokener(result);
        JSONObject json = new JSONObject(jt);
        JSONArray vLANMapArray = json.getJSONArray("vlanmap");

        assertResponse(HTTP_OK);
        Assert.assertEquals(2, vLANMapArray.length());

        // delete VLAN mappings set for test.
        for (int i = 0; i < vLANMapArray.length(); i++) {
            JSONObject vLANMap = vLANMapArray.getJSONObject(i);
            result = getJsonResult(baseURL + bname + "/vlanmaps/" +
                                   vLANMap.getString("id"), "DELETE");
            assertResponse(HTTP_OK);
        }
    }

    /**
     * Test case for MAC mapping APIs.
     *
     * <p>
     *   This method is called by {@link #testVBridgeAPI(String, String)}.
     * </p>
     *
     * @param tname   The name of existing virtual tenant.
     * @param bname1  The name of existing vBridge.
     * @param bname2  The name of existing vBridge.
     * @throws JSONException  An error occurred.
     */
    private void testMacMappingAPI(String tname, String bname1, String bname2)
        throws JSONException {
        LOG.info("Starting MAC Mapping JAX-RS client.");

        String qparam = "?param1=1&param2=2";
        String mapUri = createURI("default/vtns", tname, "vbridges", bname1,
                                  "macmap");
        String mapUri2 = createURI("default/vtns", tname, "vbridges", bname2,
                                   "macmap");

        // Ensure that no MAC mapping is configured.
        assertNoMacMapping(mapUri);
        assertNoMacMapping(mapUri2);

        LinkedList<String> allowedHosts = new LinkedList<String>();
        allowedHosts.add(null);
        allowedHosts.add(null);

        byte[] allowedAddr = {
            (byte)0x10, (byte)0x20, (byte)0x30,
            (byte)0x40, (byte)0x50, (byte)0x00,
        };

        for (int i = 0; i < 20; i++) {
            String mac;
            if ((i & 1) == 0) {
                mac = null;
            } else {
                allowedAddr[5] = (byte)i;
                mac = HexEncode.bytesToHexStringFormat(allowedAddr);
            }

            String vlan = (i == 0) ? "4095" : String.valueOf(i);
            allowedHosts.add(mac);
            allowedHosts.add(vlan);
        }

        LinkedList<String> deniedHosts = new LinkedList<String>();
        byte[] deniedAddr = {
            (byte)0xf0, (byte)0xf1, (byte)0xf2,
            (byte)0xf3, (byte)0x00, (byte)0xf5,
        };

        for (int i = 0; i < 20; i++) {
            deniedAddr[4] = (byte)i;
            String mac = HexEncode.bytesToHexStringFormat(deniedAddr);
            String vlan = String.valueOf(i + 100);
            deniedHosts.add(mac);
            deniedHosts.add(vlan);
        }
        deniedHosts.add("00:00:ff:ff:ff:0f");
        deniedHosts.add(null);

        // Install MAC mapping configuration.
        JSONObject config = createMacMapConfig(allowedHosts, deniedHosts);
        JSONObject configExpected = completeMacMapConfig(config);

        String cf = config.toString();
        getJsonResult(mapUri + qparam, "PUT", cf);
        assertResponse(HTTP_CREATED);
        Assert.assertEquals(mapUri, httpLocation);

        for (String method: new String[]{"PUT", "POST"}) {
            getJsonResult(mapUri, method, cf);
            assertResponse(HTTP_NO_CONTENT);
        }

        assertMacMapping(mapUri, configExpected);

        // Try to create MAC mapping with specifying invalid MAC address.
        String[] badMacs = {
            // broken
            "invalid_mac_address",
            // zero
            "00:00:00:00:00:00",
            // broadcast
            "ff:ff:ff:ff:ff:ff",
            // multicast
            "81:00:11:22:33:44",
        };

        String[] acls = {"allow", "deny"};
        for (String mac: badMacs) {
            JSONObject machost = new JSONObject();
            machost.put("address", mac);
            machost.put("vlan", 1);

            JSONArray maclist = new JSONArray();
            maclist.put(machost);

            JSONObject set = new JSONObject();
            set.put("machost", maclist);

            for (String acl: acls) {
                String uri = createRelativeURI(mapUri, acl, mac, "1");
                getJsonResult(uri, "PUT");
                assertResponse(HTTP_BAD_REQUEST);

                for (String method: new String[]{"PUT", "POST"}) {
                    JSONObject conf = new JSONObject();
                    conf.put(acl, set);

                    getJsonResult(mapUri, method, conf.toString());
                    assertResponse(HTTP_BAD_REQUEST);

                    uri = createRelativeURI(mapUri, acl);
                    getJsonResult(uri, method, set.toString());
                    assertResponse(HTTP_BAD_REQUEST);
                }
            }
        }

        // Try to create MAC mapping with specifying invalid VLAN ID.
        int[] badVlans = {
            -1, 4096, 10000,
        };

        for (int vlan: badVlans) {
            String mac = "00:00:00:00:00:10";
            JSONObject machost = new JSONObject();
            machost.put("address", mac);
            machost.put("vlan", vlan);

            JSONArray maclist = new JSONArray();
            maclist.put(machost);

            JSONObject set = new JSONObject();
            set.put("machost", maclist);

            for (String acl: acls) {
                String uri = createRelativeURI(mapUri, acl, mac,
                                               String.valueOf(vlan));
                getJsonResult(uri, "PUT");
                assertResponse(HTTP_BAD_REQUEST);

                for (String method: new String[]{"PUT", "POST"}) {
                    JSONObject conf = new JSONObject();
                    conf.put(acl, set);

                    getJsonResult(mapUri, method, conf.toString());
                    assertResponse(HTTP_BAD_REQUEST);

                    uri = createRelativeURI(mapUri, acl);
                    getJsonResult(uri, method, set.toString());
                    assertResponse(HTTP_BAD_REQUEST);
                }
            }
        }

        // Try to register NULL address to deny set.
        for (String method: new String[]{"POST", "PUT"}) {
            String acl = "deny";
            JSONObject set = createMacHostSet(null, "110");
            JSONObject conf = new JSONObject();
            conf.put(acl, set);

            getJsonResult(mapUri, method, conf.toString());
            assertResponse(HTTP_BAD_REQUEST);

            String uri = createRelativeURI(mapUri, acl);
            getJsonResult(uri, method, set.toString());
            assertResponse(HTTP_BAD_REQUEST);
        }

        // Try to remove hosts that are not registered.
        String remove = "?action=remove";
        LinkedList<String> allowedHosts1 = new LinkedList<String>();
        for (int i = 0; i < 5; i++) {
            String mac;
            if ((i & 1) == 0) {
                mac = null;
            } else {
                allowedAddr[5] = (byte)i;
                mac = HexEncode.bytesToHexStringFormat(allowedAddr);
            }

            String vlan = String.valueOf(i + 100);
            allowedHosts1.add(mac);
            allowedHosts1.add(vlan);
        }

        for (int i = 5; i < 10; i++) {
            String mac;
            if ((i & 1) == 0) {
                allowedAddr[5] = (byte)(i + 1);
                mac = HexEncode.bytesToHexStringFormat(allowedAddr);
            } else {
                mac = null;
            }

            String vlan = String.valueOf(i);
            allowedHosts1.add(mac);
            allowedHosts1.add(vlan);
        }

        LinkedList<String> deniedHosts1 = new LinkedList<String>();
        for (int i = 0; i < 5; i++) {
            deniedAddr[4] = (byte)(i + 1);
            String mac = HexEncode.bytesToHexStringFormat(deniedAddr);
            String vlan = String.valueOf(i + 100);
            deniedHosts1.add(mac);
            deniedHosts1.add(vlan);
        }
        for (int i = 5; i < 10; i++) {
            deniedAddr[4] = (byte)i;
            String mac = HexEncode.bytesToHexStringFormat(deniedAddr);
            String vlan = String.valueOf(i + 1000);
            deniedHosts1.add(mac);
            deniedHosts1.add(vlan);
        }

        JSONObject allow1 = createMacHostSet(allowedHosts1);
        JSONObject deny1 = createMacHostSet(deniedHosts1);
        config = new JSONObject();
        config.put("allow", allow1).put("deny", deny1);
        getJsonResult(createRelativeURI(mapUri + remove), "POST",
                      config.toString());
        assertResponse(HTTP_NO_CONTENT);
        for (String acl: acls) {
            JSONObject set;
            LinkedList<String> hostList;
            if (acl.equals("allow")) {
                set = allow1;
                hostList = allowedHosts1;
            } else {
                set = deny1;
                hostList = deniedHosts1;
            }

            getJsonResult(createRelativeURI(mapUri, acl + remove), "POST",
                          set.toString());
            assertResponse(HTTP_NO_CONTENT);

            Iterator<String> it = hostList.iterator();
            while (it.hasNext()) {
                String m = it.next();
                String v = it.next();
                String uri = createRelativeURI(mapUri, acl,
                                               (m == null) ? "ANY" : m,
                                               (v == null) ? "0" : v);
                getJsonResult(uri, "DELETE");
                assertResponse(HTTP_NO_CONTENT);
            }
        }

        String[] allowedArray = allowedHosts.toArray(new String[0]);
        for (int i = 0; i < allowedArray.length; i += 2) {
            String acl = "allow";
            String mac = allowedArray[i];
            String vlan = allowedArray[i + 1];

            // Try to map MAC addresses which are already mapped by another
            // vBridge.
            String uri = createRelativeURI(mapUri2, acl,
                                           (mac == null) ? "ANY" : mac,
                                           (vlan == null) ? "0" : vlan);
            getJsonResult(uri, "PUT");
            assertResponse(HTTP_CONFLICT);

            JSONObject set = createMacHostSet(mac, vlan);
            JSONObject conf = new JSONObject();
            conf.put(acl, set);

            for (String method: new String[]{"POST", "PUT"}) {
                getJsonResult(mapUri2, method, conf.toString());
                assertResponse(HTTP_CONFLICT);

                uri = createRelativeURI(mapUri2, acl);
                getJsonResult(uri, method, set.toString());
                assertResponse(HTTP_CONFLICT);
            }

            if (mac == null) {
                continue;
            }

            // Try to register duplicate MAC address.
            uri = createRelativeURI(mapUri, acl, mac, "1000");
            getJsonResult(uri, "PUT");
            assertResponse(HTTP_CONFLICT);

            set = createMacHostSet(mac, "1000");
            conf = new JSONObject();
            conf.put(acl, set);

            String method = "POST";
            getJsonResult(mapUri, method, conf.toString());
            assertResponse(HTTP_CONFLICT);

            uri = createRelativeURI(mapUri, acl);
            getJsonResult(uri, method, set.toString());
            assertResponse(HTTP_CONFLICT);
        }

        // Duplicate MAC address in allow set.
        String acl = "allow";
        String mac = "00:11:22:33:55:88";
        JSONObject set = createMacHostSet(mac, "1000", mac, "1001");
        for (String method: new String[]{"POST", "PUT"}) {
            JSONObject conf = new JSONObject();
            conf.put(acl, set);

            getJsonResult(mapUri, method, conf.toString());
            assertResponse(HTTP_BAD_REQUEST);

            String uri = createRelativeURI(mapUri, acl);
            getJsonResult(uri, method, set.toString());
            assertResponse(HTTP_BAD_REQUEST);
        }

        // Configuration should not be affected.
        assertMacMapping(mapUri, configExpected);

        // Remove 2 hosts from both list.
        for (int i = 0; i < 4; i++) {
            allowedHosts.removeFirst();
            deniedHosts.removeFirst();
        }

        config = createMacMapConfig(allowedHosts, deniedHosts);
        getJsonResult(mapUri, "PUT", config.toString());
        assertResponse(HTTP_OK);

        configExpected = completeMacMapConfig(config);
        assertMacMapping(mapUri, configExpected);

        for (String acl1: acls) {
            LinkedList<String> hostList = (acl1.equals("allow"))
                ? allowedHosts : deniedHosts;

            // Remove 2 hosts by specifying the list.
            String mac1 = hostList.removeFirst();
            String vlan1 = hostList.removeFirst();
            String mac2 = hostList.removeFirst();
            String vlan2 = hostList.removeFirst();
            JSONObject set1 = createMacHostSet(mac1, vlan1, mac2, vlan2);

            String uri = createRelativeURI(mapUri, acl1 + remove);
            getJsonResult(uri, "POST", set1.toString());
            assertResponse(HTTP_OK);
            assertMacMapping(mapUri, allowedHosts, deniedHosts);
        }

        // Remove 2 hosts from both lists.
        config = new JSONObject();
        for (String acl1: acls) {
            LinkedList<String> hostList = (acl1.equals("allow"))
                ? allowedHosts : deniedHosts;
            String mac1 = hostList.removeFirst();
            String vlan1 = hostList.removeFirst();
            String mac2 = hostList.removeFirst();
            String vlan2 = hostList.removeFirst();
            JSONObject set1 = createMacHostSet(mac1, vlan1, mac2, vlan2);
            config.put(acl1, set1);
        }
        getJsonResult(mapUri + remove, "POST", config.toString());
        assertResponse(HTTP_OK);
        assertMacMapping(mapUri, allowedHosts, deniedHosts);

        // Remove 3 hosts by DELETE.
        for (String acl1: acls) {
            LinkedList<String> hostList = (acl1.equals("allow"))
                ? allowedHosts : deniedHosts;

            for (int i = 0; i < 3; i++) {
                String m = hostList.removeFirst();
                String v = hostList.removeFirst();
                String uri = createRelativeURI(mapUri, acl1,
                                               (m == null) ? "ANY" : m,
                                               (v == null) ? "0" : v);
                getJsonResult(uri, "DELETE");
                assertResponse(HTTP_OK);
            }
        }
        assertMacMapping(mapUri, allowedHosts, deniedHosts);

        // Remove 3 hosts, and add 4 hosts by PUT.
        byte[] base = {
            (byte)0xa0, (byte)0xa1, (byte)0xa2,
            (byte)0x00, (byte)0xa4, (byte)0xa5,
        };
        for (String acl1: acls) {
            LinkedList<String> hostList = (acl1.equals("allow"))
                ? allowedHosts : deniedHosts;
            for (int i = 0; i < 6; i++) {
                hostList.removeFirst();
            }

            for (int i = 0; i < 4; i++) {
                base[3] = (byte)i;
                hostList.add(HexEncode.bytesToHexStringFormat(base));
                hostList.add(String.valueOf(i + 500));
            }
        }

        config = createMacMapConfig(allowedHosts, deniedHosts);
        configExpected = completeMacMapConfig(config);
        getJsonResult(mapUri, "PUT", config.toString());
        assertResponse(HTTP_OK);
        assertMacMapping(mapUri, allowedHosts, deniedHosts);

        for (String acl1: acls) {
            LinkedList<String> hostList = (acl1.equals("allow"))
                ? allowedHosts : deniedHosts;
            for (int i = 0; i < 6; i++) {
                hostList.removeFirst();
            }

            for (int i = 20; i < 24; i++) {
                base[3] = (byte)i;
                hostList.add(HexEncode.bytesToHexStringFormat(base));
                hostList.add(String.valueOf(i + 500));
            }

            JSONObject set1 = createMacHostSet(hostList);
            getJsonResult(createRelativeURI(mapUri, acl1), "PUT",
                          set1.toString());
            assertResponse(HTTP_OK);
            assertMacMapping(mapUri, allowedHosts, deniedHosts);
        }

        // Remove MAC mapping by specifying host list.
        LinkedList<String> saveAllowed = new LinkedList<String>(allowedHosts);
        LinkedList<String> saveDenied = new LinkedList<String>(deniedHosts);
        for (int i = 0; i < acls.length; i++) {
            String acl1 = acls[i];
            LinkedList<String> hostList = (acl1.equals("allow"))
                ? allowedHosts : deniedHosts;
            hostList.clear();

            String uri = createRelativeURI(mapUri, acl1);
            getJsonResult(uri, "DELETE");
            assertResponse(HTTP_OK);

            if (i == acls.length - 1) {
                assertNoMacMapping(mapUri);
            } else {
                assertMacMapping(mapUri, allowedHosts, deniedHosts);
            }
        }

        // Install the same MAC mapping to another vBridge.
        config = createMacMapConfig(saveAllowed, saveDenied);
        configExpected = completeMacMapConfig(config);
        getJsonResult(mapUri2, "PUT", config.toString());
        assertResponse(HTTP_CREATED);
        assertMacMapping(mapUri2, configExpected);

        getJsonResult(mapUri2, "DELETE");
        assertResponse(HTTP_OK);
        assertNoMacMapping(mapUri2);

        // Create MAC mapping by installing a host to the specific URI.
        mac = "00:11:22:33:44:55";
        String vlan = "4095";
        JSONObject hostSet = createMacHostSet(mac, vlan);
        List<String> empty = new LinkedList<String>();

        for (String acl1: acls) {
            JSONObject set1;
            if (acl1.equals("allow")) {
                set1 = createMacHostSet(saveAllowed);
                config = createMacMapConfig(saveAllowed, empty);
            } else {
                set1 = createMacHostSet(saveDenied);
                config = createMacMapConfig(empty, saveDenied);
            }

            configExpected = completeMacMapConfig(config);
            String uri = createRelativeURI(mapUri2, acl1);
            for (String method: new String[]{"PUT", "POST"}) {
                getJsonResult(uri + qparam, method, set1.toString());
                assertResponse(HTTP_CREATED);
                Assert.assertEquals(uri, httpLocation);
                assertMacMapping(mapUri2, configExpected);

                getJsonResult(uri, "DELETE");
                assertResponse(HTTP_OK);
                assertNoMacMapping(mapUri2);
            }

            config = new JSONObject();
            config.put(acl1, hostSet);
            String hostUri = createRelativeURI(uri, mac, vlan);
            getJsonResult(hostUri + qparam, "PUT");
            assertResponse(HTTP_CREATED);
            Assert.assertEquals(hostUri, httpLocation);
            assertMacMapping(mapUri2, config);

            getJsonResult(hostUri, "DELETE");
            assertResponse(HTTP_OK);
            assertNoMacMapping(mapUri2);
        }
    }

    /**
     * Ensure that no MAC mapping is configured.
     *
     * @param mapUri  A URI to the MAC mapping.
     */
    private void assertNoMacMapping(String mapUri) {
        String[] uris = {
            mapUri,
            createRelativeURI(mapUri, "allow"),
            createRelativeURI(mapUri, "allow/00:00:00:00:00:01/0"),
            createRelativeURI(mapUri, "allow/ANY/0"),
            createRelativeURI(mapUri, "deny"),
            createRelativeURI(mapUri, "deny/00:00:00:00:00:01/0"),
            createRelativeURI(mapUri, "mapped"),
            createRelativeURI(mapUri, "mapped/00:00:00:00:00:01"),
        };

        for (String uri: uris) {
            for (String method: new String[]{"GET", "DELETE"}) {
                getJsonResult(uri);
                assertResponse(HTTP_NO_CONTENT);
            }
        }
    }

    /**
     * Ensure that MAC mapping is configured as expected.
     *
     * @param mapUri   A URI to the MAC mapping.
     * @param allowed  A list which contains hosts in allow list.
     * @param denied   A list which contains hosts in deny list.
     * @return  A {@link JSONObject} which contains current configuration.
     * @throws JSONException  An error occurred.
     */
    private JSONObject assertMacMapping(String mapUri, List<String> allowed,
                                        List<String> denied)
        throws JSONException {
        JSONObject config = createMacMapConfig(allowed, denied);
        JSONObject expected = completeMacMapConfig(config);
        assertMacMapping(mapUri, expected);

        return expected;
    }

    /**
     * Ensure that MAC mapping is configured as expected.
     *
     * @param mapUri    A URI to the MAC mapping.
     * @param expected  A {@link JSONObject} which contains expected
     *                  configuration.
     * @throws JSONException  An error occurred.
     */
    private void assertMacMapping(String mapUri, JSONObject expected)
        throws JSONException {
        String resp = getJsonResult(mapUri);
        assertResponse(HTTP_OK);

        JSONObject json = new JSONObject(new JSONTokener(resp));
        assertEquals(expected, json);

        for (String acl: new String[]{"allow", "deny"}) {
            String aclUri = createRelativeURI(mapUri, acl);
            resp = getJsonResult(aclUri);
            assertResponse(HTTP_OK);
            json = new JSONObject(new JSONTokener(resp));
            JSONObject set = expected.optJSONObject(acl);
            if (set != null) {
                assertEquals(set, json);

                JSONArray jarray = set.getJSONArray("machost");
                int len = jarray.length();
                for (int i = 0; i < len; i++) {
                    JSONObject host = jarray.getJSONObject(i);
                    String mac = host.optString("address", "ANY");
                    String vlan = host.getString("vlan");
                    String uri = createRelativeURI(aclUri, mac, vlan);
                    resp = getJsonResult(uri);
                    assertResponse(HTTP_OK);
                    json = new JSONObject(new JSONTokener(resp));
                    assertEquals(host, json);
                }
            }
        }
    }

    /**
     * Test case for MAC Address APIs.
     *
     * <p>
     *   This method is called by {@link #testVBridgeAPI(String, String)}.
     * </p>
     *
     * @param tname     A tenant name.
     *                  Specified tenant is necessary to be configured
     *                  before method is called.
     * @param bname     A vBridge name.
     *                  Specified vBridge is necessary to be configured
     *                  before method is called.
     * @throws JSONException  An error occurred.
     */
    private void testMacAddressAPI(String tname, String bname)
        throws JSONException {
        LOG.info("Starting MAC address JAX-RS client.");
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
        assertResponse(HTTP_OK);
        JSONTokener jt = new JSONTokener(result);
        JSONObject json = new JSONObject(jt);
        JSONArray macArray = json.getJSONArray("macentry");
        Assert.assertEquals(0, macArray.length());

        // Test GET all MAC address expecting 404
        // setting dummy vtn
        result = getJsonResult(url + "default/vtns/" + dummy + "/vbridges/" +
                               bname + "/mac");
        assertResponse(HTTP_NOT_FOUND);

        // setting dummy vbridge
        result = getJsonResult(baseURL + dummy + "/mac");
        assertResponse(HTTP_NOT_FOUND);

        // Test DELETE all MAC address
        result = getJsonResult(baseURL + bname + "/mac", "DELETE");
        assertResponse(HTTP_OK);

        // Test DELETE all MAC address expecting 404
        // setting dummy vtn
        result = getJsonResult(url + "default/vtns/" + dummy + "/vbridges/" +
                               bname + "/mac", "DELETE");
        assertResponse(HTTP_NOT_FOUND);

        // setting dummy vbridge
        result = getJsonResult(baseURL + dummy + "/mac", "DELETE");
        assertResponse(HTTP_NOT_FOUND);

        // Test GET MAC address expecting 404
        // setting MAC address that don't exist
        result = getJsonResult(baseURL + bname + "/mac/" + macaddr);
        assertResponse(HTTP_NOT_FOUND);

        // Test DELETE MAC address expecting 404
        // setting MAC address that don't exist
        result = getJsonResult(baseURL + bname + "/mac/" + macaddr, "DELETE");
        assertResponse(HTTP_NOT_FOUND);


        // add MAC Address Entry.
        VBridgePath bpath = new VBridgePath(tname, bname);
        Node node = NodeCreator.createOFNode(0L);
        NodeConnector nc = NodeConnectorCreator.
            createOFNodeConnector(Short.valueOf((short)0), node);

        // test with MAC Table entry existing.
        // set VLAN Map and Mac Table Entry before test.
        short vlan = 0;
        String requestBody = "{\"vlan\":\"" + vlan + "\"}";
        result = getJsonResult(baseURL + bname + "/vlanmaps", "POST",
                               requestBody);
        assertResponse(HTTP_CREATED);

        byte[] srcMac1 = {0x00, 0x00, 0x00, 0x00, 0x00, 0x01};
        byte[] srcMac2 = {0x00, 0x00, 0x00, 0x00, 0x00, 0x02};
        byte[] sender1 = {(byte)192, (byte)168, (byte)0, (byte)1};
        byte[] sender2 = {(byte)192, (byte)168, (byte)0, (byte)2};

        putMacTableEntry(listenDataPacket, bpath, srcMac1, sender1, nc);
        putMacTableEntry(listenDataPacket, bpath, srcMac2, sender2, nc);

        Set<String> expectedMacSet = new HashSet<String>();
        expectedMacSet.add(HexEncode.bytesToHexStringFormat(srcMac1));
        expectedMacSet.add(HexEncode.bytesToHexStringFormat(srcMac2));

        // GET a list of MAC entry in vBridge.
        checkMacTableEntry(baseURL + bname, 2, expectedMacSet, vlan);

        // Test GET all MAC address expecting 404
        // setting dummy vtn
        result = getJsonResult(url + "default/vtns/" + dummy + "/vbridges/" +
                               bname + "/mac");
        assertResponse(HTTP_NOT_FOUND);

        // setting dummy vbridge
        result = getJsonResult(baseURL + dummy + "/mac");
        assertResponse(HTTP_NOT_FOUND);


        // Test DELETE MAC address expecting 404
        // setting dummy vtn
        Iterator<String> itMac = expectedMacSet.iterator();
        String expectedMac = itMac.next();
        result = getJsonResult(url + "default/vtns/" + dummy + "/vbridges/" +
                               bname + "/mac" + expectedMac, "DELETE");
        assertResponse(HTTP_NOT_FOUND);

        // setting dummy vbridge
        result = getJsonResult(baseURL + dummy + "/mac/" + expectedMac,
                               "DELETE");
        assertResponse(HTTP_NOT_FOUND);

        // setting correct URI.
        result = getJsonResult(baseURL + bname + "/mac/" + expectedMac,
                               "DELETE");
        assertResponse(HTTP_OK);

        expectedMacSet.remove(expectedMac);
        checkMacTableEntry(baseURL + bname, 1, expectedMacSet, vlan);

        // delete another entry.
        expectedMac = expectedMacSet.iterator().next();
        result = getJsonResult(baseURL + bname + "/mac/" + expectedMac,
                               "DELETE");
        assertResponse(HTTP_OK);

        expectedMacSet.clear();
        checkMacTableEntry(baseURL + bname, 0, expectedMacSet, vlan);

        // Add 10 Mac Entries.
        int numEntry = 10;
        for (int i = 0; i < numEntry; i++) {
            byte[] src = {0x00, 0x00, 0x00, 0x00, 0x01, (byte)(i + 1)};
            byte[] sender = {(byte)192, (byte)168, (byte)1, (byte)(i + 1)};

            putMacTableEntry(listenDataPacket, bpath, src, sender, nc);
            expectedMacSet.add(HexEncode.bytesToHexStringFormat(src));
        }

        // GET a list of MAC entry in vBridge
        checkMacTableEntry(baseURL + bname, numEntry, expectedMacSet, vlan);

        // Authentication failure.
        expectedMac = expectedMacSet.iterator().next();
        checkMacError(GlobalConstants.DEFAULT.toString(), tname, bname,
                      expectedMac, false, HttpURLConnection.HTTP_UNAUTHORIZED);

        // Invalid container name.
        String[] invalid = {"bad_container", "version"};
        for (String inv: invalid) {
            checkMacError(inv, tname, bname, expectedMac, true,
                          HttpURLConnection.HTTP_NOT_FOUND);
            checkMacError(inv, tname, bname, expectedMac, false,
                          HttpURLConnection.HTTP_UNAUTHORIZED);
        }

        // flush mac address table.
        result = getJsonResult(baseURL + bname + "/mac", "DELETE", null,
                               "applicatin/json", false);
        assertResponse(HTTP_UNAUTHORIZED);
    }

    private void checkMacError(String containerName, String tname,
                               String bname, String mac, boolean auth,
                               int expected) {
        String base = VTN_BASE_URL + containerName + "/vtns/" + tname +
            "/vbridges/" + bname + "/mac";
        String ct = "Application/Json";
        getJsonResult(base, "GET", null, ct, auth);
        assertResponse(expected);

        getJsonResult(base, "DELETE", null, ct, auth);
        assertResponse(expected);

        String uri = base + "/" + mac;
        getJsonResult(uri, "GET", null, ct, auth);
        assertResponse(expected);

        getJsonResult(uri, "DELETE", null, ct, auth);
        assertResponse(expected);
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
    private void checkMacTableEntry(String url, int numEntry,
                                    Set<String> expectedMacSet, short vlan)
        throws JSONException {
        String result = getJsonResult(url + "/mac");
        assertResponse(HTTP_OK);
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
     * @param listenData  {@code IListenDataPacket} service.
     * @param bpath       A path to the virtual L2 bridge.
     * @param src         A source MAC Address.
     * @param sender      A sender IP addesss.
     * @param nc          A NodeConnector.
     */
    private void putMacTableEntry(IListenDataPacket listenData,
                                  VBridgePath bpath, byte[] src, byte[] sender,
                                  NodeConnector nc) {

        byte[] dst = {(byte)0xFF, (byte)0xFF, (byte)0xFF,
                      (byte)0xFF, (byte)0xFF, (byte)0xFF};
        byte[] target = {(byte)192, (byte)168, (byte)0, (byte)250};

        RawPacket inPkt = createARPRawPacket(src, dst, sender, target,
                                             (short)-1, nc, ARP.REQUEST);

        listenData.receiveDataPacket(inPkt);
    }

    /**
     * Test case for getting version information APIs.
     *
     * This method is called by {@link #testVTNAPI()}.
     */
    private void testVtnGlobal(String base) throws JSONException {
        LOG.info("Starting IVTNGlobal JAX-RS client.");

        // Authentication failure.
        String uri = base + "version";
        getJsonResult(uri, "GET", null, "Application/Json", false);
        assertResponse(HTTP_UNAUTHORIZED);

        // Get version information of the VTN Manager.
        String result = getJsonResult(uri);
        assertResponse(HTTP_OK);

        JSONTokener jt = new JSONTokener(result);
        JSONObject json = new JSONObject(jt);
        int api = json.getInt("api");
        JSONObject bv = json.getJSONObject("bundle");
        Assert.assertTrue("API version: " + api, api > 0);

        // Determine actual bundle version of manager.implementation.
        Version ver = implBundle.getVersion();
        Assert.assertNotNull(ver);

        Assert.assertEquals(ver.getMajor(), bv.getInt("major"));
        Assert.assertEquals(ver.getMinor(), bv.getInt("minor"));
        Assert.assertEquals(ver.getMicro(), bv.getInt("micro"));

        String qf = ver.getQualifier();
        String qfkey  = "qualifier";
        if (qf == null || qf.isEmpty()) {
            Assert.assertFalse(bv.has(qfkey));
        } else {
            Assert.assertEquals(qf, bv.getString(qfkey));
        }
    }

    /**
     * Test case for all APIs with service unavailable.
     */
    @Test
    public void testServiceUnavailable() {
        LOG.info("Starting Test with Service Unavailable.");

        // stop VTNManagerImpl.
        Assert.assertNotNull(implBundle);
        try {
            implBundle.stop();
        } catch (BundleException e) {
            unexpected(e);
        }

        // VTN APIs.
        String baseURL = VTN_BASE_URL + "default/vtns";
        String tname = "testVtn";
        String queryParameter = new QueryParameter("all", "true").getString();

        String result = getJsonResult(baseURL);
        assertResponse(HTTP_UNAVAILABLE);

        result = getJsonResult(baseURL + "/" + tname);
        assertResponse(HTTP_UNAVAILABLE);

        String requestBody = "{}";
        result = getJsonResult(baseURL + "/" + tname, "POST", requestBody);
        assertResponse(HTTP_UNAVAILABLE);

        requestBody = "{}";
        result = getJsonResult(baseURL + "/" + tname + queryParameter, "PUT", requestBody);
        assertResponse(HTTP_UNAVAILABLE);

        result = getJsonResult(baseURL + "/" + tname, "DELETE");
        assertResponse(HTTP_UNAVAILABLE);

        // vBridge APIs.
        baseURL = baseURL + "/" + tname + "/vbridges";
        String bname = "testVBridge";
        result = getJsonResult(baseURL);
        assertResponse(HTTP_UNAVAILABLE);

        result = getJsonResult(baseURL + "/" + bname);
        assertResponse(HTTP_UNAVAILABLE);

        requestBody = "{}";
        result = getJsonResult(baseURL + "/" + bname, "POST", requestBody);
        assertResponse(HTTP_UNAVAILABLE);

        requestBody = "{}";
        result = getJsonResult(baseURL + "/" + bname + queryParameter, "PUT", requestBody);
        assertResponse(HTTP_UNAVAILABLE);

        result = getJsonResult(baseURL + "/" + bname, "DELETE");
        assertResponse(HTTP_UNAVAILABLE);

        // vBridgeInterface APIs.
        baseURL = baseURL + "/" + bname;
        String baseURLIf = baseURL + "/interfaces";
        String ifname = "testInterface";

        result = getJsonResult(baseURLIf);
        assertResponse(HTTP_UNAVAILABLE);

        result = getJsonResult(baseURLIf + "/" + ifname);
        assertResponse(HTTP_UNAVAILABLE);

        requestBody = "{}";
        result = getJsonResult(baseURLIf + "/" + ifname, "POST", requestBody);
        assertResponse(HTTP_UNAVAILABLE);

        requestBody = "{}";
        result = getJsonResult(baseURLIf + "/" + ifname + queryParameter, "PUT",
                               requestBody);
        assertResponse(HTTP_UNAVAILABLE);

        result = getJsonResult(baseURLIf + "/" + ifname, "DELETE");
        assertResponse(HTTP_UNAVAILABLE);

        // PortMapping APIs.
        String baseURLPmap = baseURLIf + "/" + ifname + "/portmap";
        short vlan = 0;
        String nodeid = "00:00:00:00:00:00:00:01";
        String nodeType = "OF";
        String portnum = "1";
        requestBody = "{\"vlan\":" + vlan + ", \"node\":{\"type\":\"" + nodeType
                + "\", \"id\":\"" + nodeid + "\"}, \"port\":{"
                + "\"type\":\"" + nodeType
                + "\", \"id\":\"" + portnum + "\"}}";
        result = getJsonResult(baseURLPmap + "/", "PUT", requestBody);
        assertResponse(HTTP_UNAVAILABLE);

        result = getJsonResult(baseURLPmap);
        assertResponse(HTTP_UNAVAILABLE);

        result = getJsonResult(baseURLPmap + "/", "DELETE");
        assertResponse(HTTP_UNAVAILABLE);

        // VLAN Mapping APIs.
        String baseURLVmap = baseURL + "/vlanmaps";
        result = getJsonResult(baseURLVmap);
        assertResponse(HTTP_UNAVAILABLE);

        requestBody = "{\"vlan\":\"" + vlan
                + "\",\"node\":{\"type\":\"" + nodeType
                + "\",\"id\":\"" + nodeid + "\"}}";
        result = getJsonResult(baseURLVmap, "POST", requestBody);
        assertResponse(HTTP_UNAVAILABLE);

        String loc = baseURLVmap + "/" + nodeType + "-" + nodeid + "." + vlan;

        result = getJsonResult(loc);
        assertResponse(HTTP_UNAVAILABLE);

        result = getJsonResult(loc, "DELETE");
        assertResponse(HTTP_UNAVAILABLE);

        // MAC mapping APIs.
        String mmapURL = baseURL + "/macmap";
        String macaddr = "00:00:00:00:00:01";
        getJsonResult(mmapURL);
        assertResponse(HTTP_UNAVAILABLE);
        for (String type: new String[]{"/allow", "/deny", "/mapped"}) {
            getJsonResult(mmapURL + type);
            assertResponse(HTTP_UNAVAILABLE);
        }
        String mmapAllowURL = mmapURL + "/allow/" + macaddr + "/0";
        for (String method: new String[]{"GET", "PUT", "DELETE"}) {
            getJsonResult(mmapAllowURL, method);
            assertResponse(HTTP_UNAVAILABLE);
        }

        // Mac Address Table APIs.
        String baseURLMac = baseURL + "/mac";

        result = getJsonResult(baseURL + bname + "/mac/" + macaddr);
        assertResponse(HTTP_UNAVAILABLE);

        result = getJsonResult(baseURLMac + "/" + macaddr, "DELETE");
        assertResponse(HTTP_UNAVAILABLE);

        result = getJsonResult(baseURLMac, "DELETE");
        assertResponse(HTTP_UNAVAILABLE);
    }

    /**
     * Configure the OSGi container
     */
    @Configuration
    public Option[] config() {
        // Create configuration directory.
        File confdir = new File(GlobalConstants.STARTUPHOME.toString());
        if (confdir.exists()) {
            delete(confdir);
        }
        confdir.mkdirs();

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
                mavenBundle("org.opendaylight.controller", "networkconfig.neutron").versionAsInProject(),

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

                // VTN Manager bundles
                mavenBundle("org.opendaylight.vtn", "manager").versionAsInProject(),
                mavenBundle("org.opendaylight.vtn", "manager.northbound").versionAsInProject(),
                mavenBundle("org.opendaylight.vtn", "manager.neutron").versionAsInProject(),
                mavenBundle("org.opendaylight.vtn", "manager.implementation").versionAsInProject(),

                mavenBundle("com.fasterxml.jackson.core", "jackson-annotations").versionAsInProject(),
                mavenBundle("com.fasterxml.jackson.core", "jackson-core").versionAsInProject(),
                mavenBundle("com.fasterxml.jackson.core", "jackson-databind").versionAsInProject(),
                mavenBundle("com.fasterxml.jackson.jaxrs", "jackson-jaxrs-json-provider").versionAsInProject(),
                mavenBundle("com.fasterxml.jackson.jaxrs", "jackson-jaxrs-base").versionAsInProject(),
                mavenBundle("com.fasterxml.jackson.module", "jackson-module-jaxb-annotations").versionAsInProject(),
                mavenBundle("org.codehaus.jettison", "jettison").versionAsInProject(),

                mavenBundle("commons-io", "commons-io").versionAsInProject(),

                mavenBundle("commons-fileupload", "commons-fileupload").versionAsInProject(),
                mavenBundle("commons-net", "commons-net").versionAsInProject(),

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
                //OVSDB Bundles
                mavenBundle("org.opendaylight.ovsdb", "library").versionAsInProject(),
                mavenBundle("org.opendaylight.ovsdb", "plugin").versionAsInProject(),
                mavenBundle("org.opendaylight.ovsdb", "schema.openvswitch").versionAsInProject(),
                mavenBundle("org.opendaylight.ovsdb", "schema.hardwarevtep").versionAsInProject(),

                //List needed by OVSDB modules
                mavenBundle("org.opendaylight.controller", "sal.networkconfiguration").versionAsInProject(),
                mavenBundle("org.opendaylight.controller", "sal.networkconfiguration.implementation").versionAsInProject(),
                mavenBundle("org.mockito", "mockito-all").versionAsInProject(),
                mavenBundle("com.google.guava", "guava").versionAsInProject(),
                mavenBundle("io.netty", "netty-buffer").versionAsInProject(),
                mavenBundle("io.netty", "netty-common").versionAsInProject(),
                mavenBundle("io.netty", "netty-codec").versionAsInProject(),
                mavenBundle("io.netty", "netty-transport").versionAsInProject(),
                mavenBundle("io.netty", "netty-handler").versionAsInProject(),
                mavenBundle("com.google.code.gson", "gson").versionAsInProject(),

                // Jersey needs to be started before the northbound application
                // bundles, using a lower start level
                mavenBundle("com.sun.jersey", "jersey-client").versionAsInProject(),
                mavenBundle("com.sun.jersey", "jersey-server").versionAsInProject().startLevel(2),
                mavenBundle("com.sun.jersey", "jersey-core").versionAsInProject().startLevel(2),

                junitBundles());
    }

    /**
     * Start the manager.implementation bundle.
     */
    private void startVTN() {
        for (int i = 0; i < 5; i++) {
            try {
                implBundle.start();
            } catch (Exception e) {
                unexpected(e);
            }
            if (implBundle.getState() == Bundle.ACTIVE) {
                return;
            }
            try {
                Thread.sleep(100L);
            } catch (Exception e) {
            }
        }
    }

    /**
     * Test case for flow filter APIs.
     */
    @Test
    public void testFlowFilterAPI() throws JSONException {
        String tname = "vtn_1";
        String bname = "node_1";
        String iname = "if_1";

        // Create a VTN.
        String empty = "{}";
        String tenantUri = createURI("default", RES_VTNS, tname);
        getJsonResult(tenantUri, "POST", empty);
        assertResponse(HTTP_CREATED);
        Assert.assertEquals(tenantUri, httpLocation);

        // Create a vBridge.
        String bridgeUri = createRelativeURI(tenantUri, RES_VBRIDGES, bname);
        getJsonResult(bridgeUri, "POST", empty);
        assertResponse(HTTP_CREATED);
        Assert.assertEquals(bridgeUri, httpLocation);

        // Create a vBridge interface.
        String bridgeIfUri = createRelativeURI(bridgeUri, RES_INTERFACES,
                                               iname);
        getJsonResult(bridgeIfUri, "POST", empty);
        assertResponse(HTTP_CREATED);
        Assert.assertEquals(bridgeIfUri, httpLocation);

        // Create a vTerminal.
        String vtermUri = createRelativeURI(tenantUri, RES_VTERMINALS, bname);
        getJsonResult(vtermUri, "POST", empty);
        assertResponse(HTTP_CREATED);
        Assert.assertEquals(vtermUri, httpLocation);

        // Create a vTerminal interface.
        String vtermIfUri = createRelativeURI(vtermUri, RES_INTERFACES, iname);
        getJsonResult(vtermIfUri, "POST", empty);
        assertResponse(HTTP_CREATED);
        Assert.assertEquals(vtermIfUri, httpLocation);

        // Test VTN flow filter APIs.
        HashMap<String, JSONObject> filters =
            new HashMap<String, JSONObject>();

        String vtnFilterUri = createRelativeURI(tenantUri, RES_FLOWFILTERS);
        filters.put(vtnFilterUri, testFlowFilterAPI(vtnFilterUri, 0, null));

        String vbrInFilterUri = createRelativeURI(bridgeUri, RES_FLOWFILTERS,
                                                  "in");
        filters.put(vbrInFilterUri,
                    testFlowFilterAPI(vbrInFilterUri, 100, null));

        String vbrOutFilterUri = createRelativeURI(bridgeUri, RES_FLOWFILTERS,
                                                   "out");
        filters.put(vbrOutFilterUri,
                    testFlowFilterAPI(vbrOutFilterUri, 200, null));

        String vbrIfInFilterUri = createRelativeURI(bridgeIfUri,
                                                    RES_FLOWFILTERS, "IN");
        VBridgeIfPath bridgePath = new VBridgeIfPath(tname, bname, iname);
        filters.put(vbrIfInFilterUri,
                    testFlowFilterAPI(vbrIfInFilterUri, 3333, bridgePath));

        String vbrIfOutFilterUri = createRelativeURI(bridgeIfUri,
                                                     RES_FLOWFILTERS, "OUT");
        filters.put(vbrIfOutFilterUri,
                    testFlowFilterAPI(vbrIfOutFilterUri, 456, bridgePath));

        String vtmIfInFilterUri = createRelativeURI(vtermIfUri,
                                                    RES_FLOWFILTERS, "In");
        VTerminalIfPath termPath = new VTerminalIfPath(tname, bname, iname);
        filters.put(vtmIfInFilterUri,
                    testFlowFilterAPI(vtmIfInFilterUri, 77777, termPath));

        String vtmIfOutFilterUri = createRelativeURI(vtermIfUri,
                                                     RES_FLOWFILTERS, "Out");
        filters.put(vtmIfOutFilterUri,
                    testFlowFilterAPI(vtmIfOutFilterUri, 890, termPath));

        // NOT_FOUND tests.
        String badContainer = createURI("container_1", RES_VTNS, tname);
        String badTenant = createURI("default", RES_VTNS, "vtn_99999");
        String badBridge = createURI("default", RES_VTNS, tname, RES_VBRIDGES,
                                     "vbr_10000");
        String badBridgeIf = createURI("default", RES_VTNS, tname,
                                       RES_VBRIDGES, bname,
                                       RES_INTERFACES, "if_999999");
        String badTerminal = createURI("default", RES_VTNS, tname,
                                       RES_VTERMINALS, "vtm_10000");
        String badTermIf = createURI("default", RES_VTNS, tname,
                                     RES_VTERMINALS, bname,
                                     RES_INTERFACES, "if_999999");
        String[] invalid = {
            // Specify container that does not exist.
            createRelativeURI(badContainer, RES_FLOWFILTERS),
            createRelativeURI(badContainer, RES_VBRIDGES, bname,
                              RES_FLOWFILTERS, "in"),
            createRelativeURI(badContainer, RES_VBRIDGES, bname,
                              RES_FLOWFILTERS, "out"),
            createRelativeURI(badContainer, RES_VBRIDGES, bname,
                              RES_INTERFACES, iname, RES_FLOWFILTERS, "in"),
            createRelativeURI(badContainer, RES_VBRIDGES, bname,
                              RES_INTERFACES, iname, RES_FLOWFILTERS, "out"),
            createRelativeURI(badContainer, RES_VTERMINALS, bname,
                              RES_INTERFACES, iname, RES_FLOWFILTERS, "in"),
            createRelativeURI(badContainer, RES_VTERMINALS, bname,
                              RES_INTERFACES, iname, RES_FLOWFILTERS, "in"),
            createRelativeURI(badContainer, RES_VTERMINALS, bname,
                              RES_INTERFACES, iname, RES_FLOWFILTERS, "out"),

            // Specify VTN that does not exist.
            createRelativeURI(badTenant, RES_FLOWFILTERS),
            createRelativeURI(badTenant, RES_VBRIDGES, bname,
                              RES_FLOWFILTERS, "in"),
            createRelativeURI(badTenant, RES_VBRIDGES, bname,
                              RES_FLOWFILTERS, "out"),
            createRelativeURI(badTenant, RES_VBRIDGES, bname,
                              RES_INTERFACES, iname, RES_FLOWFILTERS, "in"),
            createRelativeURI(badTenant, RES_VBRIDGES, bname,
                              RES_INTERFACES, iname, RES_FLOWFILTERS, "out"),
            createRelativeURI(badTenant, RES_VTERMINALS, bname,
                              RES_INTERFACES, iname, RES_FLOWFILTERS, "in"),
            createRelativeURI(badTenant, RES_VTERMINALS, bname,
                              RES_INTERFACES, iname, RES_FLOWFILTERS, "out"),

            // Specify vBridge that does not exist.
            createRelativeURI(badBridge, RES_FLOWFILTERS, "in"),
            createRelativeURI(badBridge, RES_FLOWFILTERS, "out"),
            createRelativeURI(badBridge, RES_INTERFACES, iname,
                              RES_FLOWFILTERS, "in"),
            createRelativeURI(badBridge, RES_INTERFACES, iname,
                              RES_FLOWFILTERS, "out"),

            // Specify vBridge interface that does not exist.
            createRelativeURI(badBridgeIf, RES_FLOWFILTERS, "in"),
            createRelativeURI(badBridgeIf, RES_FLOWFILTERS, "out"),

            // Specify vTerminal that does not exist.
            createRelativeURI(badTerminal, RES_INTERFACES, iname,
                              RES_FLOWFILTERS, "in"),
            createRelativeURI(badTerminal, RES_INTERFACES, iname,
                              RES_FLOWFILTERS, "out"),

            // Specify vTerminal interface that does not exist.
            createRelativeURI(badTermIf, RES_FLOWFILTERS, "in"),
            createRelativeURI(badTermIf, RES_FLOWFILTERS, "out"),
        };

        JSONObject body = new JSONObject().
            put("condition", "condition_1").
            put("filterType",
                new JSONObject().put("pass", new JSONObject()));
        for (String base: invalid) {
            getJsonResult(base);
            assertResponse(HTTP_NOT_FOUND);
            getJsonResult(base, "DELETE");
            assertResponse(HTTP_NOT_FOUND);

            String uri = createRelativeURI(base, "1");
            getJsonResult(uri);
            assertResponse(HTTP_NOT_FOUND);
            getJsonResult(uri, "DELETE");
            assertResponse(HTTP_NOT_FOUND);

            getJsonResult(uri, "PUT", body.toString());
            assertResponse(HTTP_NOT_FOUND);
        }

        for (Map.Entry<String, JSONObject> entry: filters.entrySet()) {
            String uri = entry.getKey();
            JSONObject expected = entry.getValue();

            // Get all flow filters.
            JSONObject json = getJSONObject(uri);
            assertEquals(expected, json);

            // Delete all flow filters.
            getJsonResult(uri, "DELETE");
            assertResponse(HTTP_OK);
            json = getJSONObject(uri);
            JSONArray array = json.getJSONArray("flowfilter");
            Assert.assertEquals(0, array.length());

            getJsonResult(uri, "DELETE");
            assertResponse(HTTP_NO_CONTENT);
        }

        // Restore all filters.
        for (Map.Entry<String, JSONObject> entry: filters.entrySet()) {
            String base = entry.getKey();
            JSONObject list = entry.getValue();
            JSONArray array = list.getJSONArray("flowfilter");
            int len = array.length();
            for (int i = 0; i < len; i++) {
                body = array.getJSONObject(i);
                String idx = body.getString("index");
                String uri = createRelativeURI(base, idx);
                getJsonResult(uri, "PUT", body.toString());
                assertResponse(HTTP_CREATED);
                Assert.assertEquals(uri, httpLocation);
            }

            JSONObject json = getJSONObject(base);
            assertEquals(list, json);
        }

        // Destroy VTN.
        getJsonResult(tenantUri, "DELETE");
        assertResponse(HTTP_OK);
    }

    /**
     * Test flow filter APIs.
     *
     * @param baseUri  Absolute URI for test.
     * @param cookie   An arbitrary integer to create test data.
     * @param ifPath   The location of the virtual interface to configure
     *                 flow filter. {@code null} means that the target virtual
     *                 node is not a virtual interface.
     * @return  A {@link JSONObject} instance that contains all flow filters
     *          configured into the URI specified by {@code base}.
     * @throws JSONException  An error occurred.
     */
    private JSONObject testFlowFilterAPI(String baseUri, int cookie,
                                         VInterfacePath ifPath)
        throws JSONException {
        LOG.info("Starting flow filter JAX-RS client: {}", baseUri);

        // Get all flow filters.
        JSONObject json = getJSONObject(baseUri);
        JSONArray array = json.getJSONArray("flowfilter");
        Assert.assertEquals(0, array.length());

        TreeMap<Integer, JSONObject> allFilters =
            new TreeMap<Integer, JSONObject>();

        // Create PASS flow filter with all supported actions.
        byte[] macAddr1 = {
            0x00, 0x11, 0x22, 0x33, 0x44, (byte)cookie,
        };
        byte[] macAddr2 = {
            (byte)0xf0, (byte)0xfa, (byte)0xfb,
            (byte)0xfc, (byte)cookie, (byte)0xfe,
        };
        JSONArray actions = new JSONArray().
            put(createJSONObject("dlsrc", "address",
                                 HexEncode.bytesToHexStringFormat(macAddr1))).
            put(createJSONObject("dldst", "address",
                                 HexEncode.bytesToHexStringFormat(macAddr2))).
            put(createJSONObject("vlanpcp", "priority",
                                 cookie & MASK_VLAN_PCP)).
            put(createJSONObject("inet4src", "address", "192.168.20.254")).
            put(createJSONObject("inet4dst", "address", "10.20.30.40")).
            put(createJSONObject("dscp", "dscp", cookie & MASK_IP_DSCP)).
            put(createJSONObject("icmptype", "type", cookie & MASK_ICMP)).
            put(createJSONObject("icmpcode", "code", ~cookie & MASK_ICMP));

        JSONObject empty = new JSONObject();
        JSONObject type = new JSONObject().put("pass", empty);
        JSONObject pass = new JSONObject().
            put("condition", "cond_1").
            put("filterType", type).
            put("actions", actions);
        int passIdx = (cookie + 1) & MASK_FLOWFILTER_INDEX;

        // Try to get flow filter that does not yet created.
        String uri = createRelativeURI(baseUri, String.valueOf(passIdx));
        getJsonResult(uri);
        assertResponse(HTTP_NO_CONTENT);

        // Create PASS filter.
        getJsonResult(uri, "PUT", pass.toString());
        assertResponse(HTTP_CREATED);
        Assert.assertEquals(uri, httpLocation);

        json = getJSONObject(uri);
        pass.put("index", passIdx);
        assertEquals(pass, json);
        allFilters.put(passIdx, json);

        getJsonResult(uri, "PUT", pass.toString());
        assertResponse(HTTP_NO_CONTENT);

        // Create one more PASS filter with 3 actions.
        byte[] macAddr3 = {
            (byte)0xa0, (byte)0xb0, (byte)cookie,
            (byte)0xd0, (byte)0xe0, (byte)0xf0,
        };
        type = new JSONObject().put("pass", empty);
        actions = new JSONArray().
            put(createJSONObject("dlsrc", "address",
                                 HexEncode.bytesToHexStringFormat(macAddr3))).
            put(createJSONObject("vlanpcp", "priority",
                                 (cookie + 13) & MASK_VLAN_PCP)).
            put(createJSONObject("icmptype", "type",
                                 (cookie - 31) & MASK_ICMP));
        JSONObject pass1 = new JSONObject().
            put("condition", "cond_2").
            put("filterType", type).
            put("actions", actions);
        int passIdx1 = (cookie + 7777) & MASK_FLOWFILTER_INDEX;

        String pass1Uri = createRelativeURI(baseUri, String.valueOf(passIdx1));
        getJsonResult(pass1Uri, "PUT", pass1.toString());
        assertResponse(HTTP_CREATED);
        Assert.assertEquals(pass1Uri, httpLocation);

        json = getJSONObject(pass1Uri);
        pass1.put("index", passIdx1);
        assertEquals(pass1, json);

        // Create DROP filter without actions.
        type = new JSONObject().put("drop", empty);

        // Index in JSON object should be ignored.
        JSONObject drop = new JSONObject().
            put("index", Integer.MAX_VALUE).
            put("condition", "cond_3").
            put("filterType", type);
        int dropIdx = (cookie + 65535) & MASK_FLOWFILTER_INDEX;

        uri = createRelativeURI(baseUri, String.valueOf(dropIdx));
        getJsonResult(uri, "PUT", drop.toString());
        assertResponse(HTTP_CREATED);
        Assert.assertEquals(uri, httpLocation);

        json = getJSONObject(uri);
        drop.put("index", dropIdx);
        assertEquals(drop, json);

        getJsonResult(uri, "PUT", drop.toString());
        assertResponse(HTTP_NO_CONTENT);

        // Append 2 actions into the DROP filter.
        byte[] macAddr4 = {
            (byte)0x00, (byte)0xaa, (byte)0xbb,
            (byte)cookie, (byte)0xdd, (byte)0xee,
        };
        actions = new JSONArray().
            put(createJSONObject("dscp", "dscp",
                                 (cookie * 7) & MASK_IP_DSCP)).
            put(createJSONObject("dldst", "address",
                                 HexEncode.bytesToHexStringFormat(macAddr4)));
        drop.put("actions", actions);

        getJsonResult(uri, "PUT", drop.toString());
        assertResponse(HTTP_OK);
        json = getJSONObject(uri);
        assertEquals(drop, json);
        allFilters.put(dropIdx, json);

        // Create REDIRECT filter with specifying one action.
        // VTN name in destination should be always ignored.
        JSONObject destination = new JSONObject().
            put("tenant", "vtn_100").
            put("bridge", "bridge_10").
            put("interface", "if_20");
        type = createJSONObject("redirect", "destination", destination,
                                "output", true);
        actions = new JSONArray().
            put(createJSONObject("vlanpcp", "priority",
                                 (cookie * 3) & MASK_VLAN_PCP));
        JSONObject redirect = new JSONObject().
            put("condition", "cond_" + cookie).
            put("filterType", type).
            put("actions", actions);
        int redirectIdx = (cookie + 5000) & MASK_FLOWFILTER_INDEX;

        uri = createRelativeURI(baseUri, String.valueOf(redirectIdx));
        getJsonResult(uri, "PUT", redirect.toString());
        assertResponse(HTTP_CREATED);
        Assert.assertEquals(uri, httpLocation);

        json = getJSONObject(uri);
        redirect.put("index", redirectIdx);
        destination.remove("tenant");
        assertEquals(redirect, json);

        getJsonResult(uri, "PUT", redirect.toString());
        assertResponse(HTTP_NO_CONTENT);

        // Update destination of the REDIRECT filter.
        destination = new JSONObject().
            put("terminal", "term_123").
            put("interface", "if_1");
        type = createJSONObject("redirect", "destination", destination,
                                "output", false);
        redirect = new JSONObject().
            put("condition", "cond_" + cookie).
            put("filterType", type).
            put("actions", actions);

        getJsonResult(uri, "PUT", redirect.toString());
        assertResponse(HTTP_OK);

        json = getJSONObject(uri);
        redirect.put("index", redirectIdx);
        assertEquals(redirect, json);
        allFilters.put(redirectIdx, json);

        getJsonResult(uri, "PUT", redirect.toString());
        assertResponse(HTTP_NO_CONTENT);

        // BAD_REQUEST tests.

        // Specify invalid index.
        int[] badIndex = {-1, 0, 65536, 65537, 100000};
        for (int idx: badIndex) {
            uri = createRelativeURI(baseUri, String.valueOf(idx));
            getJsonResult(uri, "PUT", pass.toString());
            assertResponse(HTTP_BAD_REQUEST);

            getJsonResult(uri);
            assertResponse(HTTP_NO_CONTENT);
        }

        // No flow condition name.
        JSONObject bad = new JSONObject().
            put("filterType", new JSONObject().put("pass", empty));
        getJsonResult(pass1Uri, "PUT", bad.toString());
        assertResponse(HTTP_BAD_REQUEST);

        // No flow filter type.
        bad = new JSONObject().put("condition", "cond_1");
        getJsonResult(pass1Uri, "PUT", bad.toString());
        assertResponse(HTTP_BAD_REQUEST);

        // Specify invalid action.
        JSONObject[] badActions = {
            // Bad action parameter.
            createJSONObject("dlsrc", "address", "bad_MAC_address"),
            createJSONObject("dlsrc", "address", "00:00:00:00:00:00"),
            createJSONObject("dldst", "address", "01:00:00:00:00:00"),
            createJSONObject("dldst", "address", "ff:ff:ff:ff:ff:ff"),
            createJSONObject("vlanpcp", "priority", 8),
            createJSONObject("vlanpcp", "priority", -1),
            createJSONObject("inet4src", "address", "bad_ip_address"),
            createJSONObject("inet4src", "address", "100.200.300.400"),
            createJSONObject("inet4src", "address", "::1"),
            createJSONObject("inet4dst", "address", "bad_ip_address"),
            createJSONObject("inet4dst", "address", "100.200.1.256"),
            createJSONObject("inet4dst", "address", "2400:683c:af13:801::2034"),
            createJSONObject("dscp", "dscp", 64),
            createJSONObject("dscp", "dscp", -1),
            createJSONObject("tpsrc", "port", -1),
            createJSONObject("tpsrc", "port", 65536),
            createJSONObject("tpdst", "port", -1),
            createJSONObject("tpdst", "port", 65536),
            createJSONObject("icmptype", "type", -1),
            createJSONObject("icmptype", "type", 256),
            createJSONObject("icmpcode", "code", -1),
            createJSONObject("icmpcode", "code", 256),
        };

        bad.put("filterType", new JSONObject().put("pass", empty));
        for (JSONObject act: badActions) {
            actions = new JSONArray().put(act);
            bad.put("actions", actions);
            getJsonResult(pass1Uri, "PUT", bad.toString());
            assertResponse(HTTP_BAD_REQUEST);
        }

        // Invalid destination of REDIRECT filter.
        String badName = "_badname";
        String emptyName = "";
        String longName = "12345678901234567890123456789012";
        List<JSONObject> badDestinations = new ArrayList<JSONObject>();
        badDestinations.add(null);
        badDestinations.add(empty);

        // No interface name.
        badDestinations.add(new JSONObject().put("bridge", "bridge_1"));
        badDestinations.add(new JSONObject().put("terminal", "vterm_1"));

        // Invalid node name.
        badDestinations.add(new JSONObject().put("bridge", badName).
                            put("interface", "if_1"));
        badDestinations.add(new JSONObject().put("bridge", emptyName).
                            put("interface", "if_1"));
        badDestinations.add(new JSONObject().put("terminal", longName).
                            put("interface", "if_1"));

        // Invalid interface name.
        badDestinations.add(new JSONObject().put("bridge", "bridge_1").
                            put("interface", badName));
        badDestinations.add(new JSONObject().put("bridge", "bridge_1").
                            put("interface", emptyName));
        badDestinations.add(new JSONObject().put("bridge", "bridge_1").
                            put("interface", longName));
        badDestinations.add(new JSONObject().put("terminal", "vterm_1").
                            put("interface", badName));
        badDestinations.add(new JSONObject().put("terminal", "vterm_1").
                            put("interface", emptyName));
        badDestinations.add(new JSONObject().put("terminal", "vterm_1").
                            put("interface", longName));

        if (ifPath != null) {
            // Self redirection.
            String name = ifPath.getTenantNodeName();
            String ifName = ifPath.getInterfaceName();
            String key = (ifPath instanceof VBridgeIfPath)
                ? "bridge" : "terminal";
            badDestinations.add(new JSONObject().put(key, name).
                                put("interface", ifName));
        }

        for (JSONObject dest: badDestinations) {
            JSONObject badType = createJSONObject("redirect",
                                                  "destination", dest,
                                                  "output", false);
            bad = new JSONObject().
                put("condition", "cond_" + cookie).
                put("filterType", badType);
            getJsonResult(pass1Uri, "PUT", bad.toString());
            assertResponse(HTTP_BAD_REQUEST);
        }

        // Ensure that PASS filter at index 7777 was not modified.
        json = getJSONObject(pass1Uri);
        assertEquals(pass1, json);

        // Remove actions in PASS filter at index 7777.
        pass1.remove("actions");
        getJsonResult(pass1Uri, "PUT", pass1.toString());
        assertResponse(HTTP_OK);

        json = getJSONObject(pass1Uri);
        assertEquals(pass1, json);

        // Delete PASS filter at index 7777.
        getJsonResult(pass1Uri, "DELETE");
        assertResponse(HTTP_OK);
        getJsonResult(pass1Uri, "DELETE");
        assertResponse(HTTP_NO_CONTENT);

        // Get all flow filters again.
        JSONObject all = getJSONObject(baseUri);
        array = new JSONArray(allFilters.values());
        JSONObject expected = new JSONObject().put("flowfilter", array);
        assertEquals(expected, all);

        return all;
    }
}
