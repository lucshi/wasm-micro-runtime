# return the declaration type of data type, e.g.
# "enumerated_type_declaration", "simple_type_declaration", "structure_type_declaration"
def getDatatypeDeclarationType(ast_node):
  return ""

# return the enumerated values of the enum data type, it is a list, e.g.
# ["ENUM_VALUE1", "ENUM_VALUE2"]
def getEnumDatatypeEnumValues(ast_node):
  return ""

# return the int values of the enum data type, it is a list, e.g. ["0", "1"]
def getEnumDatatypeIntValues(ast_node):
  return ""

# return the initial enumerated value of the enum data type, e.g. "ENUM_VALUE1"
def getEnumDatatypeInitEnumValue(ast_node):
  return ""

# return the initial int value of the enum data type, e.g. "0"
def getEnumDatatypeInitIntValue(ast_node):
  return ""

# return the related int value of the enum value, e.g.
# the enum vlaue is "ENUM_VALUE1", the related int value is 0.
def getEnumDatatypeIntValueOf(ast_node, enum_value):
  return ""

# return the type of the directly data type, e.g. "type_int", "type_real"
def getDirectlyDatatypeType(ast_node):
  return ""

# return the initial value of the directly data type
def getDirectlyDatatypeInitValue(ast_node):
  return ""

# return the number of the structure fields
def getStructDatatypeFieldNum(ast_node):
  return 0

# return the field name of field index
def getStructDatatypeFieldName(ast_node, field_index):
  return ""

# return the field type of field index, e.g. "type_real"
def getStructDatatypeFieldType(ast_node, field_index):
  return ""

# return the field offset of field index
def getStructDatatypeFieldOffset(ast_node, field_index):
  return ""

# return the field type of field name
def getStructDatatypeFieldType_byName(ast_node, field_name):
  return ""

# return the field offset of field name
def getStructDatatypeFieldOffset_byName(ast_node, field_name):
  return 0

# return the total size of the structure
def getStructDatatypeTotalSize(ast_node):
  return 0

