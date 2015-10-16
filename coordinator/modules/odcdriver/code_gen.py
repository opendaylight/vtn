# Copyright (c) 2013-2015 NEC Corporation
# All rights reserved.
#
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html

import parser, string, sys
import ConfigParser
import collections
import csv

# d is a dictionary which is used to write the below functions detail into files
d = {}

# It will add header files in the top of generated .hh files using .rest files
def begin_headers(item, file_desc, output_file_name, d):
    to_caps = item.split('.rest')
    to_caps = to_caps[0].upper()
    if_guard = "__" + to_caps + "_HH__"
    header = '#ifndef '  + if_guard +'\n'
    header = header + '#define ' + if_guard + '\n'
    test['headers']['Preprocessor'] = header

# It will add the required include files at the top of generated .hh files using .rest files
def begin_include(item, file_desc, output_file_name, d):
    include = '#include <unc/upll_ipc_enum.h>' + '\n' + '#include <unc/pfcdriver_ipc_enum.h>' + '\n' + '#include <odc_rest.hh>' + '\n'
    include = include + '#include <rest_util.hh>' + '\n'
    test['headers']['includes'] = include

# It will add the required namespaces in the generated .hh files
def begin_namespace(item, file_desc, output_file_name, d):
    odc_name = parser.ReadValues(item, 'ROOT')['namespace']
    namespace = "namespace unc {" + '\n' + 'namespace ' + odc_name + '{' +  '\n'
    test['headers']['namespace'] = namespace
    print  test['headers']['namespace']

# This method will generate multiple objects if parse class has nested objects
def read_nested_object(item, file_desc, members, output_file_name, d):
    print 'objects'
    print 'members', members
    url_name = parser.ReadValues(item, 'ROOT')['url_class']
    class_name = parser.ReadValues(item, 'ROOT')['parse_class']
    method = parser.ReadValues(item, 'ROOT')['methods']
    check_key = parser.ReadValues(item, url_name).has_key('interface')
    objects = ''
    print check_key
    if str(check_key) == 'True':
        interface_members = parser.ReadValues(item, url_name)['interface']
        for members in interface_members.split(','):
            print 'members-----1---', members
            objects = objects + 'std::string ' + 'get_%s'%(members.lower()) + '()'
            objects = objects + '{' +'\n\t'
            objects = objects + 'return %s_;'%(members) + '\n'
            objects = objects + '};' + '\n'
    objects = objects+ 'UncRespCode get_response(%s *parser) {' %(class_name) +'\n'
    objects = objects + ' std::string url(get_url());' + '\n'
    objects = objects + 'unc::restjson::RestUtil rest_util_obj(ctr->get_host_address(),ctr->get_user_name(),ctr->get_pass_word());' + '\n'
    objects = objects + 'unc::odcdriver::OdcController *odc_ctr = reinterpret_cast<unc::odcdriver::OdcController *>(ctr);' + '\n'
    objects = objects + 'unc::restjson::HttpResponse_t* response = rest_util_obj.send_http_request(url,restjson::HTTP_METHOD_GET,NULL,odc_ctr->get_conf_value());' + '\n'
    objects = objects + 'if (NULL == response) {' +'\n'
    objects = objects + '\t' +'pfc_log_error("Error Occured while getting httpresponse");' +'\n'
    objects = objects + '\t' +'return UNC_DRV_RC_ERR_GENERIC;' +'\n'
    objects = objects + '}' +'\n'
    objects = objects + 'int resp_code = response->code;' +'\n'
    objects = objects + 'if (HTTP_200_RESP_OK != resp_code) {' +'\n'
    objects = objects + '\t' +'pfc_log_error("Response code is not OK , resp : %d", resp_code);' +'\n'
    objects = objects + '\t' +'return UNC_DRV_RC_ERR_GENERIC;' +'\n'
    objects = objects  +'return UNC_DRV_RC_ERR_GENERIC;' +'\n'
    objects = objects  +'}' +'\n'
    objects = objects + 'if (NULL != response->write_data) {' +'\n'
    objects = objects + '\t' +'if (NULL != response->write_data->memory) {' +'\n'
    objects = objects + '\t \t' +'char *data = response->write_data->memory;' +'\n'
    objects = objects + '\t \t' +' pfc_log_trace("Data : %s", data);' +'\n'
    objects = objects + '\t \t' +'parser->jobj = restjson::JsonBuildParse::get_json_object(data);' +'\n'
    objects = objects + '\t \t' +'return UNC_RC_SUCCESS;' +'\n'
    objects = objects + '\t' +'}' +'\n'
    objects = objects + '}' +'\n'
    objects = objects + ' pfc_log_error("Response data is NULL");' +'\n'
    objects = objects + 'return UNC_DRV_RC_ERR_GENERIC;'+ '\n' + '}' +'\n'
    print objects
    d['objects'] = objects
    file = open(output_file_name, "a")
    file.write(d['objects'])
    file.close()

# This method will generate parser class and it will parse the structure members into it
def create_parser_class(item, file_desc, output_file_name, d):
    class_name = parser.ReadValues(item, 'ROOT')['parse_class']
    req_mem = parser.ReadValues(item, class_name)['request_members']
    struct_name = parser.ReadValues(item, req_mem)['struct_name']
    name = 'class %s {'%(class_name) + '\n\t' + 'public:' + '\n'  + '\t\t' + class_name + '() { };' + '\n\t\t' + '~' + class_name + '() { };'  + '\n'
    name = name + 'std::list<%s>'%(struct_name) + '%s' %(struct_name)+ '_;' +'\n'
    name = name + 'json_object *jobj;' + '\n'
    d['class'] = name
    print 'class_name', d['class']
    file = open(output_file_name, "a")
    file.write(d['class'])
    file.close()
    fill_config(item, req_mem, file_desc, output_file_name, d)
    get_config(item, req_mem, file_desc, output_file_name, d)
    type = parser.ReadValues(item, class_name)['type']
    for operation in type.split(','):
        if operation == 'CUD':
            build_config(item, req_mem, file_desc, output_file_name, d)

# It will create list object and it will push the structure objects into it
def get_config(item, req_mem, file_desc, output_file_name, d):
    class_name = parser.ReadValues(item, 'ROOT')['parse_class']
    req_mem = parser.ReadValues(item, class_name)['request_members']
    struct_name = parser.ReadValues(item, req_mem)['struct_name']
    config = 'std::list<%s> get_%s() {'%(struct_name, struct_name) + '\n'
    config = config + 'return %s_;'%(struct_name) + '\n'
    config = config + '}' + '\n'
    d['config'] = config
    file = open(output_file_name, "a")
    file.write(d['config'])
    file.close()

# This method will generate methods for structure members whenever a new data type encountered
def fill_config(item, req_mem, file_desc, output_file_name, d):
    req_mem_type = parser.ReadValues(item, req_mem)['type']
    struct_name = parser.ReadValues(item, req_mem)['struct_name']
    check_bool = parser.ReadValues(item, req_mem)['check_bool_set']
    child = parser.ReadValues(item, req_mem)['members']
    req_key = parser.ReadValues(item, req_mem)['key']
    fill_method = 'UncRespCode set_%s('%(struct_name) +'json_object* json_parser) {' + '\n'
    fill_method = fill_method + 'int ret_val = restjson::REST_OP_FAILURE;' + '\n'
    fill_method = fill_method +'json_object* obj_%s'%(req_mem) + '= NULL;' + '\n'
    if check_bool == 'no':
        fill_method = fill_method +'ret_val = restjson::JsonBuildParse::parse(json_parser,%s,0,'%(req_key) +'obj_%s);'%(req_mem) + '\n'
    elif check_bool == 'yes':
        fill_method = fill_method +'ret_val = restjson::JsonBuildParse::parse(json_parser,%s,-1,'%(req_key) +'obj_%s);'%(req_mem) + '\n'
    fill_method = fill_method + 'if ((restjson::REST_OP_SUCCESS != ret_val) || (json_object_is_type(obj_%s, json_type_null))) {'%(req_mem)  + '\n'
    fill_method = fill_method + '\t' + 'json_object_put(json_parser);' + '\n'
    fill_method = fill_method + '\t' + 'pfc_log_error(" Error while parsing %s");'%(req_mem) + '\n'
    fill_method = fill_method + '\t' + 'return UNC_DRV_RC_ERR_GENERIC;' +'\n'
    fill_method = fill_method +'}' + '\n'
    d['fill'] = fill_method
    file = open(output_file_name, "a")
    file.write(d['fill'])
    file.close()
    for child in child.split(','):
        print 'child-name---', child
        child_type = parser.ReadValues(item, child)['type']
        print 'child_type=====', child_type
    if child_type == 'array':
        obj_in = 'obj_' + req_mem
        print "obj_in-------->1", obj_in
        ar_obj = parse_array_object(item, child, obj_in, output_file_name, d)
        print "json object to parse member:", ar_obj
    func_end = 'if (restjson::REST_OP_SUCCESS != ret_val)' + '\n'
    func_end = func_end + '\t' + 'return UNC_DRV_RC_ERR_GENERIC;' + '\n'
    func_end = func_end + 'return UNC_RC_SUCCESS;' + '\n'
    func_end = func_end + '}' + '\n'
    d['func_end'] = func_end
    file = open(output_file_name, "a")
    file.write(d['func_end'])
    file.close()

# This method will generate build methods for structure members
def build_config(item, req_mem, file_desc, output_file_name, d):
    class_name = parser.ReadValues(item, 'ROOT')['parse_class']
    req_mem = parser.ReadValues(item, class_name)['request_members']
    member = parser.ReadValues(item, req_mem)['members']
    child = parser.ReadValues(item, member)['members']
    struct_name = parser.ReadValues(item, req_mem)['struct_name']
    build_begin = 'json_object *create_req (%s&  %s_st){'%(struct_name, struct_name) + '\n'
    build_begin = build_begin + 'json_object *jobj = unc::restjson::JsonBuildParse::create_json_obj();' + '\n'
    build_begin = build_begin + 'uint32_t ret_val = restjson::REST_OP_FAILURE;' + '\n'
    d['build_begin'] = build_begin
    file = open(output_file_name, "a")
    file.write(d['build_begin'])
    file.close()
    for child in child.split(','):
        print "child", child
        build_type = parser.ReadValues(item, child)['build_support']
        if build_type != 'no':
            child_type = parser.ReadValues(item, child)['type']
            key_name = parser.ReadValues(item, child)['key']
            key_s = key_name.replace('"', '')
            print key_s
            if child_type != 'struct':
                write_build(key_name, struct_name, child, key_s, child_type, output_file_name, d)
            elif child_type == 'struct':
                print "struct---", key_name
                st_name = struct_name +'_st.'+ child +'_'
                parent_obj = 'jobj'
                build_struct(item, st_name, child, parent_obj, output_file_name, d)
    build_end = 'return jobj;' + '\n'
    build_end = build_end + '}'  + '\n'
    d['build_end'] = build_end
    file = open(output_file_name, "a")
    file.write(d['build_end'])
    file.close()

# This method will generate the request body for build members
def write_build(key_name, struct_name, child, key_s, child_type, output_file_name, d):
    build_member = ''
    if child_type == 'string' or child_type == 'int' or child_type == 'bool':
        if child_type == 'string':
            build_member = build_member + 'if(!%s_st.%s.empty()){'%(struct_name, child) + '\n'
        elif child_type == 'int':
            build_member = build_member + 'if (%s_st.%s != 0){'%(struct_name, child) + '\n'
        elif child_type == 'bool':
            build_member = build_member + 'if (%s_st.%s == 0){'%(struct_name, child) + '\n'
        build_member = build_member + '\t' + 'ret_val = unc::restjson::JsonBuildParse::build(%s,%s_st.%s,jobj);'%(key_name, struct_name, child) + '\n'
        build_member = build_member + 'if (restjson::REST_OP_SUCCESS != ret_val) {' + '\n'
        build_member = build_member + '\t' + 'pfc_log_error("Error in building request body %s");'%(key_s) + '\n'
        build_member = build_member + '\t' + 'json_object_put(jobj);' + '\n'
        build_member = build_member + '\t' + 'return NULL;' + '\n'
        build_member = build_member + '\t' + '}' + '\n' + '}' + '\n'
        d['build_member'] = build_member
        file = open(output_file_name, "a")
        file.write(d['build_member'])
        file.close()
    else:
        print "unsupported type"

# This method will generate build methods for structure members
def write_st_build(key_name, st_name, child, key_s, child_type, parent, output_file_name, d):
    #for build struct members
    build_st_member = ''
    if child_type == 'string' or child_type == 'int' or child_type == 'bool':
        if child_type == 'string':
            build_st_member = build_st_member + 'if(!%s.%s.empty()){'%(st_name, child) + '\n'
        elif child_type == 'int':
            build_method = build_st_member + 'if (%s.%s != 0){'%(st_name, child) + '\n'
        elif child_type == 'bool':
            build_method = build_st_member + 'if (%s_st.%s == 0){'%(st_name, child) + '\n'
        build_st_member = '\t' + 'ret_val = unc::restjson::JsonBuildParse::build(%s,%s.%s,%s);'%(key_name, st_name, child, parent) + '\n'
        build_st_member = build_st_member + 'if (restjson::REST_OP_SUCCESS != ret_val) {' + '\n'
        build_st_member = build_st_member + '\t' + 'pfc_log_error("Error in building request body %s");'%(key_s) + '\n'
        build_st_member = build_st_member + '\t' + 'json_object_put(jobj);' + '\n'
        build_st_member = build_st_member + '\t' + 'return NULL;' + '\n'
        build_st_member = build_st_member + '}' + '\n'
        d['build_st_member'] = build_st_member
        file = open(output_file_name, "a")
        file.write(d['build_st_member'])
        file.close()
    else:
        print "unsupported type"

# This method will only generate build methods if build_type=yes in .rest files
def build_struct(item, st_name, member, parent_obj, output_file_name, d):
    build_st_begin = 'json_object *%s_obj = unc::restjson::JsonBuildParse::create_json_obj();'%(member) + '\n'
    d['build_st_begin'] = build_st_begin
    file = open(output_file_name, "a")
    file.write(d['build_st_begin'])
    file.close()
    st_child_mem = parser.ReadValues(item, member)['members']
    st_key = parser.ReadValues(item, member)['key']
    for child in st_child_mem.split(','):
        print "child", child
        parent = member + '_obj'
        build_type = parser.ReadValues(item, child)['build_support']
        if build_type != 'no':
            child_type = parser.ReadValues(item, child)['type']
            key_name = parser.ReadValues(item, child)['key']
            key_s = key_name.replace('"', '')
            print key_s
            if child_type != 'struct':
                write_st_build(key_name, st_name, child, key_s, child_type, parent, output_file_name, d)
            elif child_type == 'struct':
                key_name = parser.ReadValues(item, child)['key']
                key_s = key_name.replace('"', '')
                print "struct---", key_name
                st_name = st_name + '.'+ child +'_'
                build_struct(item, st_name, child, parent, output_file_name, d)
    i = 0
    for child in st_child_mem.split(','):
        mandatory_parm = parser.ReadValues(item, child)['mandatory']
        print "MANDAROTY PARAM", mandatory_parm
        if mandatory_parm != 'no':
            type_mem = parser.ReadValues(item, child)['type']
            if i == 0:
                if type_mem == 'string':
                    build_st_end = 'if((!%s.%s.empty())'%(st_name, child)
                elif type_mem == 'bool':
                    build_st_end = 'if(%s.%s != PFC_FALSE '%(st_name, child)
                elif type_mem == 'int':
                    build_st_end = 'if(%s.%s != 0 '%(st_name, child)
                i = i+1
            elif i > 0:
                if type_mem == 'string':
                    build_st_end = build_st_end + ' && (%s.%s.empty())'%(st_name, child)
                elif type_mem == 'bool':
                    build_st_end = build_st_end + ' && (%s.%s != PFC_FALSE '%(st_name, child)
                elif type_mem == 'int':
                    build_st_end = build_st_end + ' && (%s.%s != 0 '%(st_name, child)
                    build_st_end = build_st_end + '){' + '\n'
    build_st_end = build_st_end + '\t' + 'ret_val = unc::restjson::JsonBuildParse::build <json_object*>' + '\n'
    build_st_end = build_st_end + '\t \t \t \t (%s, %s_obj, %s);'%(st_key, member, parent_obj)  + '\n'
    build_st_end = build_st_end + 'if (restjson::REST_OP_SUCCESS != ret_val) {'+ '\n'
    build_st_end = build_st_end + '\t pfc_log_debug("Failed in framing json request body for %s");'%(member)  + '\n'
    build_st_end = build_st_end + '\t json_object_put(%s);'%(parent_obj)  + '\n'
    build_st_end = build_st_end + '\t json_object_put(%s_obj);'%(member)  + '\n'
    build_st_end = build_st_end + '\t return NULL;'  + '\n'
    build_st_end = build_st_end + '}'  + '\n' + '}' + '\n'
    d['build_st_end'] = build_st_end
    file = open(output_file_name, "a")
    file.write(d['build_st_end'])
    file.close()

# This method will parse members based on structure member data types
def parse_member(item, struct_name, obj_in, member, output_file_name, d):
    req_key = parser.ReadValues(item, member)['key']
    type_name = parser.ReadValues(item, member)['type']
    key_s = req_key.replace('"', '')
    print "type_key", type_name
    parse = parser.ReadValues(item, member)['parse_support']
    if type_name == 'struct':
        parse_struct_object(item, struct_name, member, obj_in, output_file_name, d)
    elif type_name != 'struct':
        if parse == 'yes':
            object = '\n' + 'ret_val = restjson::JsonBuildParse::parse(%s,%s'%(obj_in, req_key) +',arr_idx,st_%s.%s);'%(struct_name, member) + '\n'
            if type_name == 'string':
                object = object + 'if ((restjson::REST_OP_SUCCESS != ret_val) || (st_%s.%s.empty())) {'%(struct_name, member) + '\n'
            elif type_name == 'int':
                object = object + 'if (restjson::REST_OP_SUCCESS != ret_val) {'+ '\n'
            elif type_name == 'bool':
                object = object + 'if (restjson::REST_OP_SUCCESS != ret_val) {'+ '\n'
            object = object + '\t' + 'pfc_log_error(" Error while parsing %s");'%(key_s) + '\n'
            object = object + '\t' + 'return UNC_DRV_RC_ERR_GENERIC;' + '\n'
            object = object + '}' + '\n'
            d['object'] = object
            file = open(output_file_name, "a")
            file.write(d['object'])
            file.close()
        else:
            print"member doesnot support parse"

# This method will parse the structure objects
def parse_struct_object(item, struct_name, st_member, obj_in, output_file_name, d):
    req_key = parser.ReadValues(item, st_member)['key']
    sub_members = parser.ReadValues(item, st_member)['members']
    print 'sub_members', sub_members
    parse_st = 'json_object *jobj_'+st_member + '= NULL;' +'\n'
    parse_st = parse_st + 'ret_val = unc::restjson::JsonBuildParse::parse(%s,%s,-1,jobj_%s);'%(obj_in, req_key, st_member) + '\n'
    parse_st = parse_st + 'if (restjson::REST_OP_SUCCESS != ret_val) {' + '\n'
    parse_st = parse_st + '\t' + 'pfc_log_debug("%s is null");' %(st_member) + '\n'
    parse_st = parse_st + '\t' + 'json_object_put(%s);' %(obj_in) + '\n'
    parse_st = parse_st + '\t' +'return UNC_DRV_RC_ERR_GENERIC;' + '\n'
    parse_st = parse_st + '}' +'\n'
    d['parse_st'] = parse_st
    file = open(output_file_name, "a")
    file.write(d['parse_st'])
    file.close()
    print "parse_struct_object st_mem:", sub_members
    print 'struct %s', st_member
    for member in sub_members.split(','):
        print "submember in parse_struct_object:", member
        sub_mem_type = parser.ReadValues(item, member)['type']
        print 'sub_mem_type', sub_mem_type
        if sub_mem_type != 'array':
            st_name = struct_name +'.'+st_member+'_'
            parent_obj = 'jobj_' + st_member
            parse_member(item, st_name, parent_obj, member, output_file_name, d)
        elif sub_mem_type == 'array':
            print "array type inside struct"

# This method will generate parse methods for structure members
def parse_array_object(item, member, obj_in, output_file_name, d):
    req_key = parser.ReadValues(item, member)['key']
    st_name = parser.ReadValues(item, member)['struct_name']
    check_bool = parser.ReadValues(item, member)['check_bool_set']
    key_s = req_key.replace('"', '')
    object = 'uint32_t array_length = 0;' + '\n'
    object = object +'json_object *obj_%s = NULL;'%(member) +'\n'
    if check_bool == 'yes':
        object = object +'ret_val = restjson::JsonBuildParse::parse(%s,%s'%(obj_in, req_key) +',0,obj_%s);'%(member) + '\n'
    elif check_bool == 'no':
        object = object +'ret_val = restjson::JsonBuildParse::parse(%s,%s'%(obj_in, req_key) +',-1,obj_%s);'%(member) + '\n'
    object = object + 'if ((restjson::REST_OP_SUCCESS != ret_val) || (json_object_is_type(obj_%s, json_type_null))) {'%(member)  + '\n'
    object = object + '\t' + 'json_object_put(json_parser);' + '\n'
    object = object + '\t' + 'pfc_log_error(" Error while parsing %s");'%(key_s) + '\n'
    object = object + '\t' + 'return UNC_DRV_RC_ERR_GENERIC;' + '\n'
    object = object + '}' + '\n'
    object = object + 'if (json_object_is_type(obj_%s, json_type_array)) {'%(member) + '\n'
    check_key = parser.ReadValues(item, member).has_key('s_array')
    if str(check_key) == 'True':
        object = object + '\t' + 'array_length = restjson::JsonBuildParse::get_array_length(obj_%s,%s);'%(member, req_key) + '\n'
    else:
        object = object + '\t' + 'array_length = restjson::JsonBuildParse::get_array_length(%s,%s);'%(obj_in, req_key) + '\n'
    object = object +  '}' + '\n'
    object = object + 'if (0 == array_length) {' + '\n'
    object = object + '\t' +'pfc_log_debug("No %s present");'%(member) +'\n'
    object = object + '\t' +'json_object_put(json_parser);' + '\n'
    object = object + '\t' + 'return UNC_RC_SUCCESS;' + '\n'
    object = object +  '}' + '\n'
    object = object + 'for (uint32_t arr_idx = 0; arr_idx < array_length; arr_idx++) {' +'\n'
    object = object + st_name + ' st_%s;'%(st_name) +'\n'
    d['object'] = object
    file = open(output_file_name, "a")
    file.write(d['object'])
    file.close()
    sub_member = parser.ReadValues(item, member)['members']
    for members in sub_member.split(','):
        print "submember in parse_array_object:", members
        sub_mem_type = parser.ReadValues(item, members)['type']
        obj = 'obj_' + member
        s_name = parser.ReadValues(item, members)['struct_name']
        if sub_mem_type != 'array':
            print "member send to parse_member", obj
            parse_member(item, s_name, obj, members, output_file_name, d)
        elif sub_mem_type == 'array':
            prefix = 'st_' + st_name
            parse_nested_array_object(item, members, obj, output_file_name, d, prefix)
    #call nested parse_array obj with prefix
    loop_end = st_name +'_.push_back(st_%s);'%(st_name) + '\n'
    loop_end = loop_end +'}' + '\n'

    #find length parse child members if type array call again parse array with prefix
    d['loop_end'] = loop_end
    file = open(output_file_name, "a")
    file.write(d['loop_end'])
    file.close()
    return 'obj_' +member

# This method will generate nested array for structure members
def parse_nested_array_object(item, member, obj_in, output_file_name, d, prefix):
    print "calling array nested obj"
    req_key = parser.ReadValues(item, member)['key']
    st_name = parser.ReadValues(item, member)['struct_name']
    check_bool = parser.ReadValues(item, member)['check_bool_set']
    object = 'uint32_t array_length = 0' + '\n'
    object = object + 'json_object *obj_%s = NULL;'%(member) +'\n'
    object = object +'ret_val = restjson::JsonBuildParse::parse(%s,%s'%(obj_in, req_key) +',-1,obj_%s);'%(member) + '\n'
    object = object + 'if ((restjson::REST_OP_SUCCESS != ret_val) || (json_object_is_type(obj_%s, json_type_null))) {'%(member) + '\n'
    object = object + '\t' + 'json_object_put(json_parser);' + '\n'
    object = object + '\t' + 'pfc_log_error(" Error while parsing %s");'%(member) + '\n'
    object = object + '\t' + 'return UNC_DRV_RC_ERR_GENERIC;' + '\n'
    object = object  + '}' + '\n'
    object = object + 'if (json_object_is_type(obj_%s, json_type_array)) {'%(member) + '\n'
    object = object + '\t' + 'array_length = restjson::JsonBuildParse::get_array_length(%s,%s);'%(obj_in, req_key) + '\n'
    object = object +  '}' + '\n'
    object = object + 'if (0 == array_length) {' + '\n'
    object = object + '\t' +'pfc_log_debug("No %s present");'%(member) +'\n'
    object = object + '\t' +'json_object_put(jobj);' + '\n'
    object = object + '\t' + 'return UNC_RC_SUCCESS;' + '\n'
    object = object +  '}' + '\n'
    object = object + 'for (uint32_t arr_idx = 0; arr_idx < array_length; arr_idx++) {' +'\n'
    object = object + st_name + ' st_%s;'%(st_name) +'\n'
    d['object'] = object
    file = open(output_file_name, "a")
    file.write(d['object'])
    file.close()
    sub_member = parser.ReadValues(item, member)['members']
    for members in sub_member.split(','):
        print "submember in parse_array_object:", members
        sub_mem_type = parser.ReadValues(item, members)['type']
        print 'subtype', sub_mem_type
        if sub_mem_type != 'array':
            s_name = parser.ReadValues(item, members)['struct_name']
            obj = 'obj_' + member
            print "member send to parse_member", obj
            parse_member(item, s_name, obj, members, output_file_name, d)
        elif sub_mem_type == 'array':
            prefix = '.' + st_name
            print "prefix value", prefix
            parse_nested_array_object(item, members, obj_in, output_file_name, d, prefix)
    loop = prefix +'.push_back(st_%s);'%(st_name) + '\n'
    loop = loop +'}' + '\n'
    d['loop'] = loop
    file = open(output_file_name, "a")
    file.write(d['loop'])
    file.close()
    return 'obj_' +member

# This method will generate '}' curly braces to end of the namespace
def end_namespace(item, file_desc):
    odc_name = parser.ReadValues(item, 'ROOT')['namespace']
    end = '} // namespace ' + odc_name  + '\n'
    end = end + '} // namespace unc' + '\n'
    end = end + '#endif' + '\n'
    d['block_closure'] = end
    with open(output_file_name, "a") as f:
        f.write(d['block_closure'])

# This method will generate '};' curly braces to end of the class
def end_class(item, file_desc):
    end = '\n' + '};' + '\n'
    d['block_closure'] = end
    file = open(output_file_name, "a")
    file.write(d['block_closure'])
    file.close()

# End of the header files
def end_footers(file_desc):
    print 'footer'

# This method will generate boolean data type for structure members
def write_boolean(item, file_desc, member, output_file_name, d):
    print 'boolean'
    boolean = '\t' + 'bool %s;'%(member) + '\n'
    d['data_type'] = boolean
    with open(output_file_name, "a") as f:
        f.write(d['data_type'])

# This method will generate nested objects based on element_type
def nested_objects(item, file_desc, member, output_name, d):
    print 'entering in to objects nested object method'
    print 'members', member
    print 'object'
    members = parser.ReadValues(item, member)['members']
    print 'object_members:', members
    for obj_members in members.split(','):
        print 'obj_members:', obj_members
        element_type = parser.ReadValues(item, obj_members)['type']
        print 'element_type:', element_type
        print 'member:', obj_members
        if element_type == 'array':
            print 'calling', write_list
            write_list(item, file_desc, obj_members, output_file_name, d)
        elif element_type == 'struct':
            write_struct(item, file_desc, obj_members, output_file_name, d)
        elif element_type != 'array':
            write_options[element_type](item, file_desc, obj_members, output_file_name, d)

# This method will write the structure name into the list
def write_list(item, file_desc, member, output_file_name, d):
    print 'list'
    struct_name = parser.ReadValues(item, member)['struct_name']
    print 'write list struct name', struct_name
    list = '\t' + 'std::list<%s> %s_;'%(member, struct_name) + '\n'
    d['data_type'] = list
    with open(output_file_name, "a") as f:
        f.write(d['data_type'])

# This method will generate struct data types for structure members
def write_struct(item, file_desc, member, output_file_name, d):
    print 'struct member'
    struct = '\t' + '%s %s_;'%(member, member) + '\n'
    d['data_type'] = struct
    with open(output_file_name, "a") as f:
        f.write(d['data_type'])

# This method will generate array data types for structure members
def write_array(item, file_desc, member, output_file_name, d):
    print 'array'
    print 'array_member', member
    array = '\t' + '%s %s;'%(member, member) + '\n'
    d['data_type'] = array
    with open(output_file_name, "a") as f:
        f.write(d['data_type'])
    obj_sub_member = parser.ReadValues(item, member)['members']
    for obj_member in obj_sub_member.split(','):
        print 'arrr', obj_member
        element_type = parser.ReadValues(item, obj_member)['type']
        print element_type
        write_options[element_type](item, file_desc, obj_member, output_file_name, d)

# This method will generate integer data types for structure members
def write_integer(item, file_desc, member, output_file_name, d):
    print 'integer'
    integer = '\t' + 'int %s;'%(member) + '\n'
    d['data_type'] = integer
    with open(output_file_name, "a") as f:
        f.write(d['data_type'])

# This method will generate string data types for structure members
def write_string(item, file_desc, obj_member, output_file_name, d):
    print 'string'
    string = '\t' + 'std::string %s;'%(obj_member)+ '\n'
    d['data_type'] = string
    with open(output_file_name, "a") as f:
        f.write(d['data_type'])

# This method will generate typedef keyword for structure members
def typedef_structure(item, file_desc, method, output_file_name, d):
    struct_members = parser.ReadValues(item, 'ROOT')['data']
    struct_type = parser.ReadValues(item, 'ROOT')['struct_type']
    print 'members-->', struct_members
    for member in struct_members.split(','):
        print member
        members = 'struct %s {'%(member) + '\n'
        d['struct'] = members
        with open(output_file_name, "a")as f:
            f.write(d['struct'])
        element_type = parser.ReadValues(item, member)['type']
        print 'member-->', 'element_type-->', member, element_type
        print 'member to send', member
        nested_objects(item, file_desc, member, output_file_name, d)
        members = '};'+ '\n'
        d['end_paranthesis'] = members
        f = open(output_file_name, "a")
        f.write(d['end_paranthesis'])

write_options = {'bool': write_boolean, 'int': write_integer, 'string' : write_string, 'array' : nested_objects, 'object' : nested_objects, 'list' : write_list, 'struct' : write_struct}

# This method will generate parser class
def class_names(item, file_desc, url_name, output_file_name, d):
    method_name = str(url_name)
    print 'method_name', method_name
    print  'public:'
    d['class_name'] = 'class %s {' %(method_name.lower()) + '\n' + 'public:' + '\n\t'
    file = open(output_file_name, "a")
    file.write(d['class_name'])

# This method will generate URL class for READ and CUD URL's
def class_name(item, file_desc):
    class_name = parser.ReadValues(item, 'ROOT')['url_class']
    method = parser.ReadValues(item, 'ROOT')['methods']
    req_members = parser.ReadValues(item, class_name)['request_members']
    url_types = parser.ReadValues(item, method)['type']
    member = parser.ReadValues(item, req_members)['members']
    check_key = parser.ReadValues(item, class_name).has_key('interface')
    if str(check_key) == 'True':
        interface_members = parser.ReadValues(item, class_name)['interface']
        name = 'class %s {'%(class_name) + '\n\t\t' + 'public:' + '\n'  + '\t\t'
        name = name + class_name + '(unc::driver::controller *ctr_ptr'
        for members in interface_members.split(','):
            name = name + ',std::string %s'%(members)
        name = name + ') :ctr(ctr_ptr)'
        for members in interface_members.split(','):
            name = name + ',%s_(%s)'%(members, members)
        name = name + ' { };' + '\n\t\t' + '~' + class_name + '() { };' + '\n\t'
        name = name + '\t' + 'unc::driver::controller *ctr ;'+ '\n'
        for members in interface_members.split(','):
            name = name +'\t\t' + 'std::string %s_;'%(members) +'\n'
    else:
        name = 'class %s {'%(class_name) + '\n\t' + 'public:' + '\n'  + '\t\t'
        name = name + class_name + '(unc::driver::controller *ctr_ptr):ctr(ctr_ptr) { };' + '\n\t\t' + '~' + class_name + '() { };' + '\n\t'
        name = name + '\t' + 'unc::driver::controller *ctr ;'+ '\n\t\t'
    d['name'] = name
    file = open(output_file_name, "a")
    file.write(d['name'])
    file.close()
    for type in url_types.split(','):
        if type == 'READ':
            get_url_creation(item, file_desc, method, output_file_name, test)
        if type == 'CUD':
            get_url_Read(item, file_desc, method, output_file_name, test)
            CUD = ['CREATE', 'UPDATE', 'DELETE']
            for index in range(len(CUD)):
                print len(CUD)
                element = CUD[index]
                print element
                if 'CREATE' in element:
                    create_CUD_method(item, 'post', 'HTTP_METHOD_POST', 'HTTP_201_RESP_CREATED')
                elif 'UPDATE' in element:
                    create_CUD_method(item, 'put', 'HTTP_METHOD_PUT', 'HTTP_204_NO_CONTENT')
                elif 'DELETE' in element:
                    create_CUD_method(item, 'delete', 'HTTP_METHOD_DELETE', 'HTTP_200_RESP_OK')
                else:
                    return 0
                index = index +1
                print index

    read_nested_object(item, file_desc, member, output_file_name, d)

# This method will generate Create, Update and Delete URL's
def create_CUD_method(item, operation, HTTP_METHOD_OPERATION, HTTP_CODE_RESP):
    class_name = parser.ReadValues(item, 'ROOT')['parse_class']
    check_key = parser.ReadValues(item, class_name).has_key('set_delete')
    print str(check_key)
    if HTTP_METHOD_OPERATION == 'HTTP_METHOD_POST' or HTTP_METHOD_OPERATION == 'HTTP_METHOD_PUT':
        objects = 'UncRespCode  set_'+ operation + '(json_object *jobj){' + '\n'
        objects = objects + '\t' + 'std::string url = (get_cud_url());' + '\n'
    else:
        if(check_key):
            objects = 'UncRespCode  set_'+ operation + '(std::string  end_url){' + '\n'
            objects = objects + '\t' + 'std::string url = (get_cud_url());' + '\n'
            objects = objects + '\t' + 'url = url.append("/");' + '\n'
            objects = objects + '\t' + 'url = url.append(end_url);' + '\n'
        else:
            objects = 'UncRespCode  set_'+ operation + '(){' + '\n'
            objects = objects + '\t' + 'std::string url = (get_cud_url());' + '\n'
    objects = objects + '\t' + 'unc::restjson::RestUtil rest_util_obj(ctr->get_host_address(),ctr->get_user_name(),ctr->get_pass_word());' + '\n'
    objects = objects + '\t' + 'unc::odcdriver::OdcController *odc_ctr = reinterpret_cast<unc::odcdriver::OdcController *>(ctr);'+ '\n'
    objects = objects + '\t' + 'unc::restjson::HttpResponse_t* response = rest_util_obj.send_http_request(url,restjson::' + '\n'
    if HTTP_METHOD_OPERATION == 'HTTP_METHOD_POST' or HTTP_METHOD_OPERATION == 'HTTP_METHOD_PUT':
        objects = objects + '\t' + HTTP_METHOD_OPERATION +',unc::restjson::JsonBuildParse::get_json_string(jobj),odc_ctr->get_conf_value());' + '\n'
    else:
        objects = objects + '\t' + HTTP_METHOD_OPERATION + ', NULL,odc_ctr->get_conf_value());' + '\n'
    objects = objects + '\t' + 'if (NULL == response) {' + '\n'
    objects = objects + '\t\t' + 'pfc_log_error("Error Occured while getting httpresponse");'+ '\n'
    if HTTP_METHOD_OPERATION == 'HTTP_METHOD_POST' or HTTP_METHOD_OPERATION == 'HTTP_METHOD_PUT':
        objects = objects + '\t' + 'json_object_put(jobj);' + '\n'
    objects = objects + '\t' + 'return UNC_DRV_RC_ERR_GENERIC;' +'\n'  '}' + '\n'
    objects = objects + '\t' + 'int resp_code = response->code;' + '\n'
    if HTTP_METHOD_OPERATION == 'HTTP_METHOD_PUT':
        objects = objects + '\t' + 'if ((' + HTTP_CODE_RESP +' != resp_code) && (HTTP_200_RESP_OK != resp_code)) {' + '\n'
    else:
        objects = objects + '\t' + 'if (' + HTTP_CODE_RESP +' != resp_code) {' + '\n'
    objects = objects + '\t\t' + 'pfc_log_error("'+ operation +' is not success , resp_code %d", resp_code);'
    objects = objects + '\n' + '\t\t'+'return UNC_DRV_RC_ERR_GENERIC;' +'\n' + '}'
    objects = objects +  '\n' + 'return UNC_RC_SUCCESS;' +'\n' '}' + '\n'
    d['objects'] = objects
    print objects
    file = open(output_file_name, "a")
    file.write(d['objects'])
    file.close()

# It will create GET URL method
def get_url_creation(item, file_desc, methods, output_file_name, d):
    url_name = parser.ReadValues(item, 'READ')['url']
    print 'url_name', url_name
    url_format = parser.ReadValues(item, url_name)['url_format']
    print 'url_format', url_format
    print len(url_format)
    print url_format
    a = url_format.split(',')
    print 'split_name', a
    base_url = ''
    print 'base_url', base_url
    base_url = base_url + '\t\t' +  'std::string get_url() {' + '\n'
    base_url = base_url + '\t\t' +  'std::string url = "";' + '\n'
    for members in url_format.split(','):
        print members
        print 'url_format', url_name
        check_key = parser.ReadValues(item, members).has_key('value')
        check_get_abstract_key = parser.ReadValues(item, members).has_key('get_abstract')
        print 'get_abstract', check_get_abstract_key
        print 'check_key', check_key
        if str(check_key) == 'True':
            value = parser.ReadValues(item, members)['value']
            base_url = base_url + '\t\t' + 'url.append(%s);' %(value) + '\n'
        if str(check_get_abstract_key) == 'True':
            base_url = base_url + '\t\t' + 'url.append(get_%s());' %(members.lower()) + '\n'
    base_url = base_url + '\t\t' + 'return url;' +'\n'
    base_url = base_url + '}' + '\n'
    d['base_url'] = base_url
    file = open(output_file_name, "a")
    file.write(d['base_url'])
    file.close()

# READ method for Create, Update and Delete URL's
def get_url_Read(item, file_desc, methods, output_file_name, d):
    url_name = parser.ReadValues(item, 'CUD')['url']
    print 'url_name', url_name
    url_format = parser.ReadValues(item, url_name)['url_format']
    print 'url_format', url_format
    print len(url_format)
    print url_format
    a = url_format.split(',')
    print 'split_name', a
    base_url = ''
    print 'base_url', base_url
    base_url = 'std::string get_cud_url() {' + '\n'
    base_url = base_url + '\t\t' +  'std::string url = "";' + '\n'
    for members in url_format.split(','):
        print members
        print 'url_format', url_name
        check_key = parser.ReadValues(item, members).has_key('value')
        check_get_abstract_key = parser.ReadValues(item, members).has_key('get_abstract')
        print 'get_abstract', check_get_abstract_key
        print 'check_key', check_key
        if str(check_key) == 'True':
            value = parser.ReadValues(item, members)['value']
            base_url = base_url + '\t\t' + 'url.append(%s);' %(value) + '\n'
        if str(check_get_abstract_key) == 'True':
            base_url = base_url + '\t\t' + 'url.append(get_%s());' %(members.lower()) + '\n'
    base_url = base_url + '\t\t' + 'return url;' +'\n'
    base_url = base_url + '}' + '\n'
    d['base_url'] = base_url
    file = open(output_file_name, "a")
    file.write(d['base_url'])
    file.close()

def method(item, file_desc, output_file_name, d):
    print 'method implementation'
    methods = parser.ReadValues(item, 'ROOT')['methods']
    print 'methods', methods
    for method in methods.split(','):
        print method
        Element_Type = parser.ReadValues(item, method)['type']
        print Element_Type
        if Element_Type == 'POST' or Element_Type == 'PUT' or Element_Type == 'DELETE'\
            or Element_Type == 'GET' or Element_Type == 'VALIDATE' or Element_Type == 'READ':
            write_methods[Element_Type](item, file_desc, method, output_file_name, test)

# This method will write headers, includes and preprocessore into output file
def write_headers(item, file_desc, output_file_name, test):
    with open(output_file_name, "a") as file:
        file.write(test['headers']['Preprocessor'])
        file.write(test['headers']['includes'])
        file.write(test['headers']['namespace'])

# This method will generate the build and parse methods for structure members
def json_build_parse(item, file_desc, output_file_name, d):
    print "Json Build Parse Started"
    begin_headers(item, file_desc, output_file_name, d)
    begin_include(item, file_desc, output_file_name, d)
    begin_namespace(item, file_desc, output_file_name, d)
    write_headers(item, file_desc, output_file_name, test)
    typedef_structure(item, file_desc, method, output_file_name, d)
    create_parser_class(item, file_desc, output_file_name, d)
    end_class(item, file_desc)
    class_name(item, file_desc)
    end_class(item, file_desc)
    end_namespace(item, file_desc)
    end_footers(file_desc)

# Main Function
if __name__ == '__main__':
    test = collections.defaultdict(dict)
    test['structs'] = collections.defaultdict(dict)
    print 'dict', test['structs']
    rest_file = sys.argv[1]
    output_file_name = sys.argv[2]
    file_name = rest_file[:-4] + 'hh'
    output_file_name = output_file_name + file_name
    file_desc = open(output_file_name, "w")
    json_build_parse(rest_file, file_desc, output_file_name, d)
    file_desc.close()
else:
    print "Code_gen loaded as module"
