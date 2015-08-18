/*
 * Copyright (c) 2013, 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.it.northbound;

import static java.net.HttpURLConnection.HTTP_BAD_METHOD;
import static java.net.HttpURLConnection.HTTP_BAD_REQUEST;
import static java.net.HttpURLConnection.HTTP_CONFLICT;
import static java.net.HttpURLConnection.HTTP_CREATED;
import static java.net.HttpURLConnection.HTTP_NOT_FOUND;
import static java.net.HttpURLConnection.HTTP_NO_CONTENT;
import static java.net.HttpURLConnection.HTTP_OK;
import static java.net.HttpURLConnection.HTTP_UNAUTHORIZED;
import static java.net.HttpURLConnection.HTTP_UNSUPPORTED_TYPE;

import static org.hamcrest.CoreMatchers.is;

import static org.ops4j.pax.exam.CoreOptions.options;

import java.io.BufferedReader;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;
import java.net.HttpURLConnection;
import java.net.InetAddress;
import java.net.URL;
import java.nio.charset.Charset;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.Deque;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.TreeMap;
import java.util.concurrent.TimeUnit;

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
import org.ops4j.pax.exam.junit.PaxExam;
import org.ops4j.pax.exam.spi.reactors.ExamReactorStrategy;
import org.ops4j.pax.exam.spi.reactors.PerClass;
import org.ops4j.pax.exam.util.Filter;

import org.osgi.framework.Bundle;
import org.osgi.framework.BundleContext;
import org.osgi.framework.Version;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import org.opendaylight.vtn.manager.IVTNManager;
import org.opendaylight.vtn.manager.MacAddressEntry;
import org.opendaylight.vtn.manager.SwitchPort;
import org.opendaylight.vtn.manager.VBridgeIfPath;
import org.opendaylight.vtn.manager.VBridgePath;
import org.opendaylight.vtn.manager.VInterfacePath;
import org.opendaylight.vtn.manager.VTerminalIfPath;
import org.opendaylight.vtn.manager.util.ByteUtils;
import org.opendaylight.vtn.manager.util.EtherAddress;
import org.opendaylight.vtn.manager.util.Ip4Network;

import org.opendaylight.vtn.manager.it.ofmock.OfMockService;
import org.opendaylight.vtn.manager.it.option.TestOption;
import org.opendaylight.vtn.manager.it.util.BridgeNetwork;
import org.opendaylight.vtn.manager.it.util.TestBase;
import org.opendaylight.vtn.manager.it.util.TestHost;

import org.opendaylight.controller.sal.core.Node;
import org.opendaylight.controller.sal.core.NodeConnector;
import org.opendaylight.controller.sal.packet.address.DataLinkAddress;
import org.opendaylight.controller.sal.packet.address.EthernetAddress;
import org.opendaylight.controller.sal.utils.EtherTypes;
import org.opendaylight.controller.sal.utils.GlobalConstants;
import org.opendaylight.controller.sal.utils.IPProtocols;
import org.opendaylight.controller.usermanager.IUserManager;

@RunWith(PaxExam.class)
@ExamReactorStrategy(PerClass.class)
public final class VtnNorthboundIT extends TestBase {
    /**
     * Logger instance.
     */
    private static final Logger LOG = LoggerFactory.
        getLogger(VtnNorthboundIT.class);

    /**
     * HTTP GET method.
     */
    private static final String  HTTP_GET = "GET";

    /**
     * HTTP PUT method.
     */
    private static final String  HTTP_PUT = "PUT";

    /**
     * HTTP POST method.
     */
    private static final String  HTTP_POST = "POST";

    /**
     * HTTP DELETE method.
     */
    private static final String  HTTP_DELETE = "DELETE";

    /**
     * Base URI of VTN Manager's REST APIs.
     */
    private static final String VTN_BASE_URL =
        "http://127.0.0.1:8288/controller/nb/v2/vtn/";

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
     * Resource name for path policy APIs.
     */
    private static final String RES_PATHPOLICIES = "pathpolicies";

    /**
     * Resource name for path map APIs.
     */
    private static final String RES_PATHMAPS = "pathmaps";

    /**
     * Resource name for flow condition APIs.
     */
    private static final String RES_FLOWCONDITIONS = "flowconditions";

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

    /**
     * The minimum value of path policy identifier.
     */
    private static final int  PATH_POLICY_MIN = 1;

    /**
     * The maximum value of path policy identifier.
     */
    private static final int  PATH_POLICY_MAX = 3;

    /**
     * A list of invalid flow matches.
     */
    private static final List<JSONObject>  INVALID_MATCHES;

    /**
     * An array of invalid virtual node names.
     */
    private static final String[]  INVALID_NAMES = {
        "12345678901234567890123456789012",
        "abcABC_0123_XXXXXXXXXXXXXXXXXXXX",
        "_flow_cond",
        "flow-cond",
        "flow%25cond",
        "%3Bflowcond",
        "%26flowcond",
        "_",
        "%20",
        "%e3%80%80",
    };

    // Inject the OSGI bundle context
    @Inject
    private BundleContext  bundleContext;

    // Inject controller services.
    @Inject
    @Filter(timeout = OSGI_TIMEOUT)
    private IUserManager userManager;

    // Inject VTN Manager services.
    @Inject
    @Filter(timeout = OSGI_TIMEOUT)
    private IVTNManager vtnManager;

    @Inject
    @Filter(timeout = OSGI_TIMEOUT)
    private OfMockService  ofMockService;

    private Bundle  implBundle;

    /**
     * JSON comparator for the current test.
     */
    private final JSONComparator  jsonComparator = new JSONComparator();

    /**
     * Initialize static fields.
     */
    static {
        // Construct a list of invalid flow matches.
        List<JSONObject> matches = new ArrayList<>();

        try {
            // Invalid MAC address.
            matches.add(new JSONObject().
                        put("index", 1).
                        put("ethernet", new JSONObject().
                            put("src", "bad MAC address")));
            matches.add(new JSONObject().
                        put("index", 1).
                        put("ethernet", new JSONObject().
                            put("dst", "abcdefg")));

            // Invalid Ethernet type.
            int[] badTypes = {
                Integer.MIN_VALUE, -10000, -2, -1, 65536, 65537,
                99999999, Integer.MAX_VALUE,
            };
            for (int type: badTypes) {
                matches.add(new JSONObject().
                            put("index", 1).
                            put("ethernet", new JSONObject().
                                put("type", type)));
            }

            // Invalid VLAN ID.
            short[] badVlanIds = {
                Short.MIN_VALUE, -10000, -2, -1,
                4096, 4097, 30000, Short.MAX_VALUE,
            };
            for (short vid: badVlanIds) {
                matches.add(new JSONObject().
                            put("index", 1).
                            put("ethernet", new JSONObject().
                                put("vlan", vid)));
            }

            // Invalid VLAN priority.
            byte[] badPcps = {
                Byte.MIN_VALUE, -99, -1, 8, 9, 100, Byte.MAX_VALUE,
            };
            for (byte pcp: badPcps) {
                matches.add(new JSONObject().
                            put("index", 1).
                            put("ethernet", new JSONObject().
                                put("vlan", 1).
                                put("vlanpri", pcp)));
            }

            // Specifying VLAN priority without VLAN ID.
            matches.add(new JSONObject().
                        put("index", 1).
                        put("ethernet", new JSONObject().
                            put("vlanpri", 0)));

            // Specifying VLAN priority for untagged frame.
            matches.add(new JSONObject().
                        put("index", 1).
                        put("ethernet", new JSONObject().
                            put("vlan", 0).
                            put("vlanpri", 0)));

            // Inconsistent Ethernet type.
            matches.add(new JSONObject().
                        put("index", 1).
                        put("ethernet", new JSONObject().
                            put("type", 0x806)).
                        put("inetMatch", new JSONObject().
                            put("inet4", new JSONObject())));
            matches.add(new JSONObject().
                        put("index", 1).
                        put("ethernet", new JSONObject().
                            put("type", 0x806)).
                        put("l4Match", new JSONObject().
                            put("tcp", new JSONObject())));

            // Invalid IPv4 address.
            matches.add(new JSONObject().
                        put("index", 1).
                        put("inetMatch", new JSONObject().
                            put("inet4", new JSONObject().
                                put("src", "BAD_IP_ADDRESS"))));
            matches.add(new JSONObject().
                        put("index", 1).
                        put("inetMatch", new JSONObject().
                            put("inet4", new JSONObject().
                                put("dst", "::1"))));

            // Invalid IPv4 prefix length.
            int[] badPrefix = {
                Integer.MIN_VALUE, -3, -1, 0, 33, 34, 19999, Integer.MAX_VALUE,
            };
            for (int len: badPrefix) {
                matches.add(new JSONObject().
                            put("index", 1).
                            put("inetMatch", new JSONObject().
                                put("inet4", new JSONObject().
                                    put("src", "192.168.100.1").
                                    put("srcsuffix", len))));
                matches.add(new JSONObject().
                            put("index", 1).
                            put("inetMatch", new JSONObject().
                                put("inet4", new JSONObject().
                                    put("dst", "10.20.30.40").
                                    put("dstsuffix", len))));
            }

            // Invalid IP protocol.
            short[] badProto = {
                Short.MIN_VALUE, -3, -2, -1, 256, 257, 20000, Short.MAX_VALUE,
            };
            for (short proto: badProto) {
                matches.add(new JSONObject().
                            put("index", 1).
                            put("inetMatch", new JSONObject().
                                put("inet4", new JSONObject().
                                    put("protocol", proto))));
            }

            // Invalid IP DSCP.
            byte[] badDscp = {
                Byte.MIN_VALUE, -100, -2, -1, 64, 65, 100, Byte.MAX_VALUE,
            };
            for (byte dscp: badDscp) {
                matches.add(new JSONObject().
                            put("index", 1).
                            put("inetMatch", new JSONObject().
                                put("inet4", new JSONObject().
                                    put("dscp", dscp))));
            }

            // Inconsistent IP protocol.
            matches.add(new JSONObject().
                        put("index", 1).
                        put("inetMatch", new JSONObject().
                            put("inet4", new JSONObject().
                                put("protocol", IPProtocols.ICMP.intValue()))).
                        put("l4Match", new JSONObject().
                            put("tcp", new JSONObject())));
            matches.add(new JSONObject().
                        put("index", 1).
                        put("inetMatch", new JSONObject().
                            put("inet4", new JSONObject().
                                put("protocol", IPProtocols.TCP.intValue()))).
                        put("l4Match", new JSONObject().
                            put("udp", new JSONObject())));
            matches.add(new JSONObject().
                        put("index", 1).
                        put("inetMatch", new JSONObject().
                            put("inet4", new JSONObject().
                                put("protocol", IPProtocols.UDP.intValue()))).
                        put("l4Match", new JSONObject().
                            put("icmp", new JSONObject())));

            int[] badPort = {
                Integer.MIN_VALUE, -30000, -2, -1,
                65536, 65537, 10000000, Integer.MAX_VALUE,
            };
            String[] portProto = {"tcp", "udp"};
            String[] portType = {"src", "dst"};
            for (String proto: portProto) {
                for (String type: portType) {
                    // Invalid port number.
                    for (int port: badPort) {
                        matches.add(new JSONObject().
                                    put("index", 1).
                                    put("l4Match", new JSONObject().
                                        put(proto, new JSONObject().
                                            put(type, new JSONObject().
                                                put("from", port)))));
                        matches.add(new JSONObject().
                                    put("index", 1).
                                    put("l4Match", new JSONObject().
                                        put(proto, new JSONObject().
                                            put(type, new JSONObject().
                                                put("from", 1).
                                                put("to", port)))));
                    }

                    // "from" is not specified.
                    matches.add(new JSONObject().
                                put("index", 1).
                                put("l4Match", new JSONObject().
                                    put(proto, new JSONObject().
                                        put(type, new JSONObject()))));
                    matches.add(new JSONObject().
                                put("index", 1).
                                put("l4Match", new JSONObject().
                                    put(proto, new JSONObject().
                                        put(type, new JSONObject().
                                            put("to", 100)))));

                    // Invalid port range.
                    matches.add(new JSONObject().
                                put("index", 1).
                                put("l4Match", new JSONObject().
                                    put(proto, new JSONObject().
                                        put(type, new JSONObject().
                                            put("from", 101).
                                            put("to", 100)))));
                }
            }
        } catch (Exception e) {
            throw new IllegalStateException(
                "Failed to initialize invalid flow match.", e);
        }

        INVALID_MATCHES = Collections.unmodifiableList(matches);
    }

    /**
     * Configure the OSGi container
     */
    @Configuration
    public Option[] config() {
        return options(TestOption.vtnManagerNorthboundBundles());
    }

    /**
     * Check each bundles are ready or not before start each test methods .
     *
     * @throws Exception  An error occurred.
     */
    @Before
    public void areWeReady() throws Exception {
        if (implBundle == null) {
            assertNotNull(bundleContext);
            assertNotNull(userManager);
            assertNotNull(vtnManager);
            assertNotNull(ofMockService);

            // Determine manager.implementation bundle.
            implBundle = getManagerBundle(bundleContext);
            assertNotNull(implBundle);
            assertEquals(Bundle.ACTIVE, implBundle.getState());

            // Initialize the openflowplugin mock-up.
            ofMockService.initialize();
        }

        jsonComparator.reset();
    }

    /**
     * Called when a test suite quits.
     *
     * @throws Exception  An error occurred.
     */
    @After
    public void tearDown() throws Exception {
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
            JSONArray array = json.getJSONArray("vtn");
            for (int i = 0; i < array.length(); i++) {
                JSONObject vtn = array.getJSONObject(i);
                String name = vtn.getString("name");
                LOG.debug("Clean up VTN: {}", name);
                getJsonResult(uri + "/" + name, HTTP_DELETE);
                assertResponse(HTTP_OK);
            }

            // Remove all path policies.
            uri = createURI("default", RES_PATHPOLICIES);
            getJsonResult(uri, HTTP_DELETE);
            int resp = httpResponseCode.intValue();
            if (resp != HTTP_NO_CONTENT) {
                assertEquals(HTTP_OK, resp);
            }

            // Remove all flow conditions.
            uri = createURI("default", RES_FLOWCONDITIONS);
            getJsonResult(uri, HTTP_DELETE);
            resp = httpResponseCode.intValue();
            if (resp != HTTP_NO_CONTENT) {
                assertEquals(HTTP_OK, resp);
            }

            // Remove all global path maps.
            uri = createURI("default", RES_PATHMAPS);
            getJsonResult(uri, HTTP_DELETE);
            resp = httpResponseCode.intValue();
            if (resp != HTTP_NO_CONTENT) {
                assertEquals(HTTP_OK, resp);
            }
        } catch (Exception e) {
            unexpected(e);
        }

        // Reset the inventory configuration.
        ofMockService.reset();
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
        if (!jsonComparator.equals(expected, json)) {
            String msg = "JSONObject does not match: expected=<" + expected +
                ">, actual=<" + json + ">";
            fail(msg);
        }
    }

    /**
     * Verify that the given two JSON arrays are identical,
     *
     * <p>
     *   Note that this method ignores element order in the given arrays.
     * </p>
     *
     * @param expected  An expected value.
     * @param jarray    A {@link JSONArray} to be tested.
     * @throws JSONException  An error occurred.
     */
    private void assertEquals(JSONArray expected, JSONArray jarray)
        throws JSONException {
        if (!jsonComparator.equals(expected, jarray)) {
            String msg = "JSONArray does not match: expected=<" + expected +
                ">, actual=<" + jarray + ">";
            fail(msg);
        }
    }

    /**
     * Send request and get result
     *
     * @param restUrl   A request URL.
     * @return  A returned result for request.
     */
    private String getJsonResult(String restUrl) {
        return getJsonResult(restUrl, HTTP_GET, null);
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
            StringBuilder sb = new StringBuilder();
            if (is != null) {
                InputStreamReader in =
                    new InputStreamReader(is, Charset.forName("UTF-8"));
                BufferedReader rd = new BufferedReader(in);
                int cp;
                while ((cp = rd.read()) != -1) {
                    sb.append((char)cp);
                }
                is.close();
            }
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
     * Get a string associated with the given key in the JSON object.
     *
     * @param jobj  A JSON object.
     * @param key   A key string.
     * @return  A string associated with the given key or {@code null}.
     * @throws JSONException  An error occurred.
     */
    private String getJsonString(JSONObject jobj, String key)
        throws JSONException {
        return (jobj.has(key) && !jobj.isNull(key))
            ? jobj.getString(key) : null;
    }

    /**
     * Create a {@link JSONObject} instance which represents an OpenFlow node.
     *
     * @param dpid  The datapath ID.
     * @throws JSONException  An error occurred.
     */
    private JSONObject createNode(long dpid) throws JSONException {
        JSONObject json = new JSONObject();
        return json.put("type", "OF").
            put("id", toHexString(dpid));
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
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testVTNAPI() throws Exception {
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
        result = getJsonResult(requestUri, HTTP_POST, requestBody);
        assertResponse(HTTP_CREATED);
        Assert.assertEquals(requestUri, httpLocation);

        // Test POST vtn1, expecting 409
        requestUri = baseURL + "default/vtns/" + tname1;
        result = getJsonResult(requestUri, HTTP_POST, requestBody);
        assertResponse(HTTP_CONFLICT);

        // Test GET vtn in default container, expecting one result
        result = getJsonResult(baseURL + "default/vtns");
        assertResponse(HTTP_OK);
        jt = new JSONTokener(result);
        json = new JSONObject(jt);
        vtnArray = json.getJSONArray("vtn");
        Assert.assertEquals(1, vtnArray.length());

        // Test POST vtn2, setting "_" to vBridgeName
        requestBody = "{\"description\":\"" + desc1 +
            "\", \"idleTimeout\":\"" + itimeout1 + "\", \"hardTimeout\":\"" +
            htimeout1 + "\"}";
        requestUri = baseURL + "default/vtns/" + tname2;
        result = getJsonResult(requestUri, HTTP_POST, requestBody);
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
        result = getJsonResult(requestUri, HTTP_POST, requestBody);
        assertResponse(HTTP_CREATED);
        Assert.assertEquals(requestUri, httpLocation);

        // Test POST vtn4, setting idle timeout of negative value and one character numeric of vtn name
        requestBody = "{\"description\":\"" + desc3 +
            "\", \"idleTimeout\":\"" + timeoutNegative + "\"}";
        requestUri = baseURL + "default/vtns/" + tname4;
        result = getJsonResult(requestUri, HTTP_POST, requestBody);
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
        result = getJsonResult(requestUri, HTTP_POST, requestBody);
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
        result = getJsonResult(requestUri, HTTP_POST, requestBody);
        assertResponse(HTTP_CREATED);
        Assert.assertEquals(requestUri, httpLocation);

        // Test POST vtn7, setting idle timeout of 65535, hard timeout of 0
        // and vtn name of 31 characters
        requestBody = "{\"description\":\"" + desc2 +
            "\", \"idleTimeout\":\"" + timeout0 + "\", \"hardTimeout\":\"" +
            timeoutMax + "\"}";
        requestUri = baseURL + "default/vtns/" + tname7;
        result = getJsonResult(requestUri, HTTP_POST, requestBody);
        assertResponse(HTTP_CREATED);
        Assert.assertEquals(requestUri, httpLocation);

        // Test POST vtn8, setting invalid value after description to
        // requestBody
        requestBody = "{\"description\":\"" + desc1 +
            "\", \"Timeout\":\"" + timeout0 + "\", \"hard\":\"" + timeout0 +
            "\"}";
        requestUri = baseURL + "default/vtns/" + tname8;

        // Ensure that query parameters are eliminated from Location.
        result = getJsonResult(requestUri + "?param1=1&param2=2", HTTP_POST,
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
        result = getJsonResult(baseURL + "default/vtns/" + tname, HTTP_POST,
                               requestBody);
        assertResponse(HTTP_BAD_REQUEST);

        // Test POST vtn, expecting 400, setting invalid value to idle timeout
        requestBody = "{\"description\":\"" + desc1 +
            "\", \"idleTimeout\":\"idletimeout\", \"hardTimeout\":\"" +
            htimeout1 + "\"}";
        result = getJsonResult(baseURL + "default/vtns/" + tname, HTTP_POST,
                               requestBody);
        assertResponse(HTTP_BAD_REQUEST);

        // Test POST vtn, expecting 400, setting invalid value to hard timeout
        requestBody = "{\"description\":\"" + desc1 +
            "\", \"idleTimeout\":\"" + itimeout1 +
            "\", \"hardTimeout\":\"hardtimeout\"}";
        result = getJsonResult(baseURL + "default/vtns/" + tname, HTTP_POST,
                               requestBody);
        assertResponse(HTTP_BAD_REQUEST);

        // Test POST vtn, expecting 400, setting invalid value to requestBody
        requestBody = "{\"description\":\"" + desc3 +
            "\", \"didleTimeout\":\"rdTimeout\":\"}";
        result = getJsonResult(baseURL + "default/vtns/" + tname, HTTP_POST,
                               requestBody);
        assertResponse(HTTP_BAD_REQUEST);


        // Test POST vtn expecting 105, test when vtn name is ""
        requestBody = "{}";
        result = getJsonResult(baseURL + "default/vtns/" + "", HTTP_POST,
                               requestBody);
        assertResponse(HTTP_BAD_METHOD);

        // Test POST vtn expecting 400, specifying invalid tenant name.
        requestUri = baseURL + "default/vtns/" + desc3;
        result = getJsonResult(requestUri, HTTP_POST, requestBody);
        assertResponse(HTTP_BAD_REQUEST);

        // Test POST vtn expecting 400, specifying invalid tenant name
        // which starts with "_".
        requestUri = baseURL + "default/vtns/" + "_testVtn";
        result = getJsonResult(requestUri, HTTP_POST, requestBody);
        assertResponse(HTTP_BAD_REQUEST);

        // Test POST vtn expecting 400, specifying invalid tenant name
        // including symbol "@".
        requestUri = baseURL + "default/vtns/" + "test@Vtn";
        result = getJsonResult(requestUri, HTTP_POST, requestBody);
        assertResponse(HTTP_BAD_REQUEST);

        // Test POST vtn expecting 400, specifying 65536 as idle timeout.
        requestBody =  "{\"description\":\"" + desc1 +
            "\", \"idleTimeout\":\"65536\", \"hardTimeout\":\"" +
            htimeout1 + "\"}";
        requestUri = baseURL + "default/vtns/" + tname;
        result = getJsonResult(requestUri, HTTP_POST, requestBody);
        assertResponse(HTTP_BAD_REQUEST);

        // Test POST vtn expecting 400, specifying 65536 as hard timeout.
        requestBody =  "{\"description\":\"" + desc1 +
            "\", \"idleTimeout\":\"" + itimeout1 +
            "\", \"hardTimeout\":\"65536\"}";
        requestUri = baseURL + "default/vtns/" + tname;
        result = getJsonResult(requestUri, HTTP_POST, requestBody);
        assertResponse(HTTP_BAD_REQUEST);

        // Test POST vtn expecting 400, specifying idle timeout value greater
        // than hard timeout.
        requestBody =  "{\"description\":\"" + desc1 +
            "\", \"idleTimeout\":\"" + itimeout2 +
            "\", \"hardTimeout\":\"" + htimeout1 + "\"}";
        requestUri = baseURL + "default/vtns/" + tname;
        result = getJsonResult(requestUri, HTTP_POST, requestBody);
        assertResponse(HTTP_BAD_REQUEST);

        // Test POST vtn expecting 400, specifying too long tenant name.
        requestBody =  "{}";
        requestUri = baseURL + "default/vtns/" + tname32;
        result = getJsonResult(requestUri, HTTP_POST, requestBody);
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

        result = getJsonResult(baseURL + "default/vtns/" + tname1 +
                               queryParameter, HTTP_PUT, requestBody);
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
                               queryParameter, HTTP_PUT, requestBody);
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
                               queryParameter, HTTP_PUT, requestBody);
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
                               queryParameter, HTTP_PUT, requestBody);
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
                               queryParameter, HTTP_PUT, requestBody);
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
                               queryParameter, HTTP_PUT, requestBody);
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
                               queryParameter, HTTP_PUT, requestBody);
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
                               queryParameter, HTTP_PUT, requestBody);
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
                               queryParameter, HTTP_PUT, requestBody);
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
                               queryParameter, HTTP_PUT, requestBody);
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
                               queryParameter, HTTP_PUT, requestBody);
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
                               queryParameter, HTTP_PUT, requestBody);
        assertResponse(HTTP_BAD_REQUEST);

        // Test PUT vtn, expecting 400, setting the invalid value to hard timeout
        requestBody = "{\"idleTimeout\":\"" + itimeout1 +
            "\", \"hardTimeout\":\"" + desc1 + "\"}";
        result = getJsonResult(baseURL + "default/vtns/" + tname1 +
                               queryParameter, HTTP_PUT, requestBody);
        assertResponse(HTTP_BAD_REQUEST);

        // Test PUT vtn, expecting 400, setting invalid value to requestBody
        requestBody = "{\"description\":\"" + desc3 +
            "\", \"didleTimeout\":\"rdTimeout\":\"}";
        result = getJsonResult(baseURL + "default/vtns/" + tname1 +
                               queryParameter, HTTP_PUT, requestBody);
        assertResponse(HTTP_BAD_REQUEST);


        // Test PUT vtn expecting 404, setting the vtn that don't exist
        requestBody = "{\"description\":\"" + desc2 +
            "\", \"idleTimeout\":\"" + itimeout1 + "\", \"hardTimeout\":\"" +
            htimeout1 + "\"}";
        result = getJsonResult(baseURL + "default/vtns/" + tname +
                               queryParameter, HTTP_PUT, requestBody);
        assertResponse(HTTP_NOT_FOUND);

        // Test PUT vtn expecting 400, setting invalid value to requestBody
        requestBody = "{\"Test\"}";
        result = getJsonResult(baseURL + "default/vtns/" + tname1 +
                               queryParameter, HTTP_PUT, requestBody);
        assertResponse(HTTP_BAD_REQUEST);

        // Test PUT vtn expecting 400, specifying 65540 as idle timeout.
        requestBody = "{\"description\":\"" + desc2 +
            "\", \"idleTimeout\":\"" + timeoutOver +
            "\", \"hardTimeout\":\"" + htimeout2 + "\"}";
        result = getJsonResult(baseURL + "default/vtns/" + tname1 +
                               queryParameter, HTTP_PUT, requestBody);
        assertResponse(HTTP_BAD_REQUEST);

        // Test PUT vtn expecting 400, specifying 65540 as hard timeout.
        requestBody = "{\"description\":\"" + desc2 +
            "\", \"idleTimeout\":\"" + itimeout1 +
            "\", \"hardTimeout\":\"" + timeoutOver + "\"}";
        result = getJsonResult(baseURL + "default/vtns/" + tname1 +
                               queryParameter, HTTP_PUT, requestBody);
        assertResponse(HTTP_BAD_REQUEST);

        // Test PUT vtn expecting 400, specifying idle timeout value greater
        // than hard timeout.
        requestBody =  "{\"description\":\"" + desc1 +
            "\", \"idleTimeout\":\"" + itimeout2 +
            "\", \"hardTimeout\":\"" + htimeout1 + "\"}";
        result = getJsonResult(baseURL + "default/vtns/" + tname1 +
                               queryParameter, HTTP_PUT, requestBody);
        assertResponse(HTTP_BAD_REQUEST);

        // Test PUT vtn
        requestBody = "{\"description\":\"" + desc2 +
            "\", \"idleTimeout\":\"" + itimeout2 + "\", \"hardTimeout\":\"" +
            timeoutNegative + "\"}";
        result = getJsonResult(baseURL + "default/vtns/" + tname1 +
                               queryParameter, HTTP_PUT, requestBody);
        assertResponse(HTTP_OK);

        // Test GET all vtns in default container
        result = getJsonResult(baseURL + "default/vtns");
        jt = new JSONTokener(result);
        json = new JSONObject(jt);
        vtnArray = json.getJSONArray("vtn");

        assertResponse(HTTP_OK);
        Assert.assertEquals(8, vtnArray.length());

        // Test DELETE vtn expecting 404
        result = getJsonResult(baseURL + "default/vtns/" + tname, HTTP_DELETE);
        assertResponse(HTTP_NOT_FOUND);

        // set not supported "Content-type". expect to return 415.
        requestBody = "{}";
        result = getJsonResult(baseURL + "default/vtns/" + tname, HTTP_POST,
                               requestBody,
                               "text/plain");
        assertResponse(HTTP_UNSUPPORTED_TYPE);

        requestBody = "{\"description\":\"desc\"}";
        result = getJsonResult(baseURL + "default/vtns/" + tname1 +
                               queryParameter, HTTP_PUT, requestBody,
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
        result = getJsonResult(baseURL + "default/vtns/" + tname1, HTTP_DELETE);
        assertResponse(HTTP_OK);

        // Test DELETE vtn2
        result = getJsonResult(baseURL + "default/vtns/" + tname2, HTTP_DELETE);
        assertResponse(HTTP_OK);

        // Test DELETE vtn3
        result = getJsonResult(baseURL + "default/vtns/" + tname3, HTTP_DELETE);
        assertResponse(HTTP_OK);

        // Test DELETE vtn4
        result = getJsonResult(baseURL + "default/vtns/" + tname4, HTTP_DELETE);
        assertResponse(HTTP_OK);

        // Test DELETE vtn5
        result = getJsonResult(baseURL + "default/vtns/" + tname5, HTTP_DELETE);
        assertResponse(HTTP_OK);

        // Test DELETE vtn6
        result = getJsonResult(baseURL + "default/vtns/" + tname6, HTTP_DELETE);
        assertResponse(HTTP_OK);

        // Test DELETE vtn7
        result = getJsonResult(baseURL + "default/vtns/" + tname7, HTTP_DELETE);
        assertResponse(HTTP_OK);

        // Test DELETE vtn8
        result = getJsonResult(baseURL + "default/vtns/" + tname8, HTTP_DELETE);
        assertResponse(HTTP_OK);


        // Test DELETE vtn expecting 404
        result = getJsonResult(baseURL + "default/vtns/" + tname1, HTTP_DELETE);
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
        getJsonResult(base, HTTP_GET, null, ct,  auth);
        assertResponse(expected);

        String uri = base + "/" + tname;
        getJsonResult(uri, HTTP_POST, body, ct, auth);
        assertResponse(expected);

        getJsonResult(uri, HTTP_GET, null, ct, auth);
        assertResponse(expected);

        String qp = new QueryParameter("all", "true").toString();
        getJsonResult(uri + qp, HTTP_PUT, body, ct, auth);
        assertResponse(expected);

        getJsonResult(uri, HTTP_DELETE, null, ct, auth);
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
     * @throws Exception  An error occurred.
     */
    private void testVBridgeAPI(String tname1, String tname2)
        throws Exception {
        LOG.info("Starting vBridge JAX-RS client.");

        // A list of host entries for MAC mapping are unordered.
        jsonComparator.addUnordered("allow", "machost").
            addUnordered("deny", "machost").
            addUnordered("mapped", "machost").
            addUnordered("machost");

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
                               HTTP_POST , requestBody);
        assertResponse(HTTP_NOT_FOUND);

        // Test POST vBridge1 expecting 400, specifying too small ageInterval.
        requestBody = "{\"description\":\"" + desc1 +
            "\", \"ageInterval\":\"" + ageinter0 + "\"}";
        result = getJsonResult(baseURL + tname1 + "/vbridges/" + bname1 ,
                               HTTP_POST, requestBody);
        assertResponse(HTTP_BAD_REQUEST);

        // Test POST vBridge1
        requestBody = "{}";
        String requestUri = baseURL + tname1 + "/vbridges/" + bname1;
        result = getJsonResult(requestUri, HTTP_POST, requestBody);
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
        result = getJsonResult(baseURL + tname1 + "/vbridges/" + bname1,
                               HTTP_POST, requestBody);
        assertResponse(HTTP_CONFLICT);

        // Test POST vBridge2
        requestBody = "{\"description\":\"" + desc2 +
            "\", \"ageInterval\":\"" + ageinter1 + "\"}";
        requestUri = baseURL + tname1 + "/vbridges/" + bname2;
        result = getJsonResult(requestUri, HTTP_POST, requestBody);
        assertResponse(HTTP_CREATED);
        Assert.assertEquals(requestUri, httpLocation);

        // Test POST vBridge2 for other tenant
        requestBody = "{\"ageInterval\":\"" + ageinter2 + "\"}";
        requestUri = baseURL + tname2 + "/vbridges/" + bname2;
        result = getJsonResult(requestUri, HTTP_POST, requestBody);
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
        requestBody = "{\"description\":\"" + desc3 +
            "\", \"ageInterval\":\"" + ageinter3 + "\"}";
        requestUri = baseURL + tname1 + "/vbridges/" + bname3;
        result = getJsonResult(requestUri, HTTP_POST, requestBody);
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
        requestBody = "{\"description\":\"" + desc1 +
            "\", \"ageInterval\":\"" + ageinter4 + "\"}";
        requestUri = baseURL + tname1 + "/vbridges/" + bname4;

        // Ensure that query parameters are eliminated from Location.
        result = getJsonResult(requestUri + "?param1=1&param2=2", HTTP_POST,
                               requestBody);
        assertResponse(HTTP_CREATED);
        Assert.assertEquals(requestUri, httpLocation);

        // Test POST vBridge expecting 400
        requestBody = "{\"description\":\"" + desc1 +
            "\", \"ageInterval\":\"" + "ageInterval" + "\"}";
        requestUri = baseURL + tname1 + "/vbridges/" + ebname;
        result = getJsonResult(requestUri, HTTP_POST, requestBody);
        assertResponse(HTTP_BAD_REQUEST);

        // Test POST vBridge expecting 400, specifying too long vBridge name.
        requestBody = "{}";
        requestUri = baseURL + tname1 + "/vbridges/" + bname32;
        result = getJsonResult(requestUri, HTTP_POST, requestBody);
        assertResponse(HTTP_BAD_REQUEST);

        // Test POST vBridge expecting 405, setting "" to vBridge name
        result = getJsonResult(baseURL + tname1 + "/vbridges/",
                               HTTP_POST, requestBody);
        assertResponse(HTTP_BAD_METHOD);

        // Test POST vBridge expecting 400, specifying invalid vBridge name
        // which starts with "_".
        result = getJsonResult(baseURL + tname1 + "/vbridges/" +
                               "_vbridgename", HTTP_POST, requestBody);
        assertResponse(HTTP_BAD_REQUEST);

        // Test POST vBridge expecting 400, specifying invalid vBridge name
        // including symbol "@".
        result = getJsonResult(baseURL + tname1 + "/vbridges/" +
                               "vbridge@name", HTTP_POST, requestBody);
        assertResponse(HTTP_BAD_REQUEST);

        // Test POST vBridge expecting 400, specifying too large ageInterval.
        requestBody = "{\"ageInterval\":\"" + ageinterOver + "\"}";
        result = getJsonResult(baseURL + tname1 + "/vbridges/" + ebname,
                               HTTP_POST, requestBody);
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
        result = getJsonResult(baseURL + tname1 + "/vbridges/" + bname1 +
                               queryParameter, HTTP_PUT, requestBody);
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
        result = getJsonResult(baseURL + tname1 + "/vbridges/" + bname1 +
                               queryParameter, HTTP_PUT, requestBody);
        assertResponse(HTTP_OK);

        // Test GET vBridge1
        result = getJsonResult(baseURL + tname1 + "/vbridges/" + bname1);
        assertResponse(HTTP_OK);
        jt = new JSONTokener(result);
        json = new JSONObject(jt);

        Assert.assertFalse(json.has("description"));
        Assert.assertEquals(ageinter1, json.getString("ageInterval"));

        // Test PUT vBridge1, setting description and ageInter (queryparameter is true)
        requestBody = "{\"description\":\"" + desc2 +
            "\", \"ageInterval\":\"" + ageinter4 + "\"}";
        result = getJsonResult(baseURL + tname1 + "/vbridges/" + bname1 +
                               queryParameter, HTTP_PUT, requestBody);
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
        result = getJsonResult(baseURL + tname1 + "/vbridges/" + bname1 +
                               queryParameter, HTTP_PUT, requestBody);
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
        result = getJsonResult(baseURL + tname1 + "/vbridges/" + bname2 +
                               queryParameter, HTTP_PUT, requestBody);
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
        result = getJsonResult(baseURL + tname1 + "/vbridges/" + bname2 +
                               queryParameter, HTTP_PUT, requestBody);
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
        result = getJsonResult(baseURL + tname1 + "/vbridges/" + bname2 +
                               queryParameter, HTTP_PUT, requestBody);
        assertResponse(HTTP_OK);

        // Test GET vBridge2
        result = getJsonResult(baseURL + tname1 + "/vbridges/" + bname2);
        assertResponse(HTTP_OK);
        jt = new JSONTokener(result);
        json = new JSONObject(jt);

        Assert.assertEquals(desc1, json.getString("description"));
        Assert.assertEquals(ageinter2, json.getString("ageInterval"));

        // Test PUT vBridge2, setting description and ageInter (query parameter is false)
        requestBody = "{\"description\":\"" + desc3 +
            "\", \"ageInterval\":\"" + ageinter3 + "\"}";
        result = getJsonResult(baseURL + tname1 + "/vbridges/" + bname2 +
                               queryParameter, HTTP_PUT, requestBody);
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
        requestBody = "{\"description\":\"" + desc3 +
            "\", \"ageInterval\":\"" + ageinter3 + "\"}";
        result = getJsonResult(baseURL + tname1 + "/vbridges/" + bname2 +
                               queryParameter, HTTP_PUT, requestBody);
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
                               queryParameter, HTTP_PUT, requestBody);
        assertResponse(HTTP_BAD_REQUEST);

        // Test PUT vBridge1 expecting 404, setting dummy vtn
        requestBody = "{}";
        result = getJsonResult(baseURL +  tnameDummy + "/vbridges/" + bname1 +
                               queryParameter, HTTP_PUT, requestBody);
        assertResponse(HTTP_NOT_FOUND);

        // Test PUT vBridge1 expecting 404, setting dummy vbridge
        result = getJsonResult(baseURL +  tname2 + "/vbridges/" + bname1 +
                               queryParameter, HTTP_PUT, requestBody);
        assertResponse(HTTP_NOT_FOUND);

        // Test PUT vBridge1 expecting 400, specifying too small ageInterval.
        queryParameter = new QueryParameter("all", "false").getString();
        requestBody = "{\"description\":\"" + desc2 +
            "\", \"ageInterval\":\"" + ageinter0 + "\"}";
        result = getJsonResult(baseURL + tname1 + "/vbridges/" + bname1 +
                               queryParameter, HTTP_PUT, requestBody);
        assertResponse(HTTP_BAD_REQUEST);

        // Test PUT vBridge1 expecting 400, specifying too small ageInterval
        // ("all=true" is specified as query paramter).
        queryParameter = new QueryParameter("all", "true").getString();
        requestBody = "{\"description\":\"" + desc2 +
            "\", \"ageInterval\":\"" + ageinter0 + "\"}";
        result = getJsonResult(baseURL + tname1 + "/vbridges/" + bname1 +
                               queryParameter, HTTP_PUT, requestBody);
        assertResponse(HTTP_BAD_REQUEST);

        // Test PUT vBridge1 expecting 400, specifying too large ageInterval
        // ("all=true" is specified as query paramter).
        requestBody = "{\"description\":\"" + desc1 +
            "\", \"ageInterval\":\"" + ageinterOver + "\"}";
        result = getJsonResult(baseURL + tname1 + "/vbridges/" + bname1 +
                               queryParameter, HTTP_PUT, requestBody);
        assertResponse(HTTP_BAD_REQUEST);

        testVLANMappingDeleteAPI(tname1, bname1);
        testMacAddressAPI(tname1, bname1);
        testVBridgeInterfaceDeleteAPI(tname1, bname1);


        // Test DELETE vBridge expecting 404, setting dummy tenant
        result = getJsonResult(baseURL + tnameDummy + "/vbridges/" + bname1,
                               HTTP_DELETE);
        assertResponse(HTTP_NOT_FOUND);

        // Test DELETE vBridge1
        result = getJsonResult(baseURL + tname1 + "/vbridges/" + bname1,
                               HTTP_DELETE);
        assertResponse(HTTP_OK);

        // specify not supported Content-Type
        requestBody = "{}";
        result = getJsonResult(baseURL + tname1 + "/vbridges/" + bname1,
                               HTTP_POST, requestBody, "text/plain");
        assertResponse(HTTP_UNSUPPORTED_TYPE);

        requestBody = "{\"description\":\"test\"}";
        result = getJsonResult(baseURL + tname1 + "/vbridges/" + bname2 +
                               queryParameter, HTTP_PUT, requestBody,
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
                               HTTP_DELETE);
        assertResponse(HTTP_OK);

        // Test DELETE vBridge3
        result = getJsonResult(baseURL + tname1 + "/vbridges/" + bname3,
                               HTTP_DELETE);
        assertResponse(HTTP_OK);

        // Test DELETE vBridge4
        result = getJsonResult(baseURL + tname1 + "/vbridges/" + bname4,
                               HTTP_DELETE);
        assertResponse(HTTP_OK);

        // Test DELETE vBridge2 on other tenant
        result = getJsonResult(baseURL + tname2 + "/vbridges/" + bname2,
                               HTTP_DELETE);
        assertResponse(HTTP_OK);

        // Test DELETE vBridge expecting 404
        result = getJsonResult(baseURL + tname1 + "/vbridges/" + bname1,
                               HTTP_DELETE);
        assertResponse(HTTP_NOT_FOUND);

        // Test DELETE vBridge expecting 404, setting dummy tenant
        result = getJsonResult(baseURL + tname1 + "/vbridges/" + ebname,
                               HTTP_DELETE);
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
        getJsonResult(base + tname1 + "/vbridges/" + bname1, HTTP_POST, body,
                      ct, auth);
        assertResponse(expected);

        String qp = new QueryParameter("all", "true").getString();
        body = "{\"description\":\"test\"}";
        getJsonResult(base + tname1 + "/vbridges/" + bname2 + qp, HTTP_PUT,
                      body, ct, auth);
        assertResponse(expected);

        getJsonResult(base + tname2 + "/vbridges", HTTP_GET, null, ct, auth);
        assertResponse(expected);

        getJsonResult(base + tname2 + "/vbridges/" + bname2, HTTP_GET, null,
                      ct, auth);
        assertResponse(expected);

        getJsonResult(base + tname1 + "/vbridges/" + bname2, HTTP_DELETE, null,
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
                               HTTP_POST , requestBody);
        assertResponse(HTTP_NOT_FOUND);

        // Test POST vbridge interface expecting 201
        // setting vbridge Interface1
        requestBody = "{}";
        String requestUri = baseURL + bname1 + "/interfaces/" + ifname1;
        result = getJsonResult(requestUri, HTTP_POST, requestBody);
        assertResponse(HTTP_CREATED);
        Assert.assertEquals(requestUri, httpLocation);

        // Test POST vbridge interface expecting 409
        // setting vbridge Interface1
        result = getJsonResult(baseURL + bname1 + "/interfaces/" + ifname1,
                               HTTP_POST, requestBody);
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
        result = getJsonResult(requestUri, HTTP_POST, requestBody);
        assertResponse(HTTP_CREATED);
        Assert.assertEquals(requestUri, httpLocation);

        // setting vbridge Interface3
        requestBody = "{\"description\":\"" + desc2 + "\", \"enabled\":true}";
        requestUri = baseURL + bname2 + "/interfaces/" + ifname3;
        result = getJsonResult(requestUri, HTTP_POST, requestBody);
        assertResponse(HTTP_CREATED);
        Assert.assertEquals(requestUri, httpLocation);

        testPortMappingAPI(tname1, bname1, bname2, ifname2, ifname3);

        // Test POST vBridge Interface2, for other tenant
        requestBody = "{\"description\":\"" + desc1 + "\", \"enabled\":true}";
        requestUri = baseURL2 + bname2 + "/interfaces/" + ifname2;
        result = getJsonResult(requestUri, HTTP_POST, requestBody);
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
        result = getJsonResult(requestUri, HTTP_POST, requestBody);
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
        result = getJsonResult(requestUri + "?param1=1&param2=2", HTTP_POST,
                               requestBody);
        assertResponse(HTTP_CREATED);
        Assert.assertEquals(requestUri, httpLocation);

        // Test POST vBridge Interface expecting 400, setting invalid value
        // for ageInterval
        requestBody = "{\"description\":\"" + desc2 +
            "\", \"enabled\":enabled}";
        requestUri = baseURL + bname1 + "/interfaces/" + ifname;
        result = getJsonResult(requestUri, HTTP_POST, requestBody);
        assertResponse(HTTP_BAD_REQUEST);

        // Test POST vBridge Interface expecting 404, setting dummy tenant
        requestBody = "{}";
        result = getJsonResult(url + "default/vtns/" + tnameDummy +
                               "/vbridges/" + bname1 + "/interfaces/" + ifname,
                               HTTP_POST, requestBody);
        assertResponse(HTTP_NOT_FOUND);

        // Test POST vBridge Interface expecting 404, setting vbridge that
        // don't exist
        result = getJsonResult(baseURL + bnameDummy + "/interfaces/" + ifname,
                               HTTP_POST, requestBody);
        assertResponse(HTTP_NOT_FOUND);

        // Test POST vBridge Interface expecting 405, setting "" to vbridgeIF
        // name
        result = getJsonResult(baseURL + bnameDummy + "/interfaces/" + "",
                               HTTP_POST, requestBody);
        assertResponse(HTTP_BAD_METHOD);

        // Test POST vBridge Interface expecting 400, specifying too long
        // interface name.
        result = getJsonResult(baseURL + bname1 + "/interfaces/" + ifname32,
                               HTTP_POST, requestBody);
        assertResponse(HTTP_BAD_REQUEST);

        // Test POST vBridge Interface expecting 400, specifying invalid
        // interface name which starts with "_".
        result = getJsonResult(baseURL + bname1 + "/interfaces/" + "_ifname",
                               HTTP_POST, requestBody);
        assertResponse(HTTP_BAD_REQUEST);

        // Test POST vBridge Interface expecting 400, specifying invalid
        // interface name which includes "@".
        result = getJsonResult(baseURL + bname1 + "/interfaces/" + "if@name",
                               HTTP_POST, requestBody);
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
                               queryParameter, HTTP_PUT, requestBody);
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
                               queryParameter, HTTP_PUT, requestBody);
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
                               queryParameter, HTTP_PUT, requestBody);
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
                               queryParameter, HTTP_PUT, requestBody);
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
                               queryParameter, HTTP_PUT, requestBody);
        assertResponse(HTTP_OK);

        // Test PUT vBridge Interface2 expecting not change
        queryParameter = new QueryParameter("all", "false").getString();
        result = getJsonResult(baseURL + bname1 + "/interfaces/" + ifname2 +
                               queryParameter, HTTP_PUT, requestBody);
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
                               queryParameter, HTTP_PUT, requestBody);
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
                               queryParameter, HTTP_PUT, requestBody);
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
                               queryParameter, HTTP_PUT, requestBody);
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
                + ifname1 + queryParameter, HTTP_PUT, requestBody);
        assertResponse(HTTP_NOT_FOUND);

        // setting dummy vbridge
        result = getJsonResult(baseURL +  bnameDummy + "/interfaces/"
                + ifname1 + queryParameter, HTTP_PUT, requestBody);
        assertResponse(HTTP_NOT_FOUND);

        // setting  dummy vbridgeIF
        result = getJsonResult(baseURL +  bname1 + "/interfaces/" + ifname +
                               queryParameter,
                HTTP_PUT, requestBody);
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
                               HTTP_DELETE);
        assertResponse(HTTP_OK);

        // specify not supported Content-Type
        requestBody = "{}";
        result = getJsonResult(baseURL + bname2 + "/interfaces/" + ifname2,
                               HTTP_POST, requestBody, "text/plain");
        assertResponse(HTTP_UNSUPPORTED_TYPE);

        requestBody = "{}";
        result = getJsonResult(baseURL + bname2 + "/interfaces/" + ifname3 +
                               queryParameter, HTTP_PUT, requestBody,
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
                               HTTP_DELETE);
        assertResponse(HTTP_OK);

        // delete vBridge Interface4
        result = getJsonResult(baseURL + bname1 + "/interfaces/" + ifname4,
                               HTTP_DELETE);
        assertResponse(HTTP_OK);

        // delete vBridge Interface5
        result = getJsonResult(baseURL + bname1 + "/interfaces/" + ifname5,
                               HTTP_DELETE);
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
        getJsonResult(base + bname1 + "/interfaces", HTTP_GET, null, ct, auth);
        assertResponse(expected);

        getJsonResult(base + bname2 + "/interfaces/" + ifname1 , HTTP_GET,
                      null, ct, auth);
        assertResponse(expected);

        String body = "{}";
        getJsonResult(base + bname2 + "/interfaces/" + ifname2, HTTP_POST, body,
                      ct, auth);
        assertResponse(expected);

        String qp = new QueryParameter("all", "true").toString();
        getJsonResult(base + bname2 + "/interfaces/" + ifname1 + qp,
                      HTTP_PUT, body, ct, auth);
        assertResponse(expected);

        getJsonResult(base + bname2 + "/interfaces/" + ifname1, HTTP_DELETE,
                      null, ct, auth);
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
        String result = getJsonResult(url + "default/vtns/" + tnameDummy +
                                      "/vbridges/" + bname + "/interfaces/" +
                                      ifname1, HTTP_DELETE);
        assertResponse(HTTP_NOT_FOUND);

        // setting dummy vbridge
        result = getJsonResult(baseURL + bnameDummy + "/interfaces/" + ifname1,
                               HTTP_DELETE);
        assertResponse(HTTP_NOT_FOUND);

        // setting dummy vbridge interface
        result = getJsonResult(baseURL + bname + "/interfaces/" + ifnameDummy,
                               HTTP_DELETE);
        assertResponse(HTTP_NOT_FOUND);


        // Test DELETE vbridge interface expecting 404
        // setting vBridge Interface1
        result = getJsonResult(baseURL + bname + "/interfaces/" + ifname1,
                               HTTP_DELETE);
        assertResponse(HTTP_OK);

        // setting vBridge Interface2
        result = getJsonResult(baseURL + bname + "/interfaces/" + ifname2,
                               HTTP_DELETE);
        assertResponse(HTTP_OK);


        // Test DELETE vbridge interface expecting 404
        // setting deleted vbridge interface1
        result = getJsonResult(baseURL + bname + "/interfaces/" + ifname1,
                               HTTP_DELETE);
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
        result = getJsonResult(baseURL + ifname + "/portmap/", HTTP_PUT,
                               requestBody);
        assertResponse(HTTP_BAD_REQUEST);


        // Test PUT PortMapping expecting 400, specifying too large VLAN ID.
        requestBody = "{\"vlan\":" + vlanOver + ", \"node\":{\"type\":\"" +
            nodeType + "\", \"id\":\"" + nodeid +
            "\"}, \"port\":{\"name\":\"" + pname + "\", \"type\":\"" +
            nodeType + "\", \"id\":\"" + portnum + "\"}}";
        result = getJsonResult(baseURL + ifname + "/portmap/", HTTP_PUT,
                               requestBody);
        assertResponse(HTTP_BAD_REQUEST);

        // Specifying a negative VLAN ID.
        requestBody = "{\"vlan\":" + vlanNegative +
            ", \"node\":{\"type\":\"" + nodeType + "\", \"id\":\"" + nodeid +
            "\"}, \"port\":{\"name\":\"" + pname + "\", \"type\":\"" +
            nodeType + "\", \"id\":\"" + portnum + "\"}}";
        result = getJsonResult(baseURL + ifname + "/portmap/", HTTP_PUT,
                               requestBody);
        assertResponse(HTTP_BAD_REQUEST);

        // Specifying invalid node type.
        requestBody = "{\"vlan\":" + vlan1 + ", \"node\":{\"type\":\"" +
            test + "\", \"id\":\"" + nodeid + "\"}, \"port\":{\"name\":\"" +
            pname + "\", \"type\":\"" + nodeType + "\", \"id\":\"" + portnum +
            "\"}}";
        result = getJsonResult(baseURL + ifname + "/portmap/", HTTP_PUT,
                               requestBody);
        assertResponse(HTTP_BAD_REQUEST);

        // Without specifying node element.
        requestBody = "{\"vlan\":" + vlan1 + ", \"port\":{\"name\":\"" +
            pname + "\", \"type\":\"" + nodeType + "\", \"id\":\"" + portnum +
            "\"}}";
        result = getJsonResult(baseURL + ifname + "/portmap/", HTTP_PUT,
                               requestBody);
        assertResponse(HTTP_BAD_REQUEST);

        // Without specifying port element.
        requestBody = "{\"vlan\":" + vlan1 + ", \"node\":{\"type\":\"" +
            nodeType + "\", \"id\":\"" + nodeid + "\"}}";
        result = getJsonResult(baseURL + ifname + "/portmap/", HTTP_PUT,
                               requestBody);
        assertResponse(HTTP_BAD_REQUEST);

        // Specifying incomplete port element.
        //   - "id" is specified without "type".
        requestBody = "{\"vlan\":" + vlan1 + ", \"node\":{\"type\":\"" +
            nodeType + "\", \"id\":\"" + nodeid + "\"}, \"port\":{\"id\":\"" +
            portnum + "\"}}";
        result = getJsonResult(baseURL + ifname + "/portmap/", HTTP_PUT,
                               requestBody);
        assertResponse(HTTP_BAD_REQUEST);

        // Specifying incomplete port element.
        //   - "name" and "id" are specified without "type",
        requestBody = "{\"vlan\":" + vlan1 + ", \"node\":{\"type\":\"" +
            nodeType + "\", \"id\":\"" + nodeid +
            "\"}, \"port\":{\"type\":\"" + nodeType + "\"}}";
        result = getJsonResult(baseURL + ifname + "/portmap/", HTTP_PUT,
                               requestBody);
        assertResponse(HTTP_BAD_REQUEST);

        // Specifying invalid port element.
        //   - Invalid port type with specifying "name".
        requestBody = "{\"vlan\":" + vlan1 + ", \"node\":{\"type\":\"" +
            nodeType + "\", \"id\":\"" + nodeid +
            "\"}, \"port\":{\"name\":\"" + "" + "\", \"type\":\"" +
            nodeType + "\", \"id\":\"" + portnum + "\"}}";
        result = getJsonResult(baseURL + ifname + "/portmap/", HTTP_PUT,
                               requestBody);
        assertResponse(HTTP_BAD_REQUEST);

        // Specifying invalid node which does not contain node type.
        requestBody = "{\"vlan\":" + vlan0 + ", \"node\":{\"id\":\"" + nodeid +
            "\"}, \"port\":{\"name\":\"" + pname + "\", \"type\":\"" +
            nodeType + "\", \"id\":\"" + portnum + "\"}}";
        result = getJsonResult(url + "default/vtns/" + tname + "/vbridges/" +
                               bname2 + "/interfaces/" + ifname2 +
                               "/portmap/", HTTP_PUT, requestBody);
        assertResponse(HTTP_BAD_REQUEST);

        // Specifying invalid node which does not contain node ID.
        requestBody = "{\"vlan\":" + vlan1 + ", \"node\":{\"type\":\"" +
            nodeType + "\"}, \"port\":{\"name\":\"" + pname +
            "\", \"type\":\"" + nodeType + "\", \"id\":\"" + portnum +
            "\"}}";
        result = getJsonResult(url + "default/vtns/" + tname + "/vbridges/" +
                               bname2 + "/interfaces/" + ifname2 + "/portmap/",
                               HTTP_PUT, requestBody);
        assertResponse(HTTP_BAD_REQUEST);

        // Specifying invalid port which does not contain port type.
        requestBody = "{\"vlan\":" + vlan1 + ", \"node\":{\"type\":\"" +
            nodeType + "\", \"id\":\"" + nodeid +
            "\"}, \"port\":{\"name\":\"" + pname + "\", \"id\":\"" +
            portnum + "\"}}";
        result = getJsonResult(url + "default/vtns/" + tname + "/vbridges/" +
                               bname2 + "/interfaces/" + ifname2 + "/portmap/",
                               HTTP_PUT, requestBody);
        assertResponse(HTTP_BAD_REQUEST);

        // Specifying invalid port which does not contain port ID.
        requestBody = "{\"vlan\":" + vlan1 + ", \"node\":{\"type\":\"" +
            nodeType + "\", \"id\":\"" + nodeid +
            "\"}, \"port\":{\"name\":\"" + pname + "\", \"type\":\"" +
            nodeType + "\"}}";
        result = getJsonResult(url + "default/vtns/" + tname + "/vbridges/" +
                               bname2 + "/interfaces/" + ifname2 + "/portmap/",
                               HTTP_PUT, requestBody);
        assertResponse(HTTP_BAD_REQUEST);

        // Test PUT PortMapping 400
        // setting invalid value to node id
        requestBody = "{\"vlan\":" + vlan1 + ", \"node\":{\"type\":\"" +
            nodeType + "\", \"id\":\"" + test + "\"}, \"port\":{\"name\":\"" +
            pname + "\", \"type\":\"" + nodeType + "\", \"id\":\"" + portnum +
            "\"}}";
        result = getJsonResult(baseURL + ifname + "/portmap/", HTTP_PUT,
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
                               "/portmap/", HTTP_PUT, requestBody);
        assertResponse(HTTP_NOT_FOUND);

        // setting dummy vBridge
        requestBody = "{\"vlan\":" + vlan1 + ", \"node\":{\"type\":\"" +
            nodeType + "\", \"id\":\"" + nodeid +
            "\"}, \"port\":{\"name\":\"" + pname + "\", \"type\":\"" +
            nodeType + "\", \"id\":\"" + portnum + "\"}}";
        result = getJsonResult(url + "default/vtns/" + tenantDummy +
                               "/vbridges/" + bnameDummy + "/interfaces/" +
                               ifname + "/portmap/", HTTP_PUT, requestBody);
        assertResponse(HTTP_NOT_FOUND);

        // setting dummy vBridge interface
        requestBody = "{\"vlan\":" + vlan1 + ", \"node\":{\"type\":\"" +
            nodeType + "\", \"id\":\"" + nodeid +
            "\"}, \"port\":{\"name\":\"" + pname + "\", \"type\":\"" +
            nodeType + "\", \"id\":\"" + portnum + "\"}}";
        result = getJsonResult(baseURL + ifnameDummy + "/portmap/", HTTP_PUT,
                               requestBody);
        assertResponse(HTTP_NOT_FOUND);

        // specfiy not supported Content-Type
        result = getJsonResult(baseURL + ifname + "/portmap/", HTTP_PUT,
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
        result = getJsonResult(baseURL + ifname + "/portmap/", HTTP_PUT,
                               requestBody);
        assertResponse(HTTP_OK);

        // setting port element without port id and  port type
        requestBody = "{\"vlan\":" + vlan1 + ", \"node\":{\"type\":\"" +
            nodeType + "\", \"id\":\"" + nodeid +
            "\"}, \"port\":{\"name\":\"" + pname + "\"}}";
        result = getJsonResult(baseURL + ifname + "/portmap/", HTTP_PUT,
                               requestBody);
        assertResponse(HTTP_OK);

        // Test PUT PortMapping, change vlan value
        requestBody = "{\"vlan\":" + vlan2 + ", \"node\":{\"type\":\"" +
            nodeType + "\", \"id\":\"" + nodeid +
            "\"}, \"port\":{\"name\":\"" + pname + "\", \"type\":\"" +
            nodeType + "\", \"id\":\"" + portnum + "\"}}";
        result = getJsonResult(baseURL + ifname + "/portmap/", HTTP_PUT,
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
                               HTTP_PUT, requestBody);

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
                               HTTP_PUT, requestBody);
        assertResponse(HTTP_OK);

        // Test PUT PortMapping, except vlan
        requestBody = "{\"node\":{\"type\":\"" + nodeType + "\", \"id\":\"" +
            nodeid + "\"}, \"port\":{\"name\":\"" + pname +
            "\", \"type\":\"" + nodeType + "\", \"id\":\"" + portnum + "\"}}";
        result = getJsonResult(url + "default/vtns/" + tname + "/vbridges/" +
                               bname2 + "/interfaces/" + ifname2 + "/portmap/",
                               HTTP_PUT, requestBody);
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
        getJsonResult(uri, HTTP_PUT, body, ct, auth);
        assertResponse(expected);

        getJsonResult(uri, HTTP_GET, null, ct, auth);
        assertResponse(expected);

        getJsonResult(uri, HTTP_DELETE, null, ct, auth);
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
        String result = getJsonResult(url + "default/vtns/" + tnameDummy +
                                      "/vbridges/" + bname +  "/interfaces/" +
                                      ifnameDummy + "/portmap", HTTP_DELETE);
        assertResponse(HTTP_NOT_FOUND);

        // setting dummy vbridge
        result = getJsonResult(url + "default/vtns/" + tname + "/vbridges/" +
                               bnameDummy +  "/interfaces/" + ifnameDummy +
                               "/portmap", HTTP_DELETE);
        assertResponse(HTTP_NOT_FOUND);

        // setting dummy vbridge interface
        result = getJsonResult(baseURL + ifnameDummy + "/portmap", HTTP_DELETE);
        assertResponse(HTTP_NOT_FOUND);

        // Test DELETE PortMapping
        result = getJsonResult(baseURL + ifname + "/portmap", HTTP_DELETE);
        assertResponse(HTTP_OK);

        // Test DELETE PortMapping, setting deleted portMapping
        result = getJsonResult(baseURL + ifname + "/portmap", HTTP_DELETE);
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
                               "/vbridges/" + bname + "/vlanmaps", HTTP_POST,
                               requestBody);
        assertResponse(HTTP_NOT_FOUND);

        // Test POST VLAN Mapping expecting 404, setting dummy vbridge
        result = getJsonResult(baseURL + bnameDummy + "/vlanmaps", HTTP_POST,
                               requestBody);
        assertResponse(HTTP_NOT_FOUND);

        // Test POST VLAN Mapping expecting 400
        // Specifying a negative VLAN ID.
        requestBody = "{\"vlan\":\"" + vlanNegative +
            "\",\"node\":{\"type\":\"" + nodeType + "\",\"id\":\"" + nodeid1 +
            "\"}}";
        result = getJsonResult(baseURL + bname + "/vlanmaps", HTTP_POST,
                               requestBody);
        assertResponse(HTTP_BAD_REQUEST);

        // Specifyin too large VLAN ID.
        requestBody = "{\"vlan\":\"" + vlanOver + "\",\"node\":{\"type\":\"" +
            nodeType + "\",\"id\":\"" + nodeid1 + "\"}}";
        result = getJsonResult(baseURL + bname + "/vlanmaps", HTTP_POST,
                               requestBody);
        assertResponse(HTTP_BAD_REQUEST);

        // Specifyin invalid node type.
        requestBody = "{\"vlan\":\"" + vlan + "\",\"node\":{\"type\":\"" +
            "ERROR_TEST" + "\",\"id\":\"" + nodeid1 + "\"}}";
        result = getJsonResult(baseURL + bname + "/vlanmaps", HTTP_POST,
                               requestBody);
        assertResponse(HTTP_BAD_REQUEST);

        // Test POST VLAN Mapping
        requestBody = "{\"vlan\":\"" + vlan1 + "\",\"node\":{\"type\":\"" +
            nodeType + "\",\"id\":\"" + nodeid1 + "\"}}";
        String requestUri = baseURL + bname + "/vlanmaps";
        result = getJsonResult(requestUri, HTTP_POST, requestBody);
        assertResponse(HTTP_CREATED);
        String loc = requestUri + "/" + nodeType + "-" + nodeid1 + "." + vlan1;
        Assert.assertEquals(loc, httpLocation);

        // Test POST VLAN Mapping expecting 409
        requestBody = "{\"vlan\":\"" + vlan1 + "\",\"node\":{\"type\":\"" +
            nodeType + "\",\"id\":\"" + nodeid1 + "\"}}";
        result = getJsonResult(baseURL + bname + "/vlanmaps", HTTP_POST,
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
        result = getJsonResult(baseURL + bname + "/vlanmaps", HTTP_POST,
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
        result = getJsonResult(requestUri, HTTP_POST, requestBody);
        assertResponse(HTTP_CREATED);
        loc = requestUri + "/" + nodeType + "-" + nodeid1 + "." + vlan0;
        Assert.assertEquals(loc, httpLocation);

        // Test POST VLAN Mapping, except vlan
        requestBody = "{\"node\":{\"type\":\"" + nodeType + "\",\"id\":\"" +
            nodeid1 + "\"}}";
        requestUri = baseURL + bname + "/vlanmaps";
        result = getJsonResult(requestUri, HTTP_POST, requestBody);
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
                               "-" + nodeid1 + "." + vlan0, HTTP_DELETE);
        assertResponse(HTTP_OK);

        // Test not supported Content-Type
        requestBody = "{\"vlan\":\"" + vlan0 + "\",\"node\":{\"type\":\"" +
            nodeType + "\",\"id\":\"" + nodeid1 + "\"}}";
        requestUri = baseURL + bname + "/vlanmaps";
        result = getJsonResult(requestUri, HTTP_POST, requestBody,
                               "text/plain");
        assertResponse(HTTP_UNSUPPORTED_TYPE);

        // Test POST VLAN Mapping, except vlan
        requestBody = "{\"node\":{\"type\":\"" + nodeType + "\",\"id\":\"" +
            nodeid1 + "\"}}";
        requestUri = baseURL + bname + "/vlanmaps";
        result = getJsonResult(requestUri, HTTP_POST, requestBody);
        assertResponse(HTTP_CREATED);
        loc = requestUri + "/" + nodeType + "-" + nodeid1 + "." + vlan0;
        Assert.assertEquals(loc, httpLocation);

        // Test POST VLAN Mapping, specifying invalid node which does not
        // contain node ID.
        requestBody = "{\"vlan\":\"" + vlan + "\",\"node\":{\"type\":\"" +
            nodeType + "\"}}";
        requestUri = baseURL + bname + "/vlanmaps";
        result = getJsonResult(requestUri, HTTP_POST, requestBody);
        assertResponse(HTTP_BAD_REQUEST);

        // Test POST VLAN Mapping, specifying invalid node which does not
        // contain node type.
        requestBody = "{\"vlan\":\"" + vlan + "\",\"node\":{\"id\":\"" +
            nodeid1 + "\"}}";
        requestUri = baseURL + bname2 + "/vlanmaps";
        result = getJsonResult(requestUri, HTTP_POST, requestBody);
        assertResponse(HTTP_BAD_REQUEST);

        // Test POST VLAN Mapping, setting requestBody without node elements
        requestBody = "{\"vlan\":\"" + vlan3 + "\"}";
        requestUri = baseURL + bname2 + "/vlanmaps";

        // Ensure that query parameters are eliminated from Location.
        result = getJsonResult(requestUri + "?param1=1&param2=2", HTTP_POST,
                               requestBody);
        assertResponse(HTTP_CREATED);
        loc = requestUri + "/" + "ANY" + "." + vlan3;
        Assert.assertEquals(loc, httpLocation);

        // Test POST VLAN Mapping expecting 400
        // setting invalid value to node id
        requestBody = "{\"vlan\":\"" + vlan + "\",\"node\":{\"type\":\"" +
            nodeType + "\",\"id\":\"" + "ERROR_TEST" + "\"}}";
        requestUri = baseURL + bname2 + "/vlanmaps";
        result = getJsonResult(requestUri, HTTP_POST, requestBody);
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
        getJsonResult(uri, HTTP_POST, body, ct, auth);
        assertResponse(expected);

        getJsonResult(uri, HTTP_GET, null, ct, auth);
        assertResponse(expected);

        uri = base + bname2 + "/vlanmaps/OF-1.0";
        getJsonResult(uri, HTTP_GET, null, ct, auth);
        assertResponse(expected);

        getJsonResult(uri, HTTP_DELETE, null, ct, auth);
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
                                      HTTP_DELETE);
        assertResponse(HTTP_OK);

        // Test DELETE VLAN Mapping expecting 404
        // setting deleted vlan mapping
        result = getJsonResult(baseURL + bname + "/vlanmaps/" + nodeType +
                               "-" + nodeid1 + "." + vlan1, HTTP_DELETE);
        assertResponse(HTTP_NOT_FOUND);

        // setting dummy tenant
        result = getJsonResult(url + "default/vtns/" + tnameDummy +
                               "/vbridges/" +  bname + "/vlanmaps/" +
                               nodeType + "-" + nodeid1 + "." + vlan1,
                               HTTP_DELETE);
        assertResponse(HTTP_NOT_FOUND);

        // setting dummy vbridge mapping
        result = getJsonResult(baseURL + bnameDummy + "/vlanmaps/" + nodeType +
                               "-" + nodeid1 + "." + vlan1, HTTP_DELETE);
        assertResponse(HTTP_NOT_FOUND);

        // setting dummy vlan mapping
        result = getJsonResult(baseURL + bname + "/vlanmaps/" + nodeType +
                               "-" + nodeid1 + "." + vlanDummy, HTTP_DELETE);
        assertResponse(HTTP_NOT_FOUND);

        // Test GET VLAN Mapping expecting 404
        result = getJsonResult(baseURL + bname + "/vlanmaps" + nodeType + "-" +
                               nodeid1 + "." + vlan1);
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
                                   vLANMap.getString("id"), HTTP_DELETE);
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
                mac = ByteUtils.toHexString(allowedAddr);
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
            String mac = ByteUtils.toHexString(deniedAddr);
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
        getJsonResult(mapUri + qparam, HTTP_PUT, cf);
        assertResponse(HTTP_CREATED);
        Assert.assertEquals(mapUri, httpLocation);

        for (String method: new String[]{HTTP_PUT, HTTP_POST}) {
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
                getJsonResult(uri, HTTP_PUT);
                assertResponse(HTTP_BAD_REQUEST);

                for (String method: new String[]{HTTP_PUT, HTTP_POST}) {
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
                getJsonResult(uri, HTTP_PUT);
                assertResponse(HTTP_BAD_REQUEST);

                for (String method: new String[]{HTTP_PUT, HTTP_POST}) {
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
        for (String method: new String[]{HTTP_POST, HTTP_PUT}) {
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
                mac = ByteUtils.toHexString(allowedAddr);
            }

            String vlan = String.valueOf(i + 100);
            allowedHosts1.add(mac);
            allowedHosts1.add(vlan);
        }

        for (int i = 5; i < 10; i++) {
            String mac;
            if ((i & 1) == 0) {
                allowedAddr[5] = (byte)(i + 1);
                mac = ByteUtils.toHexString(allowedAddr);
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
            String mac = ByteUtils.toHexString(deniedAddr);
            String vlan = String.valueOf(i + 100);
            deniedHosts1.add(mac);
            deniedHosts1.add(vlan);
        }
        for (int i = 5; i < 10; i++) {
            deniedAddr[4] = (byte)i;
            String mac = ByteUtils.toHexString(deniedAddr);
            String vlan = String.valueOf(i + 1000);
            deniedHosts1.add(mac);
            deniedHosts1.add(vlan);
        }

        JSONObject allow1 = createMacHostSet(allowedHosts1);
        JSONObject deny1 = createMacHostSet(deniedHosts1);
        config = new JSONObject();
        config.put("allow", allow1).put("deny", deny1);
        getJsonResult(createRelativeURI(mapUri + remove), HTTP_POST,
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

            getJsonResult(createRelativeURI(mapUri, acl + remove), HTTP_POST,
                          set.toString());
            assertResponse(HTTP_NO_CONTENT);

            Iterator<String> it = hostList.iterator();
            while (it.hasNext()) {
                String m = it.next();
                String v = it.next();
                String uri = createRelativeURI(mapUri, acl,
                                               (m == null) ? "ANY" : m,
                                               (v == null) ? "0" : v);
                getJsonResult(uri, HTTP_DELETE);
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
            getJsonResult(uri, HTTP_PUT);
            assertResponse(HTTP_CONFLICT);

            JSONObject set = createMacHostSet(mac, vlan);
            JSONObject conf = new JSONObject();
            conf.put(acl, set);

            for (String method: new String[]{HTTP_POST, HTTP_PUT}) {
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
            getJsonResult(uri, HTTP_PUT);
            assertResponse(HTTP_CONFLICT);

            set = createMacHostSet(mac, "1000");
            conf = new JSONObject();
            conf.put(acl, set);

            String method = HTTP_POST;
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
        for (String method: new String[]{HTTP_POST, HTTP_PUT}) {
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
        getJsonResult(mapUri, HTTP_PUT, config.toString());
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
            getJsonResult(uri, HTTP_POST, set1.toString());
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
        getJsonResult(mapUri + remove, HTTP_POST, config.toString());
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
                getJsonResult(uri, HTTP_DELETE);
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
                hostList.add(ByteUtils.toHexString(base));
                hostList.add(String.valueOf(i + 500));
            }
        }

        config = createMacMapConfig(allowedHosts, deniedHosts);
        configExpected = completeMacMapConfig(config);
        getJsonResult(mapUri, HTTP_PUT, config.toString());
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
                hostList.add(ByteUtils.toHexString(base));
                hostList.add(String.valueOf(i + 500));
            }

            JSONObject set1 = createMacHostSet(hostList);
            getJsonResult(createRelativeURI(mapUri, acl1), HTTP_PUT,
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
            getJsonResult(uri, HTTP_DELETE);
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
        getJsonResult(mapUri2, HTTP_PUT, config.toString());
        assertResponse(HTTP_CREATED);
        assertMacMapping(mapUri2, configExpected);

        getJsonResult(mapUri2, HTTP_DELETE);
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
            for (String method: new String[]{HTTP_PUT, HTTP_POST}) {
                getJsonResult(uri + qparam, method, set1.toString());
                assertResponse(HTTP_CREATED);
                Assert.assertEquals(uri, httpLocation);
                assertMacMapping(mapUri2, configExpected);

                getJsonResult(uri, HTTP_DELETE);
                assertResponse(HTTP_OK);
                assertNoMacMapping(mapUri2);
            }

            config = new JSONObject();
            config.put(acl1, hostSet);
            String hostUri = createRelativeURI(uri, mac, vlan);
            getJsonResult(hostUri + qparam, HTTP_PUT);
            assertResponse(HTTP_CREATED);
            Assert.assertEquals(hostUri, httpLocation);
            assertMacMapping(mapUri2, config);

            getJsonResult(hostUri, HTTP_DELETE);
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
            for (String method: new String[]{HTTP_GET, HTTP_DELETE}) {
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
     * @throws Exception  An error occurred.
     */
    private void testMacAddressAPI(String tname, String bname)
        throws Exception {
        LOG.info("Starting MAC address JAX-RS client.");

        String baseURL = new StringBuilder(VTN_BASE_URL).
            append("default/vtns/").append(tname).append("/vbridges/").
            toString();

        String dummy = "dummy";
        String macaddr = "00:00:00:00:00:01";

        // Test GET all MAC address
        String vbrUri = baseURL + bname;
        String macTableUri = createRelativeURI(vbrUri, "mac");
        String result = getJsonResult(macTableUri);
        assertResponse(HTTP_OK);
        JSONTokener jt = new JSONTokener(result);
        JSONObject json = new JSONObject(jt);
        JSONArray macArray = json.getJSONArray("macentry");
        Assert.assertEquals(0, macArray.length());

        // Test GET all MAC address expecting 404
        // setting dummy vtn
        result = getJsonResult(VTN_BASE_URL + "default/vtns/" + dummy +
                               "/vbridges/" + bname + "/mac");
        assertResponse(HTTP_NOT_FOUND);

        // setting dummy vbridge
        result = getJsonResult(baseURL + dummy + "/mac");
        assertResponse(HTTP_NOT_FOUND);

        // Test DELETE all MAC address
        result = getJsonResult(baseURL + bname + "/mac", HTTP_DELETE);
        assertResponse(HTTP_OK);

        // Test DELETE all MAC address expecting 404
        // setting dummy vtn
        result = getJsonResult(VTN_BASE_URL + "default/vtns/" + dummy +
                               "/vbridges/" + bname + "/mac", HTTP_DELETE);
        assertResponse(HTTP_NOT_FOUND);

        // setting dummy vbridge
        result = getJsonResult(baseURL + dummy + "/mac", HTTP_DELETE);
        assertResponse(HTTP_NOT_FOUND);

        // Test GET MAC address expecting 404
        // setting MAC address that don't exist
        String badUri = createRelativeURI(macTableUri, macaddr);
        result = getJsonResult(badUri);
        assertResponse(HTTP_NOT_FOUND);

        // Test DELETE MAC address expecting 404
        // setting MAC address that don't exist
        result = getJsonResult(badUri, HTTP_DELETE);
        assertResponse(HTTP_NOT_FOUND);

        // Map VLAN 0 to the test vBridge using VLAN mapping.
        short vlan = 0;
        String requestBody = "{\"vlan\":\"" + vlan + "\"}";
        result = getJsonResult(baseURL + bname + "/vlanmaps", HTTP_POST,
                               requestBody);
        assertResponse(HTTP_CREATED);

        // Collect edge ports, and create test hosts.
        VBridgePath bpath = new VBridgePath(tname, bname);
        BridgeNetwork bridge = new BridgeNetwork(bpath);
        int hostIdx = 1;
        List<TestHost> hosts = new ArrayList<>();
        for (String nid: ofMockService.getNodes()) {
            for (String pid: ofMockService.getPorts(nid, true)) {
                TestHost host = new TestHost(hostIdx, pid, vlan);
                hostIdx++;
                hosts.add(host);
                bridge.addHost(nid, host);
            }
            for (String pid: ofMockService.getPorts(nid, false)) {
                bridge.setUnmappedPort(pid);
            }
        }

        Map<String, Set<Short>> allPorts = bridge.getMappedVlans();
        Set<MacAddressEntry> expected = new HashSet<>();
        for (TestHost host: hosts) {
            assertTrue(expected.add(host.getMacAddressEntry()));
            learnHost(ofMockService, allPorts, host);
            checkMacTableEntry(macTableUri, expected);
        }

        // Test GET all MAC address expecting 404
        // setting dummy vtn
        result = getJsonResult(VTN_BASE_URL + "default/vtns/" + dummy +
                               "/vbridges/" + bname + "/mac");
        assertResponse(HTTP_NOT_FOUND);

        // Try to get MAC address table in the vBridge that is not present.
        String dummyMacUri = createRelativeURI(baseURL, dummy, "mac");
        getJsonResult(dummyMacUri);
        assertResponse(HTTP_NOT_FOUND);

        String testMac = null;
        for (Iterator<MacAddressEntry> it = expected.iterator();
             it.hasNext();) {
            MacAddressEntry ment = it.next();
            it.remove();

            // Try to remove MAC address from the vBridge that is not present.
            DataLinkAddress dladdr = ment.getAddress();
            assertTrue(dladdr instanceof EthernetAddress);
            EthernetAddress eaddr = (EthernetAddress)dladdr;
            testMac = eaddr.getMacAddress();
            String uri = createRelativeURI(dummyMacUri, testMac);
            getJsonResult(uri, HTTP_DELETE);
            assertResponse(HTTP_NOT_FOUND);

            // Remove all MAC addresses from the MAC address table by
            // specifying MAC address.
            uri = createRelativeURI(macTableUri, testMac);
            getJsonResult(uri, HTTP_DELETE);
            assertResponse(HTTP_OK);
            checkMacTableEntry(macTableUri, expected);

            getJsonResult(uri, HTTP_DELETE);
            assertResponse(HTTP_NOT_FOUND);
        }
        assertTrue(expected.isEmpty());
        assertNotNull(testMac);

        // Add more hosts and learn all hosts.
        for (String nid: ofMockService.getNodes()) {
            for (String pid: ofMockService.getPorts(nid, true)) {
                TestHost host = new TestHost(hostIdx, pid, vlan);
                hostIdx++;
                hosts.add(host);
                bridge.addHost(nid, host);
            }
        }

        expected = bridge.getMacAddressEntries();
        for (TestHost host: hosts) {
            learnHost(ofMockService, allPorts, host);
        }
        checkMacTableEntry(macTableUri, expected);

        // Authentication failure.
        checkMacError(GlobalConstants.DEFAULT.toString(), tname, bname,
                      testMac, false, HttpURLConnection.HTTP_UNAUTHORIZED);

        // Invalid container name.
        String[] invalid = {"bad_container", "version"};
        for (String inv: invalid) {
            checkMacError(inv, tname, bname, testMac, true,
                          HttpURLConnection.HTTP_NOT_FOUND);
            checkMacError(inv, tname, bname, testMac, false,
                          HttpURLConnection.HTTP_UNAUTHORIZED);
        }

        // Flush MAC address table.
        getJsonResult(macTableUri, HTTP_DELETE);
        assertResponse(HTTP_OK);
        checkMacTableEntry(macTableUri,
                           Collections.<MacAddressEntry>emptySet());
    }

    private void checkMacError(String containerName, String tname,
                               String bname, String mac, boolean auth,
                               int expected) {
        String base = VTN_BASE_URL + containerName + "/vtns/" + tname +
            "/vbridges/" + bname + "/mac";
        String ct = "Application/Json";
        getJsonResult(base, HTTP_GET, null, ct, auth);
        assertResponse(expected);

        getJsonResult(base, HTTP_DELETE, null, ct, auth);
        assertResponse(expected);

        String uri = base + "/" + mac;
        getJsonResult(uri, HTTP_GET, null, ct, auth);
        assertResponse(expected);

        getJsonResult(uri, HTTP_DELETE, null, ct, auth);
        assertResponse(expected);
    }

    /**
     * Check the MAC address table in the given vBridge.
     *
     * @param uri       A URI that indicates the MAC address table.
     * @param expected  A set of expected MAC address table entries.
     * @throws Exception  An error occurred.
     */
    private void checkMacTableEntry(String uri, Set<MacAddressEntry> expected)
        throws Exception {
        long timeout = TimeUnit.SECONDS.toMillis(TASK_TIMEOUT);
        long limit = System.currentTimeMillis() + timeout;

        do {
            if (checkMacTableEntry(uri, expected, false)) {
                return;
            }

            sleep(SHORT_DELAY);
            timeout = limit - System.currentTimeMillis();
        } while (timeout > 0);

        checkMacTableEntry(uri, expected, true);
    }

    /**
     * Check the MAC address table in the given vBridge.
     *
     * @param uri       A URI that indicates the MAC address table.
     * @param expected  A set of expected MAC address table entries.
     * @param doAssert  If {@code true}, an error will be thrown on failure.
     * @return  {@code true} only if the check passed.
     * @throws Exception  An error occurred.
     */
    private boolean checkMacTableEntry(String uri,
                                       Set<MacAddressEntry> expected,
                                       boolean doAssert) throws Exception {
        String res = getJsonResult(uri);
        assertResponse(HTTP_OK);

        JSONTokener jt = new JSONTokener(res);
        JSONObject json = new JSONObject(jt);
        JSONArray macArray = json.getJSONArray("macentry");
        if (doAssert) {
            assertEquals(expected.size(), macArray.length());
        } else if (expected.size() != macArray.length()) {
            return false;
        }

        Set<MacAddressEntry> result = new HashSet<>();
        for (int i = 0; i < macArray.length(); i++) {
            JSONObject jobj = macArray.getJSONObject(i);
            assertTrue(result.add(toMacAddressEntry(jobj)));
        }

        if (doAssert) {
            assertEquals(expected, result);
        } else if (!expected.equals(result)) {
            return false;
        }

        return true;
    }

    /**
     * Test case for getting version information APIs.
     *
     * This method is called by {@link #testVTNAPI()}.
     *
     * @throws JSONException  Failed to handle JSON object.
     */
    private void testVtnGlobal(String base) throws JSONException {
        LOG.info("Starting IVTNGlobal JAX-RS client.");

        // Authentication failure.
        String uri = base + "version";
        getJsonResult(uri, HTTP_GET, null, "Application/Json", false);
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
     * Test case for flow filter APIs.
     *
     * @throws JSONException  Failed to handle JSON object.
     */
    @Test
    public void testFlowFilterAPI() throws JSONException {
        String tname = "vtn_1";
        String bname = "node_1";
        String iname = "if_1";

        // Create a VTN.
        String empty = "{}";
        String tenantUri = createURI("default", RES_VTNS, tname);
        getJsonResult(tenantUri, HTTP_POST, empty);
        assertResponse(HTTP_CREATED);
        Assert.assertEquals(tenantUri, httpLocation);

        // Create a vBridge.
        String bridgeUri = createRelativeURI(tenantUri, RES_VBRIDGES, bname);
        getJsonResult(bridgeUri, HTTP_POST, empty);
        assertResponse(HTTP_CREATED);
        Assert.assertEquals(bridgeUri, httpLocation);

        // Create a vBridge interface.
        String bridgeIfUri = createRelativeURI(bridgeUri, RES_INTERFACES,
                                               iname);
        getJsonResult(bridgeIfUri, HTTP_POST, empty);
        assertResponse(HTTP_CREATED);
        Assert.assertEquals(bridgeIfUri, httpLocation);

        // Create a vTerminal.
        String vtermUri = createRelativeURI(tenantUri, RES_VTERMINALS, bname);
        getJsonResult(vtermUri, HTTP_POST, empty);
        assertResponse(HTTP_CREATED);
        Assert.assertEquals(vtermUri, httpLocation);

        // Create a vTerminal interface.
        String vtermIfUri = createRelativeURI(vtermUri, RES_INTERFACES, iname);
        getJsonResult(vtermIfUri, HTTP_POST, empty);
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
            getJsonResult(base, HTTP_DELETE);
            assertResponse(HTTP_NOT_FOUND);

            String uri = createRelativeURI(base, "1");
            getJsonResult(uri);
            assertResponse(HTTP_NOT_FOUND);
            getJsonResult(uri, HTTP_DELETE);
            assertResponse(HTTP_NOT_FOUND);

            getJsonResult(uri, HTTP_PUT, body.toString());
            assertResponse(HTTP_NOT_FOUND);
        }

        for (Map.Entry<String, JSONObject> entry: filters.entrySet()) {
            String uri = entry.getKey();
            JSONObject expected = entry.getValue();

            // Get all flow filters.
            JSONObject json = getJSONObject(uri);
            assertEquals(expected, json);

            // Delete all flow filters.
            getJsonResult(uri, HTTP_DELETE);
            assertResponse(HTTP_OK);
            json = getJSONObject(uri);
            JSONArray array = json.getJSONArray("flowfilter");
            Assert.assertEquals(0, array.length());

            getJsonResult(uri, HTTP_DELETE);
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
                getJsonResult(uri, HTTP_PUT, body.toString());
                assertResponse(HTTP_CREATED);
                Assert.assertEquals(uri, httpLocation);
            }

            JSONObject json = getJSONObject(base);
            assertEquals(list, json);
        }

        // Destroy VTN.
        getJsonResult(tenantUri, HTTP_DELETE);
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
                                 ByteUtils.toHexString(macAddr1))).
            put(createJSONObject("dldst", "address",
                                 ByteUtils.toHexString(macAddr2))).
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
        getJsonResult(uri, HTTP_PUT, pass.toString());
        assertResponse(HTTP_CREATED);
        Assert.assertEquals(uri, httpLocation);

        json = getJSONObject(uri);
        pass.put("index", passIdx);
        assertEquals(pass, json);
        allFilters.put(passIdx, json);

        getJsonResult(uri, HTTP_PUT, pass.toString());
        assertResponse(HTTP_NO_CONTENT);

        // Create one more PASS filter with 3 actions.
        byte[] macAddr3 = {
            (byte)0xa0, (byte)0xb0, (byte)cookie,
            (byte)0xd0, (byte)0xe0, (byte)0xf0,
        };
        type = new JSONObject().put("pass", empty);
        actions = new JSONArray().
            put(createJSONObject("dlsrc", "address",
                                 ByteUtils.toHexString(macAddr3))).
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
        getJsonResult(pass1Uri, HTTP_PUT, pass1.toString());
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
        getJsonResult(uri, HTTP_PUT, drop.toString());
        assertResponse(HTTP_CREATED);
        Assert.assertEquals(uri, httpLocation);

        json = getJSONObject(uri);
        drop.put("index", dropIdx);
        assertEquals(drop, json);

        getJsonResult(uri, HTTP_PUT, drop.toString());
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
                                 ByteUtils.toHexString(macAddr4)));
        drop.put("actions", actions);

        getJsonResult(uri, HTTP_PUT, drop.toString());
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
        getJsonResult(uri, HTTP_PUT, redirect.toString());
        assertResponse(HTTP_CREATED);
        Assert.assertEquals(uri, httpLocation);

        json = getJSONObject(uri);
        redirect.put("index", redirectIdx);
        destination.remove("tenant");
        assertEquals(redirect, json);

        getJsonResult(uri, HTTP_PUT, redirect.toString());
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

        getJsonResult(uri, HTTP_PUT, redirect.toString());
        assertResponse(HTTP_OK);

        json = getJSONObject(uri);
        redirect.put("index", redirectIdx);
        assertEquals(redirect, json);
        allFilters.put(redirectIdx, json);

        getJsonResult(uri, HTTP_PUT, redirect.toString());
        assertResponse(HTTP_NO_CONTENT);

        // BAD_REQUEST tests.

        // Specify invalid index.
        int[] badIndex = {-1, 0, 65536, 65537, 100000};
        for (int idx: badIndex) {
            uri = createRelativeURI(baseUri, String.valueOf(idx));
            getJsonResult(uri, HTTP_PUT, pass.toString());
            assertResponse(HTTP_BAD_REQUEST);

            getJsonResult(uri);
            assertResponse(HTTP_NO_CONTENT);
        }

        // No flow condition name.
        JSONObject bad = new JSONObject().
            put("filterType", new JSONObject().put("pass", empty));
        getJsonResult(pass1Uri, HTTP_PUT, bad.toString());
        assertResponse(HTTP_BAD_REQUEST);

        // No flow filter type.
        bad = new JSONObject().put("condition", "cond_1");
        getJsonResult(pass1Uri, HTTP_PUT, bad.toString());
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
            getJsonResult(pass1Uri, HTTP_PUT, bad.toString());
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
            getJsonResult(pass1Uri, HTTP_PUT, bad.toString());
            assertResponse(HTTP_BAD_REQUEST);
        }

        // Ensure that PASS filter at index 7777 was not modified.
        json = getJSONObject(pass1Uri);
        assertEquals(pass1, json);

        // Remove actions in PASS filter at index 7777.
        pass1.remove("actions");
        getJsonResult(pass1Uri, HTTP_PUT, pass1.toString());
        assertResponse(HTTP_OK);

        json = getJSONObject(pass1Uri);
        assertEquals(pass1, json);

        // Delete PASS filter at index 7777.
        getJsonResult(pass1Uri, HTTP_DELETE);
        assertResponse(HTTP_OK);
        getJsonResult(pass1Uri, HTTP_DELETE);
        assertResponse(HTTP_NO_CONTENT);

        // Get all flow filters again.
        JSONObject all = getJSONObject(baseUri);
        array = new JSONArray(allFilters.values());
        JSONObject expected = new JSONObject().put("flowfilter", array);
        assertEquals(expected, all);

        return all;
    }

    /**
     * Test case for path policy APIs.
     *
     * @throws JSONException  Failed to handle JSON object.
     */
    @Test
    public void testPathPolicyAPI() throws JSONException {
        LOG.info("Starting path policy JAX-RS client.");

        // A list of path costs are unordered.
        jsonComparator.addUnordered("cost");

        // Get all path policy IDs.
        String base = createURI("default", RES_PATHPOLICIES);
        JSONObject expected = new JSONObject().put("integer", new JSONArray());
        assertEquals(expected, getJSONObject(base));

        Map<Integer, JSONObject> pathPolicies = new TreeMap<>();
        for (int id = PATH_POLICY_MIN; id <= PATH_POLICY_MAX; id++) {
            JSONObject pp = testPathPolicyAPI(id);
            assertNotNull(pp);
            assertNull(pathPolicies.put(id, pp));
        }

        // Specifying invalid path policy ID.
        List<String> badIds = new ArrayList<>();
        for (int i = 1; i <= 5; i++) {
            badIds.add(String.valueOf(PATH_POLICY_MIN - i));
            badIds.add(String.valueOf(PATH_POLICY_MAX + i));
        }
        badIds.add("bad_ID");
        badIds.add("1111111111111111111111111111111111111");

        JSONObject json = new JSONObject();
        json.put("default", 100);
        String body = json.toString();

        json = new JSONObject();
        json.put("value", 100);
        String xi = json.toString();
        for (String id: badIds) {
            int putErr = HTTP_BAD_REQUEST;
            try {
                Integer.parseInt(id);
            } catch (Exception e) {
                putErr = HTTP_NOT_FOUND;
            }

            String ppUri = createRelativeURI(base, id);
            getJsonResult(ppUri);
            assertResponse(HTTP_NOT_FOUND);
            getJsonResult(ppUri, HTTP_PUT, body);
            assertResponse(putErr);
            getJsonResult(ppUri, HTTP_DELETE);
            assertResponse(HTTP_NOT_FOUND);

            String[] uris = {
                createRelativeURI(ppUri, "costs", "OF", "1"),
                createRelativeURI(ppUri, "costs", "OF", "1", "name", "port-1"),
                createRelativeURI(ppUri, "costs", "OF", "1", "type", "OF", "1"),
                createRelativeURI(ppUri, "costs", "OF", "1", "type", "OF", "1",
                                  "port-1"),
            };

            for (String uri: uris) {
                getJsonResult(uri);
                assertResponse(HTTP_NOT_FOUND);
                getJsonResult(uri, HTTP_PUT, xi);
                assertResponse(HTTP_NOT_FOUND);
                getJsonResult(uri, HTTP_DELETE);
                assertResponse(HTTP_NOT_FOUND);
            }

            String uri = createRelativeURI(ppUri, "default");
            getJsonResult(uri);
            assertResponse(HTTP_NOT_FOUND);
            getJsonResult(uri, HTTP_PUT, xi);
            assertResponse(HTTP_NOT_FOUND);
        }

        for (int id = PATH_POLICY_MIN; id <= PATH_POLICY_MAX; id++) {
            JSONObject pp = pathPolicies.get(id);
            String uri = createRelativeURI(base, String.valueOf(id));
            assertEquals(pp, getJSONObject(uri));

            // Delete path policy.
            getJsonResult(uri, HTTP_DELETE);
            assertResponse(HTTP_OK);

            getJsonResult(uri);
            assertResponse(HTTP_NOT_FOUND);
            getJsonResult(uri, HTTP_DELETE);
            assertResponse(HTTP_NOT_FOUND);

            testPathPolicyError(id, null);
        }

        // Restore path policies.
        for (int id = PATH_POLICY_MIN; id <= PATH_POLICY_MAX; id++) {
            JSONObject pp = pathPolicies.get(id);
            String uri = createRelativeURI(base, String.valueOf(id));
            getJsonResult(uri, HTTP_PUT, pp.toString());
            assertResponse(HTTP_CREATED);
            assertEquals(uri, httpLocation);
            assertEquals(pp, getJSONObject(uri));
        }

        expected = toXmlLongIntegerList(pathPolicies.keySet());

        // Remove all path policies at once.
        getJsonResult(base, HTTP_DELETE);
        assertResponse(HTTP_OK);
        getJsonResult(base, HTTP_DELETE);
        assertResponse(HTTP_NO_CONTENT);
        for (int id = PATH_POLICY_MIN; id <= PATH_POLICY_MAX; id++) {
            String uri = createRelativeURI(base, String.valueOf(id));
            getJsonResult(uri);
            assertResponse(HTTP_NOT_FOUND);
        }

        expected = new JSONObject().put("integer", new JSONArray());
        assertEquals(expected, getJSONObject(base));
    }

    /**
     * Test path policy APIs.
     *
     * @param id  Path policy identifier.
     * @return  A {@link JSONObject} instance which represents the path policy
     *          configuration.
     * @throws JSONException  An error occurred.
     */
    private JSONObject testPathPolicyAPI(int id) throws JSONException {
        String base = createURI("default", RES_PATHPOLICIES,
                                String.valueOf(id));
        getJsonResult(base);
        assertResponse(HTTP_NOT_FOUND);

        Set<JSONObject> costs = new HashSet<>();
        JSONObject ploc = new JSONObject().put("node", createNode(1L));
        JSONObject pc = new JSONObject().put("location", ploc).
            put("cost", 1);
        assertTrue(costs.add(pc));

        JSONObject swport = new JSONObject().put("name", "port-1");
        ploc = new JSONObject().put("node", createNode(2L)).
            put("port", swport);
        pc = new JSONObject().put("location", ploc).put("cost", 20);
        assertTrue(costs.add(pc));

        swport = new JSONObject().put("type", "OF").put("id", "2");
        ploc = new JSONObject().put("node", createNode(3L)).
            put("port", swport);
        pc = new JSONObject().put("location", ploc).put("cost", 300);
        assertTrue(costs.add(pc));

        swport = new JSONObject().put("type", "OF").put("id", "3").
            put("name", "port-3");
        ploc = new JSONObject().put("node", createNode(4L)).
            put("port", swport);
        pc = new JSONObject().put("location", ploc).put("cost", 4000);
        assertTrue(costs.add(pc));

        // Path policy ID in JSON object should be ignored.
        JSONObject pp = new JSONObject().put("id", 10000).put("cost", costs).
            put("default", 1);
        String body = pp.toString();
        getJsonResult(base, HTTP_PUT, body);
        assertResponse(HTTP_CREATED);
        assertEquals(base, httpLocation);
        getJsonResult(base, HTTP_PUT, body);
        assertResponse(HTTP_NO_CONTENT);

        String[] attrs = {"cost", "default"};
        JSONObject expected = new JSONObject(pp, attrs).put("id", id);
        JSONObject json = getJSONObject(base);
        assertEquals(expected, json);

        // Change default cost.
        for (long l = 0; l <= 5; l++) {
            long cost = 0x100000000L + l * 10000L;
            pp.put("default", cost);
            body = pp.toString();
            getJsonResult(base, HTTP_PUT, body);
            assertResponse(HTTP_OK);
            getJsonResult(base, HTTP_PUT, body);
            assertResponse(HTTP_NO_CONTENT);

            expected.put("default", cost);
            json = getJSONObject(base);
            assertEquals(expected, json);
        }

        // Change whole path policy configuration.
        costs.clear();
        ploc = new JSONObject().put("node", createNode(10L));
        pc = new JSONObject().put("location", ploc).put("cost", 111);
        assertTrue(costs.add(pc));

        swport = new JSONObject().put("name", "eth10");
        ploc = new JSONObject().put("node", createNode(200L)).
            put("port", swport);
        pc = new JSONObject().put("location", ploc).put("cost", 2222);
        assertTrue(costs.add(pc));

        swport = new JSONObject().put("type", "OF").put("id", "33");
        ploc = new JSONObject().put("node", createNode(3333L)).
            put("port", swport);
        pc = new JSONObject().put("location", ploc).put("cost", 3333333L);
        assertTrue(costs.add(pc));

        swport = new JSONObject().put("type", "OF").put("id", "444").
            put("name", "port-444");
        ploc = new JSONObject().put("node", createNode(4444444444444444L)).
            put("port", swport);
        pc = new JSONObject().put("location", ploc).
            put("cost", 55555555555555L);
        assertTrue(costs.add(pc));

        pp = new JSONObject().put("id", 12345678).put("cost", costs);
        body = pp.toString();
        getJsonResult(base, HTTP_PUT, body);
        assertResponse(HTTP_OK);
        getJsonResult(base, HTTP_PUT, body);
        assertResponse(HTTP_NO_CONTENT);

        expected = new JSONObject(pp, attrs).put("id", id).put("default", 0);
        json = getJSONObject(base);
        assertEquals(expected, json);

        // Set cost for a specific port.
        List<JSONObject> portCosts = new ArrayList<>();
        JSONObject node = createNode(9999L);
        ploc = new JSONObject().put("node", node);
        pc = new JSONObject().put("location", ploc).put("cost", 77777777L);
        portCosts.add(pc);

        swport = new JSONObject().put("name", "port-name-1");
        ploc = new JSONObject().put("node", node).put("port", swport);
        pc = new JSONObject().put("location", ploc).put("cost", 666666L);
        portCosts.add(pc);

        swport = new JSONObject().put("type", "OF").put("id", "5555");
        ploc = new JSONObject().put("node", node).put("port", swport);
        pc = new JSONObject().put("location", ploc).put("cost", 555555555L);
        portCosts.add(pc);

        swport = new JSONObject().put("type", "OF").put("id", "5555").
            put("name", "port-name-1");
        ploc = new JSONObject().put("node", node).put("port", swport);
        pc = new JSONObject().put("location", ploc).put("cost", 444444L);
        portCosts.add(pc);

        Map<JSONObject, String> portUris = new HashMap<>();
        for (JSONObject pcost: portCosts) {
            JSONObject location = pcost.getJSONObject("location");
            Object c = pcost.get("cost");
            JSONObject xi = new JSONObject().put("value", c);
            body = xi.toString();
            JSONObject nd = location.getJSONObject("node");
            String nodeType = nd.getString("type");
            String nodeId = nd.getString("id");
            String uri;
            if (location.has("port")) {
                JSONObject pt = location.getJSONObject("port");
                String portType = (pt.has("type"))
                    ? pt.getString("type") : null;
                String portId = (pt.has("id"))
                    ? pt.getString("id") : null;
                String portName = (pt.has("name"))
                    ? pt.getString("name") : null;
                if (portType == null) {
                    uri = createRelativeURI(base, "costs", nodeType, nodeId,
                                            "name", portName);
                } else {
                    uri = createRelativeURI(base, "costs", nodeType, nodeId,
                                            "type", portType, portId,
                                            portName);
                }
            } else {
                uri = createRelativeURI(base, "costs", nodeType, nodeId);
            }

            assertNull(portUris.put(pcost, uri));
            getJsonResult(uri);
            assertResponse(HTTP_NO_CONTENT);
            getJsonResult(uri, HTTP_DELETE);
            assertResponse(HTTP_NO_CONTENT);
            getJsonResult(uri, HTTP_PUT, body);
            assertResponse(HTTP_CREATED);
            assertEquals(uri, httpLocation);
            json = getJSONObject(uri);
            assertEquals(xi, json);
            getJsonResult(uri, HTTP_PUT, body);
            assertResponse(HTTP_NO_CONTENT);

            assertTrue(costs.add(pcost));
            expected.put("cost", costs);
            json = getJSONObject(base);
            assertEquals(expected, json);
        }

        for (JSONObject pcost: portCosts) {
            String uri = portUris.get(pcost);
            assertNotNull(uri);
            getJsonResult(uri, HTTP_DELETE);
            assertResponse(HTTP_OK);
            getJsonResult(uri);
            assertResponse(HTTP_NO_CONTENT);

            assertTrue(costs.remove(pcost));
            expected.put("cost", costs);
            assertEquals(expected, getJSONObject(base));

            getJsonResult(uri, HTTP_DELETE);
            assertResponse(HTTP_NO_CONTENT);
        }

        // Change default cost only.
        String uri = createRelativeURI(base, "default");
        JSONObject xi = new JSONObject().put("value", 0);
        json = getJSONObject(uri);
        assertEquals(xi, json);
        for (long l = 0; l <= 5; l++) {
            long cost = 0x9999999L + id * 100L + l;
            xi = new JSONObject().put("value", cost);
            body = xi.toString();
            getJsonResult(uri, HTTP_PUT, body);
            assertResponse(HTTP_OK);
            assertEquals(xi, getJSONObject(uri));

            getJsonResult(uri, HTTP_PUT, body);
            assertResponse(HTTP_NO_CONTENT);

            expected.put("default", cost);
            assertEquals(expected, getJSONObject(base));
        }

        testPathPolicyError(id, expected);

        return expected;
    }

    /**
     * Error test case for path policy APIs.
     *
     * @param id        Path policy identifier.
     * @param expected  A {@link JSONObject} that contains current path policy
     *                  configuration. {@code null} means that the specified
     *                  path policy is not configured.
     * @throws JSONException  An error occurred.
     */
    private void testPathPolicyError(int id, JSONObject expected)
        throws JSONException {
        String base = createURI("default", RES_PATHPOLICIES,
                                String.valueOf(id));

        // Invalid default cost.
        for (long c = -10; c <= 0; c++) {
            if (c != 0) {
                JSONObject pp = new JSONObject().put("default", c);
                getJsonResult(base, HTTP_PUT, pp.toString());
                assertResponse(HTTP_BAD_REQUEST);
                if (expected == null) {
                    getJsonResult(base);
                    assertResponse(HTTP_NOT_FOUND);
                } else {
                    assertEquals(expected, getJSONObject(base));
                }
            }

            String uri = createRelativeURI(base, "default");
            JSONObject xi = new JSONObject();
            if (c != 0) {
                xi.put("value", c);
            }
            getJsonResult(uri, HTTP_PUT, xi.toString());
            assertResponse(HTTP_BAD_REQUEST);
            if (expected == null) {
                getJsonResult(uri);
                assertResponse(HTTP_NOT_FOUND);
            } else {
                xi = (expected.has("default"))
                    ? new JSONObject().put("value", expected.get("default"))
                    : new JSONObject().put("value", 0);
                assertEquals(xi, getJSONObject(uri));
            }
        }

        List<JSONObject> badPolicies = new ArrayList<>();
        List<String> badCostUris = new ArrayList<>();

        // Null path cost.
        List<JSONObject> costs = new ArrayList<>();
        costs.add(null);
        badPolicies.add(new JSONObject().put("cost", costs));

        // Null port location.
        JSONObject pc = new JSONObject().put("cost", 10L);
        costs.clear();
        costs.add(pc);
        badPolicies.add(new JSONObject().put("cost", costs));

        // Null node.
        JSONObject ploc = new JSONObject();
        pc = new JSONObject().put("location", ploc).put("cost", 10L);
        costs.clear();
        costs.add(pc);
        badPolicies.add(new JSONObject().put("cost", costs));

        // Unsupported node.
        JSONObject node = new JSONObject().put("type", "PR").
            put("id", "node-1");
        ploc = new JSONObject().put("node", node);
        pc = new JSONObject().put("location", ploc).put("cost", 10L);
        costs.clear();
        costs.add(pc);
        badPolicies.add(new JSONObject().put("cost", costs));

        badCostUris.add(createRelativeURI(base, "costs", "PR", "node-1"));
        badCostUris.add(createRelativeURI(base, "costs", "PR", "node-1",
                                          "name", "port-1"));
        badCostUris.add(createRelativeURI(base, "costs", "PR", "node-1",
                                          "type", "PR", "port-1"));
        badCostUris.add(createRelativeURI(base, "costs", "PR", "node-1",
                                          "type", "PR", "port-1", "port-1"));

        // Incomplete node.
        node = new JSONObject().put("type", "OF");
        ploc = new JSONObject().put("node", node);
        pc = new JSONObject().put("location", ploc).put("cost", 10L);
        costs.clear();
        costs.add(pc);
        badPolicies.add(new JSONObject().put("cost", costs));

        // Broken node.
        node = new JSONObject().put("type", "OF").put("id", "node-1");
        ploc = new JSONObject().put("node", node);
        pc = new JSONObject().put("location", ploc).put("cost", 10L);
        costs.clear();
        costs.add(pc);
        badPolicies.add(new JSONObject().put("cost", costs));

        String nodeType = "OF";
        String[] badUris = {
            createRelativeURI(base, "costs", nodeType, "node-1"),
            createRelativeURI(base, "costs", nodeType, "node-1", "name",
                              "port-1"),
            createRelativeURI(base, "costs", nodeType, "node-1", "type", "OF",
                              "1"),
            createRelativeURI(base, "costs", nodeType, "node-1", "type", "OF",
                              "1", "port-1"),
        };
        for (String uri: badUris) {
            badCostUris.add(uri);
        }

        // Unsuported node connector type.
        JSONObject swport = new JSONObject().put("type", "PR").
            put("id", "port-1");
        node = createNode(1L);
        ploc = new JSONObject().put("node", node).put("port", swport);
        pc = new JSONObject().put("location", ploc).put("cost", 10L);
        costs.clear();
        costs.add(pc);
        badPolicies.add(new JSONObject().put("cost", costs));

        String nodeId = "00:00:00:00:00:00:00:01";
        badCostUris.add(createRelativeURI(base, "costs", nodeType, nodeId,
                                          "type", "PR", "port-1"));
        badCostUris.add(createRelativeURI(base, "costs", nodeType, nodeId,
                                          "type", "PR", "port-1", "port-1"));

        // Empty port.
        swport = new JSONObject();
        ploc = new JSONObject().put("node", node).put("port", swport);
        pc = new JSONObject().put("location", ploc).put("cost", 10L);
        costs.clear();
        costs.add(pc);
        badPolicies.add(new JSONObject().put("cost", costs));

        swport = new JSONObject().put("name", JSONObject.NULL);
        ploc = new JSONObject().put("node", node).put("port", swport);
        pc = new JSONObject().put("location", ploc).put("cost", 10L);
        costs.clear();
        costs.add(pc);
        badPolicies.add(new JSONObject().put("cost", costs));

        // Incomplete port.
        swport = new JSONObject().put("type", "OF");
        ploc = new JSONObject().put("node", node).put("port", swport);
        pc = new JSONObject().put("location", ploc).put("cost", 10L);
        costs.clear();
        costs.add(pc);
        badPolicies.add(new JSONObject().put("cost", costs));

        // Broken OF port.
        swport = new JSONObject().put("type", "OF").put("id", "port-1");
        ploc = new JSONObject().put("node", node).put("port", swport);
        pc = new JSONObject().put("location", ploc).put("cost", 10L);
        costs.clear();
        costs.add(pc);
        badPolicies.add(new JSONObject().put("cost", costs));

        badCostUris.add(createRelativeURI(base, "costs", nodeType, nodeId,
                                          "type", "OF", "port-1"));
        badCostUris.add(createRelativeURI(base, "costs", nodeType, nodeId,
                                          "type", "OF", "port-1", "name-1"));

        // Empty port name.
        swport = new JSONObject().put("name", "");
        ploc = new JSONObject().put("node", node).put("port", swport);
        pc = new JSONObject().put("location", ploc).put("cost", 10L);
        costs.clear();
        costs.add(pc);
        badPolicies.add(new JSONObject().put("cost", costs));

        // Cost is undefined.
        swport = new JSONObject().put("name", "port-1");
        ploc = new JSONObject().put("node", node).put("port", swport);
        pc = new JSONObject().put("location", ploc);
        costs.clear();
        costs.add(pc);
        badPolicies.add(new JSONObject().put("cost", costs));

        // Duplicate port location.
        swport = new JSONObject().put("name", "port-1");
        ploc = new JSONObject().put("node", node).put("port", swport);
        pc = new JSONObject().put("location", ploc).put("cost", 10L);
        costs.clear();
        costs.add(pc);

        swport = new JSONObject().put("type", "OF").put("id", "2");
        ploc = new JSONObject().put("node", node).put("port", swport);
        pc = new JSONObject().put("location", ploc).put("cost", 20L);
        costs.add(pc);

        swport = new JSONObject().put("name", "port-1");
        ploc = new JSONObject().put("node", node).put("port", swport);
        pc = new JSONObject().put("location", ploc).put("cost", 30L);
        costs.add(pc);

        badPolicies.add(new JSONObject().put("cost", costs));

        // Invalid cost.
        for (long c = -10; c <= 0; c++) {
            swport = new JSONObject().put("name", "port-1");
            ploc = new JSONObject().put("node", node).put("port", swport);
            pc = new JSONObject().put("location", ploc).put("cost", c);
            costs.clear();
            costs.add(pc);
            badPolicies.add(new JSONObject().put("cost", costs));
        }

        for (JSONObject bad: badPolicies) {
            getJsonResult(base, HTTP_PUT, bad.toString());
            assertResponse(HTTP_BAD_REQUEST);
            if (expected == null) {
                getJsonResult(base);
                assertResponse(HTTP_NOT_FOUND);
            } else {
                assertEquals(expected, getJSONObject(base));
            }
        }

        JSONObject xi = new JSONObject().put("value", 1L);
        String body = xi.toString();
        for (String uri: badCostUris) {
            getJsonResult(uri, HTTP_PUT, body);
            assertResponse(HTTP_BAD_REQUEST);
            if (expected == null) {
                getJsonResult(base);
                assertResponse(HTTP_NOT_FOUND);
            } else {
                assertEquals(expected, getJSONObject(base));
            }
        }

        // Link cost is not specified in XmlLongInteger.
        String[] uris = {
            createRelativeURI(base, "costs", nodeType, nodeId),
            createRelativeURI(base, "costs", nodeType, nodeId, "name",
                              "port-1"),
            createRelativeURI(base, "costs", nodeType, nodeId, "type",
                              "OF", "1"),
            createRelativeURI(base, "costs", nodeType, nodeId, "type",
                              "OF", "1", "port-1"),
        };
        body = "{}";
        for (String uri: uris) {
            getJsonResult(uri, HTTP_PUT, body);
            assertResponse(HTTP_BAD_REQUEST);
            if (expected == null) {
                getJsonResult(base);
                assertResponse(HTTP_NOT_FOUND);
            } else {
                assertEquals(expected, getJSONObject(base));
            }
        }
    }


    /**
     * Test case for path map APIs.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testPathMapAPI() throws Exception {
        // Test for global path map.
        String globalUri = createURI("default", RES_PATHMAPS);
        JSONObject global = testPathMapAPI(globalUri, 0);

        // Specifying an invalid VTN name should cause NOT_FOUND error.
        String emptyBody = "{}";
        for (String name: INVALID_NAMES) {
            String base = createURI("default", RES_VTNS, name, RES_PATHMAPS);
            getJsonResult(base);
            assertResponse(HTTP_NOT_FOUND);
            String uri = createRelativeURI(base, "1");
            getJsonResult(uri);
            assertResponse(HTTP_NOT_FOUND);
            getJsonResult(uri, HTTP_PUT, emptyBody);
            assertResponse(HTTP_NOT_FOUND);
            getJsonResult(uri, HTTP_DELETE);
            assertResponse(HTTP_NOT_FOUND);
        }

        // Test for VTN path map.
        String[] tenants = {
            "vtn_1", "vtn_2", "vtn_3",
        };

        int cookie = 0;
        Map<String, JSONObject> configs = new HashMap<>();
        for (String tname: tenants) {
            String vtnUri = createURI("default", RES_VTNS, tname);
            String base = createRelativeURI(vtnUri, RES_PATHMAPS);
            getJsonResult(base);
            assertResponse(HTTP_NOT_FOUND);
            String uri = createRelativeURI(base, "1");
            getJsonResult(uri);
            assertResponse(HTTP_NOT_FOUND);
            getJsonResult(uri, HTTP_PUT, emptyBody);
            assertResponse(HTTP_NOT_FOUND);
            getJsonResult(uri, HTTP_DELETE);
            assertResponse(HTTP_NOT_FOUND);

            // Create a VTN.
            getJsonResult(vtnUri, HTTP_POST, emptyBody);
            assertResponse(HTTP_CREATED);
            assertEquals(vtnUri, httpLocation);

            configs.put(base, testPathMapAPI(base, cookie));
            cookie += 33;
        }

        // Verify configurations.
        assertEquals(global, getJSONObject(globalUri));
        for (Map.Entry<String, JSONObject> entry: configs.entrySet()) {
            String base = entry.getKey();
            JSONObject expected = entry.getValue();
            assertEquals(expected, getJSONObject(base));
        }

        // Remove global path maps.
        removePathMaps(globalUri, global);

        // Remove tenant path maps except for tenants[0].
        for (int i = 1; i < tenants.length; i++) {
            String base = createURI("default", RES_VTNS, tenants[i],
                                    RES_PATHMAPS);
            removePathMaps(base, configs.get(base));
        }

        // Remove all VTNs.
        for (String tname: tenants) {
            String vtnUri = createURI("default", RES_VTNS, tname);
            String base = createRelativeURI(vtnUri, RES_PATHMAPS);
            getJsonResult(vtnUri, HTTP_DELETE);
            assertResponse(HTTP_OK);
            getJsonResult(base);
            assertResponse(HTTP_NOT_FOUND);
        }
    }

    /**
     * Run tests for path map APIs.
     *
     * @param base    Base URI for test.
     * @param cookie  An integer value used to create parameters.
     * @return  A {@link JSONObject} instance that contains all path maps
     *          configured into the given URI.
     * @throws Exception  An error occurred.
     */
    private JSONObject testPathMapAPI(String base, int cookie)
        throws Exception {
        LOG.info("Starting path map JAX-RS client: {}", base);

        // Get all path maps.
        JSONObject json = getJSONObject(base);
        JSONArray array = json.getJSONArray("pathmap");
        assertEquals(0, array.length());

        // Configure path maps.
        int index = 1 + cookie;
        String cond = "fcond_" + index;
        int policyId = cookie;
        if (policyId > PATH_POLICY_MAX) {
            policyId = 0;
        }
        int idle = 3000 + cookie;
        int hard = idle + 1;

        // Index in the request body should be ignored.
        JSONObject pmap1 = new JSONObject().
            put("index", Integer.MAX_VALUE).
            put("condition", cond).
            put("policy", policyId).
            put("idleTimeout", idle).
            put("hardTimeout", hard);
        JSONObject expected = new JSONObject().
            put("index", index).
            put("condition", cond).
            put("policy", policyId).
            put("idleTimeout", idle).
            put("hardTimeout", hard);
        String uri = createRelativeURI(base, Integer.toString(index));
        getJsonResult(uri, HTTP_PUT, pmap1.toString());
        assertResponse(HTTP_CREATED);
        assertEquals(uri, httpLocation);
        assertEquals(expected, getJSONObject(uri));
        pmap1 = expected;

        index = 123 + cookie;
        cond = "fcond_" + index;
        policyId++;
        if (policyId > PATH_POLICY_MAX) {
            policyId = 0;
        }
        JSONObject pmap123 = new JSONObject().
            put("condition", cond).
            put("policy", policyId);
        expected = new JSONObject().
            put("index", index).
            put("condition", cond).
            put("policy", policyId);
        String uri123 = createRelativeURI(base, Integer.toString(index));
        getJsonResult(uri123, HTTP_PUT, pmap123.toString());
        assertResponse(HTTP_CREATED);
        assertEquals(uri123, httpLocation);
        assertEquals(expected, getJSONObject(uri123));
        pmap123 = expected;

        index = 98 + cookie;
        cond = "fcond_" + cookie;
        idle = 0;
        hard = 65535;
        JSONObject pmap98 = new JSONObject().
            put("condition", cond).
            put("idleTimeout", idle).
            put("hardTimeout", hard);
        expected = new JSONObject().
            put("index", index).
            put("condition", cond).
            put("policy", 0).
            put("idleTimeout", idle).
            put("hardTimeout", hard);
        uri = createRelativeURI(base, Integer.toString(index));
        getJsonResult(uri, HTTP_PUT, pmap98.toString());
        assertResponse(HTTP_CREATED);
        assertEquals(uri, httpLocation);
        assertEquals(expected, getJSONObject(uri));
        pmap98 = expected;

        index = 65535;
        policyId++;
        if (policyId > PATH_POLICY_MAX) {
            policyId = 0;
        }
        idle = 0;
        hard = 0;
        JSONObject pmap65535 = new JSONObject().
            put("index", index).
            put("condition", cond).
            put("policy", policyId).
            put("idleTimeout", idle).
            put("hardTimeout", hard);
        uri = createRelativeURI(base, Integer.toString(index));
        getJsonResult(uri, HTTP_PUT, pmap65535.toString());
        assertResponse(HTTP_CREATED);
        assertEquals(uri, httpLocation);
        assertEquals(pmap65535, getJSONObject(uri));

        int index987 = 987 + cookie;
        cond = "fcond_" + index;
        policyId++;
        if (policyId > PATH_POLICY_MAX) {
            policyId = 0;
        }
        idle = 10000;
        hard = 0;
        JSONObject pmap987 = new JSONObject().
            put("condition", cond).
            put("policy", policyId).
            put("idleTimeout", idle).
            put("hardTimeout", hard);
        expected = new JSONObject().
            put("index", index987).
            put("condition", cond).
            put("policy", policyId).
            put("idleTimeout", idle).
            put("hardTimeout", hard);
        String uri987 = createRelativeURI(base, Integer.toString(index987));
        getJsonResult(uri987, HTTP_PUT, pmap987.toString());
        assertResponse(HTTP_CREATED);
        assertEquals(uri987, httpLocation);
        assertEquals(expected, getJSONObject(uri987));
        pmap987 = expected;

        // Path maps should be sorted by index.
        JSONObject pmaps = new JSONObject().
            put("pathmap", new JSONArray().
                put(pmap1).
                put(pmap98).
                put(pmap123).
                put(pmap987).
                put(pmap65535));
        assertEquals(pmaps, getJSONObject(base));

        // Update pmap987.
        pmap987 = new JSONObject().
            put("condition", "updated_cond_987");
        expected = new JSONObject().
            put("index", index987).
            put("policy", 0).
            put("condition", "updated_cond_987");
        getJsonResult(uri987, HTTP_PUT, pmap987.toString());
        assertResponse(HTTP_OK);
        assertEquals(expected, getJSONObject(uri987));
        pmap987 = expected;

        pmaps = new JSONObject().
            put("pathmap", new JSONArray().
                put(pmap1).
                put(pmap98).
                put(pmap123).
                put(pmap987).
                put(pmap65535));
        assertEquals(pmaps, getJSONObject(base));

        // Remove pmap123.
        getJsonResult(uri123, HTTP_DELETE);
        assertResponse(HTTP_OK);
        getJsonResult(uri123, HTTP_DELETE);
        assertResponse(HTTP_NO_CONTENT);
        getJsonResult(uri123);
        assertResponse(HTTP_NO_CONTENT);
        pmaps = new JSONObject().
            put("pathmap", new JSONArray().
                put(pmap1).
                put(pmap98).
                put(pmap987).
                put(pmap65535));
        assertEquals(pmaps, getJSONObject(base));

        if ((cookie & 0x1) != 0) {
            // Add one more path map.
            index = 3344 + cookie;
            cond = "fcond_" + index;
            policyId++;
            if (policyId > PATH_POLICY_MAX) {
                policyId = 0;
            }
            idle = 30000;
            hard = 65535;
            JSONObject pmap3344 = new JSONObject().
                put("condition", cond).
                put("policy", policyId).
                put("idleTimeout", idle).
                put("hardTimeout", hard);
            expected = new JSONObject().
                put("index", index).
                put("condition", cond).
                put("policy", policyId).
                put("idleTimeout", idle).
                put("hardTimeout", hard);
            uri = createRelativeURI(base, Integer.toString(index));
            getJsonResult(uri, HTTP_PUT, pmap3344.toString());
            assertResponse(HTTP_CREATED);
            assertEquals(uri, httpLocation);
            assertEquals(expected, getJSONObject(uri));
            pmap3344 = expected;

            pmaps = new JSONObject().
                put("pathmap", new JSONArray().
                    put(pmap1).
                    put(pmap98).
                    put(pmap987).
                    put(pmap3344).
                    put(pmap65535));
            assertEquals(pmaps, getJSONObject(base));
        }

        // Index range check should be skipped on GET and DELETE method.
        int[] invalidIndices = {
            Integer.MIN_VALUE, -1, 0, 65536, 65537, 100000, Integer.MAX_VALUE,
        };

        String body = new JSONObject().put("condition", "cond").toString();
        for (int idx: invalidIndices) {
            uri = createRelativeURI(base, Integer.toString(idx));
            getJsonResult(uri);
            assertResponse(HTTP_NO_CONTENT);
            getJsonResult(uri, HTTP_DELETE);
            assertResponse(HTTP_NO_CONTENT);

            // Invalid index should be rejected on PUT method.
            getJsonResult(uri, HTTP_PUT, body);
            assertResponse(HTTP_BAD_REQUEST);
        }

        String[] uris = {
            uri987,
            createRelativeURI(base, Integer.toString(65530)),
        };
        int[] invalidPolicies = {
            Integer.MIN_VALUE, -1000000, -333, -3, -2, -1,
            PATH_POLICY_MAX + 1, PATH_POLICY_MAX + 2, 1000, 999999,
            Integer.MAX_VALUE,
        };
        int[] invalidTimeouts = {
            Integer.MIN_VALUE, -99999, -100, -3, -2, -1,
            65536, 65537, 100000, 30000000, Integer.MAX_VALUE,
        };
        int[] timeouts = {1, 3000, 65535};
        for (String testUri: uris) {
            // Invalid flow condition name.
            for (String c: INVALID_NAMES) {
                JSONObject pmap = new JSONObject().put("condition", c);
                getJsonResult(testUri, HTTP_PUT, pmap.toString());
                assertResponse(HTTP_BAD_REQUEST);
            }

            // No flow condition name.
            getJsonResult(testUri, HTTP_PUT, "{}");
            assertResponse(HTTP_BAD_REQUEST);

            // Invalid path policy ID.
            for (int p: invalidPolicies) {
                JSONObject pmap = new JSONObject().
                    put("condition", "cond").
                    put("policy", p);
                getJsonResult(testUri, HTTP_PUT, pmap.toString());
                assertResponse(HTTP_BAD_REQUEST);
            }

            // Invalid idle/hard timeout.
            for (int timeout: invalidTimeouts) {
                JSONObject pmap = new JSONObject().
                    put("condition", "cond").
                    put("idleTimeout", timeout).
                    put("hardTimeout", 65535);
                getJsonResult(testUri, HTTP_PUT, pmap.toString());
                assertResponse(HTTP_BAD_REQUEST);

                pmap = new JSONObject().
                    put("condition", "cond").
                    put("idleTimeout", 0).
                    put("hardTimeout", timeout);
                getJsonResult(testUri, HTTP_PUT, pmap.toString());
                assertResponse(HTTP_BAD_REQUEST);
            }

            for (int timeout: timeouts) {
                // Specifying idle timeout without hard timeout.
                JSONObject pmap = new JSONObject().
                    put("condition", "cond").
                    put("idleTimeout", timeout);
                getJsonResult(testUri, HTTP_PUT, pmap.toString());
                assertResponse(HTTP_BAD_REQUEST);

                // Specifying hard timeout without specifying idle timeout.
                pmap = new JSONObject().
                    put("condition", "cond").
                    put("hardTimeout", timeout);
                getJsonResult(testUri, HTTP_PUT, pmap.toString());
                assertResponse(HTTP_BAD_REQUEST);

                // Inconsistent timeout.
                pmap = new JSONObject().
                    put("condition", "cond").
                    put("idleTimeout", timeout).
                    put("hardTimeout", timeout);
                getJsonResult(testUri, HTTP_PUT, pmap.toString());
                assertResponse(HTTP_BAD_REQUEST);

                if (timeout != 65535) {
                    pmap = new JSONObject().
                        put("condition", "cond").
                        put("idleTimeout", timeout + 1).
                        put("hardTimeout", timeout);
                    getJsonResult(testUri, HTTP_PUT, pmap.toString());
                    assertResponse(HTTP_BAD_REQUEST);
                }
            }
        }

        // Any error should never affect existing configuration.
        assertEquals(pmaps, getJSONObject(base));

        return pmaps;
    }

    /**
     * Remove all the path maps configured in the given URI.
     *
     * @param base     Base URI for test.
     * @param current  A {@link JSONObject} instance that contains current
     *                 path map configuration in the given URI.
     */
    private void removePathMaps(String base, JSONObject current)
        throws Exception {
        Map<Integer, JSONObject> pmaps = new TreeMap<>();
        JSONArray array = current.getJSONArray("pathmap");
        for (int i = 0; i < array.length(); i++) {
            JSONObject pmap = array.getJSONObject(i);
            int index = pmap.getInt("index");
            pmaps.put(index, pmap);
        }

        Map<Integer, JSONObject> pmaps1 = new TreeMap<>(pmaps);
        for (Iterator<JSONObject> it = pmaps1.values().iterator();
             it.hasNext();) {
            JSONObject pmap = it.next();
            String uri = createRelativeURI(base, pmap.getString("index"));
            assertEquals(pmap, getJSONObject(uri));
            getJsonResult(uri, HTTP_DELETE);
            assertResponse(HTTP_OK);
            getJsonResult(uri, HTTP_DELETE);
            assertResponse(HTTP_NO_CONTENT);
            getJsonResult(uri);
            assertResponse(HTTP_NO_CONTENT);
            it.remove();

            JSONArray modified = new JSONArray(pmaps1.values());
            JSONObject all = new JSONObject().
                put("pathmap", modified);
            assertEquals(all, getJSONObject(base));
        }

        // Restore path maps.
        for (JSONObject pmap: pmaps.values()) {
            String uri = createRelativeURI(base, pmap.getString("index"));
            getJsonResult(uri, HTTP_PUT, pmap.toString());
            assertResponse(HTTP_CREATED);
            assertEquals(uri, httpLocation);
            assertEquals(pmap, getJSONObject(uri));
        }

        JSONObject expected = new JSONObject().
            put("pathmap", new JSONArray(pmaps.values()));
        assertEquals(expected, getJSONObject(base));

        // Remove all path maps at once.
        getJsonResult(base, HTTP_DELETE);
        assertResponse(HTTP_OK);

        expected = new JSONObject().put("pathmap", new JSONArray());
        assertEquals(expected, getJSONObject(base));
        getJsonResult(base, HTTP_DELETE);
        assertResponse(HTTP_NO_CONTENT);
    }

    /**
     * Custom JSON comparator for flow match.
     */
    private final class JsonFlowMatchComparator
        implements JSONCustomComparator {
        /**
         * {@inheritDoc}
         */
        @Override
        public boolean equals(Deque<String> path, JSONObject json1,
                              JSONObject json2) throws JSONException {
            String key = "index";
            Set<String> keys1 = JSONComparator.getKeys(json1);
            Set<String> keys2 = JSONComparator.getKeys(json2);
            assertTrue(keys1.remove(key));
            assertTrue(keys2.remove(key));

            if (json1.getInt(key) != json2.getInt(key)) {
                return false;
            }

            JSONObject eth1 = getEthernetMatch(json1, keys1);
            JSONObject eth2 = getEthernetMatch(json2, keys2);
            if (eth1 == null) {
                if (eth2 != null) {
                    return false;
                }
            } else if (eth2 == null) {
                return false;
            } else if (!jsonComparator.equals(eth1, eth2)) {
                return false;
            }

            JSONObject inet1 = getInetMatch(json1, keys1);
            JSONObject inet2 = getInetMatch(json2, keys2);
            if (inet1 == null) {
                if (inet2 != null) {
                    return false;
                }
            } else if (inet2 == null) {
                return false;
            } else if (!jsonComparator.equals(inet1, inet2)) {
                return false;
            }

            JSONObject l41 = getLayer4Match(json1, keys1);
            JSONObject l42 = getLayer4Match(json1, keys2);
            if (l41 == null) {
                if (l42 != null) {
                    return false;
                }
            } else if (l42 == null) {
                return false;
            } else if (!jsonComparator.equals(l41, l42)) {
                return false;
            }

            Set<String> empty = Collections.<String>emptySet();
            assertEquals(empty, keys1);
            assertEquals(empty, keys2);

            return true;
        }

        /**
         * Return JSON object which represents the Ethernet match.
         *
         * @param json  A {@link JSONObject} instance.
         * @param keys  A set of JSON keys in the given JSON object.
         * @throws JSONException  An error occurred.
         */
        private JSONObject getEthernetMatch(JSONObject json, Set<String> keys)
            throws JSONException {
            String key = "ethernet";
            JSONObject ether = null;
            if (keys.remove(key)) {
                ether = json.getJSONObject(key);
                if (ether.length() == 0) {
                    ether = null;
                }
            }

            return ether;
        }

        /**
         * Return JSON object which represents the IP match.
         *
         * @param json  A {@link JSONObject} instance.
         * @param keys  A set of JSON keys in the given JSON object.
         * @throws JSONException  An error occurred.
         */
        private JSONObject getInetMatch(JSONObject json, Set<String> keys)
            throws JSONException {
            String key = "inetMatch";
            JSONObject inet = null;
            if (keys.remove(key)) {
                inet = json.getJSONObject(key);
                String ipv4Key = "inet4";
                if (inet.length() == 1 && inet.has(ipv4Key)) {
                    JSONObject ipv4 = inet.getJSONObject(ipv4Key);
                    if (ipv4.length() == 0) {
                        inet = null;
                    }
                }
            }

            return inet;
        }

        /**
         * Return JSON object which represents the layer 4 match.
         *
         * @param json  A {@link JSONObject} instance.
         * @param keys  A set of JSON keys in the given JSON object.
         * @throws JSONException  An error occurred.
         */
        private JSONObject getLayer4Match(JSONObject json, Set<String> keys)
            throws JSONException {
            String key = "l4Match";
            JSONObject l4 = null;
            if (keys.remove(key)) {
                l4 = json.getJSONObject(key);
                if (l4.length() == 1) {
                    String[] protocols = {"tcp", "udp", "icmp"};
                    for (String proto: protocols) {
                        if (l4.has(proto)) {
                            JSONObject obj = l4.getJSONObject(proto);
                            if (obj.length() == 0) {
                                l4 = null;
                            }
                            break;
                        }
                    }
                }
            }

            return l4;
        }
    }

    /**
     * Test case for path policy APIs.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testFlowConditionAPI() throws Exception {
        LOG.info("Starting flow condition JAX-RS client.");

        // Empty match can be omitted.
        JsonFlowMatchComparator comp = new JsonFlowMatchComparator();
        jsonComparator.addCustomComparator(comp, "match",
                                           JSONComparator.KEY_ARRAY).
            addCustomComparator(comp,
                                "flowcondition", JSONComparator.KEY_ARRAY,
                                "match", JSONComparator.KEY_ARRAY);

        // Get all flow conditions.
        String base = createURI("default", RES_FLOWCONDITIONS);
        JSONObject json = getJSONObject(base);
        JSONArray array = json.getJSONArray("flowcondition");
        assertEquals(0, array.length());

        int cookie = 0;
        String[] names = {
            "flow_cond_1",
            "a",
            "1234567890123456789012345678901",
            "flow_cond_2",
            "fc3",
        };

        Map<String, JSONObject> fcmap = new TreeMap<>();
        for (String name: names) {
            fcmap.put(name, testFlowConditionAPI(name, cookie));
            cookie += 3333;

            // Flow conditions should be sorted by name.
            json = getJSONObject(base);
            array = json.getJSONArray("flowcondition");
            assertEquals(fcmap.size(), array.length());
            int idx = 0;
            for (JSONObject exp: fcmap.values()) {
                assertEquals(exp, array.getJSONObject(idx));
                idx++;
            }
        }

        String empty = new JSONObject().toString();
        for (String name: INVALID_NAMES) {
            String uri = createRelativeURI(base, name);
            getJsonResult(uri, HTTP_PUT, empty);
            assertResponse(HTTP_BAD_REQUEST);

            // Name check should be skipped on GET, DELETE, and flow match
            // APIs.
            getJsonResult(uri, HTTP_GET);
            assertResponse(HTTP_NOT_FOUND);
            getJsonResult(uri, HTTP_DELETE);
            assertResponse(HTTP_NOT_FOUND);

            uri = createRelativeURI(uri, "1");
            getJsonResult(uri, HTTP_GET);
            assertResponse(HTTP_NOT_FOUND);
            getJsonResult(uri, HTTP_DELETE);
            assertResponse(HTTP_NOT_FOUND);
            getJsonResult(uri, HTTP_PUT, empty);
            assertResponse(HTTP_NOT_FOUND);
        }

        // Remove flow conditions.
        Map<String, JSONObject> current = new TreeMap<>(fcmap);
        for (int i = names.length - 1; i >= 0; i--) {
            String name = names[i];
            String uri = createRelativeURI(base, name);
            JSONObject fc = current.remove(name);
            assertEquals(fc, getJSONObject(uri));
            getJsonResult(uri, HTTP_DELETE);
            assertResponse(HTTP_OK);
            getJsonResult(uri, HTTP_DELETE);
            assertResponse(HTTP_NOT_FOUND);
            getJsonResult(uri, HTTP_GET);
            assertResponse(HTTP_NOT_FOUND);

            JSONObject expected = new JSONObject().
                put("flowcondition", new JSONArray(current.values()));
            assertEquals(expected, getJSONObject(base));
        }

        // Restore flow conditions.
        for (JSONObject fc: fcmap.values()) {
            String name = fc.getString("name");
            String uri = createRelativeURI(base, name);
            getJsonResult(uri, HTTP_PUT, fc.toString());
            assertResponse(HTTP_CREATED);
            assertEquals(uri, httpLocation);
            assertEquals(fc, getJSONObject(uri));
        }

        JSONObject expected = new JSONObject().
            put("flowcondition", new JSONArray(fcmap.values()));
        assertEquals(expected, getJSONObject(base));

        // Remove all flow conditions at once.
        getJsonResult(base, HTTP_DELETE);
        assertResponse(HTTP_OK);

        expected = new JSONObject().put("flowcondition", new JSONArray());
        assertEquals(expected, getJSONObject(base));

        getJsonResult(base, HTTP_DELETE);
        assertResponse(HTTP_NO_CONTENT);
    }

    /**
     * Test flow condition APIs.
     *
     * @param name    The name of the flow condition.
     * @param cookie  An integer value used to create parameters.
     * @return  A {@link JSONObject} instance which represents the flow
     *          condition configurations.
     * @throws Exception  An error occurred.
     */
    private JSONObject testFlowConditionAPI(String name, int cookie)
        throws Exception {
        String base = createURI("default", RES_FLOWCONDITIONS, name);
        getJsonResult(base);
        assertResponse(HTTP_NOT_FOUND);

        // Flow match APIs should return HTTP_NOT_FOUND if the target
        // flow condition is not present.
        String uri = createRelativeURI(base, Integer.toString(1));
        JSONObject empty = new JSONObject();
        String emptyBody = empty.toString();
        getJsonResult(uri);
        assertResponse(HTTP_NOT_FOUND);
        getJsonResult(uri, HTTP_PUT, emptyBody);
        assertResponse(HTTP_NOT_FOUND);
        getJsonResult(uri, HTTP_DELETE);
        assertResponse(HTTP_NOT_FOUND);

        // Create an empty flow condition.
        // The name in the body should be ignored.
        JSONObject fc = new JSONObject().put("name", "Invalid Name");
        getJsonResult(base, HTTP_PUT, fc.toString());
        assertResponse(HTTP_CREATED);
        assertEquals(base, httpLocation);
        getJsonResult(base, HTTP_PUT, emptyBody);
        assertResponse(HTTP_NO_CONTENT);
        fc.put("name", name);
        assertEquals(fc, getJSONObject(base));

        fc.put("match", new JSONArray());
        getJsonResult(base, HTTP_PUT, fc.toString());
        assertResponse(HTTP_NO_CONTENT);

        // Index range check should be skipped on GET and DELETE method.
        int[] invalidIndices = {
            Integer.MIN_VALUE, -1, 0, 65536, 65537, 100000, Integer.MAX_VALUE,
        };
        for (int idx: invalidIndices) {
            uri = createRelativeURI(base, Integer.toString(idx));
            getJsonResult(uri);
            assertResponse(HTTP_NO_CONTENT);
            getJsonResult(uri, HTTP_DELETE);
            assertResponse(HTTP_NO_CONTENT);
        }
        int[] validIndices = {
            0, 1, 100, 4000, 12345, 65534, 65535,
        };
        for (int idx: validIndices) {
            uri = createRelativeURI(base, Integer.toString(idx));
            getJsonResult(uri);
            assertResponse(HTTP_NO_CONTENT);
        }

        // Configure Ethernet match.
        int index = cookie + 3;
        String matchUri = createRelativeURI(base, Integer.toString(index));
        EtherAddress mac = new EtherAddress((long)(0x00aabbccddeeL + cookie));
        JSONObject match3 = new JSONObject().
            put("index", index).
            put("ethernet", new JSONObject().
                put("src", mac.getText()));

        String body = match3.toString();
        getJsonResult(matchUri, HTTP_PUT, body);
        assertResponse(HTTP_CREATED);
        assertEquals(matchUri, httpLocation);
        getJsonResult(matchUri, HTTP_PUT, body);
        assertResponse(HTTP_NO_CONTENT);
        assertEquals(match3, getJSONObject(matchUri));

        // Configure IPv4 match.
        // This will also configure Ethernet type.
        index = cookie + 100;
        String matchUri100 = createRelativeURI(base, Integer.toString(index));
        Ip4Network ip = new Ip4Network(0x12345678 + cookie);
        JSONObject ipv4 = new JSONObject().
            put("inet4", new JSONObject().
                put("dst", ip.getHostAddress()).
                put("dscp", 63));
        JSONObject match100 = new JSONObject().
            put("inetMatch", ipv4);
        JSONObject etherIpv4 = new JSONObject().
            put("type", EtherTypes.IPv4.intValue());
        JSONObject expected = new JSONObject().
            put("index", index).
            put("inetMatch", ipv4).
            put("ethernet", etherIpv4);

        body = match100.toString();
        getJsonResult(matchUri100, HTTP_PUT, body);
        assertResponse(HTTP_CREATED);
        assertEquals(matchUri100, httpLocation);
        getJsonResult(matchUri100, HTTP_PUT, body);
        assertResponse(HTTP_NO_CONTENT);
        assertEquals(expected, getJSONObject(matchUri100));
        match100 = expected;

        // Configure TCP match with all supported parameters.
        EtherAddress src = new EtherAddress((long)(0x001122334455L + cookie));
        EtherAddress dst = new EtherAddress((long)(0xf0fafbfcfcfeL + cookie));
        Ip4Network srcIp = new Ip4Network(0x0c220000 + (cookie << 16), 16);
        Ip4Network dstIp = new Ip4Network(0xc0a8648a + (cookie << 1), 31);
        int index5 = cookie + 5;
        String matchUri5 = createRelativeURI(base, Integer.toString(index5));
        JSONObject match5 = new JSONObject().
            put("index", index5).
            put("ethernet", new JSONObject().
                put("src", src.getText()).
                put("dst", dst.getText()).
                put("type", EtherTypes.IPv4.intValue()).
                put("vlan", 4095).
                put("vlanpri", 7)).
            put("inetMatch", new JSONObject().
                put("inet4", new JSONObject().
                    put("src", srcIp.getHostAddress()).
                    put("srcsuffix", srcIp.getPrefixLength()).
                    put("dst", dstIp.getHostAddress()).
                    put("dstsuffix", dstIp.getPrefixLength()).
                    put("protocol", IPProtocols.TCP.intValue()).
                    put("dscp", 63))).
            put("l4Match", new JSONObject().
                put("tcp", new JSONObject().
                    put("src", new JSONObject().
                        put("from", 1).put("to", 65535)).
                    put("dst", new JSONObject().
                        put("from", 100).put("to", 200))));

        String body5 = match5.toString();
        getJsonResult(matchUri5, HTTP_PUT, body5);
        assertResponse(HTTP_CREATED);
        assertEquals(matchUri5, httpLocation);
        getJsonResult(matchUri5, HTTP_PUT, body5);
        assertResponse(HTTP_NO_CONTENT);
        assertEquals(match5, getJSONObject(matchUri5));

        // Configure UDP match.
        // This will also configure Ethernet type and IPv4 protocol.
        // Note that match index in the request body is always ignored.
        index = 65535;
        matchUri = createRelativeURI(base, Integer.toString(index));
        JSONObject match65535 = new JSONObject().
            put("index", Integer.MAX_VALUE).
            put("l4Match", new JSONObject().
                put("udp", new JSONObject().
                    put("dst", new JSONObject().
                        put("from", 53))));
        JSONObject ipv4Udp = new JSONObject().
            put("inet4", new JSONObject().
                put("protocol", IPProtocols.UDP.intValue()));
        expected = new JSONObject().
            put("index", index).
            put("l4Match", new JSONObject().
                put("udp", new JSONObject().
                    put("dst", new JSONObject().
                        put("from", 53).put("to", 53)))).
            put("inetMatch", ipv4Udp).
            put("ethernet", etherIpv4);

        body = match65535.toString();
        getJsonResult(matchUri, HTTP_PUT, body);
        assertResponse(HTTP_CREATED);
        assertEquals(matchUri, httpLocation);
        getJsonResult(matchUri, HTTP_PUT, body);
        assertResponse(HTTP_NO_CONTENT);
        assertEquals(expected, getJSONObject(matchUri));
        match65535 = expected;

        // Configure ICMP match.
        // This will also configure Ethernet type and IPv4 protocol.
        index = cookie + 1234;
        matchUri = createRelativeURI(base, Integer.toString(index));
        JSONObject icmp = new JSONObject().
            put("icmp", new JSONObject().
                put("type", 0).
                put("code", 255));
        JSONObject match1234 = new JSONObject().
            put("l4Match", icmp);
        JSONObject ipv4Icmp = new JSONObject().
            put("inet4", new JSONObject().
                put("protocol", IPProtocols.ICMP.intValue()));
        expected = new JSONObject().
            put("index", index).
            put("l4Match", icmp).
            put("inetMatch", ipv4Icmp).
            put("ethernet", etherIpv4);

        body = match1234.toString();
        getJsonResult(matchUri, HTTP_PUT, body);
        assertResponse(HTTP_CREATED);
        assertEquals(matchUri, httpLocation);
        getJsonResult(matchUri, HTTP_PUT, body);
        assertResponse(HTTP_NO_CONTENT);
        assertEquals(expected, getJSONObject(matchUri));
        match1234 = expected;

        // Configure an empty Ethernet match.
        index = cookie + 3333;
        matchUri = createRelativeURI(base, Integer.toString(index));
        JSONObject match3333 = new JSONObject().
            put("index", index).
            put("ethernet", empty);

        body = match3333.toString();
        getJsonResult(matchUri, HTTP_PUT, body);
        assertResponse(HTTP_CREATED);
        assertEquals(matchUri, httpLocation);
        getJsonResult(matchUri, HTTP_PUT, body);
        assertResponse(HTTP_NO_CONTENT);
        JSONObject result = getJSONObject(matchUri);
        if (result.length() == 1) {
            assertEquals(index, result.getInt("index"));
        } else {
            assertEquals(match3333, result);
        }

        // Configure an empty IPv4 match.
        // This will also configure Ethernet type.
        index = cookie + 99;
        matchUri = createRelativeURI(base, Integer.toString(index));
        JSONObject match99 = new JSONObject().
            put("inetMatch", new JSONObject().
                put("inet4", empty));
        expected = new JSONObject().
            put("index", index).
            put("ethernet", etherIpv4).
            put("inetMatch", new JSONObject().
                put("inet4", empty));

        body = match99.toString();
        getJsonResult(matchUri, HTTP_PUT, body);
        assertResponse(HTTP_CREATED);
        assertEquals(matchUri, httpLocation);
        getJsonResult(matchUri, HTTP_PUT, body);
        assertResponse(HTTP_NO_CONTENT);
        result = getJSONObject(matchUri);
        if (result.length() == 2) {
            assertEquals(index, result.getInt("index"));
            assertEquals(etherIpv4, result.getJSONObject("ethernet"));
        } else {
            assertEquals(expected, result);
        }
        match99 = expected;

        // Configure an empty TCP match.
        // This will also configure Ethernet type and IPv4 protocol.
        index = cookie + 1;
        matchUri = createRelativeURI(base, Integer.toString(index));
        JSONObject match1 = new JSONObject().
            put("l4Match", new JSONObject().
                put("tcp", empty));
        JSONObject ipv4Tcp = new JSONObject().
            put("inet4", new JSONObject().
                put("protocol", IPProtocols.TCP.intValue()));
        expected = new JSONObject().
            put("index", index).
            put("ethernet", etherIpv4).
            put("inetMatch", ipv4Tcp).
            put("l4Match", new JSONObject().
                put("tcp", empty));

        body = match1.toString();
        getJsonResult(matchUri, HTTP_PUT, body);
        assertResponse(HTTP_CREATED);
        assertEquals(matchUri, httpLocation);
        getJsonResult(matchUri, HTTP_PUT, body);
        assertResponse(HTTP_NO_CONTENT);
        result = getJSONObject(matchUri);
        if (result.length() == 3) {
            assertEquals(index, result.getInt("index"));
            assertEquals(etherIpv4, result.getJSONObject("ethernet"));
            assertEquals(ipv4Tcp, result.getJSONObject("inetMatch"));
        } else {
            assertEquals(expected, result);
        }
        match1 = expected;

        // Configure an empty UDP match.
        // This will also configure Ethernet type and IPv4 protocol.
        index = cookie + 23456;
        matchUri = createRelativeURI(base, Integer.toString(index));
        JSONObject match23456 = new JSONObject().
            put("l4Match", new JSONObject().
                put("udp", empty));
        expected = new JSONObject().
            put("index", index).
            put("ethernet", etherIpv4).
            put("inetMatch", ipv4Udp).
            put("l4Match", new JSONObject().
                put("udp", empty));

        body = match23456.toString();
        getJsonResult(matchUri, HTTP_PUT, body);
        assertResponse(HTTP_CREATED);
        assertEquals(matchUri, httpLocation);
        getJsonResult(matchUri, HTTP_PUT, body);
        assertResponse(HTTP_NO_CONTENT);
        result = getJSONObject(matchUri);
        if (result.length() == 3) {
            assertEquals(index, result.getInt("index"));
            assertEquals(etherIpv4, result.getJSONObject("ethernet"));
            assertEquals(ipv4Udp, result.getJSONObject("inetMatch"));
        } else {
            assertEquals(expected, result);
        }
        match23456 = expected;

        // Configure an empty ICMP match.
        // This will also configure Ethernet type and IPv4 protocol.
        index = cookie + 256;
        matchUri = createRelativeURI(base, Integer.toString(index));
        JSONObject match256 = new JSONObject().
            put("l4Match", new JSONObject().
                put("icmp", empty));
        expected = new JSONObject().
            put("index", index).
            put("ethernet", etherIpv4).
            put("inetMatch", ipv4Icmp).
            put("l4Match", new JSONObject().
                put("icmp", empty));

        body = match256.toString();
        getJsonResult(matchUri, HTTP_PUT, body);
        assertResponse(HTTP_CREATED);
        assertEquals(matchUri, httpLocation);
        getJsonResult(matchUri, HTTP_PUT, body);
        assertResponse(HTTP_NO_CONTENT);
        result = getJSONObject(matchUri);
        if (result.length() == 3) {
            assertEquals(index, result.getInt("index"));
            assertEquals(etherIpv4, result.getJSONObject("ethernet"));
            assertEquals(ipv4Icmp, result.getJSONObject("inetMatch"));
        } else {
            assertEquals(expected, result);
        }
        match256 = expected;

        // Matches should be sorted by index.
        fc.put("match", new JSONArray().
               put(match1).
               put(match3).
               put(match5).
               put(match99).
               put(match100).
               put(match256).
               put(match1234).
               put(match3333).
               put(match23456).
               put(match65535));
        assertEquals(fc, getJSONObject(base));

        // Make match5 empty.
        expected = new JSONObject().put("index", index5);
        getJsonResult(matchUri5, HTTP_PUT, emptyBody);
        assertResponse(HTTP_OK);
        getJsonResult(matchUri5, HTTP_PUT, emptyBody);
        assertResponse(HTTP_NO_CONTENT);
        assertEquals(expected, getJSONObject(matchUri5));
        fc.put("match", new JSONArray().
               put(match1).
               put(match3).
               put(expected).
               put(match99).
               put(match100).
               put(match256).
               put(match1234).
               put(match3333).
               put(match23456).
               put(match65535));
        assertEquals(fc, getJSONObject(base));

        // Remove match100.
        getJsonResult(matchUri100, HTTP_DELETE);
        assertResponse(HTTP_OK);
        getJsonResult(matchUri100, HTTP_DELETE);
        assertResponse(HTTP_NO_CONTENT);
        getJsonResult(matchUri100);
        assertResponse(HTTP_NO_CONTENT);
        fc.put("match", new JSONArray().
               put(match1).
               put(match3).
               put(expected).
               put(match99).
               put(match256).
               put(match1234).
               put(match3333).
               put(match23456).
               put(match65535));
        assertEquals(fc, getJSONObject(base));

        // Restore match5.
        getJsonResult(matchUri5, HTTP_PUT, body5);
        assertResponse(HTTP_OK);
        getJsonResult(matchUri5, HTTP_PUT, body5);
        assertResponse(HTTP_NO_CONTENT);
        assertEquals(match5, getJSONObject(matchUri5));
        fc.put("match", new JSONArray().
               put(match1).
               put(match3).
               put(match5).
               put(match99).
               put(match256).
               put(match1234).
               put(match3333).
               put(match23456).
               put(match65535));
        assertEquals(fc, getJSONObject(base));

        // Replace whole configuration.
        mac = new EtherAddress((long)(0x0123456789abL + cookie));
        JSONArray newMatches = new JSONArray().
            put(new JSONObject().
                put("index", 1).
                put("ethernet", new JSONObject().
                    put("dst", mac.getText()).
                    put("vlan", 0))).
            put(new JSONObject().
                put("index", 7777).
                put("ethernet", etherIpv4).
                put("inetMatch", ipv4Udp).
                put("l4Match", new JSONObject().
                    put("udp", new JSONObject().
                        put("src", new JSONObject().
                            put("from", 10000).
                            put("to", 20000)).
                        put("dst", new JSONObject().
                            put("from", 53).
                            put("to", 53)))));
        JSONObject newFc = new JSONObject().
            put("match", newMatches);
        getJsonResult(base, HTTP_PUT, newFc.toString());
        assertResponse(HTTP_OK);
        newFc.put("name", name);
        assertEquals(newFc, getJSONObject(base));

        // Remove flow condition.
        getJsonResult(base, HTTP_DELETE);
        assertResponse(HTTP_OK);
        getJsonResult(base, HTTP_DELETE);
        assertResponse(HTTP_NOT_FOUND);
        getJsonResult(base);
        assertResponse(HTTP_NOT_FOUND);

        // Restore flow condition.
        getJsonResult(base, HTTP_PUT, fc.toString());
        assertResponse(HTTP_CREATED);
        assertEquals(base, httpLocation);
        assertEquals(fc, getJSONObject(base));

        // Invalid match index.
        for (int idx: invalidIndices) {
            uri = createRelativeURI(base, Integer.toString(idx));
            getJsonResult(uri, HTTP_PUT, emptyBody);
            assertResponse(HTTP_BAD_REQUEST);

            JSONObject f = new JSONObject().
                put("match", new JSONArray().
                    put(new JSONObject().
                        put("index", idx)));
            getJsonResult(base, HTTP_PUT, f.toString());
            assertResponse(HTTP_BAD_REQUEST);
        }

        // Invalid matches.
        for (JSONObject m: INVALID_MATCHES) {
            getJsonResult(matchUri5, HTTP_PUT, m.toString());
            assertResponse(HTTP_BAD_REQUEST);

            JSONObject f = new JSONObject().
                put("match", new JSONArray().put(m));
            getJsonResult(base, HTTP_PUT, f.toString());
            assertResponse(HTTP_BAD_REQUEST);
        }

        // No match index.
        JSONArray newMatches1 = new JSONArray();
        for (int i = 0; i < newMatches.length(); i++) {
            newMatches1.put(newMatches.getJSONObject(i));
        }
        newMatches1.put(new JSONObject());
        JSONObject newFc1 = new JSONObject().
            put("match", newMatches1);
        getJsonResult(base, HTTP_PUT, newFc1.toString());
        assertResponse(HTTP_BAD_REQUEST);

        // Duplicate match index.
        newMatches.put(new JSONObject().put("index", 3)).
            put(new JSONObject().put("index", 1));
        getJsonResult(base, HTTP_PUT, newFc.toString());
        assertResponse(HTTP_BAD_REQUEST);

        // Any error should never affect existing configuration.
        assertEquals(fc, getJSONObject(base));

        return fc;
    }

    /**
     * Convert the given JSON object into a {@link MacAddressEntry} instance.
     *
     * @param jobj  A JSON object.
     * @return  A {@link MacAddressEntry} instance.
     * @throws Exception   An error occurred.
     */
    private MacAddressEntry toMacAddressEntry(JSONObject jobj)
        throws Exception {
        String strMac = jobj.getString("address");
        short vlan = Short.parseShort(jobj.getString("vlan"));
        Node node = toNode(jobj.getJSONObject("node"));
        SwitchPort swport = toSwitchPort(jobj.getJSONObject("port"));
        Set<InetAddress> addrs = new HashSet<>();
        String key = "inetAddresses";
        if (jobj.has(key) && !jobj.isNull(key)) {
            JSONObject iaddr = jobj.getJSONObject(key);
            key = "inetAddress";
            if (iaddr.has(key) && !iaddr.isNull(key)) {
                JSONArray array = iaddr.getJSONArray(key);
                key = "address";
                for (int i = 0; i < array.length(); i++) {
                    JSONObject ip = array.getJSONObject(i);
                    String str = ip.getString(key);
                    addrs.add(InetAddress.getByName(str));
                }
            }
        }

        byte[] mac = ByteUtils.toBytes(strMac);
        EthernetAddress eaddr = new EthernetAddress(mac);
        Short ncId = Short.valueOf(Short.parseShort(swport.getId()));
        NodeConnector nc = new NodeConnector(swport.getType(), ncId, node);

        return new MacAddressEntry(eaddr, vlan, nc, addrs);
    }

    /**
     * Convert the given JSON object into a {@link Node} instance.
     *
     * @param jobj  A JSON object.
     * @return  A {@link Node} instance.
     * @throws Exception   An error occurred.
     */
    private Node toNode(JSONObject jobj) throws Exception {
        String type = jobj.getString("type");
        String id = jobj.getString("id");
        Long dpid = toLong(id);
        return new Node(type, Long.valueOf(dpid));
    }

    /**
     * Convert the given JSON object into a {@link SwitchPort} instance.
     *
     * @param jobj  A JSON object.
     * @return  A {@link SwitchPort} instance.
     * @throws Exception   An error occurred.
     */
    private SwitchPort toSwitchPort(JSONObject jobj) throws Exception {
        String name = getJsonString(jobj, "name");
        String type = getJsonString(jobj, "type");
        String id = getJsonString(jobj, "id");
        return new SwitchPort(name, type, id);
    }

    /**
     * Convert the given numbers into a {@link JSONObject} instance which
     * represents a {@code XmlLongIntegerList} instance.
     *
     * @param numbers  A collection of numbers.
     * @return  A {@link JSONObject} instance.
     * @throws JSONException  An error occurred.
     */
    private JSONObject toXmlLongIntegerList(
        Collection<? extends Number> numbers) throws JSONException {
        JSONArray array = new JSONArray();
        for (Number num: numbers) {
            array.put(new JSONObject().put("value", num.longValue()));
        }

        return new JSONObject().put("integer", array);
    }
}
