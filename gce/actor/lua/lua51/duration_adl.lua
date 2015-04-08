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

regist_field('ty_');
regist_field('val_');


local mt = {};

local m = {
  duration,
};

m.duration = function()
  local obj = {
    val_ = 0,
    ty_ = 0,
  };
  setmetatable(obj,mt[1]);
  return obj;
end

local layout_gce_adl_duration = regist_layout_c(
  '\2\0\9\4\118\97\108\95\15\0\0\0\3\116\121\95\9\0\0\0',
  {fields.val_,fields.ty_,},
  {});
layouts.gce_adl_duration = layout_gce_adl_duration

local mc = {};
mc = {
  adtype = function(o) return m.duration end,
  skip_read = function(o,buf) return skip_read_c( field_list , mt_type_list , buf , layout_gce_adl_duration , o) end,
  size_of = function(o) return size_of_c( field_list , mt_type_list , layout_gce_adl_duration , o) end,
  read = function(o,buf) return read_c( field_list , mt_type_list , buf , layout_gce_adl_duration , o) end,
  write = function(o,buf) return write_c( field_list , mt_type_list , buf , layout_gce_adl_duration , o) end,
};
mc.__index = mc;
mt[1] = mc;
if ns.gce_adl == nil then
  ns.gce_adl = m;
else
  ns.gce_adl.duration = m.duration
end
set_layout_mt_c( layout_gce_adl_duration , regist_mt_type(mt[1]) , 'ad_mt_gce_adl.duration' , mt[1]);

return m;
