from global_vars import *

# return the declaration type of data type, e.g.
# "enumerated_type_declaration", "simple_type_declaration", "structure_type_declaration"
def getDatatypeDeclarationType(datatype_name):
  return datatype_map[datatype_name][0][0]

# return the enumerated values of the enum data type, it is a list, e.g.
# ["ENUM_VALUE1", "ENUM_VALUE2"]
def getEnumDatatypeEnumValues(enum_name):
  node = []
  length = len(datatype_map[enum_name][0][1][1][1][0][1])
  for i in range(0, length):
    node.append(datatype_map[enum_name][0][1][1][1][0][1][i][1][0])
  return node

# return the int values of the enum data type, it is a list, e.g. ["0", "1"]
def getEnumDatatypeIntValues(enum_name):
  temp = 0
  node = []
  length = len(datatype_map[enum_name][0][1][1][1][0][1])
  for i in range(0, length):
    if len(datatype_map[enum_name][0][1][1][1][0][1][i][1]) == 1:
      node.append(temp)
      temp = temp + 1
    elif len(datatype_map[enum_name][0][1][1][1][0][1][i][1]) == 2:
      temp = int(datatype_map[enum_name][0][1][1][1][0][1][i][1][1][1][0])
      node.append(temp)
      temp = temp + 1
  return node

# return the initial enumerated value of the enum data type, e.g. "ENUM_VALUE1"
def getEnumDatatypeInitEnumValue(enum_name):
  return datatype_map[enum_name][0][1][1][1][0][1][0][1][0]

# return the initial int value of the enum data type, e.g. "0"
def getEnumDatatypeInitIntValue(enum_name):
  return datatype_map[enum_name][0][1][1][1][1][1][0]

# return the related int value of the enum value, e.g.
# the enum value is "ENUM_VALUE1", the related int value is 0.
def getEnumDatatypeIntValueOf(enum_name, enum_value):
  temp = 0
  node = []
  length = len(datatype_map[enum_name][0][1][1][1][0][1])
  for i in range(0, length):
    if len(datatype_map[enum_name][0][1][1][1][0][1][i][1]) == 1:
      node.append(temp)
      temp = temp + 1
    elif len(datatype_map[enum_name][0][1][1][1][0][1][i][1]) == 2:
      temp = int(datatype_map[enum_name][0][1][1][1][0][1][i][1][1][1][0])
      node.append(temp)
      temp = temp + 1
  for j in range(0, length):
    if datatype_map[enum_name][0][1][1][1][0][1][j][1][0] == enum_value:
      return node[j]

# return the type of the directly data type, e.g. "type_int", "type_real"
def getDirectlyDatatypeType(pou_name, in_out, var_name):
  return var_map[pou_name][in_out][var_name][1][0][1][0][0]

# return the initial value of the directly data type
def getDirectlyDatatypeInitValue(pou_name, in_out, var_name):
  if (len(var_map[pou_name][in_out][var_name][1]) > 1) and (var_map[pou_name][in_out][var_name][1][1][0] == "expression"):
    return var_map[pou_name][in_out][var_name][1][1][1][0][1][0]
  else:
    if var_map[pou_name][in_out][var_name][1][0][1][0][0] == "type_int":
      return 0
    elif var_map[pou_name][in_out][var_name][1][0][1][0][0] == "type_dint":
      return 0
    elif var_map[pou_name][in_out][var_name][1][0][1][0][0] == "type_real":
      return 0.0
    elif var_map[pou_name][in_out][var_name][1][0][1][0][0] == "type_bool":
      return FALSE
    elif var_map[pou_name][in_out][var_name][1][0][1][0][0] == "type_datetime":
      return "DT#1970-01-01-00:00:00"
    else:
      print("unsupported type")

# return the number of the structure fields
def getStructDatatypeFieldNum(struct_name):
  return (len(datatype_map[struct_name][0][1]) - 1)

# return the field name of field index
def getStructDatatypeFieldName(struct_name, field_index):
  return datatype_map[struct_name][0][1][field_index + 1][1][0][1][0]

# return the field type of field index, e.g. "type_real"
def getStructDatatypeFieldType(struct_name, field_index):
  return datatype_map[struct_name][0][1][field_index + 1][1][1][1][0][1][0][0]

# return the field offset of field index
def getStructDatatypeFieldOffset(struct_name, field_index):
  offset = 0
  for i in range (0, field_index):
    field_type = datatype_map[struct_name][0][1][field_index + 1][1][1][1][0][1][0][0]
    if field_type == "type_int":
      field_size = 16
    offset = offset + field_size
  return offset

# return the field type of field name
def getStructDatatypeFieldType_byName(struct_name, field_name):
  for i in range (0, len(datatype_map[struct_name][0][1]) - 1):
    if datatype_map[struct_name][0][1][i + 1][1][0][1][0] == field_name:
      return datatype_map[struct_name][0][1][i + 1][1][1][1][0][1][0][0]

# return the field offset of field name
def getStructDatatypeFieldOffset_byName(struct_name, field_name):
  offset = 0
  for i in range (0, len(datatype_map[struct_name][0][1]) - 1):
    if datatype_map[struct_name][0][1][i + 1][1][0][1][0] == field_name:
      return offset
    field_type = datatype_map[struct_name][0][1][i + 1][1][1][1][0][1][0][0]
    if field_type == "type_int":
      field_size = 16
    offset = offset + field_size

# return the total size of the structure
def getStructDatatypeTotalSize(struct_name):
  total_size = 0
  for i in range (0, len(datatype_map[struct_name][0][1]) - 1):
    field_type = datatype_map[struct_name][0][1][i + 1][1][1][1][0][1][0][0]
    if field_type == "type_int":
      field_size = 16
    total_size = total_size + field_size
  return total_size
