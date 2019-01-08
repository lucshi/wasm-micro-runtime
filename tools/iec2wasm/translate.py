import ctypes
from ctypes import *
from global_vars import *
from pou import *
from operand import *
from trans_const import *
from trans_variable import *
from trans_if import *
from trans_for import *
from trans_func_call import *
from trans_arithmetic import *
from trans_logical import *
from trans_comparison import *
from trans_conversion import *

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

def isBasicType(type_name):
  global basic_types
  return type_name in basic_types

def basictype2Wasmtype(type_name):
  return wasm_types_map[type_name];

def translateAssignment(module, pou_name, ast_node):
  ast_node_var = ast_node[0]
  ast_node_expr = ast_node[1]
  operand = translateExpression(module, pou_name, ast_node_expr)
  if (ast_node_var[0] == "variable_name"):
    return translateSetVariable(module, pou_name, ast_node_var, operand)
  elif (ast_node_var[0] == "multi_element_variable"):
    return translateSetMultiElemVariable(module, pou_name, ast_node_var, operand)
  else:
    print "TODO: unknown assignment type: " + ast_node_var[0]

class ExprQueueElem:
  def __init__(self, elem_type, elem):
    self.elem_type = elem_type
    self.elem = elem

def ExprQueueAddOperand(queue, ast_node_operand):
  queue_elem = ExprQueueElem("operand", ast_node_operand)
  queue.append(queue_elem)

def ExprQueueAddOperator(queue, operator):
  queue_elem = ExprQueueElem("operator", operator)
  queue.append(queue_elem)

def ExprStackAddOperator(stack, queue, operator):
  while (len(stack) > 0 and
         operator_priorities[operator] <= operator_priorities[stack[len(stack) - 1]]):
    ExprQueueAddOperator(queue, stack[len(stack) - 1])
    stack.pop(len(stack) - 1)
  stack.append(operator)

def generateSuffixExprQueue(module, pou_name, ast_node):
  queue = []
  stack = []
  for i in range(0, len(ast_node[1])):
    queue_elem = None
    operand = None

    expr = ast_node[1][i]
    if expr[0] in const_exprs:
      ExprQueueAddOperand(queue, expr)
    elif (expr[0] == "variable_name"
          or expr[0] == "multi_element_variable"
          or expr[0] == "function_call"):
      ExprQueueAddOperand(queue, expr)
    elif (expr[0] in arithmetic_operators
          or expr[0] in logical_operators
          or expr[0] in comparison_operators):
      ExprStackAddOperator(stack, queue, expr[0])
    elif (expr[0] == "expression"):
      queue1 = generateSuffixExprQueue(module, pou_name, expr)
      for j in range (0, len(queue1)):
        queue.append(queue1[j])
    else:
      print "TODO: unknown expression type: " + expr[0]

  for i in range(0, len(stack)):
    ExprQueueAddOperator(queue, stack[len(stack) - 1 - i])

  return queue

# translate an expression, return Operand
def translateExpression(module, pou_name, ast_node):
  if (ast_node[0] != "expression"):
    print "Fail to translate expression"
    #TODO: throw exception

  # generate the queue of suffix expression
  queue = generateSuffixExprQueue(module, pou_name, ast_node)
  '''
  print "queue:"
  for i in range(0, len(queue)):
    if (queue[i].elem_type == "operand"):
      print queue[i].elem
    else:
      print queue[i].elem
  '''

  # travel the queue
  stack = []
  for i in range(0, len(queue)):
    operand = None
    if queue[i].elem_type == "operand":
      expr = queue[i].elem
      if expr[0] in const_exprs:
        operand = translateConst(module, pou_name, expr)
      elif expr[0] == "variable_name":
        operand = translateGetVariable(module, pou_name, expr)
      elif (expr[0] == "multi_element_variable"):
        operand = translateGetMultiElemVariable(module, pou_name, expr)
      elif (expr[0] == "function_call"):
        operand = translateFunctionCall(module, pou_name, expr)
      else:
        print "TODO: unknown expression type: " + expr[0]
    else: # operator
      operator = queue[i].elem
      if (operator in arithmetic_operators):
        operand2 = stack.pop(len(stack) - 1)
        operand1 = stack.pop(len(stack) - 1)
        operand = translateArithmetic(module, pou_name, operator, operand1, operand2)
      elif operator in ["logical_and", "logical_or", "logical_xor"]:
        operand2 = stack.pop(len(stack) - 1)
        operand1 = stack.pop(len(stack) - 1)
        operand = translateLogical(module, pou_name, operator, operand1, operand2)
      elif operator == "logical_not":
        operand1 = stack.pop(len(stack) - 1)
        operand = translateLogical(module, pou_name, operator, operand1, None)
      elif operator in comparison_operators:
        operand2 = stack.pop(len(stack) - 1)
        operand1 = stack.pop(len(stack) - 1)
        operand = translateComparison(module, pou_name, operator, operand1, operand2)
      # elif operator in shift_operators: # TODO: handle shift operators
      else:
        print "TODO: unknown operator type: " + operator

    stack.append(operand)

  return stack[0]

# translate an action, return binaryen_ref
def translateAction(module, pou_name, ast_node):
  return None

# translate case..of statements, return binaryen_ref
def translateCaseOf(module, pou_name, ast_node):
  return None

# translate while..do statements, return binaryen_ref
def translateWhileDo(module, pou_name, ast_node):
  return None

# translate repeat..until statements, return binaryen_ref
def translateRepeatUntil(module, pou_name, ast_node):
  return None

# translate shift statement, return Operand
def translateShift(module, pou_name, operator, operand1, operand2):
  return None

# translate statements, return binaryen_ref
def translateStatements(module, pou_name, ast_node):
  statements = []
  ast_statements = ast_node[0][1]
  for i in range(0, len(ast_statements)):
    ast_statement = ast_statements[i]

    if (ast_statement[0] == "assignment_statement"):
      ref = translateAssignment(module, pou_name, ast_statement[1])
    elif (ast_statement[0] == "if_statement"):
      ref = translateIfThen(module, pou_name, ast_statement[1])
    elif (ast_statement[0] == "for_statement"):
      ref = translateForLoop(module, pou_name, ast_statement[1])
    elif (ast_statement[0] == "while_statement"):
      ref = translateWhileDo(module, pou_name, ast_statement[1])
    elif (ast_statement[0] == "repeat_statement"):
      ref = translateRepeatUntil(module, pou_name, ast_statement[1])
    elif (ast_statement[0] == "case_statement"):
      ref = translateCaseOf(module, pou_name, ast_statement[1])
    else:
      print "TODO: unknown statement type: " + ast_statement[0]

    statements.append(ref)

  block_children = (c_void_p * len(statements))()
  for i in range(0, len(statements)):
    block_children[i] = statements[i]

  block_ref = lib_bny.BinaryenBlock(module, None, block_children,
                                    c_int(len(statements)),
                                    lib_bny.BinaryenTypeNone())
  return block_ref

def translateFunc(module, func_name):
  ast_node = function_map[func_name]
  for i in range(0, len(ast_node)):
    if (ast_node[i][0] == "function_body"):
      return translateStatements(module, func_name, ast_node[i][1])
  print "Fail to translate function " + func_name
  # TODO: throw exception

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

def translateInit():
  global lib_bny
  global datatype_map

  #lib_bny.BinaryenSetAPITracing(c_int(0));

  lib_bny.BinaryenLiteralInt32.restype = BinaryenLiteral
  lib_bny.BinaryenLiteralInt64.restype = BinaryenLiteral
  lib_bny.BinaryenLiteralFloat32.restype = BinaryenLiteral
  lib_bny.BinaryenLiteralFloat64.restype = BinaryenLiteral
  lib_bny.BinaryenLiteralVec128.restype = BinaryenLiteral
  lib_bny.BinaryenLiteralFloat32Bits.restype = BinaryenLiteral
  lib_bny.BinaryenLiteralFloat64Bits.restype = BinaryenLiteral

def translate():
  global lib_bny
  global datatype_map
  global basic_types

  translateInit()

  # Create Binaryen Moduel
  module = lib_bny.BinaryenModuleCreate()

  # Create Memory
  lib_bny.BinaryenAddMemoryImport(module, b"memory", b"env", b"memory", c_ubyte(0))
  lib_bny.BinaryenSetMemory(module, c_int(1), c_int(1), None, None, None, None, c_int(0), c_ubyte(0))

  translateFuncs(module)
  translateFuncBlocks(module)

  lib_bny.BinaryenModulePrint(module)

