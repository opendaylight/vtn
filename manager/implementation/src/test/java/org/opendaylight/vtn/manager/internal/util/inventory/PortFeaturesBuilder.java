/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.inventory;

import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.types.port.rev130925.PortFeatures;

/**
 * Builder class for {@link PortFeatures}.
 */
public final class PortFeaturesBuilder {
    /**
     * Current value of ten-mb-hd bit.
     */
    private Boolean  tenMbHd;

    /**
     * Current value of ten-mb-fd bit.
     */
    private Boolean  tenMbFd;

    /**
     * Current value of hundred-mb-hd bit.
     */
    private Boolean  hundredMbHd;

    /**
     * Current value of hundred-mb-fd bit.
     */
    private Boolean  hundredMbFd;

    /**
     * Current value of one-gb-hd bit.
     */
    private Boolean  oneGbHd;

    /**
     * Current value of one-gb-fd bit.
     */
    private Boolean  oneGbFd;

    /**
     * Current value of ten-gb-fd bit.
     */
    private Boolean  tenGbFd;

    /**
     * Current value of forty-gb-fd bit.
     */
    private Boolean  fortyGbFd;

    /**
     * Current value of hundred-gb-fd bit.
     */
    private Boolean  hundredGbFd;

    /**
     * Current value of one-tb-fd bit.
     */
    private Boolean  oneTbFd;

    /**
     * Current value of other bit.
     */
    private Boolean  other;

    /**
     * Current value of copper bit.
     */
    private Boolean  copper;

    /**
     * Current value of fiber bit.
     */
    private Boolean  fiber;

    /**
     * Current value of autoeng bit.
     */
    private Boolean  autoeng;

    /**
     * Current value of pause bit.
     */
    private Boolean  pause;

    /**
     * Current value of pause-asym bit.
     */
    private Boolean  pauseAsym;

    /**
     * Return the current value if ten-mb-hd bit.
     *
     * @return  A {@link Boolean} instance which represents the value of
     *          ten-mb-hd bit.
     */
    public Boolean isTenMbHd() {
        return tenMbHd;
    }

    /**
     * Return the current value if ten-mb-fd bit.
     *
     * @return  A {@link Boolean} instance which represents the value of
     *          ten-mb-fd bit.
     */
    public Boolean isTenMbFd() {
        return tenMbFd;
    }

    /**
     * Return the current value if hundred-mb-hd bit.
     *
     * @return  A {@link Boolean} instance which represents the value of
     *          hundred-mb-hd bit.
     */
    public Boolean isHundredMbHd() {
        return hundredMbHd;
    }

    /**
     * Return the current value if hundred-mb-fd bit.
     *
     * @return  A {@link Boolean} instance which represents the value of
     *          hundred-mb-fd bit.
     */
    public Boolean isHundredMbFd() {
        return hundredMbFd;
    }

    /**
     * Return the current value if one-gb-hd bit.
     *
     * @return  A {@link Boolean} instance which represents the value of
     *          one-gb-hd bit.
     */
    public Boolean isOneGbHd() {
        return oneGbHd;
    }

    /**
     * Return the current value if one-gb-fd bit.
     *
     * @return  A {@link Boolean} instance which represents the value of
     *          one-gb-fd bit.
     */
    public Boolean isOneGbFd() {
        return oneGbFd;
    }

    /**
     * Return the current value if ten-gb-fd bit.
     *
     * @return  A {@link Boolean} instance which represents the value of
     *          ten-gb-fd bit.
     */
    public Boolean isTenGbFd() {
        return tenGbFd;
    }

    /**
     * Return the current value if forty-gb-fd bit.
     *
     * @return  A {@link Boolean} instance which represents the value of
     *          forty-gb-fd bit.
     */
    public Boolean isFortyGbFd() {
        return fortyGbFd;
    }

    /**
     * Return the current value if hundred-gb-fd bit.
     *
     * @return  A {@link Boolean} instance which represents the value of
     *          hundred-gb-fd bit.
     */
    public Boolean isHundredGbFd() {
        return hundredGbFd;
    }

    /**
     * Return the current value if one-tb-fd bit.
     *
     * @return  A {@link Boolean} instance which represents the value of
     *          one-tb-fd bit.
     */
    public Boolean isOneTbFd() {
        return oneTbFd;
    }

    /**
     * Return the current value if other bit.
     *
     * @return  A {@link Boolean} instance which represents the value of
     *          other bit.
     */
    public Boolean isOther() {
        return other;
    }

    /**
     * Return the current value if copper bit.
     *
     * @return  A {@link Boolean} instance which represents the value of
     *          copper bit.
     */
    public Boolean isCopper() {
        return copper;
    }

    /**
     * Return the current value if fiber bit.
     *
     * @return  A {@link Boolean} instance which represents the value of
     *          fiber bit.
     */
    public Boolean isFiber() {
        return fiber;
    }

    /**
     * Return the current value if autoeng bit.
     *
     * @return  A {@link Boolean} instance which represents the value of
     *          autoeng bit.
     */
    public Boolean isAutoeng() {
        return autoeng;
    }

    /**
     * Return the current value if pause bit.
     *
     * @return  A {@link Boolean} instance which represents the value of
     *          pause bit.
     */
    public Boolean isPause() {
        return pause;
    }

    /**
     * Return the current value if pause-asym bit.
     *
     * @return  A {@link Boolean} instance which represents the value of
     *          pause-asym bit.
     */
    public Boolean isPauseAsym() {
        return pauseAsym;
    }

    /**
     * Set the value of ten-mb-hd bit.
     *
     * @param b  A value to be set to ten-mb-hd bit.
     * @return  This instance.
     */
    public PortFeaturesBuilder setTenMbHd(Boolean b) {
        tenMbHd = b;
        return this;
    }

    /**
     * Set the value of ten-mb-fd bit.
     *
     * @param b  A value to be set to ten-mb-fd bit.
     * @return  This instance.
     */
    public PortFeaturesBuilder setTenMbFd(Boolean b) {
        tenMbFd = b;
        return this;
    }

    /**
     * Set the value of hundred-mb-hd bit.
     *
     * @param b  A value to be set to hundred-mb-hd bit.
     * @return  This instance.
     */
    public PortFeaturesBuilder setHundredMbHd(Boolean b) {
        hundredMbHd = b;
        return this;
    }

    /**
     * Set the value of hundred-mb-fd bit.
     *
     * @param b  A value to be set to hundred-mb-fd bit.
     * @return  This instance.
     */
    public PortFeaturesBuilder setHundredMbFd(Boolean b) {
        hundredMbFd = b;
        return this;
    }


    /**
     * Set the value of one-gb-hd bit.
     *
     * @param b  A value to be set to one-gb-hd bit.
     * @return  This instance.
     */
    public PortFeaturesBuilder setOneGbHd(Boolean b) {
        oneGbHd = b;
        return this;
    }

    /**
     * Set the value of one-gb-fd bit.
     *
     * @param b  A value to be set to one-gb-fd bit.
     * @return  This instance.
     */
    public PortFeaturesBuilder setOneGbFd(Boolean b) {
        oneGbFd = b;
        return this;
    }

    /**
     * Set the value of ten-gb-fd bit.
     *
     * @param b  A value to be set to ten-gb-fd bit.
     * @return  This instance.
     */
    public PortFeaturesBuilder setTenGbFd(Boolean b) {
        tenGbFd = b;
        return this;
    }

    /**
     * Set the value of forty-gb-fd bit.
     *
     * @param b  A value to be set to forty-gb-fd bit.
     * @return  This instance.
     */
    public PortFeaturesBuilder setFortyGbFd(Boolean b) {
        fortyGbFd = b;
        return this;
    }

    /**
     * Set the value of hundred-gb-fd bit.
     *
     * @param b  A value to be set to hundred-gb-fd bit.
     * @return  This instance.
     */
    public PortFeaturesBuilder setHundredGbFd(Boolean b) {
        hundredGbFd = b;
        return this;
    }

    /**
     * Set the value of one-tb-fd bit.
     *
     * @param b  A value to be set to one-tb-fd bit.
     * @return  This instance.
     */
    public PortFeaturesBuilder setOneTbFd(Boolean b) {
        oneTbFd = b;
        return this;
    }

    /**
     * Set the value of other bit.
     *
     * @param b  A value to be set to other bit.
     * @return  This instance.
     */
    public PortFeaturesBuilder setOther(Boolean b) {
        other = b;
        return this;
    }

    /**
     * Set the value of copper bit.
     *
     * @param b  A value to be set to copper bit.
     * @return  This instance.
     */
    public PortFeaturesBuilder setCopper(Boolean b) {
        copper = b;
        return this;
    }

    /**
     * Set the value of fiber bit.
     *
     * @param b  A value to be set to fiber bit.
     * @return  This instance.
     */
    public PortFeaturesBuilder setFiber(Boolean b) {
        fiber = b;
        return this;
    }

    /**
     * Set the value of autoeng bit.
     *
     * @param b  A value to be set to autoeng bit.
     * @return  This instance.
     */
    public PortFeaturesBuilder setAutoeng(Boolean b) {
        autoeng = b;
        return this;
    }

    /**
     * Set the value of pause bit.
     *
     * @param b  A value to be set to pause bit.
     * @return  This instance.
     */
    public PortFeaturesBuilder setPause(Boolean b) {
        pause = b;
        return this;
    }

    /**
     * Set the value of pause-asym bit.
     *
     * @param b  A value to be set to pause-asym bit.
     * @return  This instance.
     */
    public PortFeaturesBuilder setPauseAsym(Boolean b) {
        pauseAsym = b;
        return this;
    }

    /**
     * Construct a new {@link PortFeatures} instance with specifying
     * a set of values configured in this instance.
     *
     * @return  A {@link PortFeatures} instance.
     */
    public PortFeatures build() {
        return new PortFeatures(autoeng, copper, fiber, fortyGbFd, hundredGbFd,
                                hundredMbFd, hundredMbHd, oneGbFd, oneGbHd,
                                oneTbFd, other, pause, pauseAsym, tenGbFd,
                                tenMbFd, tenMbHd);
    }
}
