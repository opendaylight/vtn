/*
 * Copyright (c) 2013, 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager;

import java.util.EnumMap;
import java.util.HashMap;
import java.util.Map;

import org.junit.Test;

import org.opendaylight.controller.sal.utils.Status;
import org.opendaylight.controller.sal.utils.StatusCode;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnErrorTag;

/**
 * JUnit test for {@link VTNException}.
 */
public class VTNExceptionTest extends TestBase {
    /**
     * Associate {@link StatusCode} with {@link VtnErrorTag}.
     */
    private static final Map<VtnErrorTag, StatusCode>  CODES;

    /**
     * Associate {@link VtnErrorTag} with {@link StatusCode}.
     */
    private static final Map<StatusCode, VtnErrorTag>  ERROR_TAGS;

    /**
     * Initialize static fields.
     */
    static {
        Map<VtnErrorTag, StatusCode> map = new HashMap<>();
        map.put(null, StatusCode.INTERNALERROR);
        map.put(VtnErrorTag.BADREQUEST, StatusCode.BADREQUEST);
        map.put(VtnErrorTag.UNAUTHORIZED, StatusCode.UNAUTHORIZED);
        map.put(VtnErrorTag.NOTFOUND, StatusCode.NOTFOUND);
        map.put(VtnErrorTag.NOTACCEPTABLE, StatusCode.NOTACCEPTABLE);
        map.put(VtnErrorTag.TIMEOUT, StatusCode.TIMEOUT);
        map.put(VtnErrorTag.CONFLICT, StatusCode.CONFLICT);
        map.put(VtnErrorTag.GONE, StatusCode.GONE);
        map.put(VtnErrorTag.INTERNALERROR, StatusCode.INTERNALERROR);
        map.put(VtnErrorTag.NOSERVICE, StatusCode.NOSERVICE);
        CODES = map;

        Map<StatusCode, VtnErrorTag> tagMap = new EnumMap<>(StatusCode.class);
        tagMap.put(StatusCode.BADREQUEST, VtnErrorTag.BADREQUEST);
        tagMap.put(StatusCode.UNAUTHORIZED, VtnErrorTag.UNAUTHORIZED);
        tagMap.put(StatusCode.NOTFOUND, VtnErrorTag.NOTFOUND);
        tagMap.put(StatusCode.NOTACCEPTABLE, VtnErrorTag.NOTACCEPTABLE);
        tagMap.put(StatusCode.TIMEOUT, VtnErrorTag.TIMEOUT);
        tagMap.put(StatusCode.CONFLICT, VtnErrorTag.CONFLICT);
        tagMap.put(StatusCode.GONE, VtnErrorTag.GONE);
        tagMap.put(StatusCode.INTERNALERROR, VtnErrorTag.INTERNALERROR);
        tagMap.put(StatusCode.NOSERVICE, VtnErrorTag.NOSERVICE);
        ERROR_TAGS = tagMap;
    }

    /**
     * Test case for {@link VTNException#VTNException(VtnErrorTag, String)}
     * and the followings.
     *
     * <ul>
     *   <li>{@link Throwable#getMessage()}</li>
     *   <li>{@link Throwable#getCause()}</li>
     *   <li>{@link VTNException#getVtnErrorTag()}</li>
     *   <li>{@link VTNException#getStatus()}</li>
     * </ul>
     */
    @Test
    public void testException1() {
        for (Map.Entry<VtnErrorTag, StatusCode> entry: CODES.entrySet()) {
            for (String msg: createStrings("Exception")) {
                VtnErrorTag etag = entry.getKey();
                VtnErrorTag expTag = (etag == null)
                    ? VtnErrorTag.INTERNALERROR : etag;
                StatusCode code = entry.getValue();
                Status st = new Status(code, msg);
                VTNException e = new VTNException(etag, msg);
                assertEquals(msg, e.getMessage());
                assertEquals(null, e.getCause());
                assertEquals(expTag, e.getVtnErrorTag());
                assertEquals(st, e.getStatus());
            }
        }
    }

    /**
     * Test case for
     * {@link VTNException#VTNException(VtnErrorTag, String, Thowable)} and
     * the followings.
     *
     * <ul>
     *   <li>{@link Throwable#getMessage()}</li>
     *   <li>{@link Throwable#getCause()}</li>
     *   <li>{@link VTNException#getVtnErrorTag()}</li>
     *   <li>{@link VTNException#getStatus()}</li>
     * </ul>
     */
    @Test
    public void testException2() {
        Throwable[] causes = {
            new NullPointerException(),
            new IllegalArgumentException(),
            new IllegalStateException(),
        };

        for (Map.Entry<VtnErrorTag, StatusCode> entry: CODES.entrySet()) {
            for (Throwable cause: causes) {
                for (String msg: createStrings("Exception")) {
                    VtnErrorTag etag = entry.getKey();
                    VtnErrorTag expTag = (etag == null)
                        ? VtnErrorTag.INTERNALERROR : etag;
                    StatusCode code = entry.getValue();
                    Status st = new Status(code, msg);
                    VTNException e = new VTNException(etag, msg, cause);
                    assertEquals(msg, e.getMessage());
                    assertEquals(cause, e.getCause());
                    assertEquals(expTag, e.getVtnErrorTag());
                    assertEquals(st, e.getStatus());
                }
            }
        }
    }

    /**
     * Test case for {@link VTNException#VTNException(String)}
     * and the followings.
     *
     * <ul>
     *   <li>{@link Throwable#getMessage()}</li>
     *   <li>{@link Throwable#getCause()}</li>
     *   <li>{@link VTNException#getVtnErrorTag()}</li>
     *   <li>{@link VTNException#getStatus()}</li>
     * </ul>
     */
    @Test
    public void testException3() {
        for (String msg: createStrings("Exception")) {
            Status st = new Status(StatusCode.INTERNALERROR, msg);
            VTNException e = new VTNException(msg);
            assertEquals(msg, e.getMessage());
            assertEquals(null, e.getCause());
            assertEquals(VtnErrorTag.INTERNALERROR, e.getVtnErrorTag());
            assertEquals(st, e.getStatus());
        }
    }

    /**
     * Test case for
     * {@link VTNException#VTNException(String, Thowable)} and
     * the followings.
     *
     * <ul>
     *   <li>{@link Throwable#getMessage()}</li>
     *   <li>{@link Throwable#getCause()}</li>
     *   <li>{@link VTNException#getVtnErrorTag()}</li>
     *   <li>{@link VTNException#getStatus()}</li>
     * </ul>
     */
    @Test
    public void testException4() {
        Throwable[] causes = {
            new NullPointerException(),
            new IllegalArgumentException(),
            new IllegalStateException(),
        };

        for (Throwable cause: causes) {
            for (String msg: createStrings("Exception")) {
                Status st = new Status(StatusCode.INTERNALERROR, msg);
                VTNException e = new VTNException(msg, cause);
                assertEquals(msg, e.getMessage());
                assertEquals(cause, e.getCause());
                assertEquals(VtnErrorTag.INTERNALERROR, e.getVtnErrorTag());
                assertEquals(st, e.getStatus());
            }
        }
    }

    /**
     * Test case for {@link VTNException#VTNException(Status)}
     * and the followings.
     *
     * <ul>
     *   <li>{@link Throwable#getMessage()}</li>
     *   <li>{@link Throwable#getCause()}</li>
     *   <li>{@link VTNException#getVtnErrorTag()}</li>
     *   <li>{@link VTNException#getStatus()}</li>
     * </ul>
     */
    @Test
    public void testException5() {
        for (StatusCode code: StatusCode.values()) {
            for (String msg: createStrings("Exception")) {
                Status st = new Status(code, msg);
                VTNException e = new VTNException(st);
                if (msg == null) {
                    assertEquals(code.toString(), e.getMessage());
                } else {
                    assertEquals(msg, e.getMessage());
                }
                assertEquals(null, e.getCause());

                VtnErrorTag etag = ERROR_TAGS.get(code);
                if (etag == null) {
                    etag = VtnErrorTag.INTERNALERROR;
                    st = new Status(StatusCode.INTERNALERROR, msg);
                }
                assertEquals(etag, e.getVtnErrorTag());
                assertEquals(st, e.getStatus());
            }
        }
    }
}
