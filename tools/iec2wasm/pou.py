import ctypes
from ctypes import *
from global_vars import *

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
  vars = []
  if getPOUType(pou_name) == "function":
      func_vars = var_map[pou_name]
      for var_type in func_vars:
        variable_map = func_vars[var_type]
        for var_name in variable_map:
          vars.append(var_name)
  elif getPOUType(pou_name) == "function_block":
      fb_vars = var_map[pou_name]
      for var_type in fb_vars:
        variable_map = fb_vars[var_type]
        for var_name in variable_map:
          vars.append(var_name)
  elif getPOUType(pou_name) == "program":
      pro_vars = var_map[pou_name]
      for var_type in pro_vars:
        variable_map = pro_vars[var_type]
        for var_name in variable_map:
          vars.append(var_name)
  return vars

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
        for var_name in func_vars["var_local"]:
          var_local.append(var_name)
  elif getPOUType(pou_name) == "function_block":
      fb_vars = var_map[pou_name]
      if fb_vars.has_key("var") == True:
        for var_name in fb_vars["var_local"]:
          var_local.append(var_name)
  elif getPOUType(pou_name) == "program":
      pro_vars = var_map[pou_name]
      if pro_vars.has_key("var") == True:
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
    type = var_node[1][0][1][0][0]
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

