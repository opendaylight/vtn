
#!/usr/bin/python

#
# Copyright (c) 2014 NEC Corporation
# All rights reserved.
#
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v1.0 which accompanies this
# distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
#

import parser, string, sys


def begin_headers(item, file_desc):
 to_caps = item.split('.rest')
 to_caps = to_caps[0].upper()
 if_guard = "__" + to_caps + "_HH__"
 header = '#ifndef '  + if_guard + '**=='
 header = header + '#define ' + if_guard + '**=='
 set_sec = parser.ReadValues(item, 'class')['set_sections']
 header = header + '#include <unc/upll_ipc_enum.h>**==#include <unc/pfcdriver_ipc_enum.h>**==#include <odc_rest.hh>**=='
 add_header = parser.ReadValues(item, 'additional_header')['header']
 if add_header != 'empty':
  header = header + add_header
 for line in header.split('**=='):
  file_desc.write("%s\n" % line)

def begin_namespace(item, file_desc):
 odc_name = parser.ReadValues(item, 'code')['namespace']
 namespace = "namespace unc {" + '**==' + 'namespace ' + odc_name + '{**=='
 for line in namespace.split('**=='):
   file_desc.write("%s\n" % line)

def declare_private_var(item, file_desc):
  args = parser.ReadValues(item, 'para_constructor')['args']
  data_type = parser.ReadValues(item, 'para_constructor')['data_type']
  data_type=data_type.split(',')
  temp = 0 
  if args != 'null':
    private_var = 'private:**=='
    for argument in args.split(','):
      data = data_type[temp]
      private_var = private_var + data + ' ' + argument
      private_var = private_var +  ';**=='
      temp = temp + 1
  
    for line in private_var.split('**=='):
       file_desc.write("%s\n" % line)

def class_name(item, file_desc):
 class_name = parser.ReadValues(item, 'class')['name']
 class_name = 'class ' + class_name + ' {'
 file_desc.write("%s\n" % class_name) 

def default_cons_des(item, file_desc):
 args = parser.ReadValues(item, 'para_constructor')['args']

 class_name = parser.ReadValues(item, 'class')['name']
 const_dest = 'public:**=='
 const_dest = const_dest + class_name + '() {}**=='
 const_dest = const_dest + '~' + class_name + '() {}**=='
 for line in const_dest.split('**=='):
    file_desc.write("%s\n" % line)

def para_constructor(item, file_desc):
  class_name = parser.ReadValues(item, 'class')['name']
  args = parser.ReadValues(item, 'para_constructor')['args']
  data_type = parser.ReadValues(item, 'para_constructor')['data_type']

  data_type=data_type.split(',')
  length = len(data_type) - 1
  temp = 0
  arg_append = '('
  for argument in args.split(','):
    data = data_type[temp]
    arg_append = arg_append + data + ' ' + argument
    if length > temp:
      arg_append = arg_append +  ', '
      temp = temp + 1
 
  arg_append = arg_append + '):'
  temp = 0

  for argument in args.split(','):
    arg_append = arg_append + argument + '(' + argument
    if length > temp:
      arg_append = arg_append + '), '
    temp = temp + 1

  arg_append = class_name + ' ' + arg_append + ') {}'
  file_desc.write("%s\n" % arg_append)

def end_namespace (item, file_desc):
  end = '};**=='
  odc_name = parser.ReadValues(item, 'code')['namespace']
  end = end + '} // namespace ' + odc_name + '**=='
  end = end + '} // namespace unc'
  for line in end.split('**=='):
    file_desc.write("%s\n" % line)

def end_footers(file_desc):
  file_desc.write("#endif\n")

def set_boolean(item, file_desc, member, args1, args2, args3):
 ipc_valid_flag = parser.ReadValues(item, member)['ipc_valid_flag']
 ipc_valid_enum = parser.ReadValues(item, member)['ipc_valid_enum']
 ipc_name = parser.ReadValues(item, member)['ipc_name']
 category = parser.ReadValues(item, member)['category']
 struct = parser.ReadValues(item, member)['ipc_struct']
 key = parser.ReadValues(item, member)['key']
 if struct == args2:
   key_val = 'key.' + ipc_name
 else:
   key_val = 'val.' + ipc_name
 if ipc_valid_flag == 'yes':
   boole = 'bool %s;**=='%(key)
   boole = boole + 'if (%s == %s)**=='%(ipc_valid_enum, key_val)
   boole = boole + '%s = true;**=='%(key)
   boole = boole + 'else**==%s = false;**=='%(key)
   boole = boole + 'int ret_val = unc::restjson::JsonBuildParse::build("%s", %s, out);**=='%(key, key)
   boole = boole + 'if (restjson::REST_OP_SUCCESS != ret_val) {**=='
   boole = boole + 'pfc_log_error("Error occured in request body build %s", PFC_FUNCNAME);**=='
   boole = boole + 'return UNC_DRV_RC_ERR_GENERIC;**=='
   boole = boole + '}**=='
   boole = boole + 'return UNC_RC_SUCCESS;**==}**=='
 for line in boole.split('**=='):
   file_desc.write("%s\n" % line)

def set_integer(item, file_desc, member, args1, args2, args3):
 ipc_valid_flag = parser.ReadValues(item, member)['ipc_valid_flag']
 ipc_name = parser.ReadValues(item, member)['ipc_name']
 category = parser.ReadValues(item, member)['category']
 struct = parser.ReadValues(item, member)['ipc_struct']
 key = parser.ReadValues(item, member)['key']
 min_ = parser.ReadValues(item, member)['min']
 max_ = parser.ReadValues(item, member)['max']
 if struct == args2:
   key_val = 'key.' + ipc_name
 else:
   key_val = 'val.' + ipc_name
 integer = 'int %s = %s;**=='%(key, key_val)
 #if json_build_type == 'object':
 integer = integer + 'if ((%s > %s) && (%s < %s)) {**=='%(key, min_, key, max_)
 integer = integer + 'int ret_val = unc::restjson::JsonBuildParse::build("%s", %s, out);**=='%(key, key)
 #else:
  # integer = integer + 'UncRespCode ret_val = (JsonBuildParse::build("%s", %s, out));**=='%(key, key)
 integer = integer + 'if (restjson::REST_OP_SUCCESS != ret_val) {**=='
 integer = integer + 'pfc_log_error("Error occured in request body build %s", PFC_FUNCNAME);**=='
 integer = integer + 'return UNC_DRV_RC_ERR_GENERIC;**=='
 integer = integer + '}**==}**=='
 integer = integer + 'return UNC_RC_SUCCESS;**=='
 integer = integer + '}**=='
 for line in integer.split('**=='):
   file_desc.write("%s\n" % line)

def set_string(item, file_desc, member, args1, args2, args3):
  ipc_valid_flag = parser.ReadValues(item, member)['ipc_valid_flag']
  ipc_name = parser.ReadValues(item, member)['ipc_name']
  category = parser.ReadValues(item, member)['category']
  key = parser.ReadValues(item, member)['key']
  struct = parser.ReadValues(item, member)['ipc_struct']
  print 'struct:', struct
  print 'args2:', args2
  if struct == args2:
    key_val = 'key.' + ipc_name
  else:
    key_val = 'val.' + ipc_name
  if ipc_valid_flag == 'yes':
    ipc_valid_enum = parser.ReadValues(item, member)['ipc_valid_enum']
    strings = 'std::string %s = (char *)%s;**=='%(key, key_val)
    strings = strings + 'if (UNC_VF_VALID == val.valid[%s]) {**=='%(ipc_valid_enum)
    #strings = strings + 'const char* %s = reinterpret_cast<const char*>(%s);**=='%(key, key_val)
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
    print key_val
    strings =  'std::string %s = (char *)%s;**=='%(key, key_val)
    #strings = 'const char* %s = reinterpret_cast<const char*>(%s);**=='%(key, key_val)
    strings = strings + 'int ret_val = unc::restjson::JsonBuildParse::build("%s", %s, out);**=='%(key,key)
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
 key = parser.ReadValues(item, member)['key']
 print 'members',members
 if members != 'empty':
  temp = 1
  arr = 'UncRespCode ' + 'set_' + member + '(' + args1 + ' *out, ' + args2 + ' &key, ' + args3 + ' &val' + ') {**=='
  arr = arr +'int ret_val = 1;**=='
  arr = arr + 'json_object *%s_arr = unc::restjson::JsonBuildParse::create_json_array_obj();**=='%(member)
 #arr = arr + 'json_object *%s_obj = unc::restjson::JsonBuildParse::create_json_obj();**=='%(member)
  for arr_member in members.split(','):
    print arr_member, member
    ipc_valid_flag = parser.ReadValues(item, arr_member)['ipc_valid_flag']
    struct = parser.ReadValues(item, arr_member)['ipc_struct']
    ipc_name = parser.ReadValues(item, arr_member)['ipc_name']
    if struct == args2:
     key_val = 'key.' + ipc_name
    else:
     key_val = 'val.' + ipc_name
     print 'ipc_name:', ipc_name
    if ipc_valid_flag == 'yes':
      ipc_valid_enum = parser.ReadValues(item, arr_member)['ipc_valid_enum']
      arr = arr + 'if (%s[%s] == UNC_VF_VALID) {**=='%(key_val, ipc_valid_enum)
    arr = arr + 'json_object *%s_obj%d = unc::restjson::JsonBuildParse::create_json_obj();**=='%(member, temp)
    arr = arr + 'ret_val = set_%s(%s_obj%d , key , val);**=='%(arr_member, member, temp) 
    arr = arr + 'if (UNC_RC_SUCCESS != ret_val)**=='
    arr = arr + 'return UNC_DRV_RC_ERR_GENERIC;**==' 
    arr = arr + 'unc::restjson::JsonBuildParse::add_to_array(%s_arr, %s_obj%d);**=='%(member, member, temp)
    if ipc_valid_flag == 'yes':
      arr = arr + '}**=='
    temp += 1

 #arr = arr + 'unc::restjson::JsonBuildParse::add_to_array(%s_arr, %s_obj);**=='%(member, member)
  arr = arr + 'ret_val = unc::restjson::JsonBuildParse::build("%s", %s_arr, out);**=='%(member, member)
  arr = arr + 'if (restjson::REST_OP_SUCCESS != ret_val) {**=='
  arr = arr + 'pfc_log_error("Error occured in request body build %s", PFC_FUNCNAME);**=='
  arr = arr + 'return UNC_DRV_RC_ERR_GENERIC;**=='
  arr = arr + '}**=='
  arr = arr + 'return UNC_RC_SUCCESS;**=='
  arr = arr + '}**=='
  for line in arr.split('**=='):
   file_desc.write("%s\n" % line)
  for arr_member in members.split(','):
    element_type = parser.ReadValues(item, arr_member)['type']
    print element_type
    if element_type != 'object': 
      arr_sub_member = 'UncRespCode ' + 'set_' + arr_member + '(' + 'json_object *out' + ', ' + args2 + ' &key, ' + args3 + ' &val' + ') {'
      file_desc.write("%s\n" % arr_sub_member)
     
    set_options[element_type](item, file_desc, arr_member, args1, args2, args3)
 else:
  arr = 'UncRespCode ' + 'set_' + member + '(' + args1 + ' *out, ' + args2 + ' &key, ' + args3 + ' &val' + ') {**=='
  arr = arr + 'json_object *%s_arr = unc::restjson::JsonBuildParse::create_json_array_obj();**=='%(member)
  arr = arr + 'int ret_val = unc::restjson::JsonBuildParse::build("%s", %s_arr, out);**=='%(member, member)
  arr = arr + 'if (ret_val != UNC_RC_SUCCESS)**=='
  arr = arr + 'return UNC_DRV_RC_ERR_GENERIC;**=='
  arr = arr + 'return UNC_RC_SUCCESS;**=='
  arr = arr + '}**=='
  for line in arr.split('**=='):
   file_desc.write("%s\n" % line)

 
 #ipc_valid_flag = parser.ReadValues(item, arr_member)['ipc_valid_flag']
 
 #if ipc_valid_flag == 'yes':
 #  ipc_valid_enum = parser.ReadValues(item, arr_member)['ipc_valid_enum']
 #  arr = arr + 'if (val.valid[%s] == UNC_VF_VALID) {**=='%(ipc_valid_enum)
 #  arr_sub_member = parser.ReadValues(item, arr_member)['members']
 #  element_type = parser.ReadValues(item, arr_sub_member)['type']
 #  ipc_struct = parser.ReadValues(item, arr_sub_member)['ipc_struct']
 #   ipc_name = parser.ReadValues(item, arr_sub_member)['ipc_name']
 #   key = parser.ReadValues(item, arr_sub_member)['key']
 #  arr = arr + 'json_object *%s = unc::restjson::JsonBuildParse::create_json_obj();**=='%(key)
 #  arr = arr + 'UncRespCode ret_val = unc::restjson::JsonBuildParse::build(%s, "%s", %s);**=='%(key, key, member)
 #  arr = arr + 'JsonBuildParse::add_to_array(%s, out);**=='%(member)
 #  arr = arr + '}**=='
       

def set_objects(item, file_desc, member, args1, args2, args3):
 members = parser.ReadValues(item, member)['members']
 set_support = parser.ReadValues(item, member)['set_support']
 key = parser.ReadValues(item, member)['key']
 ipc_valid_flag = parser.ReadValues(item, member)['ipc_valid_flag']
 struct = parser.ReadValues(item, member)['ipc_struct']
 ipc_name = parser.ReadValues(item, member)['ipc_name']
 if struct == args2:
   key_val = 'key.' + ipc_name
 else:
   key_val = 'val.' + ipc_name
 for obj_member in members.split(','):
   if obj_member == 'empty':
     obj = 'UncRespCode ' + 'set_' + member + '( json_object *out, ' + args2 + ' &key, ' + args3 + ' &val' + ') {**=='
     ipc_valid_flag = parser.ReadValues(item, member)['ipc_valid_flag']
     if ipc_valid_flag == 'yes':
       ipc_valid_enum = parser.ReadValues(item, member)['ipc_valid_enum']
       obj = obj + 'if (%s == %s) {**=='%(ipc_valid_enum, key_val)
       obj = obj + 'json_object *%s = unc::restjson::JsonBuildParse::create_json_obj();**=='%(member)
       obj = obj + 'int ret_val = unc::restjson::JsonBuildParse::build("%s", %s, out);**=='%(key, key)
       obj = obj + 'if (restjson::REST_OP_SUCCESS != ret_val) {**=='
       obj = obj + 'pfc_log_error("Error occured in request body build %s", PFC_FUNCNAME);**=='
       obj = obj + 'return UNC_DRV_RC_ERR_GENERIC;**==}**=='
       obj = obj + '}**==return UNC_RC_SUCCESS;**==}**=='
       for item in obj.split('**=='):
         file_desc.write("%s\n" % item)
     return
   set_methods(item, file_desc, obj_member, set_support, args1, args2, args3)
 
 print 'member:', member
 obj = 'UncRespCode ' + 'set_' + member + '(' + 'json_object *out, ' + args2 + ' &key, ' + args3 + ' &val' + ') {**=='
 obj = obj + 'json_object *%s = unc::restjson::JsonBuildParse::create_json_obj();**=='%(member)
 obj = obj + 'int ret_val = 0;**=='

 if ipc_valid_flag == 'yes':
   ipc_valid_enum = parser.ReadValues(item, member)['ipc_valid_enum']
   ipc_enum_compare = parser.ReadValues(item, member)['ipc_enum_compare']
   if ipc_enum_compare == 'yes':
     print "ipc_enum_compare----------------->yes"
     ipc_enum_compare_value = parser.ReadValues(item, member)['ipc_enum_compare_value']
     obj = obj + 'if (%s[%s] == %s) {**=='%(key_val, ipc_valid_enum,ipc_enum_compare_value)
   else:  
     print "ipc_enum_compare----------------->no"
     obj = obj + 'if (%s == %s) {**=='%(key_val ,ipc_valid_enum)
 for obj_sub_member in members.split(','):
   obj = obj + 'ret_val = set_%s(%s, key, val);**=='%(obj_sub_member, member)
   obj = obj + 'if (UNC_RC_SUCCESS != ret_val)**=='
   obj = obj + 'return UNC_DRV_RC_ERR_GENERIC;**=='
 

 obj = obj + 'ret_val = unc::restjson::JsonBuildParse::build("%s", %s, out);**=='%(member, member)
 obj = obj + 'if (restjson::REST_OP_SUCCESS != ret_val) {**=='
 obj = obj + 'pfc_log_error("Error occured in request body build %s", PFC_FUNCNAME);**=='
 obj = obj + 'return UNC_DRV_RC_ERR_GENERIC;**=='
 obj = obj + '}**=='
 if ipc_valid_flag == 'yes':
   obj = obj + '}**=='
 obj = obj + 'return UNC_RC_SUCCESS;**=='
 obj = obj + '}**=='
 for line in obj.split('**=='):
   file_desc.write("%s\n" % line)


def get_boolean(item, file_desc, member, args1, args2, args3):
  print "get_boolean"

def get_integer(item, file_desc, member, args1, args2, args3):
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
    min_ = parser.ReadValues(item, member)['min']
    max_ = parser.ReadValues(item, member)['max']
    integer = 'int %s = -1;**=='%(key)
    integer = integer + 'int ret_val = restjson::JsonBuildParse::parse(in, "%s", -1, %s);**=='%(key, key)
    integer = integer + 'if (restjson::REST_OP_SUCCESS != ret_val) {**=='
    integer = integer + 'pfc_log_error("parse failed %s", PFC_FUNCNAME);**==}**=='
    integer = integer + 'if ((%s > %s) && (%s < %s)) {**=='%(key, min_, key, max_)
    integer = integer + '%s.%s = %s;**=='%(key_val, ipc_name, key)
    integer = integer + '%s.valid[%s] = UNC_VF_VALID;**=='%(key_val,ipc_valid_enum)
    integer = integer + '}**==else {**=='
    integer = integer + '%s.valid[%s] = UNC_VF_INVALID;**=='%(key_val, ipc_valid_enum)
    integer = integer + '}**=='
    integer = integer + 'return UNC_RC_SUCCESS;**=='
    integer = integer + '}**=='
    for item in integer.split('**=='):
       file_desc.write("%s\n" % item)
  else:
    integer = 'int %s = -1;**=='%(key)
    integer = integer + 'int ret_val = restjson::JsonBuildParse::parse(in, "%s", -1, %s);**=='%(key, key)
    integer = integer + 'if (restjson::REST_OP_SUCCESS != ret_val) {**=='
    integer = integer + 'pfc_log_error("parse failed %s", PFC_FUNCNAME);**==}**=='
    integer = integer + 'else {**==%s.%s = %s;**==}**=='%(key_val, ipc_name, key)
    integer = integer + 'return UNC_RC_SUCCESS;**=='
    integer = integer + '}**=='
    for line in integer.split('**=='):
        file_desc.write("%s\n" % line)
 
def get_string(item, file_desc, member, args1, args2, args3):
  ipc_valid_flag = parser.ReadValues(item, member)['ipc_valid_flag']
  ipc_name = parser.ReadValues(item, member)['ipc_name']
  category = parser.ReadValues(item, member)['category']
  key = parser.ReadValues(item, member)['key']
  struct = parser.ReadValues(item, member)['ipc_struct']
  split_name = ipc_name.split('.')
  split_name = split_name[0]
  print split_name
  print ipc_name
  print 'struct:',struct
  print 'args2:', args2
  if struct == args2:
    key_val = 'key'
  else:
    if  '.' in ipc_name :
      key_val = 'val.' + split_name
    else:  
      key_val = 'val' 
  print 'key_val:', key_val
  if ipc_valid_flag == 'yes':
    ipc_valid_enum = parser.ReadValues(item, member)['ipc_valid_enum']
    if key_val == 'val' or key_val == 'val.' + split_name:
     strings = 'std::string %s = (char *)val.%s;**=='%(key,ipc_name)
    else:
     strings = 'std::string %s = (char *)key.%s;**=='%(key,ipc_name)
    strings = strings + 'int ret_val = restjson::JsonBuildParse::parse(in, "%s", -1, %s);**=='%(key, key)
    strings = strings + 'if (restjson::REST_OP_SUCCESS != ret_val) {**=='
    strings = strings + 'pfc_log_error("Error occured in parsing %s", PFC_FUNCNAME);**=='
    strings = strings + 'return UNC_DRV_RC_ERR_GENERIC;**=='
    strings = strings + '}**=='
    strings = strings + 'if (0 == strlen(%s.c_str())) {**=='%(key)
    if split_name != 'val_ff_entry':
     strings = strings + 'val.valid[%s] = UNC_VF_INVALID;**=='%(ipc_valid_enum)
     strings = strings + '} else {**=='
     strings = strings + 'val.valid[%s] = UNC_VF_VALID;**=='%(ipc_valid_enum)
    else:
     print 'split_name:',split_name
     strings = strings + 'val.%s.valid[%s] = UNC_VF_INVALID;**=='%(split_name,ipc_valid_enum)
     strings = strings + '} else {**=='
     strings = strings + 'val.%s.valid[%s] = UNC_VF_VALID;**=='%(split_name,ipc_valid_enum)

    strings = strings + 'strncpy(reinterpret_cast<char*> (val.%s), %s.c_str(), sizeof(val.%s)-1);**=='%(ipc_name, key, ipc_name)
    strings = strings + '}**=='
    strings = strings + 'pfc_log_debug("get_%s success");**=='%(member)
    strings = strings + 'return UNC_RC_SUCCESS;**=='
    strings = strings + '}**=='
    for item in strings.split('**=='):
       file_desc.write("%s\n" % item)
  else:
    if key_val == 'val' or key_val == 'val.' + split_name:
     strings = 'std::string %s = (char *)val.%s;**=='%(key,ipc_name)
    else:
      strings = 'std::string %s = (char *)key.%s;**=='%(key,ipc_name)

    strings = strings + 'int ret_val = restjson::JsonBuildParse::parse(in, "%s", -1, %s);**=='%(key, key)
    strings = strings + 'if (restjson::REST_OP_SUCCESS != ret_val) {**=='
    strings = strings + 'pfc_log_error("Error occured in parsing %s", PFC_FUNCNAME);**=='
    strings = strings + 'return UNC_DRV_RC_ERR_GENERIC;**==}**=='
    if key_val == 'val' or key_val == 'val.' + split_name:  
       strings = strings + 'strncpy(reinterpret_cast<char*> (val.%s), %s.c_str(), sizeof(val.%s)-1);**=='%(ipc_name, key, ipc_name)
    else:
        strings = strings + 'strncpy(reinterpret_cast<char*> (key.%s), %s.c_str(), sizeof(key.%s)-1);**=='%(ipc_name, key, ipc_name)
    strings = strings + 'pfc_log_debug("%s");**=='%(ipc_name)
    strings = strings + 'return UNC_RC_SUCCESS;**=='
    strings = strings + '}**=='
    for line in strings.split('**=='):
        file_desc.write("%s\n" % line)

def get_array(item, file_desc, member, args1, args2, args3):
 members = parser.ReadValues(item, member)['members']
 arr = 'UncRespCode ' + 'get_' + member + '(' + 'json_object *in, ' + args2 + ' &key, ' + args3 + ' &val' + ') {**=='
 arr = arr + 'int ret_val = 1;**=='
 arr = arr + 'json_object *%s = NULL;**=='%(member)
 arr = arr + 'ret_val = unc::restjson::JsonBuildParse::parse(in, "%s", -1, %s);**=='%(member, member)
 arr = arr + 'if (restjson::REST_OP_SUCCESS != ret_val) {**=='
 arr = arr + 'pfc_log_error("Error occured in parsing %s", PFC_FUNCNAME);**=='
 arr = arr + 'return UNC_RC_SUCCESS;**=='
 arr = arr + '}**=='
 arr = arr + 'json_object *arr_members = NULL;**==';
 arr = arr + 'if (json_object_is_type(%s, json_type_array)) {**=='%(member)
 arr = arr + 'int array_length = unc::restjson::JsonBuildParse::get_array_length(in, "%s");**=='%(member)
 arr = arr + 'for (int arr_idx = 0; arr_idx < array_length; arr_idx++) {**=='
 arr = arr + 'get_array_idx(&arr_members, %s, arr_idx);**=='%(member)

 for arr_member in members.split(','):
   print arr_member, member
   arr = arr + 'ret_val = ' + 'get_' + arr_member + '(' + 'arr_members' + ', ' + 'key, ' + 'val);**==' 
   arr = arr + 'if (UNC_RC_SUCCESS != ret_val)**=='
   arr = arr + 'return UNC_DRV_RC_ERR_GENERIC;**=='

 arr = arr + '}**==}**=='
 arr = arr + 'return UNC_RC_SUCCESS;**==}**=='
 for line in arr.split('**=='):
   file_desc.write("%s\n" % line)

 #file_desc.write("%s\n" % '}')

 for arr_member in members.split(','):
   element_type = parser.ReadValues(item, arr_member)['type']
   print element_type
   if element_type != 'object': 
     arr_sub_member = 'UncRespCode ' + 'get_' + arr_member + '(' + 'json_object* in, ' + args2 + ' &key, ' + args3 + ' &val' + ') {'
     file_desc.write("%s\n" % arr_sub_member)
     
   get_options[element_type](item, file_desc, arr_member, args1, args2, args3)

def get_objects(item, file_desc, member, args1, args2, args3):
 print 'get_objects', member
 members = parser.ReadValues(item, member)['members']
 get_support = parser.ReadValues(item, member)['get_support']
 key = parser.ReadValues(item, member)['key']
 for obj_member in members.split(','):
   if obj_member == 'empty':
     obj = 'UncRespCode ' + 'get_' + member + '( json_object *in, ' + args2 + ' &key, ' + args3 + ' &val' + ') {**=='
     ipc_valid_flag = parser.ReadValues(item, member)['ipc_valid_flag']
     if ipc_valid_flag == 'yes':
       ipc_valid_enum = parser.ReadValues(item, member)['ipc_valid_enum']
       key = parser.ReadValues(item, member)['key']
       obj = obj + 'json_object *%s = NULL;**=='%(member)
       obj = obj + 'int ret_val = unc::restjson::JsonBuildParse::parse(in, "%s", -1, %s);**=='%(key, member)
       obj = obj + 'if (UNC_VF_VALID == [%s]) {**=='%(ipc_valid_enum)
       obj = obj + '}**=='
       obj = obj + '}**=='
       for item in obj.split('**=='):
         file_desc.write("%s\n" % item)
     return
   get_methods(item, file_desc, obj_member, get_support, args1, args2, args3)
 
 obj = 'UncRespCode ' + 'get_' + member + '(' + 'json_object *in, ' + args2 + ' &key, ' + args3 + ' &val' + ') {**=='
 obj = obj + 'json_object *%s = NULL;**=='%(member)
 obj = obj + 'int ret_val = unc::restjson::JsonBuildParse::parse(in, "%s", -1, %s);**=='%(member, member)
 for obj_sub_member in members.split(','):
   obj = obj + 'ret_val = get_%s(%s, key, val);**=='%(obj_sub_member, member)
   obj = obj + 'if (UNC_RC_SUCCESS != ret_val)**=='
   obj = obj + 'return UNC_DRV_RC_ERR_GENERIC;**=='

 obj = obj + 'return UNC_RC_SUCCESS;**==}**=='
 for line in obj.split('**=='):
   file_desc.write("%s\n" % line)
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
       print 'args:',args1
       set_me = 'UncRespCode ' + 'set_' + member + '(' + args1 + ' *out, ' + args2 + ' &key, ' + args3 + ' &val' + ') {'
       file_desc.write("%s\n" % set_me)
     key = parser.ReadValues(item, member)['key']
     set_options[element_type](item, file_desc, member, args1, args2, args3)
   else:
     set_abstract = parser.ReadValues(item, member)['set_abstract']
     if set_abstract == 'yes':
       element_type = parser.ReadValues(item, member)['type']
       #ipc_name = parser.ReadValues(item, member) ['ipc_name']
       virtual_class_name = parser.ReadValues(item, member)['virtual_class_name']
       if virtual_class_name != 'null':
         set_me = 'virtual UncRespCode ' + 'set_' + member + '(' + args1 + ' *out, ' + args2 + ' &key, ' + args3 + ' &val' + ') {**=='
         set_me = set_me + '%s obj;**=='%(virtual_class_name)
         set_me = set_me + 'int ret_val = obj.set_%s(out, key ,val);**=='%(member)
         print 'virtual member:', member
         set_me = set_me + 'if (ret_val != UNC_RC_SUCCESS)**=='
         set_me = set_me + 'return UNC_DRV_RC_ERR_GENERIC;**=='
         set_me = set_me + 'return UNC_RC_SUCCESS;**==}**=='
       else:
         set_me = 'virtual UncRespCode ' + 'set_' + member + '(' + args1 + ' *out, ' + args2 + ' &key, ' + args3 + ' &val' + ') {**=='
         set_me = set_me + 'return UNC_RC_SUCCESS;**==}**=='

       for line in set_me.split('**=='):
         file_desc.write("%s\n" % line)

def get_methods(item, file_desc, member, get_support, args1, args2, args3):
 if get_support == 'yes':
   get_support = parser.ReadValues(item, member)['get_support']
   print member, get_support
   if get_support == 'yes':
     element_type = parser.ReadValues(item, member)['type']
     if element_type != 'object' and element_type != 'array':
       get_me = 'UncRespCode ' + 'get_' + member + '(' + 'json_object *in, ' + args2 + ' &key, ' + args3 + ' &val' + ') {'
       file_desc.write("%s\n" % get_me)
     key = parser.ReadValues(item, member)['key']
     get_options[element_type](item, file_desc, member, args1, args2, args3)
   else:
     get_abstract = parser.ReadValues(item, member)['get_abstract']
     if get_abstract == 'yes':
       element_type = parser.ReadValues(item, member)['type']
       #ipc_name = parser.ReadValues(item, member) ['ipc_name']
       virtual_class_name = parser.ReadValues(item, member)['virtual_class_name']
       if virtual_class_name != 'null':
         get_me = 'virtual UncRespCode ' + 'get_' + member + '(' + 'json_object *in, ' + args2 + ' &key, ' + args3 + ' &val' + ') {**=='
         get_me = get_me + '%s obj;**=='%(virtual_class_name)
         get_me = get_me + 'int ret_val = obj.get_%s(in, key ,val);**=='%(member)
         get_me = get_me + 'if (ret_val != UNC_RC_SUCCESS)**=='
         get_me = get_me + 'return UNC_DRV_RC_ERR_GENERIC;**=='
         get_me = get_me + 'return UNC_RC_SUCCESS;**==}**=='
       else:
         get_me = 'virtual UncRespCode ' + 'get_' + member + '(' + 'json_object *in, ' + args2 + ' &key, ' + args3 + ' &val' + ') {**=='
         get_me = get_me + 'return UNC_RC_SUCCESS;**==}**=='

       for line in get_me.split('**=='):
          file_desc.write("%s\n" % line)


       #get_me = 'virtual UncRespCode ' + 'get_' + member + '(' + 'json_object +  *in', + args2 + ' &key, ' + args3 + ' &val' + ') {**=='
       #get_me = get_me + 'return UNC_RC_SUCCESS;**==}**=='
       #for line in get_me.split('**=='):
        # file_desc.write("%s\n" % line)

def construct_methods(item, file_desc):
 class_name(item, file_desc)
 para_cons = parser.ReadValues(item, 'class')['parameterized_constructor']
 declare_private_var(item, file_desc)
 default_cons_des(item, file_desc)
 if para_cons == 'yes':
   para_constructor(item, file_desc)
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
 set_method = parser.ReadValues(item, 'class')['set_sections']
 set_base_method = 'UncRespCode ' + set_base_func_name + '(' + args1 + ' *out, ' + args2 + ' &key, ' + args3 + ' &val' + ') {**=='
 set_base_method = set_base_method + 'pfc_log_info("entering in to %s");**=='%(set_base_func_name)
 set_base_method = set_base_method + 'int retval = 1;**=='
 for member in members.split(','):
   print 'set_member:', member
   member_set_support = parser.ReadValues(item, member)['set_support']
   print member_set_support
   set_abstract = parser.ReadValues(item, member) ['set_abstract']
   if member_set_support == 'yes' or set_abstract == 'yes':
     set_base_method = set_base_method + 'retval = set_%s(out, key, val);**=='%(member)
     set_base_method = set_base_method + 'if (retval != UNC_RC_SUCCESS)**=='
     set_base_method = set_base_method + 'return UNC_DRV_RC_ERR_GENERIC;**=='
  
 set_base_method = set_base_method + 'return UNC_RC_SUCCESS;**=='
 set_base_method = set_base_method + 'pfc_log_info("leaving from %s");**=='%(set_base_func_name)
 set_base_method = set_base_method + '}**=='
 for line in set_base_method.split('**=='):
   file_desc.write("%s\n" % line)
 key =   parser.ReadValues(item, get_sec)['key']
 parent_name = parser.ReadValues(item, get_sec)['parent_name']
 get_method =  parser.ReadValues(item, 'class')['get_method']
 print get_method
 if get_method != 'null':
   args = parser.ReadValues(item, 'para_constructor')['args']
   #parent_name1 = parser.ReadValues(item, get_sec)['parent_name1']
   #split_name = parent_name1.rsplit(".",1)
   #split_name=split_name[1]
   get_extra_parameters = parser.ReadValues(item, 'extra_para')['extra_parameter']
   print get_extra_parameters
   if get_extra_parameters == 'yes':
     get_extra_arguments = parser.ReadValues(item, 'extra_para')['get_extra_argument']
     print get_extra_arguments
     get_base_method = 'UncRespCode %s (%s *in, unc::driver::controller* ctr, unc::restjson::ConfFileValues_t conf_values_,%s &key, %s &val, std::vector<unc::vtndrvcache::ConfigNode *> &cfgnode_vector)**== {**=='%(get_base_func_name,args1,args2,args3)
   else:

     get_base_method = 'UncRespCode ' + get_base_func_name + '(' + args1 + ' *in, ' + args2 + ' &key, ' + args3 + ' &val,' + ' std::vector<unc::vtndrvcache::ConfigNode *> &cfgnode_vector ' + ') {**=='
   get_base_method = get_base_method + 'pfc_log_info("entering in to %s");**=='%(get_base_func_name)
   get_base_method = get_base_method + 'int retval = 1;**=='
   member_portmaps = parser.ReadValues(item, 'get_fill_portmap')['portmaps']
   if member_portmaps != 'no':
    for member in members.split(','):
      member_get_support = parser.ReadValues(item, member)['get_support']
      get_abstract = parser.ReadValues(item, member) ['get_abstract']

      if member_get_support == 'yes' or get_abstract == 'yes' :
       get_base_method = get_base_method + 'retval = get_%s(in, key, val);**=='%(member)
       get_base_method = get_base_method + 'if (retval != UNC_RC_SUCCESS)**=='
       get_base_method = get_base_method + 'return UNC_DRV_RC_ERR_GENERIC;**=='
    get_base_method  = get_base_method + 'pfc_log_info("leaving from %s");**=='%(get_base_func_name)
    get_base_method = get_base_method + 'return UNC_RC_SUCCESS;**=='
    get_base_method = get_base_method + '}**=='
 
   else:

    get_base_method = get_base_method + 'uint32_t array_length = 0;**=='
    get_base_method = get_base_method + 'pfc_log_debug("arr_obj:%s", unc::restjson::JsonBuildParse::get_json_string(in));**=='
    get_base_method = get_base_method + 'json_object *json_obj_%s = NULL;**=='%(key)
    get_base_method = get_base_method + 'uint32_t ret_val = unc::restjson::JsonBuildParse::parse(in, "%s", -1, json_obj_%s );**=='%(set_sec,key)
    get_base_method = get_base_method + 'if(restjson::REST_OP_SUCCESS != ret_val)**=='
    get_base_method = get_base_method + '{**=='
    get_base_method = get_base_method + 'pfc_log_error("Error occured in parsing %s" ,PFC_FUNCNAME);**=='
    get_base_method = get_base_method + 'return UNC_DRV_RC_ERR_GENERIC;**=='
    get_base_method = get_base_method + '}**=='
    get_base_method = get_base_method + 'if (json_object_is_type(json_obj_%s, json_type_array))**=='%(key)
    print key
    get_base_method = get_base_method + '{**=='
    get_base_method = get_base_method + 'array_length = unc::restjson::JsonBuildParse::get_array_length(in, "%s");**=='%(set_sec)
    if key == 'vtn':
     get_base_method = get_base_method + 'if ( 0 == array_length)**=='
     get_base_method = get_base_method + 'return UNC_RC_NO_SUCH_INSTANCE;**=='
    get_base_method = get_base_method + '}**=='
    get_base_method = get_base_method + 'for (uint32_t arr_idx = 0; arr_idx < array_length; arr_idx++)**=='
    get_base_method = get_base_method + '{**=='
    parent_name1 = parser.ReadValues(item, get_sec)['parent_name1']
    parent_name2 = parser.ReadValues(item, get_sec)['parent_name2']
    set_filter_value = parser.ReadValues(item, 'extra_para')['filterlist']
    if set_filter_value == 'yes':
     if parent_name !='empty' and parent_name1 !='empty' and parent_name2 !='empty':
      get_base_method = get_base_method + 'retval = set_filter(parent_name,parent_name1,parent_name2,cfgnode_vector);**=='
     else:
      if parent_name != 'empty' and parent_name1 != 'empty':
       get_base_method = get_base_method + 'retval = set_filter(parent_name,parent_name1,cfgnode_vector);**=='
      else:
       get_base_method = get_base_method + 'retval = set_filter(parent_name,cfgnode_vector);**=='
     get_base_method = get_base_method + 'if (retval != UNC_RC_SUCCESS)**=='
     get_base_method = get_base_method + ' return UNC_DRV_RC_ERR_GENERIC;**=='
    get_base_method = get_base_method + '%s key;**=='%(args2)
    get_base_method = get_base_method + '%s val;**=='%(args3)
    get_base_method = get_base_method + 'memset(&key, 0 , sizeof(%s));**=='%(args2)
    get_base_method = get_base_method + 'memset(&val, 0 , sizeof(%s));**=='%(args3)
    parent_name1 = parser.ReadValues(item, get_sec)['parent_name1']
    parent_name2 = parser.ReadValues(item, get_sec)['parent_name2']
    if parent_name !='empty' and parent_name1 !='empty' and parent_name2 !='empty':
     split_name = parent_name1.rsplit(".",1)

     get_base_method = get_base_method + 'strncpy(reinterpret_cast<char*> (key.%s), parent_name.c_str(),sizeof(key.%s) - 1);**=='%(parent_name,parent_name)  
     get_base_method = get_base_method + 'strncpy(reinterpret_cast<char*> (key.%s), parent_name1.c_str(),sizeof(key.%s) - 1);**=='%(parent_name1,parent_name1)
     get_base_method = get_base_method + 'strncpy(reinterpret_cast<char*> (key.%s), parent_name2.c_str(),sizeof(key.%s) - 1);**=='%(parent_name2,parent_name2)
    else :
     if parent_name != 'empty' and parent_name1 != 'empty':
      #split_name = parent_name1.rsplit(".",1)
      #split_name=split_name[1]

      get_base_method = get_base_method + 'strncpy(reinterpret_cast<char*> (key.%s), parent_name.c_str(),sizeof(key.%s) - 1);**=='%(parent_name,parent_name)
      get_base_method = get_base_method + 'strncpy(reinterpret_cast<char*> (key.%s), parent_name1.c_str(),sizeof(key.%s) - 1);**=='%(parent_name1,parent_name1)
     
     if parent_name != 'empty' and parent_name1 == 'empty': 
      get_base_method = get_base_method + 'strncpy(reinterpret_cast<char*> (key.%s), parent_name.c_str(),sizeof(key.%s) - 1);**=='%(parent_name,parent_name)

    get_base_method = get_base_method + 'json_object *json_obj_%s_idx = NULL;**=='%(key)
    print key
    get_base_method = get_base_method + 'get_array_idx(&json_obj_%s_idx, json_obj_%s, arr_idx);**=='%(key,key)
    get_base_method = get_base_method + 'pfc_log_debug("arr_idx:%s",' + ' unc::restjson::JsonBuildParse::get_json_string(json_obj_%s_idx));**=='%(key)

    for member in members.split(','):
     member_get_support = parser.ReadValues(item, member)['get_support']
     get_abstract = parser.ReadValues(item, member) ['get_abstract']

     if member_get_support == 'yes' or get_abstract == 'yes' :
      get_base_method = get_base_method + 'retval = get_%s(json_obj_%s_idx, key, val);**=='%(member,key)
      get_base_method = get_base_method + 'if (retval != UNC_RC_SUCCESS)**=='
      get_base_method = get_base_method + 'return UNC_DRV_RC_ERR_GENERIC;**=='
 
    member_get_virtual = parser.ReadValues(item, 'virtual_method')['get_virtual_method']
    if member_get_virtual != 'empty':
     get_virtual_method_name = parser.ReadValues(item, 'virtual_method')['get_virtual_method_name']
     get_base_method = get_base_method + 'retval = get_%s(ctr, key, val);**=='%(get_virtual_method_name)
     get_base_method = get_base_method + 'if (retval != UNC_RC_SUCCESS)**=='
     get_base_method = get_base_method + 'return UNC_DRV_RC_ERR_GENERIC;**=='
    member_get_fill_portmap = parser.ReadValues(item, 'get_fill_portmap')['get_fill_portmaps'] 
    if member_get_fill_portmap != 'empty':
    #get_base_method = get_base_method + 'url_.append("/");**=='
    #get_base_method = get_base_method + 'url_.append((char *)key.if_name);**=='
    #get_base_method = get_base_method + 'url_.append("/portmap");**=='
     get_base_method = get_base_method + 'retval = get_fill_portmap(url_, key, val, ctr, conf_values_,cfgnode_vector);**=='
     get_base_method = get_base_method + 'if (retval != UNC_RC_SUCCESS)**=='
     get_base_method = get_base_method + 'return UNC_DRV_RC_ERR_GENERIC;**=='
     get_base_method = get_base_method + 'pfc_log_debug("returning from get_fill_portmap");**=='
    member_portmaps = parser.ReadValues(item, 'get_fill_portmap')['portmaps']
    if member_portmaps == 'no':
     get_base_method = get_base_method + 'unc::vtndrvcache::ConfigNode *cfgptr = new unc::vtndrvcache::CacheElementUtil<%s , %s, uint32_t>(&key, &val, uint32_t(UNC_OP_READ));**=='%(args2,args3)
     get_base_method = get_base_method + 'PFC_ASSERT(cfgptr != NULL);**=='
     get_base_method = get_base_method + 'cfgnode_vector.push_back(cfgptr);**=='
     get_base_method = get_base_method + '}**=='
     get_base_method = get_base_method + 'json_object_put(json_obj_%s);**=='%(key)
    else: 
     get_base_method = get_base_method + '}**=='
    get_base_method  = get_base_method + 'pfc_log_info("leaving from %s");**=='%(get_base_func_name)
    get_base_method = get_base_method + 'return UNC_RC_SUCCESS;**=='
    get_base_method = get_base_method + '}**=='
   for line in get_base_method.split('**=='):
    file_desc.write("%s\n" % line)

 for member in members.split(','):
   set_methods(item, file_desc, member, set_support, args1, args2, args3)
   get_methods(item, file_desc, member, get_support, args1, args2, args3)

def set_filter(item, file_desc):
  set_fill_filter = parser.ReadValues(item, 'extra_para') ['filterlist']
  virtual_class_name = parser.ReadValues(item, 'virtual_method')['virtual_class_name']
  get_sec = parser.ReadValues(item, 'class')['get_sections']
  parent_name1 = parser.ReadValues(item, get_sec)['parent_name1']
  parent_name2 = parser.ReadValues(item, get_sec)['parent_name2']
  parent_name = parser.ReadValues(item, get_sec)['parent_name']
  if set_fill_filter == 'yes':
   if parent_name !='empty' and parent_name1 !='empty' and parent_name2 !='empty':
    set_filter = 'UncRespCode set_filter(std::string parent_name, std::string parent_name1, std::string parent_name2,std::vector<unc::vtndrvcache::ConfigNode *> &cfgnode_vector) {**=='
    set_filter = set_filter + '%s obj;**=='%(virtual_class_name)
    set_filter = set_filter + 'int ret_val = obj.set_filter(parent_name,parent_name1,parent_name2,cfgnode_vector);**=='
   else:
    if parent_name != 'empty' and parent_name1 != 'empty':
     set_filter = 'UncRespCode set_filter(std::string parent_name, std::string parent_name1,std::vector<unc::vtndrvcache::ConfigNode *> &cfgnode_vector) {**=='
     set_filter = set_filter + '%s obj;**=='%(virtual_class_name)
     set_filter = set_filter + 'int ret_val = obj.set_filter(parent_name,parent_name1,cfgnode_vector);**=='
    else:
     set_filter = 'UncRespCode set_filter(std::string parent_name, std::vector<unc::vtndrvcache::ConfigNode *> &cfgnode_vector) {**=='
     set_filter = set_filter + '%s obj;**=='%(virtual_class_name)
     set_filter = set_filter + 'int ret_val = obj.set_filter(parent_name,cfgnode_vector);**=='

   set_filter = set_filter + 'if (ret_val != UNC_RC_SUCCESS)**=='
   set_filter = set_filter + 'return UNC_DRV_RC_ERR_GENERIC;**=='
   set_filter = set_filter + 'return UNC_RC_SUCCESS;**=='
   set_filter = set_filter + '}**=='
   for line in set_filter.split('**=='):
    file_desc.write("%s\n" % line)





def get_fill_portmap(item, file_desc):
 get_fill_portmap = parser.ReadValues(item, 'get_fill_portmap')['get_fill_portmaps']
 if get_fill_portmap != 'empty':
   args2 = parser.ReadValues(item, 'class')['ipc_key_struct']
   args3 = parser.ReadValues(item, 'class')['ipc_val_struct']
   get_fill_portmap = 'UncRespCode get_fill_portmap(std::string url, %s &key, %s &val, unc::driver::controller* ctr, unc::restjson::ConfFileValues_t conf_values_, std::vector<unc::vtndrvcache::ConfigNode *> &cfgnode_vector) {**=='%(args2,args3)
   get_fill_portmap = get_fill_portmap + 'pfc_log_debug("url is :%s",url.c_str());**=='
   virtual_class_name = parser.ReadValues(item, 'get_fill_portmap')['virt_class_name']
   get_fill_portmap = get_fill_portmap + '%s obj;**=='%(virtual_class_name)
   get_fill_portmap = get_fill_portmap + 'pfc_log_debug("entering in to fill_node");**=='
   get_fill_portmap = get_fill_portmap + 'int ret_val = obj.get_fill_portmap(url, key, val, ctr, conf_values_, cfgnode_vector);**=='
   get_fill_portmap = get_fill_portmap + 'pfc_log_debug("leaving from fill_node");**=='
   get_fill_portmap = get_fill_portmap + 'if (ret_val != UNC_RC_SUCCESS)**=='
   get_fill_portmap = get_fill_portmap + 'return UNC_DRV_RC_ERR_GENERIC;**=='
   get_fill_portmap = get_fill_portmap + 'pfc_log_trace("get_fill_portmap_:%s", key.if_name);**=='
   get_fill_portmap = get_fill_portmap + 'return UNC_RC_SUCCESS;**==' '}**=='
   for line in get_fill_portmap.split('**=='):
     file_desc.write("%s\n" % line)


def get_array_idx():
 get_array_idx ='void get_array_idx (json_object **out, json_object *in, uint32_t  arr_idx) {**=='
 get_array_idx = get_array_idx + '*out = json_object_array_get_idx(in, arr_idx);**=='
 get_array_idx = get_array_idx + ' pfc_log_debug("arr_idx_one:%s", unc::restjson::JsonBuildParse::get_json_string(*out));**==}**=='
 for line in get_array_idx.split('**=='):
   file_desc.write("%s\n" % line)

def get_virtual_method(item, file_desc):
 
  args2 = parser.ReadValues(item, 'class')['ipc_key_struct']
  args3 = parser.ReadValues(item, 'class')['ipc_val_struct']

  member_get_virtual = parser.ReadValues(item, 'virtual_method')['get_virtual_method']
  if member_get_virtual != 'empty':
   get_virtual_method = 'UncRespCode get_virtual_method (unc::driver::controller* ctr, %s &key, %s &val) {**=='%(args2, args3)
   get_virtual_method = get_virtual_method + 'int retval = 1;**=='
   members = parser.ReadValues(item,'virtual_method')['get_virtual_members']
   for member in members.split(','):
    get_virtual_methods = parser.ReadValues(item, 'virtual_method')['get_virtual_method']
    if get_virtual_methods == 'yes': 
     get_virtual_method = get_virtual_method + 'retval = get_%s(ctr, key, val);**=='%(member)
     get_virtual_method = get_virtual_method + 'if (retval != UNC_RC_SUCCESS)**=='
     get_virtual_method = get_virtual_method + 'return UNC_DRV_RC_ERR_GENERIC;**=='
   get_virtual_method = get_virtual_method + 'return UNC_RC_SUCCESS;**=='
   get_virtual_method = get_virtual_method + '}**=='

   for line in get_virtual_method.split('**=='):
    file_desc.write("%s\n" % line)

   virtual_class_name = parser.ReadValues(item, 'virtual_method')['virtual_class_name']
   members = parser.ReadValues(item,'virtual_method')['get_virtual_members']
   print members
   for member in members.split(','):
    print member
    get_vir_method =  'virtual UncRespCode get_%s (unc::driver::controller* ctr, %s &key, %s &val)**=='% (member,args2,args3)
    get_vir_method = get_vir_method + ' {**=='
    get_vir_method = get_vir_method + '%s obj;**=='%(virtual_class_name)
    get_vir_method = get_vir_method + 'pfc_log_info("entering in to %s");**=='% (member)
    print member
    get_vir_method = get_vir_method + 'int ret_val = obj.get_%s(ctr,key ,val);**=='%(member)
    get_vir_method = get_vir_method + 'if (ret_val != UNC_RC_SUCCESS)**=='
    get_vir_method = get_vir_method + 'return UNC_DRV_RC_ERR_GENERIC;**=='
    get_vir_method = get_vir_method + 'pfc_log_info("leaving from %s");**=='%(member)
    get_vir_method = get_vir_method + 'return UNC_RC_SUCCESS;**==}**=='

    for line in get_vir_method.split('**=='):
     file_desc.write("%s\n" % line)

 

def send_httprequest():
  send_httprequest = 'int  send_httprequest(unc::driver::controller* ctr, std::string url, unc::restjson::ConfFileValues_t conf_values_ , unc::restjson::HttpMethod http_method, json_object *jobj_req_body)**=={**=='
  send_httprequest = send_httprequest + 'unc::restjson::RestUtil rest_util_obj(ctr->get_host_address(), ctr->get_user_name(), ctr->get_pass_word());**=='
  send_httprequest = send_httprequest + 'unc::restjson::HttpResponse_t* response = rest_util_obj.send_http_request( url, http_method, unc::restjson::JsonBuildParse::get_json_string(jobj_req_body), conf_values_);**=='
  send_httprequest = send_httprequest + 'if (NULL == response)**== {**=='
  send_httprequest = send_httprequest + ' pfc_log_error("Error Occured while getting httpresponse");**=='
  send_httprequest = send_httprequest + 'return  UNC_DRV_RC_ERR_GENERIC;**==}**=='
  send_httprequest = send_httprequest + 'int resp_code = response -> code;**=='
  send_httprequest = send_httprequest + ' return resp_code;**==}**=='
  for line in send_httprequest.split('**=='):
            file_desc.write("%s\n" % line)


def json_build_parse(item, file_desc):
 print "Json Build Parse Started"
 begin_headers(item, file_desc)
 begin_namespace(item, file_desc)
 construct_methods(item, file_desc)
 set_filter(item,  file_desc)
 get_fill_portmap(item,file_desc)
 get_array_idx()
 get_virtual_method(item, file_desc)
 send_httprequest()
 end_namespace(item, file_desc)
 end_footers(file_desc)

# Main Block
if __name__ == '__main__':
 rest_file = sys.argv[1]
 output_file_name = sys.argv[2]
 file_name = rest_file[:-4] + 'hh'
 output_file_name = output_file_name + file_name
 file_desc = open(output_file_name, "w")
 json_build_parse(rest_file, file_desc)
 file_desc.close()
else:
 print "Code_gen loaded as module"
