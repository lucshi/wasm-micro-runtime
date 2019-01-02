import ctypes
from ctypes import *
from global_vars import *

# return the POU type, e.g. "function", "function block", "program"
def getPOUType(pou_name):
  return ""

# return the POU return type
def getPOURetType(pou_name):
  return ""

# return the POU variables, it is a list of var names, the element order of
# the list is: VAR_INPUT, VAR_OUTPUT, VAR_INOUT, VAR, VAR_TEMP, VAR_EXTERNAL
def getPOUVarsAll(pou_name):
  return ""

def getPOUVarInputs(pou_name):
  return ""

def getPOUVarOutputs(pou_name):
  return ""

def getPOUVarInOuts(pou_name):
  return ""

def getPOUVars(pou_name):
  return ""

def getPOUVarTemps(pou_name):
  return ""

def getPOUVarExternals(pou_name):
  return ""

# return the POU variable class, e.g. "var_input", "var_output", "var"
def getPOUVarClass(pou_name, var_name):
  return ""

# return the POU variable type, e.g. "type-int", "type-real", or user defined types
def getPOUVarType(pou_name, var_name):
  return ""

# return the POU variable option, e.g. "retain", "non-retain", "const", ""
def getPOUVarOption(pou_name, var_name):
  return ""

# return the POU variable initial value
def getPOUVarInitValue(pou_name, var_name):
  return ""

class BinaryenLiteralUnion(Union):
  _fields_ = [("i32", c_int),
              ("i64", c_longlong),
              ("f32", c_float),
              ("f64", c_double),
              ("v128", c_ubyte * 16)]

class BinaryenLiteral(Structure):
  _fields_ = [("type", c_int),
              ("u", BinaryenLiteralUnion)]

def BinaryenLiteralInt32(x):
  literal = BinaryenLiteral()
  literal.type = c_int(1)
  literal.u.i32 = c_int(x)
  return literal

basic_types = { "type_bool" "type_int", "type_dint", "type_real", "type_lreal" }
wasm_type_map = {}

def isBasicType(type_name):
  global basic_types
  return type_name in basic_types

def initWasmtypeMap():
  global wasm_type_map
  global lib_bny
  wasm_type_map["type_bool"] = lib_bny.BinaryenTypeInt32()
  wasm_type_map["type_int"] = lib_bny.BinaryenTypeInt32()
  wasm_type_map["type_dint"] = lib_bny.BinaryenTypeInt32()
  wasm_type_map["type_real"] = lib_bny.BinaryenTypeFloat32()
  wasm_type_map["type_lreal"] = lib_bny.BinaryenTypeFloat64()

def basictype2Wasmtype(type_name):
  return wasm_types_map[type_name];

def translateFunc(module, func_name):
  #i32_const_1 = lib_bny.BinaryenConst(module, lib_bny.BinaryenLiteralInt32(c_int(1234)))
  #i32_const_2 = lib_bny.BinaryenConst(module, lib_bny.BinaryenLiteralInt32(c_int(5678)))
  #add_res = lib_bny.BinaryenBinary(module, lib_bny.BinaryenAddInt32(), i32_const_1, i32_const_2)
  #func_body = lib_bny.BinaryenReturn(module, add_res)
  i32_const = lib_bny.BinaryenConst(module, lib_bny.BinaryenLiteralInt32(c_int(1234)))
  func_body = lib_bny.BinaryenReturn(module, i32_const)
  return func_body

def translateFuncBlock(module, func_block_name):
  call_res = lib_bny.BinaryenCall(module, "function0", None, c_int(0),
      lib_bny.BinaryenLiteralInt32(c_int(1234)))
  func_body = lib_bny.BinaryenReturn(module, call_res)
  return func_body

def translateFuncs(module):
  global lib_bny
  global function_map
  for func_name in function_map:
    # Parse function propotype
    func_vars = getPOUVarsAll(func_name)
    func_ret_type = getPOURetType(func_name)

    param_types = []
    local_types = []
    for var_name in func_vars:
      var_class = getPOUVarClass(func_name, var_name)
      var_type = getPOUVarType(func_name, var_name)
      if var_class == "var_input" and isBasicType(var_type):
        param_types.append(wasm_type_map[var_type])
      elif (var_class != "var" and var_class != "var_temp"):
        param_types.append(lib_bny.BinaryenTypeInt32())
      else:
        local_types.append(wasm_type_map[var_type])

    if (func_ret_type != ""):
      param_types.append(lib_bny.BinaryenTypeInt32())

    # Convert to Binaryen function type
    bny_param_types = (c_int * len(param_types))()
    for i in range (0, len(param_types)):
      bny_param_types[i] = param_types[i]

    bny_local_types = (c_int * len(local_types))()
    for i in range (0, len(local_types)):
      bny_local_types[i] = local_types[i]

    # Add function type and function
    func_type = lib_bny.BinaryenAddFunctionType(module, None, None,
        bny_param_types, c_int(len(bny_param_types)))
    func_body = translateFunc(module, func_name)
    func = lib_bny.BinaryenAddFunction(module, c_char_p(func_name), func_type,
        bny_local_types, len(local_types), func_body)


def translateFuncBlocks(module):
  global lib_bny
  global function_block_map
  for func_block_name in function_block_map:
    func_var_temps = getPOUVarTemps(func_block_name)

    local_types = []
    for var_name in func_var_temps:
      var_class = getPOUVarClass(func_block_name, var_name)
      var_type = getPOUVarType(func_block_name, var_name)
      local_types.append(wasm_type_map[var_type])

    bny_param_types = (c_int * 1)(lib_bny.BinaryenTypeInt32())
    bny_local_types = (c_int * len(local_types))()
    for i in range (0, len(local_types)):
      bny_local_types[i] = local_types[i]

    # Add function type and function
    func_type = lib_bny.BinaryenAddFunctionType(module, None, None,
        bny_param_types, c_int(len(bny_param_types)))
    func_body = translateFuncBlock(module, func_block_name)
    func = lib_bny.BinaryenAddFunction(module, c_char_p(func_block_name), func_type,
        bny_local_types, len(local_types), func_body)

def translate():
  global lib_bny
  global datatype_map
  global basic_types

  # Load Binaryen Library
  ll = ctypes.cdll.LoadLibrary
  lib_bny = ll("./libbinaryen.so")
  #lib_bny.BinaryenSetAPITracing(c_int(0));

  lib_bny.BinaryenLiteralInt32.restype = BinaryenLiteral
  lib_bny.BinaryenLiteralInt64.restype = BinaryenLiteral
  lib_bny.BinaryenLiteralFloat32.restype = BinaryenLiteral
  lib_bny.BinaryenLiteralFloat64.restype = BinaryenLiteral
  lib_bny.BinaryenLiteralVec128.restype = BinaryenLiteral
  lib_bny.BinaryenLiteralFloat32Bits.restype = BinaryenLiteral
  lib_bny.BinaryenLiteralFloat64Bits.restype = BinaryenLiteral

  initWasmtypeMap()

  # Create Binaryen Moduel
  module = lib_bny.BinaryenModuleCreate()

  # Create Memory
  lib_bny.BinaryenAddMemoryImport(module, b"memory", b"env", b"memory", c_ubyte(0))
  lib_bny.BinaryenSetMemory(module, c_int(1), c_int(1), None, None, None, None, c_int(0), c_ubyte(0))

  translateFuncs(module)
  translateFuncBlocks(module)

  lib_bny.BinaryenModulePrint(module)

