import ctypes
from ctypes import *
from global_vars import *
from pou import *

# Const, return Operand
def translateConst(module, pou_name, ast_node_const):
  if (ast_node_const[0] == "integer_literal"):
    bny_literal = lib_bny.BinaryenLiteralInt32(c_int(int(ast_node_const[1][0])))
    return Operand(lib_bny.BinaryenConst(module, bny_literal), "type_int", "i32")
  elif (ast_node_const[0] == "real_literal"):
    bny_literal = lib_bny.BinaryenLiteralFloat32(c_float(float(ast_node_const[1][0])))
    return Operand(lib_bny.BinaryenConst(module, bny_literal), "type_real", "f32")
  else:
    print "TODO: unknown const type: " + ast_node_const[0]
