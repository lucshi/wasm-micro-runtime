import ctypes
from ctypes import *
from global_vars import *

# return the POU type, e.g. "function", "function block", "program"
def getPOUType(pou_name):
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

def translate():
  global lib_bny
  global datatype_map
  ll = ctypes.cdll.LoadLibrary
  lib_bny = ll("./libbinaryen.so")
  lib_bny.BinaryenSetAPITracing(c_int(0));

  module = lib_bny.BinaryenModuleCreate()
  lib_bny.BinaryenAddMemoryImport(module, b"memory", b"env", b"memory", c_ubyte(0))
  lib_bny.BinaryenSetMemory(module, c_int(1), c_int(1), c_void_p(), c_void_p(), c_void_p(), c_void_p(), c_int(0), c_ubyte(0))
  lib_bny.BinaryenAddFunctionType(module, c_void_p(), lib_bny.BinaryenTypeInt32(), c_void_p(), c_int(0));
  lib_bny.BinaryenModulePrint(module)

