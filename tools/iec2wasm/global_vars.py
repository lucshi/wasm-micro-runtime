import ctypes
from ctypes import *

# global hash maps
datatype_map = {}
function_map = {}
program_map = {}
function_block_map = {}
configuration_map = {}
var_map = {}
global_maps = {}
resource_maps = {}
task_maps = {}

# Load Binaryen Library
lib_bny = ctypes.cdll.LoadLibrary("./libbinaryen.so")

basic_types = { "type_bool", "type_int", "type_dint", "type_real", "type_lreal" }

basic_types_size_map = {
  "type_bool" : 4,
  "type_int" : 4,
  "type_dint" : 4,
  "type_real" : 4,
  "type_lreal" : 8
}

basic_types_init_value_map = {
  "type_bool" : "0",
  "type_int" : "0",
  "type_dint" : "0",
  "type_real" : "0.0",
  "type_lreal" : "0.0",
  "type_datetime" : "DT#1970-01-01-00:00:00"
}

wasm_type_map = {
  "type_bool" : lib_bny.BinaryenTypeInt32(),
  "type_int" : lib_bny.BinaryenTypeInt32(),
  "type_dint" : lib_bny.BinaryenTypeInt32(),
  "type_real" : lib_bny.BinaryenTypeFloat32(),
  "type_lreal" : lib_bny.BinaryenTypeFloat64(),
}

const_exprs = ["integer_literal", "real_literal"]
arithmetic_operators = ["adding", "subtracting", "multiply_with", "divide_by", "modulo"]
logical_operators = ["logical_and", "logical_or", "logical_xor", "logical_not"]
comparison_operators = ["equals", "equals_not", "greater_than", "greater_or_equal",
                        "less_than", "less_or_equal"]

operator_priorities = { "logical_or" : 1, "logical_xor" : 2, "logical_and" : 3,
                        "equals_not" : 4, "equals" : 5,
                        "greater_than" : 6, "greater_or_equal" : 6,
                        "less_than" :    6, "less_or_equal"    : 6,
                        "subtracting" : 7, "adding" : 8, "modulo" : 9,
                        "divide_by" : 10, "multiply_with" : 11, "logical_not" : 12 }

