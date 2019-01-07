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

basic_types = { "type_bool" "type_int", "type_dint", "type_real", "type_lreal" }
wasm_type_map = {}

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

