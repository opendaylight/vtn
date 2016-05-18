/*
 * Copyright (c) 2015, 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.it.util;

/**
 * {@code RpcErrorTag} defines RPC error tags suggested by
 * <a href="https://tools.ietf.org/html/rfc6241#page-89">RFC6241</a>.
 */
public enum RpcErrorTag {
    /**
     * Indicates that the request requires a resource that already in use.
     */
    IN_USE("in-use"),

    /**
     * Indicates that the request specifies an unacceptable value for one
     * or more parameters.
     */
    INVALID_VALUE("invalid-value"),

    /**
     * Indicates that the request or response (that would be generated) is
     * too large for the implementation to handle.
     */
    TOO_BIG("too-big"),

    /**
     * Indicates that an expected attribute is missing.
     */
    MISSING_ATTRIBUTE("missing-attribute"),

    /**
     * Indicates that an attribute value is not correct;
     * e.g., wrong type, out of range, pattern missmatch.
     */
    BAD_ATTRIBUTE("bad-attribute"),

    /**
     * Indicates that an unexpected attribute is present.
     */
    UNKNOWN_ATTRIBUTE("unknown-attribute"),

    /**
     * Indicates that an expected element is missing.
     */
    MISSING_ELEMENT("missing-element"),

    /**
     * Indicates that an element value is no correct;
     * e.g., wrong type, out of range, patern mismach.
     */
    BAD_ELEMENT("bad-element"),

    /**
     * Indicates that an unexpected element is present.
     */
    UNKNOWN_ELEMENT("unknown-element"),

    /**
     * Indicates that an unexpected namespace is present.
     */
    UNKNOWN_NAMESPACE("unknown-namespace"),

    /**
     * Indicates that access to the requested protocol operation or data model
     * is denied because authorization failed.
     */
    ACCESS_DENIED("access-denied"),

    /**
     * Indicates that access to the requested lock is denied because the lock
     * is currently held by another entity.
     */
    LOCK_DENIED("lock-denied"),

    /**
     * Indicates that request could not be completed because of insufficient
     * resources.
     */
    RESOURCE_DENIED("resource-denied"),

    /**
     * Indicates that request to roll back some configuration change
     * (via rollback-on-error or &lt;discard-changes&gt; operations)
     * was not completed for some reason.
     */
    ROLLBACK_FAILED("rollback-failed"),

    /**
     * Request could not be completed because the relevant data model content
     * already exists.  For example, a 'create' operation was attempted on
     * data that already exists.
     */
    DATA_EXISTS("data-exists"),

    /**
     * Indicates that request could not be completed because the relevant
     * data model content does not exist.  For example, a 'delete' operation
     * was attempted on data that does not exist.
     */
    DATA_MISSING("data-missing"),

    /**
     * Indicates that request could not be completed because the requested
     * operation is not supported by this implementation.
     */
    OPERATION_NOT_SUPPORTED("operation-not-supported"),

    /**
     * Indicates that request could not be completed because the requested
     * operation failed for some reason not covered by any other error
     * condition.
     */
    OPERATION_FAILED("operation-failed");

    /**
     * A string that will be returned as an error tag of RPC result.
     */
    private final String  errorTag;

    /**
     * Construct a new instance.
     *
     * @param tag  An error tag.
     */
    RpcErrorTag(String tag) {
        errorTag = tag;
    }

    /**
     * Return a string to be returned as an error tag of RPC result.
     *
     * @return  An error tag.
     */
    public String getErrorTag() {
        return errorTag;
    }
}
