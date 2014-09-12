/*
 * Copyright (c) 2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.flow.action;

import java.io.Serializable;
import java.lang.reflect.Constructor;
import java.lang.reflect.InvocationTargetException;
import java.util.HashMap;
import java.util.Map;

import javax.xml.bind.annotation.XmlAccessType;
import javax.xml.bind.annotation.XmlAccessorType;
import javax.xml.bind.annotation.XmlRootElement;
import javax.xml.bind.annotation.XmlSeeAlso;

import com.fasterxml.jackson.databind.annotation.JsonSerialize;

import org.opendaylight.controller.sal.action.Action;
import org.opendaylight.controller.sal.action.Drop;
import org.opendaylight.controller.sal.action.PopVlan;
import org.opendaylight.controller.sal.action.PushVlan;
import org.opendaylight.controller.sal.action.SetDlDst;
import org.opendaylight.controller.sal.action.SetDlSrc;
import org.opendaylight.controller.sal.action.SetNwDst;
import org.opendaylight.controller.sal.action.SetNwSrc;
import org.opendaylight.controller.sal.action.SetNwTos;
import org.opendaylight.controller.sal.action.SetTpDst;
import org.opendaylight.controller.sal.action.SetTpSrc;
import org.opendaylight.controller.sal.action.SetVlanId;
import org.opendaylight.controller.sal.action.SetVlanPcp;
import org.opendaylight.controller.sal.utils.IPProtocols;

/**
 * This class describes an abstract information about an action in a flow
 * entry.
 *
 * @since  Helium
 */
@JsonSerialize(include = JsonSerialize.Inclusion.NON_NULL)
@XmlRootElement(name = "flowaction")
@XmlAccessorType(XmlAccessType.NONE)
@XmlSeeAlso({
    DropAction.class,
    PopVlanAction.class,
    PushVlanAction.class,
    SetDlDstAction.class,
    SetDlSrcAction.class,
    SetDscpAction.class,
    SetIcmpCodeAction.class,
    SetIcmpTypeAction.class,
    SetInet4DstAction.class,
    SetInet4SrcAction.class,
    SetTpDstAction.class,
    SetTpSrcAction.class,
    SetVlanIdAction.class,
    SetVlanPcpAction.class
})
public abstract class FlowAction implements Serializable {
    /**
     * Version number for serialization.
     */
    private static final long serialVersionUID = -7681330147348966804L;

    /**
     * Constructors that instantiate {@code FlowAction} variants from
     * SAL action except for L4 actions.
     */
    private static final Map<Class<?>, Constructor<?>>  CTORS;

    /**
     * Constructors that instantiate {@code FlowAction} variants from
     * L4 SAL action.
     */
    private static final Map<Integer, Map<Class<?>, Constructor<?>>>  CTORS_L4;

    /**
     * Initialize constructors for {@code FlowAction} variants.
     */
    static {
        CTORS = new HashMap<Class<?>, Constructor<?>>();
        setConstructor(CTORS, DropAction.class, Drop.class);
        setConstructor(CTORS, PopVlanAction.class, PopVlan.class);
        setConstructor(CTORS, PushVlanAction.class, PushVlan.class);
        setConstructor(CTORS, SetDlDstAction.class, SetDlDst.class);
        setConstructor(CTORS, SetDlSrcAction.class, SetDlSrc.class);
        setConstructor(CTORS, SetDscpAction.class, SetNwTos.class);
        setConstructor(CTORS, SetInet4DstAction.class, SetNwDst.class);
        setConstructor(CTORS, SetInet4SrcAction.class, SetNwSrc.class);
        setConstructor(CTORS, SetVlanIdAction.class, SetVlanId.class);
        setConstructor(CTORS, SetVlanPcpAction.class, SetVlanPcp.class);

        CTORS_L4 = new HashMap<Integer, Map<Class<?>, Constructor<?>>>();
        HashMap<Class<?>, Constructor<?>> tcpMap =
            new HashMap<Class<?>, Constructor<?>>();
        CTORS_L4.put(Integer.valueOf(IPProtocols.TCP.intValue()), tcpMap);
        setConstructor(tcpMap, SetTpDstAction.class, SetTpDst.class);
        setConstructor(tcpMap, SetTpSrcAction.class, SetTpSrc.class);

        // Share TCP constructors with UDP.
        CTORS_L4.put(Integer.valueOf(IPProtocols.UDP.intValue()), tcpMap);

        HashMap<Class<?>, Constructor<?>> icmpMap =
            new HashMap<Class<?>, Constructor<?>>();
        CTORS_L4.put(Integer.valueOf(IPProtocols.ICMP.intValue()), icmpMap);
        setConstructor(icmpMap, SetIcmpCodeAction.class, SetTpDst.class);
        setConstructor(icmpMap, SetIcmpTypeAction.class, SetTpSrc.class);
    }

    /**
     * Convert a SAL action into {@code FlowAction} instance.
     *
     * @param act      A SAL action.
     * @param ipproto  IP protocol number.
     *                 This parameter is used if SET_TP_SRC or SET_TP_DST
     *                 action is passed to {@code act}.
     * @return  A {@link FlowAction} instance converted from the given
     *          SAL action. {@code null} is returned if the given SAL action
     *          is not supported.
     */
    public static final FlowAction create(Action act, int ipproto) {
        if (act == null) {
            return null;
        }

        Class<? extends Action> salClass = act.getClass();
        Constructor<?> ctor = getConstructor(salClass, ipproto);
        if (ctor == null) {
            return null;
        }

        Object[] params = (useDefault(salClass)) ? null : new Object[]{act};
        Exception error;
        try {
            return (FlowAction)ctor.newInstance(params);
        } catch (InvocationTargetException e) {
            Throwable cause = e.getCause();
            if (cause instanceof RuntimeException) {
                // Constructor threw an exception.
                throw (RuntimeException)cause;
            }

            error = e;
        } catch (Exception e) {
            error = e;
        }

        // This should never happen.
        String msg = "Failed to instantiate action: act=" + act +
            ", ipproto=" + ipproto;
        throw new IllegalStateException(msg, error);
    }

    /**
     * Set constructor for {@code FlowAction} variants to the given map.
     *
     * @param map       A map to keep constructors.
     * @param actClass  Class of {@code FlowAction} variants.
     * @param salClass  SAL action class associated with {@code actClass}.
     */
    private static void setConstructor(Map<Class<?>, Constructor<?>> map,
                                       Class<? extends FlowAction> actClass,
                                       Class<? extends Action> salClass) {
        Constructor<?> ctor;
        Class<?>[] paramTypes = (useDefault(salClass))
            ? null : new Class<?>[]{salClass};

        try {
            ctor = actClass.getConstructor(paramTypes);
        } catch (NoSuchMethodException e) {
            String msg = "No constructor for " + actClass.getSimpleName();
            throw new IllegalStateException(msg, e);
        }

        map.put(salClass, ctor);
    }

    /**
     * Return a constructor for {@code FlowAction} variants associated with
     * the given SAL action.
     *
     * @param c        Class of A SAL action.
     * @param ipproto  IP protocol number.
     * @return  A constructor for {@code FlowAction} variants associated with
     *          {@code act}.
     */
    private static Constructor<?> getConstructor(Class<? extends Action> c,
                                                 int ipproto) {
        Constructor<?> ctor = CTORS.get(c);
        if (ctor != null) {
            return ctor;
        }

        // Try L4 action constructors.
        Integer key = Integer.valueOf(ipproto);
        Map<Class<?>, Constructor<?>> map = CTORS_L4.get(key);
        if (map == null) {
            return null;
        }

        return map.get(c);
    }

    /**
     * Determine whether the {@code FlowAction} class associated with the given
     * SAL action can be instantiated using default constructor or not.
     *
     * @param c  SAL action class.
     * @return   {@code true} is returned if the {@code FlowAction} class
     *           associated with the given SAL action can be instantiated
     *           using default constructor.
     *           {@code false} is returned if an instance of SAL action needs
     *           to be passed to the constructor.
     */
    private static boolean useDefault(Class<? extends Action> c) {
        return (PopVlan.class.equals(c) || Drop.class.equals(c));
    }

    /**
     * Construct a new instance which describes an action in a flow entry.
     */
    FlowAction() {
    }

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
