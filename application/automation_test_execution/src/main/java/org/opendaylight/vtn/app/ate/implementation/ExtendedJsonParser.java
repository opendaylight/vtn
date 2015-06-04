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
import java.util.List;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import org.opendaylight.vtn.app.run.config.json.annotations.JsonArray;
import org.opendaylight.vtn.app.run.config.json.annotations.JsonElement;
import org.opendaylight.vtn.app.run.config.json.annotations.JsonObject;
import org.opendaylight.vtn.app.run.config.json.annotations.JsonObjectRef;
import org.opendaylight.vtn.app.run.config.rest.parser.Parser;
import org.opendaylight.vtn.app.ate.json.annotations.JsonDynamicKey;
import org.opendaylight.vtn.app.ate.json.annotations.JsonDynamicList;
import org.opendaylight.vtn.app.ate.json.annotations.JsonDynamicObjectRef;

/**
 * JSON parser - creating Java objects based on json input.
 */
public class ExtendedJsonParser extends Parser {

    /**
     * Logger instance.
     */
    static final Logger LOG = LoggerFactory.getLogger(ExtendedJsonParser.class);

    public ExtendedJsonParser() {
    }

    /**
     * Get all the annotated fields in a class to a JSON object
     * @param obj
     *         The annotated class object
     * @return
     *         JSON object having the annotated field's key and value
     *
     * @throws IllegalArgumentException
     * @throws IllegalAccessException
     * @throws JSONException
     * @throws InstantiationException
     */
    public JSONObject parseObject(Object obj) throws IllegalArgumentException, IllegalAccessException, JSONException, InstantiationException {

        JSONObject jsonObj = new JSONObject();
        /*
         * Check whether the referred object is annotated as JSON object
         */
        if (obj != null && obj.getClass().getAnnotation(JsonObject.class) != null) {
            /*
             * Iterate through each of the fields in the Object
             */
            for (Field field : obj.getClass().getDeclaredFields()) {
                field.setAccessible(true);

                /*
                 * Try to get the required JSON annotations from the fields
                 */
                JsonElement jsonElement = field.getAnnotation(JsonElement.class);
                JsonObjectRef jsonObjectRef = field.getAnnotation(JsonObjectRef.class);
                JsonDynamicObjectRef jsonDynamicObjectRef = field.getAnnotation(JsonDynamicObjectRef.class);
                JsonArray jsonList = field.getAnnotation(JsonArray.class);
                JsonDynamicList jsonDynamicList = field.getAnnotation(JsonDynamicList.class);

                if (jsonElement != null) {
                    /*
                     * Field is annotated as JSONElement.
                     * Get the field value and put in JSON object
                     */
                    jsonObj.put(jsonElement.name(), field.get(obj));

                } else if (jsonObjectRef != null) {
                    /*
                     * Field is annotated as reference to another JSON Object
                     * Get the field object and invoke the 'getAnnotatedFields' method recursively to get the JSON values
                     * on the referring object
                     */
                    jsonObj.put(jsonObjectRef.name(), parseObject(field.get(obj)));

                } else if (jsonDynamicObjectRef != null) {
                    /*
                     * Field is annotated as reference to dynamic Object (Object don't have static key, probably object value itself as key)
                     * Get the dynamic key on the Objects 'JsonDynamiKey' annotated field
                     */
                    Object dynamicObj = field.get(obj);
                    if (dynamicObj != null) {
                        Field dynamicField = getAnnotatedField(dynamicObj, JsonDynamicKey.class);
                        dynamicField.setAccessible(true);

                        /*
                         * Invoke the 'setAnnotatedFields' method recursively to get the JSON values on the referring object
                         */
                        if (dynamicField.get(dynamicObj) == null) {
                            jsonObj.put("", parseObject(dynamicObj));
                        } else {
                            jsonObj.put(dynamicField.get(dynamicObj).toString(), parseObject(dynamicObj));
                        }
                    }

                } else if (jsonList != null) {
                    /*
                     * Field is annotated as JSON array
                     */
                    if (field.get(obj) != null) {
                        JSONArray jsonArray = new JSONArray();
                        @SuppressWarnings("rawtypes")
                        List list = ((List)field.get(obj));

                        /*
                         * Iterate through the list and get the JSON object for each element
                         */
                        for (int index = 0; index < list.size(); index++) {
                            if (list.get(index) instanceof Object) {
                                jsonArray.put(parseObject(list.get(index)));
                            } else {
                                jsonArray.put(list.get(index));
                            }
                        }

                        /*
                         * Put the JSON array into the JSON object
                         */
                        jsonObj.put(jsonList.name(), jsonArray);
                    }
                } else if (jsonDynamicList != null) {
                    /*
                     * Field is annotated as reference to dynamic List (Objects don't have static key, probably object value itself as key)
                     */

                    if (field.get(obj) != null) {

                        @SuppressWarnings("rawtypes")
                        List list = ((List)field.get(obj));
                        if (list.size() > 0) {
                            Field dynamicField = getAnnotatedField(list.get(0), JsonDynamicKey.class);
                            dynamicField.setAccessible(true);
                            /*
                             * Iterate through the list and get the JSON object for each element
                             */
                            for (int index = 0; index < list.size(); index++) {
                                jsonObj.put(dynamicField.get(list.get(index)).toString(), parseObject(list.get(index)));

                            }
                        }
                    }
                }
            }
        }
        return jsonObj;
    }

    /**
     * Get JSON object for all the annotated element classes in list
     * @param list
     *         Annotated class list
     * @return
     *         JSOON object
     *
     * @throws IllegalArgumentException
     * @throws IllegalAccessException
     * @throws JSONException
     * @throws InstantiationException
     */
    public Object parseArray(List<?> list) throws IllegalArgumentException, IllegalAccessException, JSONException, InstantiationException {

        JSONObject dummyObject = new JSONObject();
        /*
         * Check whether the referred the Class is annotated as JSON object
         */
        if (list != null && list.size() > 0) {
            Object elementObj = list.get(0);

            /*
             * Check whether the class in annotated as JsonObject
             */
            if (elementObj.getClass().getAnnotation(JsonObject.class) != null) {

                /*
                 * Get the dynamic key
                 */
                Field dynamicField = getAnnotatedField(elementObj, JsonDynamicKey.class);
                if (dynamicField != null) {
                    JSONObject jsonObj = new JSONObject();
                    dynamicField.setAccessible(true);

                    /*
                     * Iterate through the list and get the annotated field key and value
                     */
                    for (int index = 0; index < list.size(); index++) {
                        if ((dynamicField.get(list.get(index))) == null) {
                            jsonObj.put("", parseObject(list.get(index)));
                        } else {
                            jsonObj.put(dynamicField.get(list.get(index)).toString(), parseObject(list.get(index)));
                        }
                    }
                    return jsonObj;
                } else {
                    /*
                     * Iterate through the list and get the annotated field key and value
                     */
                    JSONArray jsonArray = new JSONArray();
                    for (int index = 0; index < list.size(); index++) {
                        jsonArray.put(parseObject(list.get(index)));
                    }
                    return jsonArray;
                }
            }
        }
        return dummyObject;
    }

    @SuppressWarnings("unchecked")
    private Field getAnnotatedField(Object obj, @SuppressWarnings("rawtypes") Class annotationClass) {
        for (Field field : obj.getClass().getDeclaredFields()) {
            if (field.getAnnotation(annotationClass) != null) {
                return field;
            }
        }
        return null;
    }
}
