local field_count = 0;
local fields = {}
local field_list = {}
local layouts = {}
local mt_type_count = 0;
local mt_types = {}
local mt_type_list = {}

return {
  fields = fields,
  field_list = field_list,
  layouts = layouts,
  mt_type_list = mt_type_list,
  regist_field = function(field)
    if fields[field] == nil then
      field_count = field_count + 1;
      fields[field] = field_count;
      field_list[field_count] = field;
    end
  end,
  regist_mt_type = function(type)
    local ret = mt_types[type];
    if ret == nil then
      mt_type_count = mt_type_count + 1;
      mt_types[type] = mt_type_count;
      mt_type_list[mt_type_count] = type;
      ret = mt_type_count;
    end
    return ret;
  end
}
