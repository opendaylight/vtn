/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.flow;

import org.junit.Test;

import org.opendaylight.vtn.manager.internal.TestBase;

import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.types.rev131026.FlowModFlags;

/**
 * JUnit test for {@link FlowModFlagsBuilder}.
 */
public class FlowModFlagsBuilderTest extends TestBase {
    /**
     * Test case for CHECK_OVERLAP bit.
     *
     * <ul>
     *   <li>{@link FlowModFlagsBuilder#isCheckOverlap()}</li>
     *   <li>{@link FlowModFlagsBuilder#setCheckOverlap(boolean)}</li>
     * </ul>
     */
    @Test
    public void testCheckOverlap() {
        FlowModFlagsBuilder builder = new FlowModFlagsBuilder();
        assertEquals(false, builder.isCheckOverlap());
        assertEquals(false, builder.isResetCounts());
        assertEquals(false, builder.isNoPktCounts());
        assertEquals(false, builder.isNoBytCounts());
        assertEquals(false, builder.isSendFlowRem());

        assertSame(builder, builder.setCheckOverlap(false));
        assertEquals(false, builder.isCheckOverlap());
        assertEquals(false, builder.isResetCounts());
        assertEquals(false, builder.isNoPktCounts());
        assertEquals(false, builder.isNoBytCounts());
        assertEquals(false, builder.isSendFlowRem());

        assertSame(builder, builder.setCheckOverlap(true));
        assertEquals(true, builder.isCheckOverlap());
        assertEquals(false, builder.isResetCounts());
        assertEquals(false, builder.isNoPktCounts());
        assertEquals(false, builder.isNoBytCounts());
        assertEquals(false, builder.isSendFlowRem());
    }

    /**
     * Test case for RESET_COUNTS bit.
     *
     * <ul>
     *   <li>{@link FlowModFlagsBuilder#isResetCounts()}</li>
     *   <li>{@link FlowModFlagsBuilder#setResetCounts(boolean)}</li>
     * </ul>
     */
    @Test
    public void testResetCounts() {
        FlowModFlagsBuilder builder = new FlowModFlagsBuilder();
        assertEquals(false, builder.isCheckOverlap());
        assertEquals(false, builder.isResetCounts());
        assertEquals(false, builder.isNoPktCounts());
        assertEquals(false, builder.isNoBytCounts());
        assertEquals(false, builder.isSendFlowRem());

        assertSame(builder, builder.setResetCounts(false));
        assertEquals(false, builder.isCheckOverlap());
        assertEquals(false, builder.isResetCounts());
        assertEquals(false, builder.isNoPktCounts());
        assertEquals(false, builder.isNoBytCounts());
        assertEquals(false, builder.isSendFlowRem());

        assertSame(builder, builder.setResetCounts(true));
        assertEquals(false, builder.isCheckOverlap());
        assertEquals(true, builder.isResetCounts());
        assertEquals(false, builder.isNoPktCounts());
        assertEquals(false, builder.isNoBytCounts());
        assertEquals(false, builder.isSendFlowRem());
    }

    /**
     * Test case for NO_PKT_COUNTS bit.
     *
     * <ul>
     *   <li>{@link FlowModFlagsBuilder#isNoPktCounts()}</li>
     *   <li>{@link FlowModFlagsBuilder#setNoPktCounts(boolean)}</li>
     * </ul>
     */
    @Test
    public void testNoPktCounts() {
        FlowModFlagsBuilder builder = new FlowModFlagsBuilder();
        assertEquals(false, builder.isCheckOverlap());
        assertEquals(false, builder.isResetCounts());
        assertEquals(false, builder.isNoPktCounts());
        assertEquals(false, builder.isNoBytCounts());
        assertEquals(false, builder.isSendFlowRem());

        assertSame(builder, builder.setNoPktCounts(false));
        assertEquals(false, builder.isCheckOverlap());
        assertEquals(false, builder.isResetCounts());
        assertEquals(false, builder.isNoPktCounts());
        assertEquals(false, builder.isNoBytCounts());
        assertEquals(false, builder.isSendFlowRem());

        assertSame(builder, builder.setNoPktCounts(true));
        assertEquals(false, builder.isCheckOverlap());
        assertEquals(false, builder.isResetCounts());
        assertEquals(true, builder.isNoPktCounts());
        assertEquals(false, builder.isNoBytCounts());
        assertEquals(false, builder.isSendFlowRem());
    }

    /**
     * Test case for NO_BYT_COUNTS bit.
     *
     * <ul>
     *   <li>{@link FlowModFlagsBuilder#isNoBytCounts()}</li>
     *   <li>{@link FlowModFlagsBuilder#setNoBytCounts(boolean)}</li>
     * </ul>
     */
    @Test
    public void testNoBytCounts() {
        FlowModFlagsBuilder builder = new FlowModFlagsBuilder();
        assertEquals(false, builder.isCheckOverlap());
        assertEquals(false, builder.isResetCounts());
        assertEquals(false, builder.isNoPktCounts());
        assertEquals(false, builder.isNoBytCounts());
        assertEquals(false, builder.isSendFlowRem());

        assertSame(builder, builder.setNoBytCounts(false));
        assertEquals(false, builder.isCheckOverlap());
        assertEquals(false, builder.isResetCounts());
        assertEquals(false, builder.isNoPktCounts());
        assertEquals(false, builder.isNoBytCounts());
        assertEquals(false, builder.isSendFlowRem());

        assertSame(builder, builder.setNoBytCounts(true));
        assertEquals(false, builder.isCheckOverlap());
        assertEquals(false, builder.isResetCounts());
        assertEquals(false, builder.isNoPktCounts());
        assertEquals(true, builder.isNoBytCounts());
        assertEquals(false, builder.isSendFlowRem());
    }

    /**
     * Test case for SEND_FLOW_REM bit.
     *
     * <ul>
     *   <li>{@link FlowModFlagsBuilder#isSendFlowRem()}</li>
     *   <li>{@link FlowModFlagsBuilder#setSendFlowRem(boolean)}</li>
     * </ul>
     */
    @Test
    public void testSendFlowRem() {
        FlowModFlagsBuilder builder = new FlowModFlagsBuilder();
        assertEquals(false, builder.isCheckOverlap());
        assertEquals(false, builder.isResetCounts());
        assertEquals(false, builder.isNoPktCounts());
        assertEquals(false, builder.isNoBytCounts());
        assertEquals(false, builder.isSendFlowRem());

        assertSame(builder, builder.setSendFlowRem(false));
        assertEquals(false, builder.isCheckOverlap());
        assertEquals(false, builder.isResetCounts());
        assertEquals(false, builder.isNoPktCounts());
        assertEquals(false, builder.isNoBytCounts());
        assertEquals(false, builder.isSendFlowRem());

        assertSame(builder, builder.setSendFlowRem(true));
        assertEquals(false, builder.isCheckOverlap());
        assertEquals(false, builder.isResetCounts());
        assertEquals(false, builder.isNoPktCounts());
        assertEquals(false, builder.isNoBytCounts());
        assertEquals(true, builder.isSendFlowRem());
    }

    /**
     * Test case for {@link FlowModFlagsBuilder#build()}.
     */
    @Test
    public void testBuild() {
        FlowModFlagsBuilder builder = new FlowModFlagsBuilder();
        FlowModFlags flags = builder.build();
        assertEquals(Boolean.FALSE, flags.isCHECKOVERLAP());
        assertEquals(Boolean.FALSE, flags.isRESETCOUNTS());
        assertEquals(Boolean.FALSE, flags.isNOPKTCOUNTS());
        assertEquals(Boolean.FALSE, flags.isNOBYTCOUNTS());
        assertEquals(Boolean.FALSE, flags.isSENDFLOWREM());

        Boolean[] bools = {Boolean.TRUE, Boolean.FALSE};
        for (Boolean overlap: bools) {
            for (Boolean reset: bools) {
                for (Boolean nopkt: bools) {
                    for (Boolean nobyte: bools) {
                        for (Boolean send: bools) {
                            flags = builder.
                                setCheckOverlap(overlap).
                                setResetCounts(reset).
                                setNoPktCounts(nopkt).
                                setNoBytCounts(nobyte).
                                setSendFlowRem(send).
                                build();
                            assertEquals(overlap, flags.isCHECKOVERLAP());
                            assertEquals(reset, flags.isRESETCOUNTS());
                            assertEquals(nopkt, flags.isNOPKTCOUNTS());
                            assertEquals(nobyte, flags.isNOBYTCOUNTS());
                            assertEquals(send, flags.isSENDFLOWREM());
                        }
                    }
                }
            }
        }
    }
}
