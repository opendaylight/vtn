/*
 * Copyright (c) 2014, 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.cluster;

import java.io.Serializable;
import java.lang.reflect.Constructor;
import java.lang.reflect.InvocationTargetException;
import java.util.HashMap;
import java.util.Map;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import org.opendaylight.vtn.manager.internal.PacketContext;

import org.opendaylight.vtn.manager.VTNException;
import org.opendaylight.vtn.manager.flow.action.FlowAction;
import org.opendaylight.vtn.manager.flow.action.SetDlDstAction;
import org.opendaylight.vtn.manager.flow.action.SetDlSrcAction;
import org.opendaylight.vtn.manager.flow.action.SetDscpAction;
import org.opendaylight.vtn.manager.flow.action.SetIcmpCodeAction;
import org.opendaylight.vtn.manager.flow.action.SetIcmpTypeAction;
import org.opendaylight.vtn.manager.flow.action.SetInet4DstAction;
import org.opendaylight.vtn.manager.flow.action.SetInet4SrcAction;
import org.opendaylight.vtn.manager.flow.action.SetTpDstAction;
import org.opendaylight.vtn.manager.flow.action.SetTpSrcAction;
import org.opendaylight.vtn.manager.flow.action.SetVlanPcpAction;

import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;

/**
 * Implementation of flow action.
 *
 * <p>
 *   Although this class is public to other packages, this class does not
 *   provide any API. Applications other than VTN Manager must not use this
 *   class.
 * </p>
 */
public abstract class FlowActionImpl implements Serializable {
    /**
     * Version number for serialization.
     */
    private static final long serialVersionUID = 9015667825740105469L;

    /**
     * Suffix of action implementation class name.
     */
    private static final String  IMPL_SUFFIX = "Impl";

    /**
     * Constructors for action implementations.
     */
    private static final Map<Class<?>, Constructor<?>>  CONSTRUCTORS;

    /**
     * Initialize constructors for action implementations.
     *
     * <p>
     *   Implementation class for flow action is determined by appending
     *   "Impl" to the name of flow action API class. For example, the name of
     *   action implementation class for {@link SetDscpAction} must be
     *   {@link SetDscpActionImpl}.
     * </p>
     */
    static {
        CONSTRUCTORS = new HashMap<Class<?>, Constructor<?>>();
        Class<?>[] classes = {
            SetDlSrcAction.class,
            SetDlDstAction.class,
            SetVlanPcpAction.class,
            SetInet4SrcAction.class,
            SetInet4DstAction.class,
            SetDscpAction.class,
            SetTpSrcAction.class,
            SetTpDstAction.class,
            SetIcmpTypeAction.class,
            SetIcmpCodeAction.class,
        };

        Package pkg = FlowActionImpl.class.getPackage();
        for (Class<?> cl: classes) {
            try {
                setConstructor(pkg, cl);
            } catch (Exception e) {
                Logger log = LoggerFactory.getLogger(FlowActionImpl.class);
                log.error("Failed to initialize implementation of " +
                          cl.getSimpleName(), e);
            }
        }
    }

    /**
     * Create a {@link FlowActionImpl} instance.
     *
     * @param act  A {@link FlowAction} instance.
     * @return  A new {@link FlowActionImpl} instance.
     * @throws VTNException
     *    Failed to instantiate the given flow action.
     */
    public static FlowActionImpl create(FlowAction act) throws VTNException {
        if (act == null) {
            throw RpcException.getNullArgumentException("Flow action");
        }

        // Determine implementation class.
        Class<?> actClass = act.getClass();
        Constructor<?> ctor = CONSTRUCTORS.get(actClass);
        if (ctor == null) {
            throw RpcException.getBadArgumentException(
                "Unsupported action: " + actClass.getSimpleName());
        }

        // Instantiate implementation.
        Exception error;
        try {
            return (FlowActionImpl)ctor.newInstance(act);
        } catch (InvocationTargetException e) {
            Throwable cause = e.getCause();
            if (cause instanceof VTNException) {
                // Constructor of implementation class threw an exception.
                throw (VTNException)cause;
            }

            error = e;
        } catch (Exception e) {
            error = e;
        }

        // This should never happen.
        Logger log = LoggerFactory.getLogger(FlowActionImpl.class);
        String msg = "Failed to instantiate action: act=" + act;
        log.error(msg, error);
        throw new VTNException(msg, error);
    }

    /**
     * Initialize constructor for the given action.
     *
     * @param pkg        Package for this class.
     * @param actClass   A class for action.
     * @throws ClassNotFoundException
     *    Implementation class was not found.
     * @throws NoSuchMethodException
     *    Constructor for the specified implementation class was not found.
     */
    private static void setConstructor(Package pkg, Class<?> actClass)
        throws ClassNotFoundException, NoSuchMethodException {
        StringBuilder builder = new StringBuilder(pkg.getName());
        String implName = builder.append('.').append(actClass.getSimpleName()).
            append(IMPL_SUFFIX).toString();
        Class<?> implClass = Class.forName(implName);
        Constructor<?> ctor = implClass.getConstructor(actClass);
        CONSTRUCTORS.put(actClass, ctor);
    }

    /**
     * Construct a new instance.
     *
     * @param act  A {@link FlowAction} instance.
     * @throws VTNException
     *    {@code act} is {@code null}.
     */
    protected FlowActionImpl(FlowAction act) throws VTNException {
        if (act == null) {
            throw RpcException.getNullArgumentException("Flow action");
        }
    }

    /**
     * Construct an error message that indicates an invalid {@link FlowAction}
     * is specified.
     *
     * @param act   An invalid {@link FlowAction} instance.
     * @param args  Objects to be embedded in an error message.
     * @return  An error message.
     */
    protected String getErrorMessage(FlowAction act, Object ... args) {
        StringBuilder builder =
            new StringBuilder(act.getClass().getSimpleName());
        builder.append(": ");
        if (args != null) {
            for (Object o: args) {
                builder.append(o);
            }
        }

        return builder.toString();
    }

    /**
     * Return a {@link FlowAction} instance which represents this instance.
     *
     * @return  A {@link FlowAction} instance.
     */
    public abstract FlowAction getFlowAction();

    /**
     * Apply this flow action to the given packet.
     *
     * @param pctx  The context of the received packet.
     * @return  {@code true} is returned if this flow action is actually
     *          applied.
     *          {@code false} is returned if this flow action is ignored.
     */
    public abstract boolean apply(PacketContext pctx);

    /**
     * Determine whether the given object is identical to this object.
     *
     * @param o  An object to be compared.
     * @return   {@code true} if identical. Otherwise {@code false}.
     */
    @Override
    public boolean equals(Object o) {
        return (o != null && getClass().equals(o.getClass()));
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public int hashCode() {
        return getClass().getName().hashCode();
    }

    /**
     * Return a string representation of this object.
     *
     * @return  A string representation of this object.
     */
    @Override
    public String toString() {
        return getClass().getSimpleName();
    }
}
