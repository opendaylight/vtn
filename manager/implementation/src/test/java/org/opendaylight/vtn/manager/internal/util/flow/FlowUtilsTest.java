/*
 * Copyright (c) 2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.flow;

import org.junit.Test;

import org.opendaylight.vtn.manager.internal.util.rpc.RpcErrorTag;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;

import org.opendaylight.vtn.manager.internal.TestBase;

import org.opendaylight.controller.sal.utils.Status;
import org.opendaylight.controller.sal.utils.StatusCode;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnFlowTimeoutConfig;

/**
 * JUnit test for {@link FlowUtils}.
 */
public class FlowUtilsTest extends TestBase {
    /**
     * An implementation of {@link VtnFlowTimeoutConfig} for test.
     */
    private static class TimeoutConfig implements VtnFlowTimeoutConfig {
        /**
         * Value for idle-timeout field.
         */
        private Integer  idleTimeout;

        /**
         * Value for hard-timeout field.
         */
        private Integer  hardTimeout;

        /**
         * Set the idle-timeout value.
         *
         * @param value  A value for idle-timeout.
         * @return  This instance.
         */
        private TimeoutConfig setIdleTimeout(Integer value) {
            idleTimeout = value;
            return this;
        }

        /**
         * Set the hard-timeout value.
         *
         * @param value  A value for hard-timeout.
         * @return  This instance.
         */
        private TimeoutConfig setHardTimeout(Integer value) {
            hardTimeout = value;
            return this;
        }

        // VtnFlowTimeoutConfig

        /**
         * Return a class that indicates the type of this container.
         *
         * @return  This class.
         */
        @Override
        public Class<TimeoutConfig> getImplementedInterface() {
            return TimeoutConfig.class;
        }

        /**
         * Return idle-timeout value.
         */
        @Override
        public Integer getIdleTimeout() {
            return idleTimeout;
        }

        /**
         * Return hard-timeout value.
         */
        @Override
        public Integer getHardTimeout() {
            return hardTimeout;
        }
    }

    /**
     * Test case for the following methods.
     *
     * <ul>
     *   <li>{@link FlowUtils#verifyFlowTimeout(Integer, Integer)}</li>
     *   <li>{@link FlowUtils#verifyFlowTimeout(VtnFlowTimeoutConfig)}</li>
     * </ul>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testVerifyFlowTimeout() throws Exception {
        TimeoutConfig tc = new TimeoutConfig();
        Integer[] timeouts = {
            null, 0, 1, 2, 10, 300, 5555, 65534, 65535,
        };
        RpcErrorTag etag = RpcErrorTag.BAD_ELEMENT;
        StatusCode code = StatusCode.BADREQUEST;
        for (Integer tmout: timeouts) {
            if (tmout == null) {
                tc.setIdleTimeout(tmout).setHardTimeout(tmout);
                FlowUtils.verifyFlowTimeout(tc);
                FlowUtils.verifyFlowTimeout(tmout, tmout);
                continue;
            }

            tc.setIdleTimeout(tmout).setHardTimeout(0);
            FlowUtils.verifyFlowTimeout(tc);
            FlowUtils.verifyFlowTimeout(tmout, 0);
            tc.setIdleTimeout(0).setHardTimeout(tmout);
            FlowUtils.verifyFlowTimeout(tc);
            FlowUtils.verifyFlowTimeout(0, tmout);

            String msg = "idle-timeout must be less than hard-timeout.";
            int tm = tmout.intValue();
            if (tm != 0) {
                tc.setIdleTimeout(tmout).setHardTimeout(tmout);
                try {
                    FlowUtils.verifyFlowTimeout(tc);
                    unexpected();
                } catch (RpcException e) {
                    assertEquals(null, e.getCause());
                    assertEquals(etag, e.getErrorTag());
                    Status st = e.getStatus();
                    assertEquals(code, st.getCode());
                    assertEquals(msg, st.getDescription());
                }

                try {
                    FlowUtils.verifyFlowTimeout(tmout, tmout);
                    unexpected();
                } catch (RpcException e) {
                    assertEquals(null, e.getCause());
                    assertEquals(etag, e.getErrorTag());
                    Status st = e.getStatus();
                    assertEquals(code, st.getCode());
                    assertEquals(msg, st.getDescription());
                }
            }
            if (tm > 1) {
                Integer idle = Integer.valueOf(tm - 1);
                tc.setIdleTimeout(idle).setHardTimeout(tmout);
                FlowUtils.verifyFlowTimeout(tc);
                FlowUtils.verifyFlowTimeout(idle, tmout);

                tc.setIdleTimeout(tmout).setHardTimeout(idle);
                try {
                    FlowUtils.verifyFlowTimeout(tc);
                    unexpected();
                } catch (RpcException e) {
                    assertEquals(null, e.getCause());
                    assertEquals(etag, e.getErrorTag());
                    Status st = e.getStatus();
                    assertEquals(code, st.getCode());
                    assertEquals(msg, st.getDescription());
                }

                try {
                    FlowUtils.verifyFlowTimeout(tmout, idle);
                    unexpected();
                } catch (RpcException e) {
                    assertEquals(null, e.getCause());
                    assertEquals(etag, e.getErrorTag());
                    Status st = e.getStatus();
                    assertEquals(code, st.getCode());
                    assertEquals(msg, st.getDescription());
                }
            }

            // Inconsistent flow timeouts.
            msg = "idle-timeout must be specified.";
            tc.setIdleTimeout(null).setHardTimeout(tmout);
            try {
                FlowUtils.verifyFlowTimeout(tc);
                unexpected();
            } catch (RpcException e) {
                assertEquals(null, e.getCause());
                assertEquals(etag, e.getErrorTag());
                Status st = e.getStatus();
                assertEquals(code, st.getCode());
                assertEquals(msg, st.getDescription());
            }

            try {
                FlowUtils.verifyFlowTimeout(null, tmout);
                unexpected();
            } catch (RpcException e) {
                assertEquals(null, e.getCause());
                assertEquals(etag, e.getErrorTag());
                Status st = e.getStatus();
                assertEquals(code, st.getCode());
                assertEquals(msg, st.getDescription());
            }

            msg = "hard-timeout must be specified.";
            tc.setIdleTimeout(tmout).setHardTimeout(null);
            try {
                FlowUtils.verifyFlowTimeout(tc);
                unexpected();
            } catch (RpcException e) {
                assertEquals(null, e.getCause());
                assertEquals(etag, e.getErrorTag());
                Status st = e.getStatus();
                assertEquals(code, st.getCode());
                assertEquals(msg, st.getDescription());
            }

            try {
                FlowUtils.verifyFlowTimeout(tmout, null);
                unexpected();
            } catch (RpcException e) {
                assertEquals(null, e.getCause());
                assertEquals(etag, e.getErrorTag());
                Status st = e.getStatus();
                assertEquals(code, st.getCode());
                assertEquals(msg, st.getDescription());
            }
        }

        // Invalid idle/hard timeout.
        Integer[] invalidTimeouts = {
            Integer.MIN_VALUE, -99999999, -65535, -3333, -2, -1,
            65536, 65537, 1000000, 33333333, Integer.MAX_VALUE,
        };
        for (Integer tmout: invalidTimeouts) {
            String msg = "Invalid idle-timeout: " + tmout;
            tc.setIdleTimeout(tmout).setHardTimeout(0);
            try {
                FlowUtils.verifyFlowTimeout(tc);
                unexpected();
            } catch (RpcException e) {
                assertEquals(null, e.getCause());
                assertEquals(etag, e.getErrorTag());
                Status st = e.getStatus();
                assertEquals(code, st.getCode());
                assertEquals(msg, st.getDescription());
            }

            try {
                FlowUtils.verifyFlowTimeout(tmout, 0);
                unexpected();
            } catch (RpcException e) {
                assertEquals(null, e.getCause());
                assertEquals(etag, e.getErrorTag());
                Status st = e.getStatus();
                assertEquals(code, st.getCode());
                assertEquals(msg, st.getDescription());
            }

            msg = "Invalid hard-timeout: " + tmout;
            tc.setIdleTimeout(0).setHardTimeout(tmout);
            try {
                FlowUtils.verifyFlowTimeout(tc);
                unexpected();
            } catch (RpcException e) {
                assertEquals(null, e.getCause());
                assertEquals(etag, e.getErrorTag());
                Status st = e.getStatus();
                assertEquals(code, st.getCode());
                assertEquals(msg, st.getDescription());
            }

            try {
                FlowUtils.verifyFlowTimeout(0, tmout);
                unexpected();
            } catch (RpcException e) {
                assertEquals(null, e.getCause());
                assertEquals(etag, e.getErrorTag());
                Status st = e.getStatus();
                assertEquals(code, st.getCode());
                assertEquals(msg, st.getDescription());
            }
        }
    }
}
