/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.northbound;

import javax.ws.rs.Consumes;
import javax.ws.rs.core.GenericEntity;
import javax.ws.rs.core.MediaType;
import javax.ws.rs.core.Response;
import javax.ws.rs.ext.ExceptionMapper;
import javax.ws.rs.ext.Provider;

import com.fasterxml.jackson.core.JsonProcessingException;

/**
 * Exception mapper for handling Jackson exception.
 */
@Provider
@Consumes(MediaType.APPLICATION_JSON)
public final class JsonExceptionMapper
    implements ExceptionMapper<JsonProcessingException> {
    /**
     * Convert the given Jackson exception to a REST response.
     *
     * @param e  A {@link JsonProcessingException} exception.
     * @return  A {@link Response} instance.
     */
    @Override
    public Response toResponse(JsonProcessingException e) {
        String msg = e.getMessage();
        GenericEntity<String> entity = new GenericEntity<String>(msg) {};
        return Response.status(Response.Status.BAD_REQUEST).
            entity(entity).build();
    }
}
