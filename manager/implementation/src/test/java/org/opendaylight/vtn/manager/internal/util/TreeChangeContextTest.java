/*
 * Copyright (c) 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util;

import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.when;

import java.util.Deque;
import java.util.Iterator;
import java.util.LinkedList;

import org.junit.Test;

import org.opendaylight.vtn.manager.internal.TestBase;

import org.opendaylight.controller.md.sal.binding.api.DataObjectModification;

import org.opendaylight.yangtools.yang.binding.InstanceIdentifier.PathArgument;
import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.flow.action.list.VtnFlowAction;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.flow.action.list.VtnFlowActionKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.vtn.flow.filter.list.VtnFlowFilter;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.vtn.flow.filter.list.VtnFlowFilterKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.Vtns;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.vtns.Vtn;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.vtns.VtnKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeName;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.vtn.vbridge.list.Vbridge;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.vtn.vbridge.list.VbridgeKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.vtn.mappable.vinterface.list.Vinterface;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.vtn.mappable.vinterface.list.VinterfaceKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.vtn.vinterface.info.VinterfaceInputFilter;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.vtn.vinterface.info.VinterfaceOutputFilter;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vterminal.rev150907.vtn.vterminal.list.Vterminal;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vterminal.rev150907.vtn.vterminal.list.VterminalKey;

/**
 * JUnit test for {@link TreeChangeContext}.
 *
 */
public class TreeChangeContextTest extends TestBase {
    /**
     * Test case for {@link TreeChangeContext#getRootPath()} and
     * {@link TreeChangeContext#setRootPath(InstanceIdentifier)}.
     */
    @Test
    public void testRootPath() {
        // Set the path to the VTN as the root path.
        TreeChangeContext<Vtn> ctx = new TreeChangeContext<>();
        VnodeName vtnName = new VnodeName("vtn_1");
        InstanceIdentifier<Vtn> vtnPath = InstanceIdentifier.
            builder(Vtns.class).
            child(Vtn.class, new VtnKey(vtnName)).
            build();
        ctx.setRootPath(vtnPath);
        assertSame(vtnPath, ctx.getRootPath());
        assertSame(vtnPath, ctx.getPath());

        // Push path arguments.
        VnodeName vbrName = new VnodeName("bridge_1");
        VnodeName ifName = new VnodeName("if_1");
        InstanceIdentifier<Vinterface> ifPath = vtnPath.builder().
            child(Vbridge.class, new VbridgeKey(vbrName)).
            child(Vinterface.class, new VinterfaceKey(ifName)).
            build();
        Iterator<PathArgument> it = ifPath.getPathArguments().iterator();
        while (it.hasNext()) {
            PathArgument arg = it.next();
            if (arg.getType().equals(Vtn.class)) {
                break;
            }
        }
        assertEquals(true, it.hasNext());
        while (it.hasNext()) {
            push(ctx, it.next());
        }
        assertEquals(ifPath, ctx.getPath());

        // Change the root path to another VTN path.
        vtnName = new VnodeName("vtn_2");
        vtnPath = InstanceIdentifier.
            builder(Vtns.class).
            child(Vtn.class, new VtnKey(vtnName)).
            build();
        ctx.setRootPath(vtnPath);
        assertSame(vtnPath, ctx.getRootPath());

        // Tree path stack must be cleared.
        assertSame(vtnPath, ctx.getPath());
    }

    /**
     * Test case for changing tree path.
     *
     * <ul>
     *   <li>{@link TreeChangeContext#push(DataObjectModification)}</li>
     *   <li>{@link TreeChangeContext#pop()}</li>
     *   <li>{@link TreeChangeContext#getPath()}</li>
     * </ul>
     *
     * <p>
     *   This test calls {@link TreeChangeContext#getPath()} only if the
     *   tree stack is empty.
     * </p>
     */
    @Test
    public void testPath1() {
        TreeChangeContext<Vtn> ctx = new TreeChangeContext<>();
        VnodeName vtnName = new VnodeName("vtn_1");
        InstanceIdentifier<Vtn> vtnPath = InstanceIdentifier.
            builder(Vtns.class).
            child(Vtn.class, new VtnKey(vtnName)).
            build();
        ctx.setRootPath(vtnPath);

        VnodeName vbrName = new VnodeName("bridge_1");
        VnodeName ifName = new VnodeName("if_1");
        VtnFlowFilterKey filterKey = new VtnFlowFilterKey(123);
        VtnFlowActionKey actKey = new VtnFlowActionKey(456);
        InstanceIdentifier<VtnFlowAction> actPath = vtnPath.builder().
            child(Vbridge.class, new VbridgeKey(vbrName)).
            child(Vinterface.class, new VinterfaceKey(ifName)).
            child(VinterfaceInputFilter.class).
            child(VtnFlowFilter.class, filterKey).
            child(VtnFlowAction.class, actKey).
            build();
        Iterator<PathArgument> it = actPath.getPathArguments().iterator();
        while (it.hasNext()) {
            PathArgument arg = it.next();
            if (arg.getType().equals(Vtn.class)) {
                break;
            }
        }
        assertEquals(true, it.hasNext());

        int count = 0;
        while (it.hasNext()) {
            push(ctx, it.next());
            count++;
        }
        assertEquals(5, count);
        while (count > 0) {
            ctx.pop();
            count--;
        }

        // The context must point the root node.
        assertSame(vtnPath, ctx.getPath());
    }

    /**
     * Test case for changing tree path.
     *
     * <ul>
     *   <li>{@link TreeChangeContext#push(DataObjectModification)}</li>
     *   <li>{@link TreeChangeContext#pop()}</li>
     *   <li>{@link TreeChangeContext#getPath()}</li>
     * </ul>
     *
     * <p>
     *   This test doesn't call {@link TreeChangeContext#getPath()} until
     *   all the path arguments are pushed to the context.
     * </p>
     */
    @Test
    public void testPath2() {
        TreeChangeContext<Vtn> ctx = new TreeChangeContext<>();
        VnodeName vtnName = new VnodeName("vtn_1");
        InstanceIdentifier<Vtn> vtnPath = InstanceIdentifier.
            builder(Vtns.class).
            child(Vtn.class, new VtnKey(vtnName)).
            build();
        ctx.setRootPath(vtnPath);

        // Push path arguments for vtn-flow-action path.
        Deque<InstanceIdentifier<?>> pathStack = new LinkedList<>();
        VnodeName vbrName = new VnodeName("bridge_1");
        InstanceIdentifier<Vbridge> vbrPath = vtnPath.
            child(Vbridge.class, new VbridgeKey(vbrName));
        push(ctx, getLastPathArgument(vbrPath));
        pathStack.addLast(vbrPath);

        VnodeName ifName = new VnodeName("if_1");
        InstanceIdentifier<Vinterface> ifPath = vbrPath.
            child(Vinterface.class, new VinterfaceKey(ifName));
        push(ctx, getLastPathArgument(ifPath));
        pathStack.addLast(ifPath);

        InstanceIdentifier<VinterfaceOutputFilter> flistPath = ifPath.
            child(VinterfaceOutputFilter.class);
        push(ctx, getLastPathArgument(flistPath));
        pathStack.addLast(flistPath);

        VtnFlowFilterKey filterKey = new VtnFlowFilterKey(3333);
        InstanceIdentifier<VtnFlowFilter> filterPath = flistPath.
            child(VtnFlowFilter.class, filterKey);
        push(ctx, getLastPathArgument(filterPath));
        pathStack.addLast(filterPath);

        VtnFlowActionKey actKey = new VtnFlowActionKey(1);
        InstanceIdentifier<VtnFlowAction> actPath = filterPath.
            child(VtnFlowAction.class, actKey);
        push(ctx, getLastPathArgument(actPath));
        pathStack.addLast(actPath);

        // Pop path arguments with getting node path.
        while (!pathStack.isEmpty()) {
            assertEquals(pathStack.removeLast(), ctx.getPath());
            ctx.pop();
        }

        // The context must point the root node.
        assertSame(vtnPath, ctx.getPath());
    }

    /**
     * Test case for changing tree path.
     *
     * <ul>
     *   <li>{@link TreeChangeContext#push(DataObjectModification)}</li>
     *   <li>{@link TreeChangeContext#pop()}</li>
     *   <li>{@link TreeChangeContext#getPath()}</li>
     * </ul>
     *
     * <p>
     *   This test calls {@link TreeChangeContext#getPath()} for each tree
     *   node.
     * </p>
     */
    @Test
    public void testPath3() {
        TreeChangeContext<Vtn> ctx = new TreeChangeContext<>();
        VnodeName vtnName = new VnodeName("vtn_1");
        InstanceIdentifier<Vtn> vtnPath = InstanceIdentifier.
            builder(Vtns.class).
            child(Vtn.class, new VtnKey(vtnName)).
            build();
        ctx.setRootPath(vtnPath);

        // Push path arguments for vtn-flow-action path.
        Deque<InstanceIdentifier<?>> pathStack = new LinkedList<>();
        VnodeName vtermName = new VnodeName("vterm_1");
        InstanceIdentifier<Vterminal> vtermPath = vtnPath.
            child(Vterminal.class, new VterminalKey(vtermName));
        push(ctx, getLastPathArgument(vtermPath));
        assertEquals(vtermPath, ctx.getPath());
        pathStack.addLast(vtermPath);

        VnodeName ifName = new VnodeName("if_1");
        InstanceIdentifier<Vinterface> ifPath = vtermPath.
            child(Vinterface.class, new VinterfaceKey(ifName));
        push(ctx, getLastPathArgument(ifPath));
        assertEquals(ifPath, ctx.getPath());
        pathStack.addLast(ifPath);

        InstanceIdentifier<VinterfaceInputFilter> flistPath = ifPath.
            child(VinterfaceInputFilter.class);
        push(ctx, getLastPathArgument(flistPath));
        assertEquals(flistPath, ctx.getPath());
        pathStack.addLast(flistPath);

        VtnFlowFilterKey filterKey = new VtnFlowFilterKey(3333);
        InstanceIdentifier<VtnFlowFilter> filterPath = flistPath.
            child(VtnFlowFilter.class, filterKey);
        push(ctx, getLastPathArgument(filterPath));
        assertEquals(filterPath, ctx.getPath());
        pathStack.addLast(filterPath);

        VtnFlowActionKey actKey = new VtnFlowActionKey(1);
        InstanceIdentifier<VtnFlowAction> actPath = filterPath.
            child(VtnFlowAction.class, actKey);
        push(ctx, getLastPathArgument(actPath));
        pathStack.addLast(actPath);

        // Pop path arguments with getting node path.
        while (!pathStack.isEmpty()) {
            assertEquals(pathStack.removeLast(), ctx.getPath());
            ctx.pop();
        }

        // The context must point the root node.
        assertSame(vtnPath, ctx.getPath());
    }

    /**
     * Push the specified path argument to the tree change context.
     *
     * @param ctx  The tree change context.
     * @param arg  The path argumen to be pushed.
     */
    private void push(TreeChangeContext ctx, PathArgument arg) {
        @SuppressWarnings("unchecked")
        DataObjectModification<?> mod = mock(DataObjectModification.class);
        when(mod.getIdentifier()).thenReturn(arg);
        ctx.push(mod);
    }
}
