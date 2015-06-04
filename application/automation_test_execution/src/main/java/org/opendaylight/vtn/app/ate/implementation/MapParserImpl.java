/**
 * Copyright (c) 2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.app.ate.implementation;

import java.lang.reflect.Field;
import java.lang.reflect.ParameterizedType;
import java.util.ArrayList;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;

import org.opendaylight.vtn.app.ate.api.MapParser;
import org.opendaylight.vtn.app.run.config.json.annotations.JsonArray;
import org.opendaylight.vtn.app.run.config.json.annotations.JsonElement;
import org.opendaylight.vtn.app.run.config.json.annotations.JsonObject;
import org.opendaylight.vtn.app.run.config.json.annotations.JsonObjectRef;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

/**
 * Map parser implementation - Converting Map{key, value} pairs to respective java objects.
 */
public class MapParserImpl implements MapParser {

    /**
     * Logger instance.
     */
    static final Logger LOG = LoggerFactory.getLogger(MapParserImpl.class);

    public MapParserImpl() {
    }

    @SuppressWarnings("unchecked")
    public Object getInputDatas(Map<String, Object> map) {
        Object dataObject = null;

        if (map.containsKey(INPUT_KEY)) {
            dataObject = new ArrayList<Object>();
            ((List<Object>)dataObject).add((LinkedHashMap<String, Object>)map.get(INPUT_KEY));
        } else if (map.containsKey(INPUT_LIST_KEY)) {
            dataObject = (List<Object>)map.get(INPUT_LIST_KEY);
        }

        return dataObject;
    }

    @SuppressWarnings("unchecked")
    public Object getVerifyingDatas(Map<String, Object> map) {
        Object dataObject = null;

        if (map.containsKey(VERIFICATION_KEY)) {
            dataObject = new ArrayList<Object>();
            ((List<Object>)dataObject).add((LinkedHashMap<String, Object>)map.get(VERIFICATION_KEY));
        } else if (map.containsKey(VERIFICATION_LIST_KEY)) {
            dataObject = (List<Object>)map.get(VERIFICATION_LIST_KEY);
        }

        return dataObject;
    }

    @SuppressWarnings("unchecked")
    public Object setAnnotatedFields(Map<String, Object> map, Object requestObject) {
        Map<String, Object> dataWithHeader = new LinkedHashMap<String, Object>();
        try {
            for (String className : map.keySet()) {
                if (requestObject.getClass().getSimpleName().equals(className)) {
                    requestObject = setAnnotatedFields(requestObject.getClass(), map.get(className));
                    dataWithHeader.put(requestObject.getClass().getSimpleName(), requestObject);
                } else {
                    dataWithHeader.put(className, map.get(className));
                }
            }
        } catch (IllegalArgumentException e) {
            LOG.error("IllegalArgumentException at MapParser - {}", e.getMessage());
        } catch (IllegalAccessException e) {
            LOG.error("IllegalAccessException at MapParser - {}", e.getMessage());
        } catch (InstantiationException e) {
            LOG.error("InstantiationException at MapParser - {}", e.getMessage());
        } catch (ClassNotFoundException e) {
            LOG.error("ClassNotFoundException at MapParser - {}", e.getMessage());
        }
        return dataWithHeader;
    }

    @SuppressWarnings("unchecked")
    private Object setAnnotatedFields(Class<?> classRef, Object datas) throws IllegalArgumentException, IllegalAccessException, InstantiationException, ClassNotFoundException {
        Object classObject = null;
        /*
         * Check whether the referred object is annotated as JSON object
         */
        if ((classRef != null) && (classRef.getAnnotation(JsonObject.class) != null)) {
            classObject = classRef.newInstance();
            Map<String, Object> map = (Map<String, Object>)datas;
            /*
             * Iterate through each of the fields in the Object
             */
            for (Field field : classObject.getClass().getDeclaredFields()) {
                field.setAccessible(true);

                /*
                 * Try to get the required JSON annotations from the fields
                 */
                JsonElement jsonElement = field.getAnnotation(JsonElement.class);
                JsonObjectRef jsonObjectRef = field.getAnnotation(JsonObjectRef.class);
                JsonArray jsonList = field.getAnnotation(JsonArray.class);

                if (jsonElement != null) {
                    /*
                     * Field is annotated as JSONElement.
                     */
                    if (map.containsKey(jsonElement.name())) {
                        /*
                         * Get the yml value and set on the field
                         */
                        setElementValueinClassObject(classObject, field, (map.get(jsonElement.name()).toString()));
                    }
                } else if (jsonObjectRef != null) {
                    /*
                     * Field is annotated as reference to another JSON Object
                     */
                    if (map.containsKey(jsonObjectRef.name())) {
                        /*
                         * Get the JSON object and invoke the 'setAnnotatedFields' method recursively to set the JSON values
                         * on the referring object
                         */
                        setAnnotatedFields(field.get(classObject).getClass(), map.get(jsonObjectRef.name()));
                    }
                } else if (jsonList != null) {

                    /*
                     * Field is annotated as JSON array
                     */
                    if (map.containsKey(jsonList.name())) {
                        /*
                         * Get the List of objects
                         */
                        List<?> array = (List<?>)map.get(jsonList.name());

                        /*
                         * Get the list type parameter Class
                         */
                        Class<?> listTypeClass = ((Class<?>)((ParameterizedType)field.getGenericType()).getActualTypeArguments()[0]);
                        for (int index = 0; index < array.size(); index++) {
                            Map<String, Object> listMap = new LinkedHashMap<String, Object>(1);
                            //Converting objects(in list) to map(key-declared parameter name, value-objects)
                            listMap.put(listTypeClass.getSimpleName(), array.get(index));
                            ((List)field.get(classObject)).add(setAnnotatedFields(listTypeClass, listMap));
                        }
                    }
                }
            }
        }
        return classObject;
    }

    private void setElementValueinClassObject(Object classObject, Field field, String fieldValue) {
        try {
            if (fieldValue == null) {
                field.set(classObject, null);
            } else {
                String fieldTypeName = field.getType().getName();
                if ((field != null)
                        && (field.getType().isPrimitive())
                        && (!fieldValue.equals("{}"))) {
                    if (fieldTypeName.equals("long")) {
                        field.set(classObject, Long.parseLong(fieldValue));
                    } else if (fieldTypeName.equals("double")) {
                        field.set(classObject, Double.parseDouble(fieldValue));
                    } else if (fieldTypeName.equals("float")) {
                        field.set(classObject, Float.parseFloat(fieldValue));
                    } else if (fieldTypeName.equals("int")) {
                        field.set(classObject, Integer.parseInt(fieldValue));
                    } else if (fieldTypeName.equals("short")) {
                        field.set(classObject, Short.parseShort(fieldValue));
                    } else if (fieldTypeName.equals("byte")) {
                        field.set(classObject, Byte.parseByte(fieldValue));
                    } else if (fieldTypeName.equals("boolean")) {
                        field.set(classObject, Boolean.parseBoolean(fieldValue));
                    }
                } else if (fieldTypeName.equals("java.lang.String")) {
                    if (fieldValue.equals("{}")) {
                        field.set(classObject, null);
                    } else {
                        field.set(classObject, fieldValue);
                    }
                }
            }
        } catch (IllegalArgumentException e) {
            LOG.error("IllegalArgumentException at MapParser - {}", e.getMessage());
        } catch (IllegalAccessException e) {
            LOG.error("IllegalAccessException at MapParser - {}", e.getMessage());
        }
    }
}
