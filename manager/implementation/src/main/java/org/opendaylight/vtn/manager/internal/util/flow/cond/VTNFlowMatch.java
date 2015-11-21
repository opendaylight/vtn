/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.flow.cond;

import java.util.Objects;

import javax.xml.bind.annotation.XmlAccessType;
import javax.xml.bind.annotation.XmlAccessorType;
import javax.xml.bind.annotation.XmlElement;
import javax.xml.bind.annotation.XmlRootElement;
import javax.xml.bind.annotation.XmlSeeAlso;

import org.opendaylight.vtn.manager.flow.cond.FlowMatch;
import org.opendaylight.vtn.manager.util.VTNIdentifiable;

import org.opendaylight.vtn.manager.internal.util.flow.match.VTNEtherMatch;
import org.opendaylight.vtn.manager.internal.util.flow.match.VTNInetMatch;
import org.opendaylight.vtn.manager.internal.util.flow.match.VTNLayer4Match;
import org.opendaylight.vtn.manager.internal.util.flow.match.VTNMatch;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.VtnFlowMatchConfig;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.set.flow.condition.match.input.FlowMatchListBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.vtn.flow.cond.config.VtnFlowMatchBuilder;

/**
 * {@code VTNFlowMatch} describes the condition to match against packtes
 * in a flow condition.
 *
 * <p>
 *   Note that this class is not synchronized.
 * </p>
 */
@XmlRootElement(name = "vtn-flow-match")
@XmlAccessorType(XmlAccessType.NONE)
@XmlSeeAlso(VTNMatch.class)
public class VTNFlowMatch extends VTNMatch
    implements VTNIdentifiable<Integer> {
    /**
     * An index value which identifies flow match in a flow condition.
     */
    @XmlElement(required = true)
    private Integer  index;

    /**
     * Private constructor only for JAXB.
     */
    @SuppressWarnings("unused")
    private VTNFlowMatch() {
    }

    /**
     * Construct a new flow match from the given {@link FlowMatch} instance.
     *
     * @param fmatch  A {@link FlowMatch} instance.
     * @throws RpcException
     *    {@code fmatch} contains invalid value.
     */
    public VTNFlowMatch(FlowMatch fmatch) throws RpcException {
        if (fmatch == null) {
            throw RpcException.getNullArgumentException("Flow match");
        }

        index = fmatch.getIndex();
        FlowCondUtils.verifyMatchIndex(index);
        set(fmatch);
    }

    /**
     * Construct a new flow match from the given {@link VtnFlowMatchConfig}
     * instance.
     *
     * @param vfmatch  A {@link VtnFlowMatchConfig} instance.
     * @throws RpcException
     *    {@code vfmatch} contains invalid value.
     */
    public VTNFlowMatch(VtnFlowMatchConfig vfmatch) throws RpcException {
        if (vfmatch == null) {
            throw RpcException.getNullArgumentException("VTN flow match");
        }

        index = vfmatch.getIndex();
        FlowCondUtils.verifyMatchIndex(index);
        set(vfmatch);
    }

    /**
     * Return a {@link VtnFlowMatchBuilder} instance which contains the flow
     * conditions configured in this instance.
     *
     * @return  A {@link VtnFlowMatchBuilder} instance.
     */
    public VtnFlowMatchBuilder toVtnFlowMatchBuilder() {
        VtnFlowMatchBuilder builder = new VtnFlowMatchBuilder().
            setIndex(index);
        VTNEtherMatch eth = getEtherMatch();
        if (eth != null) {
            builder.setVtnEtherMatch(eth.toVtnEtherMatchBuilder().build());
            VTNInetMatch inet = getInetMatch();
            if (inet != null) {
                builder.setVtnInetMatch(inet.toVtnInetMatchBuilder().build());
                VTNLayer4Match l4 = getLayer4Match();
                if (l4 != null) {
                    builder.setVtnLayer4Match(l4.toVtnLayer4Match());
                }
            }
        }

        return builder;
    }

    /**
     * Return a {@link FlowMatchListBuilder} instance which contains the flow
     * conditions configured in this instance.
     *
     * @return  A {@link FlowMatchListBuilder} instance.
     */
    public FlowMatchListBuilder toFlowMatchListBuilder() {
        FlowMatchListBuilder builder = new FlowMatchListBuilder().
            setIndex(index);
        builder.fieldsFrom(toVtnFlowMatchBuilder().build());
        return builder;
    }

    // VTNMatch

    /**
     * Return a {@link FlowMatch} instance which represents this condition.
     *
     * @return  A {@link FlowMatch} instance.
     */
    @Override
    public FlowMatch toFlowMatch() {
        return toFlowMatch(index);
    }

    /**
     * Verify the contents of this instance.
     *
     * @throws RpcException  Verification failed.
     */
    @Override
    public void verify() throws RpcException {
        FlowCondUtils.verifyMatchIndex(index);
        super.verify();
    }

    // VTNIdentifiable

    /**
     * Return the index value assigned to this instance.
     *
     * @return  An {@link Integer} instance which represents the index value
     *          assigned to this instance. {@code null} if not assigned.
     */
    @Override
    public Integer getIdentifier() {
        return index;
    }

    // Object

    /**
     * Determine whether the given object is identical to this object.
     *
     * @param o  An object to be compared.
     * @return   {@code true} if identical. Otherwise {@code false}.
     */
    @Override
    public boolean equals(Object o) {
        if (o == this) {
            return true;
        }
        if (!super.equals(o)) {
            return false;
        }

        VTNFlowMatch vfm = (VTNFlowMatch)o;
        return Objects.equals(index, vfm.index);
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public int hashCode() {
        int h = super.hashCode();
        if (index != null) {
            h *= index.hashCode();
        }

        return h;
    }
}
