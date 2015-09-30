require'check_lua_version'(5,1);

local adata_m = require'adata_core'
local int64 = require'int64'
local detail = require'adata_detail';
local ns = require'adata';
local regist_field = detail.regist_field;
local fields = detail.fields;
local field_list = detail.field_list;
local layouts = detail.layouts;
local regist_mt_type = detail.regist_mt_type;
local mt_type_list = detail.mt_type_list;
local read_c = adata_m.read;
local skip_read_c = adata_m.skip_read;
local size_of_c = adata_m.size_of;
local write_c = adata_m.write;
local regist_layout_c = adata_m.regist_layout;
local set_layout_mt_c = adata_m.set_layout_mt;

regist_field('ptr_');


local mt = {};

local m = {
  context_id,
};

m.context_id = function()
  local obj = {
    ptr_ = 0,
  };
  setmetatable(obj,mt[1]);
  return obj;
end

local layout_gce_mysql_adl_context_id = regist_layout_c(
  '\1\0\5\4\112\116\114\95\16\0\0\0',
  {fields.ptr_,},
  {});
layouts.gce_mysql_adl_context_id = layout_gce_mysql_adl_context_id

local mc = {};
mc = {
  adtype = function(o) return m.context_id end,
  skip_read = function(o,buf) return skip_read_c( field_list , mt_type_list , buf , layout_gce_mysql_adl_context_id , o) end,
  size_of = function(o) return size_of_c( field_list , mt_type_list , layout_gce_mysql_adl_context_id , o) end,
  read = function(o,buf) return read_c( field_list , mt_type_list , buf , layout_gce_mysql_adl_context_id , o) end,
  write = function(o,buf) return write_c( field_list , mt_type_list , buf , layout_gce_mysql_adl_context_id , o) end,
};
mc.__index = mc;
mt[1] = mc;
if ns.gce_mysql_adl == nil then
  ns.gce_mysql_adl = m;
else
  ns.gce_mysql_adl.context_id = m.context_id
end
set_layout_mt_c( layout_gce_mysql_adl_context_id , regist_mt_type(mt[1]) , 'ad_mt_gce_mysql_adl.context_id' , mt[1]);

return m;
