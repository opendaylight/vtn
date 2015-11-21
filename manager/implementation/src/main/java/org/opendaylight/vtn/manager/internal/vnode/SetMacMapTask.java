/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.vnode;

import java.util.List;

import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcOutputGenerator;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcUtils;
import org.opendaylight.vtn.manager.internal.util.vnode.MacMapChange;
import org.opendaylight.vtn.manager.internal.util.vnode.VBridgeIdentifier;
import org.opendaylight.vtn.manager.internal.util.vnode.VTNMacMapConfig;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.mac.rev150907.SetMacMapInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.mac.rev150907.SetMacMapOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.mac.rev150907.SetMacMapOutputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VlanHostDesc;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateOperationType;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateType;

/**
 * {@code SetMacMapTask} describes the MD-SAL datastore transaction task
 * that configures a MAC mapping in the specified vBridge.
 *
 * @see #create(SetMacMapInput)
 */
public final class SetMacMapTask extends UpdateMacMapTask
    implements RpcOutputGenerator<VtnUpdateType, SetMacMapOutput> {
    /**
     * A list of hosts to be added to or removed from the allowed host set.
     */
    private final List<VlanHostDesc>  allowedHosts;

    /**
     * A list of hosts to be added to or removed from the denied host set.
     */
    private final List<VlanHostDesc>  deniedHosts;

    /**
     * Construct a new task that configures a MAC mapping in the specified
     * vBridge.
     *
     * @param input  A {@link SetMacMapTask} instance.
     * @return  A {@link SetMacMapTask} instance associated with the task
     *          that configures a MAC mapping in the specified vBridge.
     * @throws RpcException
     *     The given input contains invalid value.
     */
    public static SetMacMapTask create(SetMacMapInput input)
        throws RpcException {
        if (input == null) {
            throw RpcUtils.getNullInputException();
        }

        // Determine the target vBridge.
        VBridgeIdentifier vbrId = VBridgeIdentifier.create(input, true);

        return new SetMacMapTask(vbrId, input);
    }

    /**
     * Construct a new instance.
     *
     * @param vbrId  The identifier for the target vBridge.
     * @param input  A {@link SetMacMapTask} instance.
     */
    private SetMacMapTask(VBridgeIdentifier vbrId, SetMacMapInput input) {
        super(vbrId, input.getOperation());
        allowedHosts = input.getAllowedHosts();
        deniedHosts = input.getDeniedHosts();
    }

    // UpdateMacMapTask

    /**
     * {@inheritDoc}
     */
    @Override
    protected MacMapChange updateConfig(
        VTNMacMapConfig cfg, VtnUpdateOperationType op) throws RpcException {
        return cfg.update(op, allowedHosts, deniedHosts);
    }

    // RpcOutputGenerator

    /**
     * {@inheritDoc}
     */
    @Override
    public Class<SetMacMapOutput> getOutputType() {
        return SetMacMapOutput.class;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public SetMacMapOutput createOutput(VtnUpdateType result) {
        return new SetMacMapOutputBuilder().setStatus(result).build();
    }
}
