require'check_lua_version'(5,2);

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

regist_field('list_');


local mt = {};

local m = {
  endpoint_list,
};

m.endpoint_list = function()
  local obj = {
    list_ = {},
  };
  setmetatable(obj,mt[1]);
  return obj;
end

local layout_cluster5_endpoint_list = regist_layout_c(
  '\1\1\6\5\108\105\115\116\95\20\0\0\1\19\0',
  {fields.list_,},
  {});
layouts.cluster5_endpoint_list = layout_cluster5_endpoint_list

local mc = {};
mc = {
  adtype = function(o) return m.endpoint_list end,
  skip_read = function(o,buf) return skip_read_c( field_list , mt_type_list , buf , layout_cluster5_endpoint_list , o) end,
  size_of = function(o) return size_of_c( field_list , mt_type_list , layout_cluster5_endpoint_list , o) end,
  read = function(o,buf) return read_c( field_list , mt_type_list , buf , layout_cluster5_endpoint_list , o) end,
  write = function(o,buf) return write_c( field_list , mt_type_list , buf , layout_cluster5_endpoint_list , o) end,
};
mc.__index = mc;
mt[1] = mc;
if ns.cluster5 == nil then
  ns.cluster5 = m;
else
  ns.cluster5.endpoint_list = m.endpoint_list
end
set_layout_mt_c( layout_cluster5_endpoint_list , regist_mt_type(mt[1]) , 'ad_mt_cluster5.endpoint_list' , mt[1]);

return m;
