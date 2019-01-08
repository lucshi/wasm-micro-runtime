import ctypes
from ctypes import *
from global_vars import *
from pou import *
from operand import *

# translate logical statement, return Operand
def translateLogical(module, pou_name, operator, operand1, operand2):
  # TODO: check data type and handle implict data type conversion
  bny_ref = None
  bny_op = None
  if (operator == "logical_and"):
    bny_op = lib_bny.BinaryenAndInt32()
  elif (operator == "logical_or"):
    bny_op = lib_bny.BinaryenOrInt32()
  elif (operator == "logical_xor"):
    bny_op = lib_bny.BinaryenXorInt32()
  elif (operator == "logical_not"):
    bny_op = lib_bny.BinaryenXorInt32()
    bny_ref1 = lib_bny.BinaryenConst(module, lib_bny.BinaryenLiteralInt32(c_int(-1)))
    operand2 = Operand(bny_ref1, "type_int", "i32")

  bny_ref = lib_bny.BinaryenBinary(module, bny_op, operand1.bny_ref, operand2.bny_ref)
  return Operand(bny_ref, operand1.iec_type, operand2.bny_type)
