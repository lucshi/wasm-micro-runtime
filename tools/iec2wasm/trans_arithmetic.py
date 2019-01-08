import ctypes
from ctypes import *
from global_vars import *
from pou import *
from operand import *

# translate arithmetic statement, return Operand
def translateArithmetic(module, pou_name, operator, operand1, operand2):
  # TODO: check data type and handle implict data type conversion
  bny_ref = None
  bny_op = None
  if (operator == "adding"):
    bny_op = lib_bny.BinaryenAddInt32()
  elif (operator == "subtracting"):
    bny_op = lib_bny.BinaryenSubInt32()
  elif (operator == "multiply_with"):
    bny_op = lib_bny.BinaryenMulInt32()
  elif (operator == "divide_by"):
    bny_op = lib_bny.BinaryenDivSInt32()
  elif (operator == "modulo"):
    bny_op = lib_bny.BinaryenRemSInt32()

  bny_ref = lib_bny.BinaryenBinary(module, bny_op, operand1.bny_ref, operand2.bny_ref)
  return Operand(bny_ref, operand1.iec_type, operand1.bny_type)
