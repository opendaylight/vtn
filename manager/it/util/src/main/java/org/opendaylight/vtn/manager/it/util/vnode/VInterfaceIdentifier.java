/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.it.util.vnode;

import java.util.Arrays;
import java.util.List;

import javax.xml.bind.annotation.XmlAccessType;
import javax.xml.bind.annotation.XmlAccessorType;
import javax.xml.bind.annotation.XmlRootElement;
import javax.xml.bind.annotation.XmlSeeAlso;

import org.opendaylight.yangtools.yang.binding.InstanceIdentifier.InstanceIdentifierBuilder;
import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.VtnFlowFilterList;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.vtn.redirect.filter.config.RedirectDestination;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.virtual.route.info.VirtualNodePath;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.virtual.route.info.VirtualNodePathBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeName;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.VtnPortMappableBridge;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.vtn.mappable.vinterface.VinterfaceStatus;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.vtn.mappable.vinterface.list.Vinterface;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.vtn.vinterface.info.VinterfaceInputFilter;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.vtn.vinterface.info.VinterfaceOutputFilter;

/**
 * {@code VInterfaceIdentifier} describes an identifier for a virtual
 * interface which can have port mapping configuration.
 *
 * @param <B>  The type of the virtual bridge.
 */
@XmlRootElement(name = "interface-path")
@XmlAccessorType(XmlAccessType.NONE)
@XmlSeeAlso({VBridgeIfIdentifier.class, VTerminalIfIdentifier.class})
public abstract class VInterfaceIdentifier<B extends VtnPortMappableBridge>
    extends TenantNodeIdentifier<Vinterface, B> {
    /**
     * A brief description about virtual interface.
     */
    public static final String  DESCRIPTION = "vInterface";

    /**
     * Cache for instance identifier that specifies the virtual bridge status.
     */
    private InstanceIdentifier<VinterfaceStatus>  statusPath;

    /**
     * Construct a new instance.
     *
     * @param tname  The name of the VTN.
     * @param bname  The name of the virtual bridge.
     * @param iname  The name of the virtual interface.
     */
    VInterfaceIdentifier(VnodeName tname, VnodeName bname, VnodeName iname) {
        super(tname, bname, iname);
    }

    /**
     * Construct a new instance.
     *
     * @param tname  The name of the VTN.
     * @param bname  The name of the virtual bridge.
     * @param iname  The name of the virtual interface.
     */
    VInterfaceIdentifier(String tname, String bname, String iname) {
        super(tname, bname, iname);
    }

    /**
     * Return the path to the virtual interface status.
     *
     * @return  An {@link InstanceIdentifier} instance for the virtual
     *          interface status.
     */
    public final InstanceIdentifier<VinterfaceStatus> getStatusPath() {
        InstanceIdentifier<VinterfaceStatus> path = statusPath;
        if (path == null) {
            path = getIdentifierBuilder().
                child(VinterfaceStatus.class).build();
            statusPath = path;
        }

        return path;
    }

    /**
     * Create a shallow copy of this instance, and replace the VTN name
     * with the given name.
     *
     * @param tname  The VTN name to be configured into a new instance.
     * @return  A constructed instance.
     */
    public abstract VInterfaceIdentifier replaceTenantName(VnodeName tname);

    /**
     * Convert this instance into a {@link RedirectDestination} instance.
     *
     * @return  A {@link RedirectDestination} instance.
     */
    public abstract RedirectDestination toRedirectDestination();

    // VNodeIdentifier

    /**
     * Determine whether the virtua interface specified by this instance
     * is equal to the virtual interface specified by {@link VirtualNodePath}
     * instance.
     *
     * @param vpath  A {@link VirtualNodePath} instance that specifies the
     *               virtual node. {@code null} cannot be specified.
     * @return  {@code true} if {@code vpath} specifies the same virtual
     *          interface as specified by this instance.
     *          {@code false} otherwise.
     */
    public final boolean contains(VirtualNodePath vpath) {
        String iname = vpath.getInterfaceName();
        return (iname != null && super.contains(vpath) &&
                iname.equals(getInterfaceNameString()));
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected final VirtualNodePathBuilder getVirtualNodePathBuilder() {
        return super.getVirtualNodePathBuilder().
            setInterfaceName(getInterfaceNameString());
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected InstanceIdentifierBuilder<? extends VtnFlowFilterList> getFlowFilterListIdentifierBuilder(
        boolean output) {
        InstanceIdentifierBuilder<Vinterface> builder =
            getIdentifierBuilder();
        return (output)
            ? builder.child(VinterfaceOutputFilter.class)
            : builder.child(VinterfaceInputFilter.class);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected final List<String> newComponents() {
        return Arrays.asList(getTenantNameString(), getBridgeNameString(),
                             getInterfaceNameString());
    }
}
