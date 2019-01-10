import ctypes
from ctypes import *
from global_vars import *
from datatype import *

# return the POU type, e.g. "function", "function block", "program"
def getPOUType(pou_name):
  global function_map
  if function_map.has_key(pou_name) == True:
    return "function"
  elif function_block_map.has_key(pou_name) == True:
    return "function_block"
  elif program_map.has_key(pou_name) == True:
    return "program"
  return None

# return the POU return type
def getPOURetType(pou_name):
  if getPOUType(pou_name) != "function":
    return None
  return function_map[pou_name][1][1][0][0]

# return the POU variables, it is a list of var names, the element order of
# the list is: VAR_INPUT, VAR_OUTPUT, VAR_INOUT, VAR, VAR_TEMP, VAR_EXTERNAL
def getPOUVarsAll(pou_name):
  return var_map[pou_name]["var name list"]

def getPOUVarInputs(pou_name):
  var_input = []
  if getPOUType(pou_name) == "function":
      func_vars = var_map[pou_name]
      if func_vars.has_key("var_input") == True:
        for var_name in func_vars["var_input"]:
          var_input.append(var_name)
  elif getPOUType(pou_name) == "function_block":
      fb_vars = var_map[pou_name]
      if fb_vars.has_key("var_input") == True:
        for var_name in fb_vars["var_input"]:
          var_input.append(var_name)
  elif getPOUType(pou_name) == "program":
      pro_vars = var_map[pou_name]
      if pro_vars.has_key("var_input") == True:
        for var_name in pro_vars["var_input"]:
          var_input.append(var_name)
  return var_input

def getPOUVarOutputs(pou_name):
  var_output = []
  if getPOUType(pou_name) == "function":
      func_vars = var_map[pou_name]
      if func_vars.has_key("var_output") == True:
        for var_name in func_vars["var_output"]:
          var_output.append(var_name)
  elif getPOUType(pou_name) == "function_block":
      fb_vars = var_map[pou_name]
      if fb_vars.has_key("var_output") == True:
        for var_name in fb_vars["var_output"]:
          var_output.append(var_name)
  elif getPOUType(pou_name) == "program":
      pro_vars = var_map[pou_name]
      if pro_vars.has_key("var_output") == True:
        for var_name in pro_vars["var_output"]:
          var_output.append(var_name)
  return var_output

def getPOUVarInOuts(pou_name):
  var_inout = []
  if getPOUType(pou_name) == "function":
      func_vars = var_map[pou_name]
      if func_vars.has_key("var_inout") == True:
        for var_name in func_vars["var_inout"]:
          var_inout.append(var_name)
  elif getPOUType(pou_name) == "function_block":
      fb_vars = var_map[pou_name]
      if fb_vars.has_key("var_inout") == True:
        for var_name in fb_vars["var_inout"]:
          var_inout.append(var_name)
  elif getPOUType(pou_name) == "program":
      pro_vars = var_map[pou_name]
      if pro_vars.has_key("var_inout") == True:
        for var_name in pro_vars["var_inout"]:
          var_inout.append(var_name)
  return var_inout

def getPOUVars(pou_name):
  var_local = []
  if getPOUType(pou_name) == "function":
      func_vars = var_map[pou_name]
      if func_vars.has_key("var") == True:
        for var_name in func_vars["var"]:
          var_local.append(var_name)
  elif getPOUType(pou_name) == "function_block":
      fb_vars = var_map[pou_name]
      if fb_vars.has_key("var") == True:
        for var_name in fb_vars["var"]:
          var_local.append(var_name)
  elif getPOUType(pou_name) == "program":
      pro_vars = var_map[pou_name]
      if pro_vars.has_key("var_local") == True:
        for var_name in pro_vars["var_local"]:
          var_local.append(var_name)
  return var_local

def getPOUVarTemps(pou_name):
  var_temp = []
  if getPOUType(pou_name) == "function":
      func_vars = var_map[pou_name]
      if func_vars.has_key("var_temp") == True:
        for var_name in func_vars["var_temp"]:
          var_temp.append(var_name)
  elif getPOUType(pou_name) == "function_block":
      fb_vars = var_map[pou_name]
      if fb_vars.has_key("var_temp") == True:
        for var_name in fb_vars["var_temp"]:
          var_temp.append(var_name)
  elif getPOUType(pou_name) == "program":
      pro_vars = var_map[pou_name]
      if pro_vars.has_key("var_temp") == True:
        for var_name in pro_vars["var_temp"]:
          var_temp.append(var_name)
  return var_temp

def getPOUVarExternals(pou_name):
  var_external = []
  if getPOUType(pou_name) == "function":
      func_vars = var_map[pou_name]
      if func_vars.has_key("var_external") == True:
        for var_name in func_vars["var_external"]:
          var_external.append(var_name)
  elif getPOUType(pou_name) == "function_block":
      fb_vars = var_map[pou_name]
      if fb_vars.has_key("var_external") == True:
        for var_name in fb_vars["var_external"]:
          var_external.append(var_name)
  elif getPOUType(pou_name) == "program":
      pro_vars = var_map[pou_name]
      if pro_vars.has_key("var_external") == True:
        for var_name in pro_vars["var_external"]:
          var_external.append(var_name)
  return var_external

# return the POU variable class, e.g. "var_input", "var_output", "var"
def getPOUVarClass(pou_name, var_name):
  if var_name in getPOUVarInputs(pou_name):
    return "var_input"
  elif var_name in getPOUVarOutputs(pou_name):
    return "var_output"
  elif var_name in getPOUVarInOuts(pou_name):
    return "var_inout"
  elif var_name in getPOUVars(pou_name):
    return "var"
  elif var_name in getPOUVarTemps(pou_name):
    return "var_temp"
  elif var_name in getPOUVarExternals(pou_name):
    return "var_external"
  return None

# return the POU variable AST node
def getPOUVarNode(pou_name, var_name):
  global var_map
  if getPOUType(pou_name) == "function":
      func_vars = var_map[pou_name]
      for var_type in func_vars:
        variable_map = func_vars[var_type]
        if variable_map.has_key(var_name) == True:
          return variable_map[var_name]
  elif getPOUType(pou_name) == "function_block":
      fb_vars = var_map[pou_name]
      for var_type in fb_vars:
        variable_map = fb_vars[var_type]
        if variable_map.has_key(var_name) == True:
          return variable_map[var_name]
  elif getPOUType(pou_name) == "program":
      pro_vars = var_map[pou_name]
      for var_type in pro_vars:
        variable_map = pro_vars[var_type]
        if variable_map.has_key(var_name) == True:
          return variable_map[var_name]
  return None


# return the POU variable type, e.g. "type-int", "type-real", or user defined types
def getPOUVarType(pou_name, var_name):
  var_node = getPOUVarNode(pou_name, var_name)
  type = None
  if var_node is not None:
    if not isinstance(var_node[1][0][1][0], str):
      type = var_node[1][0][1][0][0]
    else:
      type = var_node[1][0][1][0]
  return type

# return the POU variable option, e.g. "retain", "non-retain", "const", ""
def getPOUVarOption(pou_name, var_name):
  pou_node = None
  option = None
  if function_map.has_key(pou_name) == True:
    pou_node = function_map[pou_name]
    for i in range (2, len(pou_node)):
      if pou_node[i][0] != "function_body":
        if pou_node[i][1][0][0] != "var_init_decl":
          option = pou_node[i][1][0][0]
          for j in range(0, len(pou_node[i][1]) - 1):
              if pou_node[i][1][1][1][0][1][0] == var_name:
                return option
        else:
          for j in range(0, len(pou_node[i][1])):
              if pou_node[i][1][j][1][0][1][0] == var_name:
                return option
  elif function_block_map.has_key(pou_name) == True:
    pou_node = function_block_map[pou_name]
    for i in range (1, len(pou_node)):
      if pou_node[i][0] != "function_block_body":
        if pou_node[i][1][0][0] != "var_init_decl":
          option = pou_node[i][1][0][0]
          for j in range(0, len(pou_node[i][1]) - 1):
              if pou_node[i][1][1][1][0][1][0] == var_name:
                return option
        else:
          for j in range(0, len(pou_node[i][1])):
              if pou_node[i][1][j][1][0][1][0] == var_name:
                return option
  elif program_map.has_key(pou_name) == True:
    pou_node = program_map[pou_name]
    for i in range (1, len(pou_node)):
      if pou_node[i][0] != "program_body":
        if pou_node[i][1][0][0] != "var_init_decl":
          option = pou_node[i][1][0][0]
          for j in range(0, len(pou_node[i][1]) - 1):
              if pou_node[i][1][1][1][0][1][0] == var_name:
                return option
        else:
          for j in range(0, len(pou_node[i][1])):
              if pou_node[i][1][j][1][0][1][0] == var_name:
                return option
  return option

# return the POU variable initial expression node
def getPOUVarInitValue(pou_name, var_name):
  var_node = getPOUVarNode(pou_name, var_name)
  var_init_node = None
  if var_node is not None:
    var_init_node = var_node[1][1]
  return var_init_node


# check whether a variable is converted to function parameter
def isFuncParam(func_name, var_name):
  var_class = getPOUVarClass(func_name, var_name);
  return var_class in ["var_input", "var_inout", "var_output"]

# return the function param num
def getFuncParamNum(func_name):
  func_vars = getPOUVarsAll(func_name)
  param_num = 0
  for var_name in func_vars:
    if isFuncParam(func_name, var_name):
      param_num = param_num + 1

  func_ret_type = getPOURetType(func_name)
  if (func_ret_type != ""):
    param_num = param_num + 1
  return param_num

# return the function param index of a variable
def getFuncParamIndex(func_name, var_name):
  func_vars = getPOUVarsAll(func_name)
  param_index = 0
  for var_name1 in func_vars:
    if isFuncParam(func_name, var_name1):
      if (var_name1 == var_name):
        return param_index
      param_index = param_index + 1

  return -1

# return the function param type of a variable
def getFuncParamType(func_name, var_name):
  if (isFuncParamAPointer(func_name, var_name)):
    return "i32"
  else:
    var_type = getPOUVarType(func_name, var_name)
    if (var_type in basic_types):
      return basic_types_wasm_type_map[var_type]
    else:
      dec_type = getDatatypeDeclarationType(var_type)
      if (dec_type == "enumerated_type_declaration"):
        return "i32"
      elif (dec_type == "simple_type_declaration"):
        directly_type = getDirectlyDatatypeType(var_type)
        if directly_type in basic_types:
          return basic_types_wasm_type_map[directly_type]
      print "Error: the function param type should be a pointer"

# check whether the function param is converted to pointer
def isFuncParamAPointer(func_name, var_name):
  var_class = getPOUVarClass(func_name, var_name)
  var_type = getPOUVarType(func_name, var_name)
  if var_class != "var_input":
    return True
  else:
    if var_type in basic_types:
      return False
    else:
      dec_type = getDatatypeDeclarationType(var_type)
      if (dec_type == "enumerated_type_declaration"):
        return False
      elif (dec_type == "simple_type_declaration"):
        directly_type = getDirectlyDatatypeType(var_type)
        if directly_type in basic_types:
          return False
        else:
          return True
      elif (dec_type == "structure_type_declaration"):
        return True
      else:
        print "TODO: unknown var declaration type " + dec_type

# check whether a variable is converted to function local variable
def isFuncLocal(func_name, var_name):
  var_class = getPOUVarClass(func_name, var_name);
  return var_class in ["var", "var_temp"]

# get the function local variable num
def getFuncLocalNum(func_name):
  func_vars = getPOUVarsAll(func_name)
  local_num = 0
  for var_name in func_vars:
    if isFuncLocal(func_name, var_name):
      local_num = local_num + 1
  return local_num

# get the function local variable index of a variable
def getFuncLocalIndex(func_name, var_name):
  func_vars = getPOUVarsAll(func_name)
  local_index = 0
  for var_name1 in func_vars:
    if isFuncLocal(func_name, var_name1):
      if var_name1 == var_name:
        return local_index
      local_index = local_index + 1
  return -1


# check whether a variable is converted to field or local variable
def isFuncBlockField(fb_name, var_name):
  var_class = getPOUVarClass(fb_name, var_name);
  return var_class in ["var_input", "var_inout", "var_output", "var"]

# return the Function Block field number
def getFuncBlockFieldNum(fb_name):
  fb_vars = getPOUVarsAll(fb_name)
  field_num = 0
  for var_name in fb_vars:
    if isFuncBlockField(fb_name, var_name):
      field_num = field_num + 1
  return field_num

# return the Function Block field index of a variable
def getFuncBlockFieldIndex(fb_name, var_name):
  fb_vars = getPOUVarsAll(fb_name)
  field_index = 0
  for var_name1 in fb_vars:
    if isFuncBlockField(fb_name, var_name1):
      if (var_name1 == var_name):
        return field_index
      field_index = field_index + 1

  return -1

# return the Function Block field offset of a variable
def getFuncBlockFieldOffset(fb_name, var_name):
  fb_vars = getPOUVarsAll(fb_name)
  field_offset = 0
  for var_name1 in fb_vars:
    if isFuncBlockField(fb_name, var_name1):
      if (var_name1 == var_name):
        return field_offset

      var_type = getPOUVarType(fb_name, var_name1)
      if (var_type in basic_types):
        field_offset += basic_types_size_map[var_type]
      else:
        dec_type = getDatatypeDeclarationType(var_type)
        if (dec_type == "enumerated_type_declaration"):
          field_offset = field_offset + basic_types_size_map["type_int"]
        elif (dec_type == "simple_type_declaration"):
          directly_type = getDirectlyDatatypeType(var_type)
          if directly_type in basic_types:
            field_offset = field_offset + basic_types_size_map[directly_type]
          else:
            print "Error: unknown directly type " + directly_type
        elif (dec_type == "structure_type_declaration"):
          field_offset = field_offset + getStructDatatypeTotalSize(var_type)
        else:
          print "Error: unknown declaration type " + dec_type

  return -1

# return the Function Block instance total size
def getFuncBlockTotalSize(fb_name):
  fb_vars = getPOUVarsAll(fb_name)
  total_size = 0
  for var_name in fb_vars:
    if isFuncBlockField(fb_name, var_name):
      var_type = getPOUVarType(fb_name, var_name)
      if (var_type in basic_types):
        total_size += basic_types_size_map[var_type]
      else:
        dec_type = getDatatypeDeclarationType(var_type)
        if (dec_type == "enumerated_type_declaration"):
          total_size = total_size + basic_types_size_map["type_int"]
        elif (dec_type == "simple_type_declaration"):
          directly_type = getDirectlyDatatypeType(var_type)
          if directly_type in basic_types:
            total_size = total_size + basic_types_size_map[directly_type]
          else:
            print "Error: unknown directly type " + directly_type
        elif (dec_type == "structure_type_declaration"):
          total_size = total_size + getStructDatatypeTotalSize(var_type)
        else:
          print "Error: unknown declaration type " + dec_type

  return total_size

def isFuncBlockLocal(fb_name, var_name):
  var_class = getPOUVarClass(fb_name, var_name);
  return var_class == "var_temp"

# return the local variable number of function block
def getFuncBlockLocalNum(fb_name):
  fb_vars = getPOUVarsAll(fb_name)
  local_num = 0
  for var_name in fb_vars:
    if isFuncBlockLocal(fb_name, var_name):
      local_num = local_num + 1
  return local_num

# return the local variable index of the var
def getFuncBlockLocalIndex(fb_name, var_name):
  fb_vars = getPOUVarsAll(fb_name)
  local_index = 0
  for var_name1 in fb_vars:
    if isFuncBlockLocal(fb_name, var_name1):
      if (var_name1 == var_name):
        return local_index
      local_index = local_index + 1

  return -1

def getFuncBlockLocalType(fb_name, var_name):
  return "i32"

def isFuncBlockLocalAPointer(fb_name, var_name):
  return True

def dumpFuncVarTranslateInfo(func_name):
  print getPOUType(func_name) + " " + func_name + ":"
  vars = getPOUVarsAll(func_name)
  for var in vars:
    is_ptr = ""
    if isFuncParamAPointer(func_name, var):
      is_ptr = " ptr"
    if isFuncParam(func_name, var):
      print "  " + var + ": param " + str(getFuncParamIndex(func_name, var)) \
            + ", type: " + getFuncParamType(func_name, var) + is_ptr
    elif isFuncLocal(func_name, var):
      print "  " + var + ": local " + str(getFuncLocalIndex(func_name, var)) \
            + ", type: " + getFuncParamType(func_name, var) + is_ptr
    else:
      print "  " + var + ": unhandled yet"

  print ""
  print "  total param num: " + str(getFuncParamNum(func_name))
  print "  total local num: " + str(getFuncLocalNum(func_name))

def dumpFuncBlockVarTranslateInfo(fb_name):
  print getPOUType(fb_name) + " " + fb_name + ":"
  vars = getPOUVarsAll(fb_name)
  for var in vars:
    if isFuncBlockField(fb_name, var):
      field_index = getFuncBlockFieldIndex(fb_name, var)
      field_offset = getFuncBlockFieldOffset(fb_name, var)
      print "  " + var + ": field " + str(field_index) + ", offset: " + str(field_offset)
    elif isFuncBlockLocal(fb_name, var):
      local_index = getFuncBlockLocalIndex(fb_name, var)
      local_type = getFuncBlockLocalType(fb_name, var)
      is_ptr = ""
      if isFuncBlockLocalAPointer(fb_name, var):
        is_ptr = " ptr"
      print "  " + var + ": local " + str(local_index) + ", type: " + str(local_type) + is_ptr
    else:
      print "  " + var + ": unhandled yet"

  print ""
  print "  total field num: " + str(getFuncBlockFieldNum(fb_name))
  print "  instance size: " + str(getFuncBlockTotalSize(fb_name))
  print "  total local num: " + str(getFuncBlockLocalNum(fb_name))

