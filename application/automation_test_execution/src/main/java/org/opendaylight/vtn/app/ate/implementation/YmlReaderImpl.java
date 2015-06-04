/**
 * Copyright (c) 2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.app.ate.implementation;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStreamReader;
import java.util.ArrayList;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;

import org.opendaylight.vtn.app.ate.api.YmlReader;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

/**
 * YML reader - Read .yml input file and convert int Map{key, value} pair.
 */
public class YmlReaderImpl implements YmlReader {

    /**
     * Logger instance.
     */
    static final Logger LOG = LoggerFactory.getLogger(YmlReaderImpl.class);

    /**
     * FileName of the yml input file.
     */
    private final String fileName;

    public YmlReaderImpl(String fileName) {
        this.fileName = fileName;
    }

    public Map<String, Object> getYmlDatas() throws Exception {
        try {
            File file = new File(fileName);
            FileInputStream fis = new FileInputStream(file);
            BufferedReader br = new BufferedReader(new InputStreamReader(fis));
            @SuppressWarnings("unchecked")
            Map<String, Object> map = (Map<String, Object>)readAndParseYmlFile(br, new LinkedHashMap<String, Object>(), 0);

            return map;
        } catch (FileNotFoundException e) {
            LOG.error("FileNotFoundException YML input file is not found - {}", e.getMessage());
            throw new Exception("Error - YML input file is not found");
        } catch (IllegalArgumentException e) {
            LOG.error("IllegalArgumentException Input format is incorrect in the file - {}", e.getMessage());
            throw new Exception("Error - Input format is incorrect in the file");
        } catch (Exception e) {
            LOG.error("Exception YML Input format is incorrect in the file - {}", e.getMessage());
            throw new Exception("Error - Input format is incorrect in the file");
        }
    }

    @SuppressWarnings("unchecked")
    private Object readAndParseYmlFile(BufferedReader br, Object map, int indentationLevel) {
        try {
            String line = markAndReadLine(br);

            while (line != null) {
                if (!validateIndentationLevel(line, indentationLevel)) {
                    br.reset();
                    return map;
                }

                String[] pairValue = line.split(KEY_VALUE_PAIR_SEPERATOR, KEY_VALUE_PAIR_COUNT);
                pairValue[KEY_INDEX] = pairValue[KEY_INDEX].substring(indentationLevel * INDENTATION_WIDTH);

                if ((pairValue.length > VALUE_INDEX) && (pairValue[VALUE_INDEX].length() > 0)) {
                    ((LinkedHashMap<String, Object>)map)
                        .put(pairValue[KEY_INDEX], pairValue[VALUE_INDEX]);
                } else if (line.indexOf(OBJECT_IDENTIFIER) < 0) {
                    // No key value pair in list
                    ((List<Object>)map).add(pairValue[KEY_INDEX]);
                } else if (pairValue[KEY_INDEX].startsWith(LIST_IDENTIFIER)) {
                    ((LinkedHashMap<String, Object>)map).put(pairValue[KEY_INDEX].substring(LIST_IDENTIFIER.length()),
                            readAndParseYmlFile(br, new ArrayList<Object>(), indentationLevel + INDENTATION_INCREMENTOR));
                } else {
                    if (map.getClass().getSimpleName().equals(ArrayList.class.getSimpleName())) {
                        ((List<Object>)map).add(
                                readAndParseYmlFile(br, new LinkedHashMap<String, Object>(), indentationLevel + INDENTATION_INCREMENTOR));
                    } else {
                        ((LinkedHashMap<String, Object>)map).put(pairValue[KEY_INDEX],
                                readAndParseYmlFile(br, new LinkedHashMap<String, Object>(), indentationLevel + INDENTATION_INCREMENTOR));
                    }
                }
                line = markAndReadLine(br);
            }
        } catch (IOException e) {
            LOG.error("IOException at YML reader - {}", e.getMessage());
            return null;
        } catch (Exception e) {
            LOG.error("Exception at YML reader - {}", e.getMessage());
            e.printStackTrace();
            return null;
        }
        return map;
    }

    private String markAndReadLine(BufferedReader br) throws IOException {
        br.mark(0);
        String line = "";

        // Eliminating Commented lines and empty lines.
        do {
            line = br.readLine();
        } while ((line != null) &&
                            ((line.length() == 0) || (line.startsWith("#"))));

        return line;
    }

    private boolean validateIndentationLevel(String line, int indentationLevel) throws Exception {
        int localIndentationLevel = 0;
        for (int i = 0; i < line.length(); i++) {
            if (indentationLevel == localIndentationLevel) {
                if (line.charAt(i) == ' ') {
                    throw new Exception("Error - Input format is incorrect in the file");
                } else {
                    return true;
                }
            }

            localIndentationLevel++;
            if (!((line.charAt(i++) == ' ') && (line.charAt(i) == ' '))) {
                return false;
            }
        }
        return false;
    }
}
