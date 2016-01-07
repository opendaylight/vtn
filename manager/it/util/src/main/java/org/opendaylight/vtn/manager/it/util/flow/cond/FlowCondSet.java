/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.it.util.flow.cond;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;

import static org.opendaylight.vtn.manager.it.util.ModelDrivenTestBase.getRpcOutput;

import java.util.Collection;
import java.util.Collections;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;

import com.google.common.base.Optional;

import org.opendaylight.vtn.manager.it.ofmock.DataStoreUtils;
import org.opendaylight.vtn.manager.it.util.VTNServices;

import org.opendaylight.controller.md.sal.binding.api.ReadTransaction;

import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.RemoveFlowConditionInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.RemoveFlowConditionInputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.VtnFlowConditionService;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.VtnFlowConditions;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.vtn.flow.conditions.VtnFlowCondition;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateOperationType;

/**
 * {@code FlowCondSet} describes a set of flow conditions.
 */
public final class FlowCondSet implements Cloneable {
    /**
     * A map that keeps flow conditions.
     */
    private Map<String, FlowCondition>  flowConditions = new HashMap<>();

    /**
     * Remove the specified flow condition.
     *
     * @param service  The vtn-flow-condition service.
     * @param name     The name of the flow condition to be removed.
     */
    public static void removeFlowCondition(VtnFlowConditionService service,
                                           String name) {
        RemoveFlowConditionInput input = new RemoveFlowConditionInputBuilder().
            setName(name).
            build();
        getRpcOutput(service.removeFlowCondition(input), true);
    }

    /**
     * Add the given flow conditions.
     *
     * @param fcs  An array of flow conditions to be added.
     * @return  This instance.
     */
    public FlowCondSet add(FlowCondition ... fcs) {
        for (FlowCondition fc: fcs) {
            String name = fc.getName();
            flowConditions.put(name, fc);
        }

        return this;
    }

    /**
     * Remove the flow conditions specified by the given names.
     *
     * @param names  An array of flow conditions  to be removed.
     * @return  This instance.
     */
    public FlowCondSet remove(String ... names) {
        for (String name: names) {
            flowConditions.remove(name);
        }

        return this;
    }

    /**
     * Remove all the flow conditions.
     *
     * @return  This instance.
     */
    public FlowCondSet clear() {
        flowConditions.clear();
        return this;
    }

    /**
     * Return the flow condition specified by the given index.
     *
     * @param name  The name of the flow condition.
     * @return  A {@link FlowCondition} instance if found.
     *          {@code null} if not found.
     */
    public FlowCondition get(String name) {
        return flowConditions.get(name);
    }

    /**
     * Return an unmodifiable collection of flow condition configurations.
     *
     * @return  An unmodifiable collection of {@link FlowCondition} instances.
     */
    public Collection<FlowCondition> getFlowConditions() {
        return Collections.unmodifiableCollection(flowConditions.values());
    }

    /**
     * Verify the vtn-flow-conditions container.
     *
     * @param rtx  A read-only MD-SAL datastore transaction.
     */
    public void verify(ReadTransaction rtx) {
        List<VtnFlowCondition> vfcs = readFlowConditions(rtx).
            getVtnFlowCondition();
        if (!flowConditions.isEmpty()) {
            assertNotNull("vtn-flow-conditions is empty", vfcs);
            Set<String> checked = new HashSet<>();
            for (VtnFlowCondition vfc: vfcs) {
                String name = vfc.getName().getValue();
                FlowCondition fc = flowConditions.get(name);
                assertNotNull("Flow condition not found: " + name, fc);
                fc.verify(vfc);
                assertEquals(true, checked.add(name));
            }
            assertEquals(checked, flowConditions.keySet());
        } else if (vfcs != null) {
            assertEquals(Collections.<VtnFlowCondition>emptyList(), vfcs);
        }
    }

    /**
     * Apply all the configurations in this instance.
     *
     * @param service  A {@link VTNServices} instance.
     * @param rtx      A read-only MD-SAL datastore transaction.
     */
    public void apply(VTNServices service, ReadTransaction rtx) {
        VtnFlowConditionService vfmSrv = service.getFlowConditionService();
        removeUnwanted(vfmSrv, rtx);

        for (FlowCondition fc: flowConditions.values()) {
            fc.update(vfmSrv, VtnUpdateOperationType.SET, false);
        }
    }

    /**
     * Read the vtn-flow-conditions container.
     *
     * @param rtx  A read-only MD-SAL datastore transaction.
     * @return  A {@link VtnFlowConditions} instance.
     */
    private VtnFlowConditions readFlowConditions(ReadTransaction rtx) {
        InstanceIdentifier<VtnFlowConditions> path = InstanceIdentifier.
            create(VtnFlowConditions.class);
        Optional<VtnFlowConditions> opt = DataStoreUtils.read(rtx, path);
        assertEquals(true, opt.isPresent());

        return opt.get();
    }

    /**
     * Remove all the flow conditions that are not assocaited with the
     * configuration in this instance.
     *
     * @param service  The vtn-flow-condition RPC service.
     * @param rtx      A read-only MD-SAL datastore transaction.
     */
    private void removeUnwanted(
        VtnFlowConditionService service, ReadTransaction rtx) {
        List<VtnFlowCondition> vfcs = readFlowConditions(rtx).
            getVtnFlowCondition();
        if (vfcs != null) {
            for (VtnFlowCondition vfc: vfcs) {
                String name = vfc.getName().getValue();
                if (!flowConditions.containsKey(name)) {
                    removeFlowCondition(service, name);
                }
            }
        }
    }

    // Object

    /**
     * Create a shallow copy of this instance.
     *
     * @return  A shallow copy of this instance.
     */
    @Override
    public FlowCondSet clone() {
        try {
            FlowCondSet fcSet = (FlowCondSet)super.clone();
            fcSet.flowConditions = new HashMap<>(flowConditions);
            return fcSet;
        } catch (CloneNotSupportedException e) {
            // This should never happen.
            throw new IllegalStateException("clone() failed", e);
        }
    }
}
