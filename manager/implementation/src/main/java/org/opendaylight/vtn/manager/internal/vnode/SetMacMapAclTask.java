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

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.mac.rev150907.SetMacMapAclInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.mac.rev150907.SetMacMapAclOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.mac.rev150907.SetMacMapAclOutputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VlanHostDesc;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnAclType;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateOperationType;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateType;

/**
 * {@code SetMacMapAclTask} describes the MD-SAL datastore transaction task
 * that configures the access control list in the MAC mapping.
 *
 * @see #create(SetMacMapAclInput)
 */
public final class SetMacMapAclTask extends UpdateMacMapTask
    implements RpcOutputGenerator<VtnUpdateType, SetMacMapAclOutput> {
    /**
     * A list of hosts to be added to or removed from the access control list.
     */
    private final List<VlanHostDesc>  hosts;

    /**
     * A {@link VtnAclType} instance that specifies the type of the access
     * control list.
     */
    private final VtnAclType  aclType;

    /**
     * Construct a new task that configures the access control list in the
     * MAC mapping.
     *
     * @param input  A {@link SetMacMapAclTask} instance.
     * @return  A {@link SetMacMapAclTask} instance associated with the task
     *          that configures the access control list in the MAC mapping.
     * @throws RpcException
     *     The given input contains invalid value.
     */
    public static SetMacMapAclTask create(SetMacMapAclInput input)
        throws RpcException {
        if (input == null) {
            throw RpcUtils.getNullInputException();
        }

        // Determine the target vBridge.
        VBridgeIdentifier vbrId = VBridgeIdentifier.create(input, true);

        return new SetMacMapAclTask(vbrId, input);
    }

    /**
     * Construct a new instance.
     *
     * @param vbrId  The identifier for the target vBridge.
     * @param input  A {@link SetMacMapAclTask} instance.
     */
    private SetMacMapAclTask(VBridgeIdentifier vbrId,
                             SetMacMapAclInput input) {
        super(vbrId, input.getOperation());
        hosts = input.getHosts();
        aclType = input.getAclType();
    }

    // UpdateMacMapTask

    /**
     * {@inheritDoc}
     */
    @Override
    protected MacMapChange updateConfig(
        VTNMacMapConfig cfg, VtnUpdateOperationType op) throws RpcException {
        return cfg.update(op, aclType, hosts);
    }

    // RpcOutputGenerator

    /**
     * {@inheritDoc}
     */
    @Override
    public Class<SetMacMapAclOutput> getOutputType() {
        return SetMacMapAclOutput.class;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public SetMacMapAclOutput createOutput(VtnUpdateType result) {
        return new SetMacMapAclOutputBuilder().setStatus(result).build();
    }
}
