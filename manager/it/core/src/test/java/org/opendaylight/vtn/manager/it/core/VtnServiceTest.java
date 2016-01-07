/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.it.core;

import java.util.Map.Entry;
import java.util.Random;

import org.opendaylight.vtn.manager.it.util.RpcErrorTag;
import org.opendaylight.vtn.manager.it.util.VirtualNetwork;
import org.opendaylight.vtn.manager.it.util.vnode.VTenantConfig;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.RemoveVtnInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.RemoveVtnInputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.UpdateVtnInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.UpdateVtnInputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.VtnService;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeUpdateMode;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnErrorTag;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateOperationType;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateType;

/**
 * Test case for {@link VtnService}.
 */
public final class VtnServiceTest extends TestMethodBase {
    /**
     * Construct a new instance.
     *
     * @param vit  A {@link VTNManagerIT} instance.
     */
    public VtnServiceTest(VTNManagerIT vit) {
        super(vit);
    }

    // TestMethodBase

    /**
     * Run the test.
     *
     * @throws Exception  An error occurred.
     */
    @Override
    protected void runTest() throws Exception {
        VTNManagerIT vit = getTest();
        VtnService vtnSrv = vit.getVtnService();
        VirtualNetwork vnet = getVirtualNetwork();

        // No VTN should be present.
        vnet.verify();

        // Create 3 VTNs.
        String tname1 = "vtn_1";
        VTenantConfig tconf1 = new VTenantConfig();
        vnet.addTenant(tname1, tconf1);
        assertEquals(VtnUpdateType.CREATED,
                     tconf1.update(vtnSrv, tname1, null, null));
        vnet.verify();

        String tname2 = "vtn_2";
        VTenantConfig tconf2 = new VTenantConfig("vtn-2", 0, null);
        vnet.addTenant(tname2, tconf2);
        assertEquals(VtnUpdateType.CREATED,
                     tconf2.update(vtnSrv, tname2, VnodeUpdateMode.CREATE,
                                   null));
        vnet.verify();

        String tname3 = "0123456789012345678901234567890";
        VTenantConfig tconf3 =
            new VTenantConfig("Virtual Tenant Network 3", 65534, 65535);
        vnet.addTenant(tname3, tconf3);
        assertEquals(VtnUpdateType.CREATED,
                     tconf3.update(vtnSrv, tname3, VnodeUpdateMode.UPDATE,
                                   null));
        vnet.verify();

        // Add random configuration.
        Random rand = new Random(0x12345L);
        for (VTenantConfig tconf: vnet.getTenants().values()) {
            tconf.add(rand);
        }

        vnet.apply().verify();

        // Try to update with the same parameter.
        for (Entry<String, VTenantConfig> entry:
                 vnet.getTenants().entrySet()) {
            String name = entry.getKey();
            VTenantConfig tconf = entry.getValue();
            for (VnodeUpdateMode mode: VNODE_UPDATE_MODES) {
                for (VtnUpdateOperationType op: MODIFY_OPERATIONS) {
                    assertEquals(null, tconf.update(vtnSrv, name, mode, op));
                }
            }
        }

        // Change parameters using ADD operation.
        tconf3.setIdleTimeout(12345);
        UpdateVtnInput input = new UpdateVtnInputBuilder().
            setTenantName(tname3).
            setIdleTimeout(tconf3.getIdleTimeout()).
            build();
        assertEquals(VtnUpdateType.CHANGED,
                     getRpcResult(vtnSrv.updateVtn(input)));
        vnet.verify();

        tconf2.setHardTimeout(60000);
        input = new UpdateVtnInputBuilder().
            setTenantName(tname2).
            setHardTimeout(tconf2.getHardTimeout()).
            setOperation(VtnUpdateOperationType.ADD).
            setUpdateMode(VnodeUpdateMode.MODIFY).
            build();
        assertEquals(VtnUpdateType.CHANGED,
                     getRpcResult(vtnSrv.updateVtn(input)));
        vnet.verify();

        tconf2.setDescription("test VTN 2");
        input = new UpdateVtnInputBuilder().
            setTenantName(tname2).
            setDescription(tconf2.getDescription()).
            setOperation(VtnUpdateOperationType.ADD).
            setUpdateMode(VnodeUpdateMode.UPDATE).
            build();
        assertEquals(VtnUpdateType.CHANGED,
                     getRpcResult(vtnSrv.updateVtn(input)));
        vnet.verify();

        // Change parameters using SET operation.
        tconf1.setDescription("test VTN 1").
            setIdleTimeout(1).
            setHardTimeout(2);
        assertEquals(VtnUpdateType.CHANGED,
                     tconf1.update(vtnSrv, tname1, null,
                                   VtnUpdateOperationType.SET));
        vnet.verify();

        tconf2.setDescription(null).
            setHardTimeout(33333);
        assertEquals(VtnUpdateType.CHANGED,
                     tconf2.update(vtnSrv, tname2, VnodeUpdateMode.UPDATE,
                                   VtnUpdateOperationType.SET));

        tconf3.setIdleTimeout(null).
            setHardTimeout(null);
        assertEquals(VtnUpdateType.CHANGED,
                     tconf3.update(vtnSrv, tname3, VnodeUpdateMode.MODIFY,
                                   VtnUpdateOperationType.SET));

        // Error tests.

        // Null input.
        checkRpcError(vtnSrv.updateVtn(null), RpcErrorTag.MISSING_ELEMENT,
                      VtnErrorTag.BADREQUEST);
        checkRpcError(vtnSrv.removeVtn(null), RpcErrorTag.MISSING_ELEMENT,
                      VtnErrorTag.BADREQUEST);

        // Null tenant-name.
        for (VnodeUpdateMode mode: VnodeUpdateMode.values()) {
            input = new UpdateVtnInputBuilder().
                setUpdateMode(mode).
                build();
            checkRpcError(vtnSrv.updateVtn(input),
                          RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);
        }

        RemoveVtnInput rinput = new RemoveVtnInputBuilder().build();
        checkRpcError(vtnSrv.removeVtn(rinput),
                      RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);

        // Invalid tenant-name.
        for (String name: INVALID_VNODE_NAMES) {
            for (VnodeUpdateMode mode: VnodeUpdateMode.values()) {
                input = new UpdateVtnInputBuilder().
                    setTenantName(name).
                    setUpdateMode(mode).
                    build();
                RpcErrorTag etag;
                VtnErrorTag vtag;
                if (mode == VnodeUpdateMode.MODIFY) {
                    etag = RpcErrorTag.DATA_MISSING;
                    vtag = VtnErrorTag.NOTFOUND;
                } else {
                    etag = RpcErrorTag.BAD_ELEMENT;
                    vtag = VtnErrorTag.BADREQUEST;
                }
                checkRpcError(vtnSrv.updateVtn(input), etag, vtag);
            }

            rinput = new RemoveVtnInputBuilder().
                setTenantName(name).build();
            checkRpcError(vtnSrv.removeVtn(rinput),
                          RpcErrorTag.DATA_MISSING, VtnErrorTag.NOTFOUND);
        }

        // Modifying VTN that is not present.
        String unknownName = "unknown";
        input = new UpdateVtnInputBuilder().
            setTenantName(unknownName).
            setUpdateMode(VnodeUpdateMode.MODIFY).
            build();
        checkRpcError(vtnSrv.updateVtn(input), RpcErrorTag.DATA_MISSING,
                      VtnErrorTag.NOTFOUND);

        // Removing VTN that is not present.
        rinput = new RemoveVtnInputBuilder().
            setTenantName(unknownName).
            build();
        checkRpcError(vtnSrv.removeVtn(rinput), RpcErrorTag.DATA_MISSING,
                      VtnErrorTag.NOTFOUND);

        // Invalid update operation.
        input = new UpdateVtnInputBuilder().
            setTenantName(tname1).
            setOperation(VtnUpdateOperationType.REMOVE).
            build();
        checkRpcError(vtnSrv.updateVtn(input), RpcErrorTag.BAD_ELEMENT,
                      VtnErrorTag.BADREQUEST);

        // Removing VTN that is not present.
        rinput = new RemoveVtnInputBuilder().
            setTenantName(unknownName).build();
        checkRpcError(vtnSrv.removeVtn(rinput), RpcErrorTag.DATA_MISSING,
                      VtnErrorTag.NOTFOUND);

        // Name confliction.
        for (String name: vnet.getTenants().keySet()) {
            input = new UpdateVtnInputBuilder().
                setTenantName(name).
                setUpdateMode(VnodeUpdateMode.CREATE).
                build();
            checkRpcError(vtnSrv.updateVtn(input), RpcErrorTag.DATA_EXISTS,
                          VtnErrorTag.CONFLICT);
        }

        // Inconsistent flow timeouts.
        input = new UpdateVtnInputBuilder().
            setTenantName(unknownName).
            setHardTimeout(VTenantConfig.DEFAULT_IDLE_TIMEOUT).
            build();
        checkRpcError(vtnSrv.updateVtn(input), RpcErrorTag.BAD_ELEMENT,
                      VtnErrorTag.BADREQUEST);

        input = new UpdateVtnInputBuilder().
            setTenantName(unknownName).
            setIdleTimeout(30000).
            setHardTimeout(29999).
            build();
        checkRpcError(vtnSrv.updateVtn(input), RpcErrorTag.BAD_ELEMENT,
                      VtnErrorTag.BADREQUEST);

        for (Entry<String, VTenantConfig> entry:
                 vnet.getTenants().entrySet()) {
            String name = entry.getKey();
            VTenantConfig tconf = entry.getValue().complete();
            Integer idle = tconf.getIdleTimeout();
            Integer hard = tconf.getHardTimeout();
            for (VnodeUpdateMode mode: VNODE_UPDATE_MODES) {
                if (hard.intValue() != 0) {
                    input = new UpdateVtnInputBuilder().
                        setTenantName(name).
                        setUpdateMode(mode).
                        setIdleTimeout(65535).
                        build();
                    checkRpcError(vtnSrv.updateVtn(input),
                                  RpcErrorTag.BAD_ELEMENT,
                                  VtnErrorTag.BADREQUEST);
                }
                if (idle.intValue() > 1) {
                    input = new UpdateVtnInputBuilder().
                        setTenantName(name).
                        setUpdateMode(mode).
                        setOperation(VtnUpdateOperationType.ADD).
                        setHardTimeout(2).
                        build();
                    checkRpcError(vtnSrv.updateVtn(input),
                                  RpcErrorTag.BAD_ELEMENT,
                                  VtnErrorTag.BADREQUEST);
                }

                input = new UpdateVtnInputBuilder().
                    setTenantName(name).
                    setUpdateMode(mode).
                    setOperation(VtnUpdateOperationType.SET).
                    setIdleTimeout(2).
                    setHardTimeout(1).
                    build();
                checkRpcError(vtnSrv.updateVtn(input),
                              RpcErrorTag.BAD_ELEMENT, VtnErrorTag.BADREQUEST);
            }
        }

        // Errors should never affect existing VTNs.
        vnet.verify();

        // Remove VTNs.
        String[] tenants = {tname2, tname1, tname3};
        for (String tname: tenants) {
            removeVtn(tname);
            vnet.removeTenant(tname).verify();
        }
    }
}
