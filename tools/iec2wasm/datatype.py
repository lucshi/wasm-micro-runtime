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
def getDirectlyDatatypeType(directly_name):
  return datatype_map[directly_name][0][1][1][1][0][1][0][0]

# return the initial value of the directly data type
def getDirectlyDatatypeInitValue(directly_name):
  if (len(datatype_map[directly_name][0][1][1][1]) > 1):
   return datatype_map[directly_name][0][1][1][1][1][1][0][1][0]
  else:
   directly_type = getDirectlyDatatypeType(directly_name)
   return basic_types_init_value_map[directly_type]

# return the number of the structure fields
def getStructDatatypeFieldNum(struct_name):
  return (len(datatype_map[struct_name][0][1]) - 1)

# return the field name of field index
def getStructDatatypeFieldName(struct_name, field_index):
  return datatype_map[struct_name][0][1][field_index + 1][1][0][1][0]

def getStructDatatypeFieldIndex(struct_name, field_name):
  field_num = getStructDatatypeFieldNum(struct_name)
  for i in range(0, field_num):
    if (getStructDatatypeFieldName(struct_name, i) == field_name):
      return i
  return -1

def getStructDatatypeFieldTypeInfo(struct_name, field_index):
  return datatype_map[struct_name][0][1][field_index + 1][1][1][1][0]

# return the field type of field index, e.g. "type_real"
def getStructDatatypeFieldType(struct_name, field_index):
  field_type_info = getStructDatatypeFieldTypeInfo(struct_name, field_index)
  if (field_type_info[0] != "simple_type_name"):
    return field_type_info[1][0][0]
  else:
   return field_type_info[1][0]

# return the field offset of field index
def getStructDatatypeFieldOffset(struct_name, field_index):
  offset = 0
  for i in range (0, field_index):
    field_size = 0
    field_type = getStructDatatypeFieldType(struct_name, i)
    if field_type in basic_types:
      field_size = basic_types_size_map[field_type]
    else:
      field_type_dec_type = getDatatypeDeclarationType(field_type)
      if (field_type_dec_type == "enumerated_type_declaration"):
        field_size = basic_types_size_map["type_int"]
      elif (field_type_dec_type == "simple_type_declaration"):
        directly_type = getDirectlyDatatypeType(field_type)
        field_size = basic_types_size_map[directly_type]
      elif (field_type_dec_type == "structure_type_declaration"):
        field_size = getStructDatatypeTotalSize(field_type)
      else:
        print "TODO: unknown field declaration type " + field_type_dec_type
    offset = offset + field_size
  return offset

# return the field type of field name
def getStructDatatypeFieldType_byName(struct_name, field_name):
  field_index = getStructDatatypeFieldIndex(struct_name, field_name)
  if (field_index >= 0):
    return getStructDatatypeFieldType(struct_name, field_index);
  return ""

# return the field offset of field name
def getStructDatatypeFieldOffset_byName(struct_name, field_name):
  field_index = getStructDatatypeFieldIndex(struct_name, field_name)
  if (field_index >= 0):
    return getStructDatatypeFieldOffset(struct_name, field_index);
  return -1

# return the total size of the structure
def getStructDatatypeTotalSize(struct_name):
  field_num = getStructDatatypeFieldNum(struct_name);
  return getStructDatatypeFieldOffset(struct_name, field_num)
