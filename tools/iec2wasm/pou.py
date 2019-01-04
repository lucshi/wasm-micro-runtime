import ctypes
from ctypes import *
from global_vars import *

# return the POU type, e.g. "function", "function block", "program"
def getPOUType(pou_name):
  global function_map
  if function_map.has_key(pou_name) == True:
    return "function"
  elif function_block_map.has_key(pou_name) == True:
    return "function_block"
  elif program_map.has_key(pou_name) == True:
    return "program"
  return None

# return the POU return type
def getPOURetType(pou_name):
  if getPOUType(pou_name) != "function":
    return None
  return function_map[pou_name][1][1][0][0]

# return the POU variables, it is a list of var names, the element order of
# the list is: VAR_INPUT, VAR_OUTPUT, VAR_INOUT, VAR, VAR_TEMP, VAR_EXTERNAL
def getPOUVarsAll(pou_name):
  vars = []
  if getPOUType(pou_name) == "function":
      func_vars = var_map[pou_name]
      for var_type in func_vars:
        variable_map = func_vars[var_type]
        for var_name in variable_map:
          vars.append(var_name)
  elif getPOUType(pou_name) == "function_block":
      fb_vars = var_map[pou_name]
      for var_type in fb_vars:
        variable_map = fb_vars[var_type]
        for var_name in variable_map:
          vars.append(var_name)
  elif getPOUType(pou_name) == "program":
      pro_vars = var_map[pou_name]
      for var_type in pro_vars:
        variable_map = pro_vars[var_type]
        for var_name in variable_map:
          vars.append(var_name)
  return vars

def getPOUVarInputs(pou_name):
  var_input = []
  if getPOUType(pou_name) == "function":
      func_vars = var_map[pou_name]
      if func_vars.has_key("var_input") == True:
        for var_name in func_vars["var_input"]:
          var_input.append(var_name)
  elif getPOUType(pou_name) == "function_block":
      fb_vars = var_map[pou_name]
      if fb_vars.has_key("var_input") == True:
        for var_name in fb_vars["var_input"]:
          var_input.append(var_name)
  elif getPOUType(pou_name) == "program":
      pro_vars = var_map[pou_name]
      if pro_vars.has_key("var_input") == True:
        for var_name in pro_vars["var_input"]:
          var_input.append(var_name)
  return var_input

def getPOUVarOutputs(pou_name):
  var_output = []
  if getPOUType(pou_name) == "function":
      func_vars = var_map[pou_name]
      if func_vars.has_key("var_output") == True:
        for var_name in func_vars["var_output"]:
          var_output.append(var_name)
  elif getPOUType(pou_name) == "function_block":
      fb_vars = var_map[pou_name]
      if fb_vars.has_key("var_output") == True:
        for var_name in fb_vars["var_output"]:
          var_output.append(var_name)
  elif getPOUType(pou_name) == "program":
      pro_vars = var_map[pou_name]
      if pro_vars.has_key("var_output") == True:
        for var_name in pro_vars["var_output"]:
          var_output.append(var_name)
  return var_output

def getPOUVarInOuts(pou_name):
  var_inout = []
  if getPOUType(pou_name) == "function":
      func_vars = var_map[pou_name]
      if func_vars.has_key("var_inout") == True:
        for var_name in func_vars["var_inout"]:
          var_inout.append(var_name)
  elif getPOUType(pou_name) == "function_block":
      fb_vars = var_map[pou_name]
      if fb_vars.has_key("var_inout") == True:
        for var_name in fb_vars["var_inout"]:
          var_inout.append(var_name)
  elif getPOUType(pou_name) == "program":
      pro_vars = var_map[pou_name]
      if pro_vars.has_key("var_inout") == True:
        for var_name in pro_vars["var_inout"]:
          var_inout.append(var_name)
  return var_inout

def getPOUVars(pou_name):
  var_local = []
  if getPOUType(pou_name) == "function":
      func_vars = var_map[pou_name]
      if func_vars.has_key("var") == True:
        for var_name in func_vars["var_local"]:
          var_local.append(var_name)
  elif getPOUType(pou_name) == "function_block":
      fb_vars = var_map[pou_name]
      if fb_vars.has_key("var") == True:
        for var_name in fb_vars["var_local"]:
          var_local.append(var_name)
  elif getPOUType(pou_name) == "program":
      pro_vars = var_map[pou_name]
      if pro_vars.has_key("var") == True:
        for var_name in pro_vars["var_local"]:
          var_local.append(var_name)
  return var_local

def getPOUVarTemps(pou_name):
  var_temp = []
  if getPOUType(pou_name) == "function":
      func_vars = var_map[pou_name]
      if func_vars.has_key("var_temp") == True:
        for var_name in func_vars["var_temp"]:
          var_temp.append(var_name)
  elif getPOUType(pou_name) == "function_block":
      fb_vars = var_map[pou_name]
      if fb_vars.has_key("var_temp") == True:
        for var_name in fb_vars["var_temp"]:
          var_temp.append(var_name)
  elif getPOUType(pou_name) == "program":
      pro_vars = var_map[pou_name]
      if pro_vars.has_key("var_temp") == True:
        for var_name in pro_vars["var_temp"]:
          var_temp.append(var_name)
  return var_temp

def getPOUVarExternals(pou_name):
  var_external = []
  if getPOUType(pou_name) == "function":
      func_vars = var_map[pou_name]
      if func_vars.has_key("var_external") == True:
        for var_name in func_vars["var_external"]:
          var_external.append(var_name)
  elif getPOUType(pou_name) == "function_block":
      fb_vars = var_map[pou_name]
      if fb_vars.has_key("var_external") == True:
        for var_name in fb_vars["var_external"]:
          var_external.append(var_name)
  elif getPOUType(pou_name) == "program":
      pro_vars = var_map[pou_name]
      if pro_vars.has_key("var_external") == True:
        for var_name in pro_vars["var_external"]:
          var_external.append(var_name)
  return var_external

# return the POU variable class, e.g. "var_input", "var_output", "var"
def getPOUVarClass(pou_name, var_name):
  if var_name in getPOUVarInputs(pou_name):
    return "var_input"
  elif var_name in getPOUVarOutputs(pou_name):
    return "var_output"
  elif var_name in getPOUVarInOuts(pou_name):
    return "var_inout"
  elif var_name in getPOUVars(pou_name):
    return "var"
  elif var_name in getPOUVarTemps(pou_name):
    return "var_temp"
  elif var_name in getPOUVarExternals(pou_name):
    return "var_external"
  return None

# return the POU variable AST node
def getPOUVarNode(pou_name, var_name):
  global var_map
  if getPOUType(pou_name) == "function":
      func_vars = var_map[pou_name]
      for var_type in func_vars:
        variable_map = func_vars[var_type]
        if variable_map.has_key(var_name) == True:
          return variable_map[var_name]
  elif getPOUType(pou_name) == "function_block":
      fb_vars = var_map[pou_name]
      for var_type in fb_vars:
        variable_map = fb_vars[var_type]
        if variable_map.has_key(var_name) == True:
          return variable_map[var_name]
  elif getPOUType(pou_name) == "program":
      pro_vars = var_map[pou_name]
      for var_type in pro_vars:
        variable_map = pro_vars[var_type]
        if variable_map.has_key(var_name) == True:
          return variable_map[var_name]
  return None


# return the POU variable type, e.g. "type-int", "type-real", or user defined types
def getPOUVarType(pou_name, var_name):
  var_node = getPOUVarNode(pou_name, var_name)
  type = None
  if var_node is not None:
    type = var_node[1][0][1][0][0]
  return type

# return the POU variable option, e.g. "retain", "non-retain", "const", ""
def getPOUVarOption(pou_name, var_name):
  pou_node = None
  option = None
  if function_map.has_key(pou_name) == True:
    pou_node = function_map[pou_name]
    for i in range (2, len(pou_node)):
      if pou_node[i][0] != "function_body":
        if pou_node[i][1][0][0] != "var_init_decl":
          option = pou_node[i][1][0][0]
          for j in range(0, len(pou_node[i][1]) - 1):
              if pou_node[i][1][1][1][0][1][0] == var_name:
                return option
        else:
          for j in range(0, len(pou_node[i][1])):
              if pou_node[i][1][j][1][0][1][0] == var_name:
                return option
  elif function_block_map.has_key(pou_name) == True:
    pou_node = function_block_map[pou_name]
    for i in range (1, len(pou_node)):
      if pou_node[i][0] != "function_block_body":
        if pou_node[i][1][0][0] != "var_init_decl":
          option = pou_node[i][1][0][0]
          for j in range(0, len(pou_node[i][1]) - 1):
              if pou_node[i][1][1][1][0][1][0] == var_name:
                return option
        else:
          for j in range(0, len(pou_node[i][1])):
              if pou_node[i][1][j][1][0][1][0] == var_name:
                return option
  elif program_map.has_key(pou_name) == True:
    pou_node = program_map[pou_name]
    for i in range (1, len(pou_node)):
      if pou_node[i][0] != "program_body":
        if pou_node[i][1][0][0] != "var_init_decl":
          option = pou_node[i][1][0][0]
          for j in range(0, len(pou_node[i][1]) - 1):
              if pou_node[i][1][1][1][0][1][0] == var_name:
                return option
        else:
          for j in range(0, len(pou_node[i][1])):
              if pou_node[i][1][j][1][0][1][0] == var_name:
                return option
  return option

# return the POU variable initial expression node
def getPOUVarInitValue(pou_name, var_name):
  var_node = getPOUVarNode(pou_name, var_name)
  var_init_node = None
  if var_node is not None:
    var_init_node = var_node[1][1]
  return var_init_node

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

def isBasicType(type_name):
  global basic_types
  return type_name in basic_types

def initWasmtypeMap():
  global wasm_type_map
  global lib_bny
  wasm_type_map["type_bool"] = lib_bny.BinaryenTypeInt32()
  wasm_type_map["type_int"] = lib_bny.BinaryenTypeInt32()
  wasm_type_map["type_dint"] = lib_bny.BinaryenTypeInt32()
  wasm_type_map["type_real"] = lib_bny.BinaryenTypeFloat32()
  wasm_type_map["type_lreal"] = lib_bny.BinaryenTypeFloat64()

def basictype2Wasmtype(type_name):
  return wasm_types_map[type_name];


class Operand:
  def __init__(self, bny_ref, iec_type, bny_type):
    self.bny_ref = bny_ref
    self.iec_type = iec_type
    self.bny_type = bny_type

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

# translate if statements, return binaryen_ref
def translateIfThen(module, pou_name, ast_node):
  return None

# translate for statements, return binaryen_ref
def translateForLoop(module, pou_name, ast_node):
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

# translate function call statements, return Operand
def translateFunctionCall(module, pou_name, ast_node):
  # TODO: handle function paramenters, and check data types
  func_name = ast_node[1][0][1][0]
  bny_ref = lib_bny.BinaryenCall(module, func_name, None, c_int(0),
                                 lib_bny.BinaryenTypeInt32())
  return Operand(bny_ref, "type_int", "i32")

# translate data conversion statement, return Operand
def translateDataConversion(module, pou_name, ast_node):
  return None

# translate data conversion to dest iec type, return Operand
def translateConvertTo(module, pou_name, dst_iec_type, operand):
  return None

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

# translate shift statement, return Operand
def translateShift(module, pou_name, operator, operand1, operand2):
  return None


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
      ref = translateIfThen(module, pou_name, ast_statement[1])
    elif (ast_statement[0] == "while_statement"):
      ref = translateIfThen(module, pou_name, ast_statement[1])
    elif (ast_statement[0] == "repeat_statement"):
      ref = translateIfThen(module, pou_name, ast_statement[1])
    elif (ast_statement[0] == "case_statement"):
      ref = translateIfThen(module, pou_name, ast_statement[1])
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

  # Load Binaryen Library
  ll = ctypes.cdll.LoadLibrary
  lib_bny = ll("./libbinaryen.so")
  #lib_bny.BinaryenSetAPITracing(c_int(0));

  lib_bny.BinaryenLiteralInt32.restype = BinaryenLiteral
  lib_bny.BinaryenLiteralInt64.restype = BinaryenLiteral
  lib_bny.BinaryenLiteralFloat32.restype = BinaryenLiteral
  lib_bny.BinaryenLiteralFloat64.restype = BinaryenLiteral
  lib_bny.BinaryenLiteralVec128.restype = BinaryenLiteral
  lib_bny.BinaryenLiteralFloat32Bits.restype = BinaryenLiteral
  lib_bny.BinaryenLiteralFloat64Bits.restype = BinaryenLiteral

  initWasmtypeMap()


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

