/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.it.util.flow.match;

import static org.junit.Assert.assertEquals;

import java.util.Objects;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.VtnMatchFields;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.vtn.match.fields.VtnEtherMatch;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.vtn.match.fields.VtnInetMatch;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.vtn.match.fields.VtnLayer4Match;

import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.inet.types.rev100924.IpVersion;

/**
 * {@code VTNMatch} describes the condition to match against packtes.
 */
public class VTNMatch implements Cloneable {
    /**
     * Conditions against Ethernet header.
     */
    private VTNEtherMatch  etherMatch;

    /**
     * Conditions against IP header.
     */
    private VTNInetMatch  inetMatch;

    /**
     * Conditions against layer 4 protocol header.
     */
    private VTNLayer4Match  layer4Match;

    /**
     * Create a new flow match that matches every packet.
     */
    public VTNMatch() {
    }

    /**
     * Construct a new flow match.
     *
     * @param ematch   A {@link VTNEtherMatch} instance which specifies the
     *                 condition for Ethernet header.
     * @param imatch   A {@link VTNInetMatch} instance which specifies the
     *                 condition for IP header.
     * @param l4match  A {@link VTNLayer4Match} instance which specifies the
     *                 condition for layer 4 protocol header.
     */
    public VTNMatch(VTNEtherMatch ematch, VTNInetMatch imatch,
                    VTNLayer4Match l4match) {
        etherMatch = ematch;
        inetMatch = imatch;
        layer4Match = l4match;
    }

    /**
     * Return the condition against Ethernet header.
     *
     * @return  A {@link VTNEtherMatch} instance.
     *          {@code null} if no condition is specified.
     */
    public final VTNEtherMatch getEtherMatch() {
        return etherMatch;
    }

    /**
     * Set the condition against Ethernet header.
     *
     * @param ematch  A {@link VTNEtherMatch} instance.
     * @return  This instance.
     */
    public VTNMatch setEtherMatch(VTNEtherMatch ematch) {
        etherMatch = ematch;
        return this;
    }

    /**
     * Return the condition against IP header.
     *
     * @return  A {@link VTNInetMatch} instance.
     *          {@code null} if no condition is specified.
     */
    public final VTNInetMatch getInetMatch() {
        return inetMatch;
    }

    /**
     * Return the condition against IP header.
     *
     * @param type  A class that specifies the expected type of match.
     * @param <T>   The type of match.
     * @return  A {@link VTNInetMatch} instance.
     *          {@code null} if no condition is specified.
     */
    public final <T extends VTNInetMatch> T getInetMatch(Class<T> type) {
        return type.cast(inetMatch);
    }

    /**
     * Set the condition against IP header.
     *
     * @param imatch  A {@link VTNInetMatch} instance.
     * @return  This instance.
     */
    public VTNMatch setInetMatch(VTNInetMatch imatch) {
        inetMatch = imatch;
        return this;
    }

    /**
     * Return the condition against layer 4 protocol header.
     *
     * @return  A {@link VTNLayer4Match} instance.
     *          {@code null} if no condition is specified.
     */
    public final VTNLayer4Match getLayer4Match() {
        return layer4Match;
    }

    /**
     * Return the condition against layer 4 protocol header.
     *
     * @param type  A class that specifies the expected type of match.
     * @param <T>   The type of match.
     * @return  A {@link VTNLayer4Match} instance.
     *          {@code null} if no condition is specified.
     */
    public final <T extends VTNLayer4Match> T getLayer4Match(Class<T> type) {
        return type.cast(layer4Match);
    }

    /**
     * Set the condition against layer 4 protocol header.
     *
     * @param l4match  A {@link VTNLayer4Match} instance.
     * @return  This instance.
     */
    public VTNMatch setLayer4Match(VTNLayer4Match l4match) {
        layer4Match = l4match;
        return this;
    }

    /**
     * Complete match conditions.
     *
     * @return  A {@link VTNMatch} instance that contains completed
     *          match conditions.
     */
    public final VTNMatch complete() {
        VTNEtherMatch ematch = etherMatch;
        VTNInetMatch imatch = inetMatch;
        VTNLayer4Match l4match = layer4Match;

        if (l4match != null) {
            // IP protocol number must be specified.
            // Defaults to IPv4 if IP match is not specified.
            if (imatch == null) {
                Short proto = l4match.getInetProtocol(IpVersion.Ipv4);
                imatch = new VTNInet4Match(proto);
            } else {
                Short proto = l4match.getInetProtocol(imatch.getIpVersion());
                imatch = imatch.complete(proto);
            }

            if (l4match.isEmpty()) {
                l4match = null;
            }
        }

        if (imatch != null) {
            // Ethernet type must be specified.
            Integer etype = imatch.getEtherType();
            if (ematch == null) {
                ematch = new VTNEtherMatch(etype);
            } else {
                ematch = ematch.complete(etype);
            }

            if (imatch.isEmpty()) {
                imatch = null;
            }
        }

        if (ematch != null && ematch.isEmpty()) {
            ematch = null;
        }

        return newIfChanged(ematch, imatch, l4match);
    }

    /**
     * Verify the given match condition.
     *
     * @param vmf  A {@link VtnMatchFields} instance.
     */
    public final void verifyMatch(VtnMatchFields vmf) {
        VTNMatch vmatch = complete();
        VtnEtherMatch vematch = vmf.getVtnEtherMatch();
        VtnInetMatch vimatch = vmf.getVtnInetMatch();
        VtnLayer4Match vl4match = vmf.getVtnLayer4Match();

        VTNEtherMatch ematch = vmatch.etherMatch;
        VTNInetMatch imatch = vmatch.inetMatch;
        VTNLayer4Match l4match = vmatch.layer4Match;

        if (ematch == null) {
            assertEquals(null, vematch);
        } else {
            ematch.verify(vematch);
        }

        if (imatch == null) {
            assertEquals(null, vimatch);
        } else {
            imatch.verify(vimatch);
        }

        if (l4match == null) {
            assertEquals(null, vl4match);
        } else {
            l4match.verify(vl4match);
        }
    }

    /**
     * Construct a new instance only if the given match conditions are
     * different from conditions in this instance.
     *
     * @param ematch   A {@link VTNEtherMatch} instance which specifies the
     *                 condition for Ethernet header.
     * @param imatch   A {@link VTNInetMatch} instance which specifies the
     *                 condition for IP header.
     * @param l4match  A {@link VTNLayer4Match} instance which specifies the
     *                 condition for layer 4 protocol header.
     * @return  A {@link VTNMatch} instance.
     */
    private VTNMatch newIfChanged(VTNEtherMatch ematch, VTNInetMatch imatch,
                                  VTNLayer4Match l4match) {
        VTNMatch vmatch;
        if (Objects.equals(ematch, etherMatch) &&
            Objects.equals(imatch, inetMatch) &&
            Objects.equals(l4match, layer4Match)) {
            vmatch = this;
        } else {
            vmatch = clone();
            vmatch.etherMatch = ematch;
            vmatch.inetMatch = imatch;
            vmatch.layer4Match = l4match;
        }

        return vmatch;
    }

    // Object

    /**
     * Create a copy of this instance.
     *
     * @return  A copy of this instance.
     */
    @Override
    public VTNMatch clone() {
        try {
            return (VTNMatch)super.clone();
        } catch (CloneNotSupportedException e) {
            throw new IllegalStateException("Unable to clone.", e);
        }
    }
}
