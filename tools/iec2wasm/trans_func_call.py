import ctypes
from ctypes import *
from global_vars import *
from pou import *

# translate function call statements, return Operand
def translateFunctionCall(module, pou_name, ast_node):
  # TODO: handle function paramenters, and check data types
  func_name = ast_node[1][0][1][0]
  bny_ref = lib_bny.BinaryenCall(module, func_name, None, c_int(0),
                                 lib_bny.BinaryenTypeInt32())
  return Operand(bny_ref, "type_int", "i32")

