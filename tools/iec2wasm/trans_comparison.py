import ctypes
from ctypes import *
from global_vars import *
from pou import *
from operand import *

# translate comparison statement, return Operand
def translateComparison(module, pou_name, operator, operand1, operand2):
  # TODO: check data type and handle implict data type conversion
  bny_ref = None
  bny_op = None
  if (operator == "equals"):
    bny_op = lib_bny.BinaryenEqInt32()
    bny_ref = lib_bny.BinaryenUnary(module, bny_op, operand1.bny_ref)
    return Operand(bny_ref, "type_int", "i32")
  elif (operator == "equals_not"):
    bny_op = lib_bny.BinaryenNeInt32()
    bny_ref = lib_bny.BinaryenUnary(module, bny_op, operand1.bny_ref)
    return Operand(bny_ref, "type_int", "i32")
  elif (operator == "greater_than"):
    bny_op = lib_bny.BinaryenGtSInt32()
  elif (operator == "greater_or_equal"):
    bny_op = lib_bny.BinaryenGeSInt32()
  elif (operator == "less_than"):
    bny_op = lib_bny.BinaryenLtSInt32()
  elif (operator == "less_or_equal"):
    bny_op = lib_bny.BinaryenLeSInt32()

  bny_ref = lib_bny.BinaryenBinary(module, bny_op, operand1.bny_ref, operand2.bny_ref)
  return Operand(bny_ref, "type_int", "i32")

