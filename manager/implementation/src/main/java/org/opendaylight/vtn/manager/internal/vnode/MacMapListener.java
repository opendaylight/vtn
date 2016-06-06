/*
 * Copyright (c) 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.vnode;

import static org.opendaylight.vtn.manager.internal.vnode.VTenantManager.LOG;

import java.util.Objects;

import org.opendaylight.vtn.manager.internal.util.ChangedData;
import org.opendaylight.vtn.manager.internal.util.IdentifiedData;
import org.opendaylight.vtn.manager.internal.util.MiscUtils;
import org.opendaylight.vtn.manager.internal.util.vnode.VNodeIdentifier;

import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.mac.rev150907.vtn.mac.map.config.AllowedHosts;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.mac.rev150907.vtn.mac.map.status.MappedHost;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.mac.rev150907.vtn.mac.mappable.MacMap;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateType;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.vlan.host.desc.set.VlanHostDescList;

/**
 * Data change listener that logs changes of MAC mapping configuration.
 */
final class MacMapListener extends VNodeChangeListener<MacMap> {
    /**
     * Return a string that indicates the change of value.
     *
     * @param old    The old value.
     * @param value  The current value.
     * @param <T>    The type of value.
     * @return  A string that indicates the change of value.
     */
    private static <T> String getChangedMessage(T old, T value) {
        return (old.equals(value))
            ? old.toString()
            : "(" + old + "->" + value + ")";
    }

    /**
     * Data change listener for mapped-host.
     */
    static final class MappedHostListener
        extends VNodeChangeListener<MappedHost> {
        /**
         * Construct a new instance.
         */
        MappedHostListener() {
            super(MappedHost.class, false);
        }

        /**
         * Log the given host information mapped by MAC mapping.
         *
         * @param data  A {@link IdentifiedData} instance.
         * @param msg   A message to be logged.
         */
        private void onChanged(IdentifiedData<?> data, String msg) {
            if (LOG.isInfoEnabled()) {
                IdentifiedData<MappedHost> cdata = cast(data);
                MappedHost host = cdata.getValue();
                VNodeIdentifier<?> ident =
                    getVNodeIdentifier(cdata.getIdentifier());
                LOG.info("{}: A host has been {} MAC mapping: addr={}, " +
                         "vlan-id={}, port={}", ident, msg,
                         host.getMacAddress().getValue(),
                         host.getVlanId().getValue(),
                         MiscUtils.getValue(host.getPortId()));
            }
        }

        // VNodeChangeListener

        /**
         * {@inheritDoc}
         */
        @Override
        void onCreated(VTenantChange ectx, IdentifiedData<?> data) {
            onChanged(data, "registered to");
        }

        /**
         * {@inheritDoc}
         */
        @Override
        void onUpdated(VTenantChange ectx, ChangedData<?> data) {
            ChangedData<MappedHost> cdata = cast(data);
            VNodeIdentifier<?> ident =
                getVNodeIdentifier(cdata.getIdentifier());
            MappedHost host = cdata.getValue();
            MappedHost old = cdata.getOldValue();
            String vidMsg = getChangedMessage(
                old.getVlanId().getValue(), host.getVlanId().getValue());
            String portMsg = getChangedMessage(
                MiscUtils.getValue(old.getPortId()),
                MiscUtils.getValue(host.getPortId()));
            LOG.info("{}: A MAC mapped host has been changed: " +
                     "addr={}, vlan-id={}, port={}", ident,
                     host.getMacAddress().getValue(), vidMsg, portMsg);
        }

        /**
         * {@inheritDoc}
         */
        @Override
        void onRemoved(VTenantChange ectx, IdentifiedData<?> data) {
            onChanged(data, "unregistered from");
        }

        /**
         * Determine whether the specified virtual node was updated or not.
         *
         * @param data  A {@link ChangedData} instance that contains values
         *              before and after modification.
         * @return  {@code true} if the target data was updated.
         *          {@code false} otherwise.
         */
        @Override
        boolean isUpdated(ChangedData<?> data) {
            boolean changed;
            if (LOG.isInfoEnabled()) {
                ChangedData<MappedHost> cdata = cast(data);
                MappedHost old = cdata.getOldValue();
                MappedHost host = cdata.getValue();
                changed = !(Objects.equals(old.getMacAddress(),
                                           host.getMacAddress()) &&
                            Objects.equals(old.getPortId(),
                                           host.getPortId()) &&
                            Objects.equals(old.getVlanId(), host.getVlanId()));
            } else {
                // No need to call onUpdated() if info log is disabled.
                changed = false;
            }

            return changed;
        }
    }

    /**
     * Data change listener for vlan-host-desc-list in the mac-map-config.
     */
    static final class VlanHostDescListListener
        extends VNodeChangeListener<VlanHostDescList> {
        /**
         * Construct a new instance.
         */
        VlanHostDescListListener() {
            super(VlanHostDescList.class, true);
        }

        /**
         * Invoked when a host informatiion has been added to or removed from
         * the access control list in the MAC mapping configuration.
         *
         * @param data  An {@link IdentifiedData} instance which contains
         *              added or removed data.
         * @param msg   A string used to construct a log message.
         */
        private void onChanged(IdentifiedData<?> data, String msg) {
            // Determine the name of the access control list.
            IdentifiedData<VlanHostDescList> cdata = cast(data);
            InstanceIdentifier<VlanHostDescList> path = cdata.getIdentifier();
            InstanceIdentifier<AllowedHosts> cpath =
                path.firstIdentifierOf(AllowedHosts.class);
            String acl = (cpath == null) ? "denied-hosts" : "allowed-hosts";

            LOG.info("{}: A host has been {} {}: {}",
                     getVNodeIdentifier(path), msg, acl,
                     cdata.getValue().getHost().getValue());
        }

        // VNodeChangeListener

        /**
         * {@inheritDoc}
         */
        @Override
        void onCreated(VTenantChange ectx, IdentifiedData<?> data) {
            if (LOG.isInfoEnabled()) {
                onChanged(data, "added to");
            }
        }

        /**
         * {@inheritDoc}
         */
        @Override
        void onRemoved(VTenantChange ectx, IdentifiedData<?> data) {
            if (LOG.isInfoEnabled()) {
                onChanged(data, "removed from");
            }
        }
    }

    /**
     * Construct a new instance.
     */
    MacMapListener() {
        super(MacMap.class, true);
    }

    /**
     * Invoked when the MAC mapping has been created or removed.
     *
     * @param data  An {@link IdentifiedData} instance which contains
     *              added or removed data.
     * @param type  {@link VtnUpdateType#CREATED} on added,
     *              {@link VtnUpdateType#REMOVED} on removed.
     */
    private void onChanged(IdentifiedData<?> data, VtnUpdateType type) {
        if (LOG.isInfoEnabled()) {
            IdentifiedData<MacMap> cdata = cast(data);
            InstanceIdentifier<MacMap> path = cdata.getIdentifier();
            LOG.info("{}: MAC mapping has been {}.", getVNodeIdentifier(path),
                     MiscUtils.toLowerCase(type));
        }
    }

    // VNodeChangeListener

    /**
     * {@inheritDoc}
     */
    @Override
    void onCreated(VTenantChange ectx, IdentifiedData<?> data) {
        onChanged(data, VtnUpdateType.CREATED);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    void onRemoved(VTenantChange ectx, IdentifiedData<?> data) {
        onChanged(data, VtnUpdateType.REMOVED);
    }

    /**
     * Determine whether the tatget virtual node should be treated as a
     * leaf node or not.
     *
     * <p>
     *   This method always returns {@code false} because mac-map
     *   contains children such as vlan-host-desc-list.
     * </p>
     *
     * @return  {@code false}.
     */
    @Override
    boolean isLeafNode() {
        return false;
    }
}
