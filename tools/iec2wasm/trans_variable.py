import ctypes
from ctypes import *
from global_vars import *
from pou import *

# Get variable, return Operand
def translateGetVariable(module, pou_name, ast_node_var):
  # TODO, just generate get_local for a test
  if (ast_node_var[1][0] in ["x", "y", "z"]):
    i = ["x", "y", "z"].index(ast_node_var[1][0])
    bny_ref = lib_bny.BinaryenGetLocal(module, c_int(i), lib_bny.BinaryenTypeInt32())
    return Operand(bny_ref, "type_int", "i32")
  else:
    bny_ref = lib_bny.BinaryenGetLocal(module, c_int(20), lib_bny.BinaryenTypeInt32())
    return Operand(bny_ref, "type_int", "i32")

# Set variable, return bny_ref
def translateSetVariable(module, pou_name, ast_node_var, operand):
  # TODO, generate set_local $0 just for a test
  bny_ref = lib_bny.BinaryenSetLocal(module, c_int(0), operand.bny_ref)
  return bny_ref

# Get structure field, return Operand
def translateGetMultiElemVariable(module, pou_name, ast_node_var):
  return None

# Set structure field, return bny_ref
def translateSetMultiElemVariable(module, pou_name, ast_node_var, operand):
  return None

