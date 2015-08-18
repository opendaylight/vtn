/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.flow.match;

import org.opendaylight.vtn.manager.flow.cond.L4Match;

import org.opendaylight.vtn.manager.internal.util.packet.Layer4Header;

import org.opendaylight.vtn.manager.internal.TestBase;
import org.opendaylight.vtn.manager.internal.XmlNode;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.vtn.match.fields.VtnLayer4Match;

import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.types.rev131026.flow.MatchBuilder;

/**
 * {@code Layer4MatchParams} describes parameters for condition to match
 * against layer 4 protocol header.
 *
 * @param <T>  The type of this instance.
 */
public abstract class Layer4MatchParams<T extends Layer4MatchParams>
    extends TestBase implements Layer4Header, Cloneable {
    /**
     * Construct a {@link VtnLayer4Match} instance.
     *
     * @return  A {@link VtnLayer4Match} instance that contains the settings
     *          configured in this instance.
     */
    public final VtnLayer4Match toVtnLayer4Match() {
        return toVtnLayer4Match(false);
    }

    /**
     * Construct a {@link L4Match} instance.
     *
     * @return  A {@link L4Match} instance that contains the settings
     *          configured in this instance.
     */
    public abstract L4Match toL4Match();

    /**
     * Construct a {@link VtnLayer4Match} instance.
     *
     * @param comp  Complete the settings if {@code true}.
     * @return  A {@link VtnLayer4Match} instance that contains the settings
     *          configured in this instance.
     */
    public abstract VtnLayer4Match toVtnLayer4Match(boolean comp);

    /**
     * Construct a {@link VTNLayer4Match} instance.
     *
     * @return  A {@link VTNLayer4Match} instance that contains the settings
     *          configured in this instance.
     * @throws Exception  An error occurred.
     */
    public abstract VTNLayer4Match toVTNLayer4Match() throws Exception;

    /**
     * Return a {@link XmlNode} instance which represents this instance.
     *
     * <p>
     *   This method returns a {@link XmlNode} to be configured in a
     *   {@link VTNMatch} instance.
     * </p>
     *
     * @return  A {@link XmlNode} instance.
     */
    public abstract XmlNode toXmlNode();

    /**
     * Return a {@link XmlNode} instance which represents this instance.
     *
     * @param name  The name of the root node.
     * @return  A {@link XmlNode} instance.
     */
    public abstract XmlNode toXmlNode(String name);

    /**
     * Determine whether this condition is empty or not.
     *
     * @return  {@code true} only if this instance does not contain any
     *          condition.
     */
    public abstract boolean isEmpty();

    /**
     * Ensure that the given {@link VTNLayer4Match} instance contains the same
     * conditions as this instance.
     *
     * @param l4m  A {@link VTNLayer4Match} instance.
     * @return  The given instance.
     */
    public abstract VTNLayer4Match verifyValues(VTNLayer4Match l4m);

    /**
     * Ensure that the given {@link VTNLayer4Match} instance contains the same
     * conditions as this instance.
     *
     * @param l4m  A {@link VTNLayer4Match} instance.
     * @throws Exception  An errror occurred.
     */
    public abstract void verify(VTNLayer4Match l4m) throws Exception;

    /**
     * Ensure that the given {@link MatchBuilder} instance contains the same
     * conditions as this instance.
     *
     * @param builder  A {@link MatchBuilder} instance.
     * @return  {@code true} only if the given match contains the condition
     *          for the layer4 protocol header corresponding to this instance.
     */
    public abstract boolean verify(MatchBuilder builder);

    /**
     * Ensure that the given {@link VTNLayer4Match} instance created from the
     * MD-SAL match contains the same conditions as this instance.
     *
     * @param l4m  A {@link VTNLayer4Match} instance created from the MD-SAL
     *             match.
     * @throws Exception  An errror occurred.
     */
    public abstract void verifyMd(VTNLayer4Match l4m) throws Exception;

    /**
     * Return a class for the type of this instance.
     *
     * @return  A class for the type of this instance.
     */
    protected abstract Class<T> getMatchType();

    // Layer4Header

    /**
     * {@inheritDoc}
     */
    @Override
    public final void setDescription(StringBuilder builder) {
    }

    // Cloneable

    /**
     * Return a deep copy of this instance.
     *
     * @return  A deep copy of this instance.
     */
    @Override
    public Layer4MatchParams clone() {
        try {
            return (Layer4MatchParams)super.clone();
        } catch (CloneNotSupportedException e) {
            // This should never happen.
            throw new IllegalStateException("clone() failed.", e);
        }
    }
}
