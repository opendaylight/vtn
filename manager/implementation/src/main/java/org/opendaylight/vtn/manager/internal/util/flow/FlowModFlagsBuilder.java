/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.flow;

import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.types.rev131026.FlowModFlags;

/**
 * {@code FlowModFlagsBuilder} is a builder class for MD-SAL flow-mod flags.
 */
public final class FlowModFlagsBuilder {
    /**
     * A value for CHECK_OVERLAP bit.
     */
    private boolean  checkOverlap;

    /**
     * A value for RESET_COUNTS bit.
     */
    private boolean  resetCounts;

    /**
     * A value for NO_PKT_COUNTS bit.
     */
    private boolean noPktCounts;

    /**
     * A value for NO_BYT_COUNTS bit.
     */
    private boolean noBytCounts;

    /**
     * A value for SEND_FLOW_REM bit.
     */
    private boolean  sendFlowRem;

    /**
     * Return a value for CHECK_OVERLAP bit.
     *
     * @return  A value for CHECK_OVERLAP bit.
     */
    public boolean isCheckOverlap() {
        return checkOverlap;
    }

    /**
     * Set a value for CHECK_OVERLAP bit.
     *
     * @param value  A value to be set.
     * @return  This instance.
     */
    public FlowModFlagsBuilder setCheckOverlap(boolean value) {
        checkOverlap = value;
        return this;
    }

    /**
     * Return a value for RESET_COUNTS bit.
     *
     * @return  A value for RESET_COUNTS bit.
     */
    public boolean isResetCounts() {
        return resetCounts;
    }

    /**
     * Set a value for RESET_COUNTS bit.
     *
     * @param value  A value to be set.
     * @return  This instance.
     */
    public FlowModFlagsBuilder setResetCounts(boolean value) {
        resetCounts = value;
        return this;
    }

    /**
     * Return a value for NO_PKT_COUNTS bit.
     *
     * @return  A value for NO_PKT_COUNTS bit.
     */
    public boolean isNoPktCounts() {
        return noPktCounts;
    }

    /**
     * Set a value for NO_PKT_COUNTS bit.
     *
     * @param value  A value to be set.
     * @return  This instance.
     */
    public FlowModFlagsBuilder setNoPktCounts(boolean value) {
        noPktCounts = value;
        return this;
    }

    /**
     * Return a value for NO_BYT_COUNTS bit.
     *
     * @return  A value for NO_BYT_COUNTS bit.
     */
    public boolean isNoBytCounts() {
        return noBytCounts;
    }

    /**
     * Set a value for NO_BYT_COUNTS bit.
     *
     * @param value  A value to be set.
     * @return  This instance.
     */
    public FlowModFlagsBuilder setNoBytCounts(boolean value) {
        noBytCounts = value;
        return this;
    }

    /**
     * Return a value for SEND_FLOW_REM bit.
     *
     * @return  A value for SEND_FLOW_REM bit.
     */
    public boolean isSendFlowRem() {
        return sendFlowRem;
    }

    /**
     * Set a value for SEND_FLOW_REM bit.
     *
     * @param value  A value to be set.
     * @return  This instance.
     */
    public FlowModFlagsBuilder setSendFlowRem(boolean value) {
        sendFlowRem = value;
        return this;
    }

    /**
     * Construct a new {@link FlowModFlags} instance.
     *
     * @return  A {@link FlowModFlags} instance.
     */
    public FlowModFlags build() {
        return new FlowModFlags(checkOverlap, noBytCounts, noPktCounts,
                                resetCounts, sendFlowRem);
    }
}
