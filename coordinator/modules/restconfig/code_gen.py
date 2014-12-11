#!/usr/bin/python

#
# Copyright (c) 2014 NEC Corporation
# All rights reserved.
#
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

import parser, string


def begin_headers(item, file_desc):
 to_caps = item.split('.rest')
 to_caps = to_caps[0].upper()
 if_guard = "__" + to_caps + "_HH__"
 header = '#ifndef '  + if_guard + '**=='
 header = header + '#define ' + if_guard + '**=='
 header = header + '#include <unc/upll_ipc_enum.h>**==#include <unc/pfcdriver_ipc_enum.h>**==#include <odc_rest.hh>**=='
 for line in header.split('**=='):
  file_desc.write("%s\n" % line)

def begin_namespace(item, file_desc):
 odc_name = parser.ReadValues(item, 'code')['namespace']
 namespace = "namespace unc {" + '**==' + 'namespace ' + odc_name + '{**=='
 for line in namespace.split('**=='):
   file_desc.write("%s\n" % line)

def cons_des(item, file_desc):
 class_name = parser.ReadValues(item, 'class')['name']
 const_dest = 'class ' + class_name + ' {' + '**=='
 const_dest = const_dest + 'public:**=='
 const_dest = const_dest + class_name + '() {}**=='
 const_dest = const_dest + '~' + class_name + '() {}**=='
 for line in const_dest.split('**=='):
    file_desc.write("%s\n" % line)

def end_namespace (item, file_desc):
  end = '};**=='
  odc_name = parser.ReadValues(item, 'code')['namespace']
  end = end + '} // namespace ' + odc_name + '**=='
  end = end + '} // namespace unc'
  for line in end.split('**=='):
    file_desc.write("%s\n" % line)

def end_footers(file_desc):
  file_desc.write("#endif\n")

def set_boolean(file_desc, item, args1, args2, args3):
 print "Booleen"

def set_integer(item, file_desc, member, args1, args2, args3):
 ipc_valid_flag = parser.ReadValues(item, member)['ipc_valid_flag']
 ipc_name = parser.ReadValues(item, member)['ipc_name']
 category = parser.ReadValues(item, member)['category']
 struct = parser.ReadValues(item, member)['ipc_struct']
 key = parser.ReadValues(item, member)['key']
 if struct == args2:
   key_val = 'key'
 else:
   key_val = 'val'
 integer = 'UncRespCode ret_val = unc::restjson::JsonBuildParse::build("%s", %s.%s, %s);**=='%(key, key_val, ipc_name, args1)
 integer = integer + '}'
 for line in integer.split('**=='):
   file_desc.write("%s\n" % line)

def set_string(item, file_desc, member, args1, args2, args3):
  ipc_valid_flag = parser.ReadValues(item, member)['ipc_valid_flag']
  ipc_name = parser.ReadValues(item, member)['ipc_name']
  category = parser.ReadValues(item, member)['category']
  key = parser.ReadValues(item, member)['key']
  struct = parser.ReadValues(item, member)['ipc_struct']
  if struct == args2:
    key_val = 'key'
  else:
    key_val = 'val'
  if ipc_valid_flag == 'yes':
    ipc_valid_enum = parser.ReadValues(item, member)['ipc_valid_enum']
    strings = 'const char* %s = reinterpret_cast<const char*>(%s.%s);**=='%(key, key_val, ipc_name)
    strings = strings + 'if (UNC_VF_VALID == %s.valid[%s]) {**=='%(key_val,ipc_valid_enum)
    strings = strings + 'int ret_val = unc::restjson::JsonBuildParse::build("%s", %s, out);**=='%(key, key)
    strings = strings + 'if (restjson::REST_OP_SUCCESS != ret_val) {**=='
    strings = strings + 'pfc_log_error("Error occured in request body build %s", PFC_FUNCNAME);**=='
    strings = strings + 'return UNC_DRV_RC_ERR_GENERIC;**=='
    strings = strings + '}**=='
    strings = strings + '}**=='
    strings = strings + 'return UNC_RC_SUCCESS;**=='
    strings = strings + '}**=='
    for item in strings.split('**=='):
       file_desc.write("%s\n" % item)
  else:
    strings = 'const char* %s = reinterpret_cast<const char*>(%s.%s);**=='%(key,key_val,ipc_name)
    strings = strings + 'UncRespCode ret_val = (JsonBuildParse::build("%s", %s, args1));**=='%(key,key)
    strings = strings + 'if (restjson::REST_OP_SUCCESS != ret_val) {**=='
    strings = strings + 'pfc_log_error("Error occured in request body build %s", PFC_FUNCNAME);**=='
    strings = strings + 'return UNC_DRV_RC_ERR_GENERIC;**=='
    strings = strings + '}**=='
    strings = strings + 'return UNC_RC_SUCCESS;**=='
    strings = strings + '}**=='
    for line in strings.split('**=='):
        file_desc.write("%s\n" % line)

def set_array(item, file_desc, member, args1, args2, args3):
 members = parser.ReadValues(item, member)['members']
 arr = 'UncRespCode ' + member + '(' + args1 + ' *out, ' + args2 + ' &key, ' + args3 + ' &val' + ') {**=='
 arr = arr + 'json_object *%s = unc::restjson::JsonBuildParse::create_json_array_obj();**=='%(member)
 for arr_member in members.split(','):
   ipc_valid_flag = parser.ReadValues(item, arr_member)['ipc_valid_flag']
   if ipc_valid_flag == 'yes':
     ipc_valid_enum = parser.ReadValues(item, arr_member)['ipc_valid_enum']
     arr = arr + 'if (val.valid[%s] == UNC_VF_VALID) {**=='%(ipc_valid_enum)
     arr_sub_member = parser.ReadValues(item, arr_member)['members']
     element_type = parser.ReadValues(item, arr_sub_member)['type']
     ipc_struct = parser.ReadValues(item, arr_sub_member)['ipc_struct']
     ipc_name = parser.ReadValues(item, arr_sub_member)['ipc_name']
     key = parser.ReadValues(item, arr_sub_member)['key']
     arr = arr + 'json_object *%s = unc::restjson::JsonBuildParse::create_json_obj();**=='%(key)
     arr = arr + 'UncRespCode ret_val = unc::restjson::JsonBuildParse::build(%s, "%s", %s);**=='%(key, key, member)
     arr = arr + 'JsonBuildParse::add_to_array(%s, out);**=='%(member)
     arr = arr + '}**=='
       
 arr = arr + '}**=='
 for line in arr.split('**=='):
   file_desc.write("%s\n" % line)
     
def set_objects(item, file_desc, member, args1, args2, args3):
 members = parser.ReadValues(item, member)['members']
 set_support = parser.ReadValues(item, member)['set_support']
 for obj_member in members.split(','):
   if obj_member == 'empty':
     obj = 'UncRespCode ' + member + '( json_object *out, ' + args2 + ' &key, ' + args3 + ' &val' + ') {**=='
     ipc_valid_flag = parser.ReadValues(item, member)['ipc_valid_flag']
     if ipc_valid_flag == 'yes':
       ipc_valid_enum = parser.ReadValues(item, member)['ipc_valid_enum']
       key = parser.ReadValues(item, member)['key']
       obj = obj + 'if (UNC_VF_VALID == [%s]) {**=='%(ipc_valid_enum)
       obj = obj + 'json_object *%s = unc::restjson::JsonBuildParse::create_json_obj();**=='%(member)
       obj = obj + 'UncRespCode ret_val = unc::restjson::JsonBuildParse::build(%s, "%s", out);**=='%(key, key)
       obj = obj + '}**=='
       obj = obj + '}**=='
       for item in obj.split('**=='):
         file_desc.write("%s\n" % item)
     return
   set_methods(item, file_desc, obj_member, set_support, member, args2, args3)
 
 obj = 'UncRespCode ' + member + '(' + args1 + ' *out, ' + args2 + ' &key, ' + args3 + ' &val' + ') {**=='
 obj = obj + 'json_object *%s = unc::restjson::JsonBuildParse::create_json_obj();**=='%(member)
 obj = obj + 'JsonBuildParse::build("%s", %s, out);**=='%(member, member)
 obj = obj + 'int retval = 0;**=='
 for obj_sub_member in members.split(','):
   obj = obj + 'retval = %s(%s, %s, %s)**=='%(obj_sub_member, member, args2, args3)

 obj = obj + 'JsonBuildParse::build("%s", %s, out);**=='%(member, member)
 obj = obj + '}**=='
 for line in obj.split('**=='):
   file_desc.write("%s\n" % line)


def get_boolean():
  print "get_boolean"

def get_integer():
  print "get_integer"
 
def get_string(item, file_desc, member, args1, args2, args3):
  ipc_valid_flag = parser.ReadValues(item, member)['ipc_valid_flag']
  ipc_name = parser.ReadValues(item, member)['ipc_name']
  category = parser.ReadValues(item, member)['category']
  key = parser.ReadValues(item, member)['key']
  struct = parser.ReadValues(item, member)['ipc_struct']
  if struct == args2:
    key_val = 'key'
  else:
    key_val = 'val'
  if ipc_valid_flag == 'yes':
    ipc_valid_enum = parser.ReadValues(item, member)['ipc_valid_enum']
    strings = 'std::string %s;**=='%(ipc_name)
    strings = strings + 'int ret_val = restjson::JsonBuildParse::parse(in, "%s", -1, %s);**=='%(key, ipc_name)
    strings = strings + 'if (restjson::REST_OP_SUCCESS != ret_val) {**=='
    strings = strings + 'pfc_log_error("Error occured in parsing %s", PFC_FUNCNAME);**=='
    strings = strings + 'return UNC_DRV_RC_ERR_GENERIC;**=='
    strings = strings + '}**=='
    strings = strings + 'if (0 == strlen(%s.c_str())) {**=='%(ipc_name)
    strings = strings + '%s.valid[%s] = UNC_VF_INVALID;**=='%(key_val,ipc_valid_enum)
    strings = strings + '} else {**=='
    strings = strings + '%s.valid[%s] = UNC_VF_VALID;**=='%(key_val,ipc_valid_enum)
    strings = strings + '}**=='
    strings = strings + 'return UNC_RC_SUCCESS;**=='
    strings = strings + '}**=='
    for item in strings.split('**=='):
       file_desc.write("%s\n" % item)
  else:
    strings = 'std::string %s;**=='%(ipc_name)
    strings = strings + 'int ret_val = restjson::JsonBuildParse::parse(in, "%s", -1, %s);**=='%(key, ipc_name)
    strings = strings + 'if (restjson::REST_OP_SUCCESS != ret_val) {**=='
    strings = strings + 'pfc_log_error("Error occured in parsing %s", PFC_FUNCNAME);**=='
    strings = strings + 'return UNC_DRV_RC_ERR_GENERIC;**==}**=='
    strings = strings + 'strncpy(reinterpret_cast<char*> (%s.%s), %s.c_str(), sizeof(%s.%s)-1);**=='%(key_val, ipc_name, ipc_name, key_val, ipc_name)
    strings = strings + 'return UNC_RC_SUCCESS;**=='
    strings = strings + '}**=='
    for line in strings.split('**=='):
        file_desc.write("%s\n" % line)

def get_array():
 print 'array'

def get_objects():
 print 'object'

set_options = {'bool': set_boolean, 'int': set_integer, 'string' : set_string, 'array' : set_array, 'object' : set_objects}
get_options = {'bool': get_boolean, 'int': get_integer, 'string' : get_string, 'array' : get_array, 'object' : get_objects}

def set_methods(item, file_desc, member, set_support, args1, args2, args3):
 if set_support == 'yes':
   set_support = parser.ReadValues(item, member)['set_support']
   if set_support == 'yes':
     element_type = parser.ReadValues(item, member)['type']
     #if element_type == 'object':
      # ret_type = 'json_object'
     #else:
      # ret_type = 'int'
     if element_type != 'object' and element_type != 'array':
       set_me = 'UncRespCode ' + 'set_' + member + '(' + args1 + ' *out, ' + args2 + ' &key, ' + args3 + ' &val' + ') {'
       file_desc.write("%s\n" % set_me)
     key = parser.ReadValues(item, member)['key']
     set_options[element_type](item, file_desc, member, args1, args2, args3)
   else:
     set_abstract = parser.ReadValues(item, member)['set_abstract']
     if set_abstract == 'yes':
       element_type = parser.ReadValues(item, member)['type']
       set_me = 'virtual UncRespCode ' + 'set_' + member + '(' + args1 + ' *out, ' + args2 + ' &key, ' + args3 + ' &val' + ') {**=='
       set_me = set_me + 'return UNC_RC_SUCCESS;**==}**=='
       for line in set_me.split('**=='):
         file_desc.write("%s\n" % line)

def get_methods(item, file_desc, member, get_support, args1, args2, args3):
 if get_support == 'yes':
   get_support = parser.ReadValues(item, member)['get_support']
   if get_support == 'yes':
     element_type = parser.ReadValues(item, member)['type']
     if element_type != 'object' and element_type != 'array':
       get_me = 'UncRespCode ' + 'get_' + member + '(' + args1 + ' *in, ' + args2 + ' &key, ' + args3 + ' &val' + ') {'
       file_desc.write("%s\n" % get_me)
     key = parser.ReadValues(item, member)['key']
     get_options[element_type](item, file_desc, member, args1, args2, args3)
   else:
     get_abstract = parser.ReadValues(item, member)['set_abstract']
     if get_abstract == 'yes':
       element_type = parser.ReadValues(item, member)['type']
       get_me = 'virtual UncRespCode ' + 'get_' + member + '(' + args1 + ' *in, ' + args2 + ' &key, ' + args3 + ' &val' + ') {**=='
       get_me = get_me + 'return UNC_RC_SUCCESS;**==}**=='
       for line in get_me.split('**=='):
         file_desc.write("%s\n" % line)

def construct_methods(item, file_desc):
 cons_des(item, file_desc)
 args1 = parser.ReadValues(item, 'class')['rest_type']
 args2 = parser.ReadValues(item, 'class')['ipc_key_struct']
 args3 = parser.ReadValues(item, 'class')['ipc_val_struct']
 set_sec = parser.ReadValues(item, 'class')['set_sections']
 members = parser.ReadValues(item, set_sec)['members']
 set_support = parser.ReadValues(item, set_sec)['set_support']
 set_base_func_name = parser.ReadValues(item, 'class')['set_method']

 get_sec = parser.ReadValues(item, 'class')['get_sections']
 get_support = parser.ReadValues(item, get_sec)['get_support']
 get_base_func_name = parser.ReadValues(item, 'class')['get_method']

 set_base_method = 'UncRespCode ' + set_base_func_name + '(' + args1 + ' *out, ' + args2 + ' &key, ' + args3 + ' &val' + ') {**=='
 set_base_method = set_base_method + 'int retval = 1;**=='
 for member in members.split(','):
   member_set_support = parser.ReadValues(item, member)['set_support']
   if member_set_support == 'yes':
     set_base_method = set_base_method + 'retval = set_%s(out, key, val);**=='%(member)
     set_base_method = set_base_method + 'if (retval != UNC_RC_SUCCESS)**=='
     set_base_method = set_base_method + 'return UNC_DRV_RC_ERR_GENERIC;**=='
  
 set_base_method = set_base_method + 'return UNC_RC_SUCCESS;**=='
 set_base_method = set_base_method + '}**=='
 for line in set_base_method.split('**=='):
   file_desc.write("%s\n" % line)
 
 get_base_method = 'UncRespCode ' + get_base_func_name + '(' + args1 + ' *in, ' + args2 + ' &key, ' + args3 + ' &val' + ') {**=='
 get_base_method = get_base_method + 'int retval = 1;**=='
 for member in members.split(','):
   member_get_support = parser.ReadValues(item, member)['get_support']
   if member_get_support == 'yes':
     get_base_method = get_base_method + 'retval = get_%s(in, key, val);**=='%(member)
     get_base_method = get_base_method + 'if (retval != UNC_RC_SUCCESS)**=='
     get_base_method = get_base_method + 'return UNC_DRV_RC_ERR_GENERIC;**=='
  
 get_base_method = get_base_method + 'return UNC_RC_SUCCESS;**=='
 get_base_method = get_base_method + '}**=='
 for line in get_base_method.split('**=='):
   file_desc.write("%s\n" % line)

 for member in members.split(','):
   set_methods(item, file_desc, member, set_support, args1, args2, args3)
   get_methods(item, file_desc, member, get_support, args1, args2, args3)

def json_build_parse(item, file_desc):
 print "Json Build Parse Started"
 begin_headers(item, file_desc)
 begin_namespace(item, file_desc)
 construct_methods(item, file_desc)
 end_namespace(item, file_desc)
 end_footers(file_desc)

# Main Block
if __name__ == '__main__':
 rest_file = parser.REST_SPEC_FILE
 for item in rest_file.split(','):
  file_name = item[:-4] + 'hh'
  file_desc = open(file_name, "w")
  json_build_parse(item, file_desc)
  file_desc.close()
else:
 print "Code_gen loaded as module"

