require'check_lua_version'(5,3);

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

regist_field('ctxid_');
regist_field('name_');
regist_field('valid_');

require'match_adl'

local mt = {};

local m = {
  service_id,
};

m.service_id = function()
  local obj = {
    valid_ = 0,
    ctxid_ = ns.gce_adl.match(),
    name_ = ns.gce_adl.match(),
  };
  setmetatable(obj,mt[1]);
  return obj;
end

local layout_gce_adl_service_id = regist_layout_c(
  '\3\0\20\6\118\97\108\105\100\95\9\0\0\0\6\99\116\120\105\100\95\22\0\0\0\5\110\97\109\101\95\22\0\0\0',
  {fields.valid_,fields.ctxid_,fields.name_,},
  {layouts.gce_adl_match,layouts.gce_adl_match,});
layouts.gce_adl_service_id = layout_gce_adl_service_id

local mc = {};
mc = {
  adtype = function(o) return m.service_id end,
  skip_read = function(o,buf) return skip_read_c( field_list , mt_type_list , buf , layout_gce_adl_service_id , o) end,
  size_of = function(o) return size_of_c( field_list , mt_type_list , layout_gce_adl_service_id , o) end,
  read = function(o,buf) return read_c( field_list , mt_type_list , buf , layout_gce_adl_service_id , o) end,
  write = function(o,buf) return write_c( field_list , mt_type_list , buf , layout_gce_adl_service_id , o) end,
};
mc.__index = mc;
mt[1] = mc;
if ns.gce_adl == nil then
  ns.gce_adl = m;
else
  ns.gce_adl.service_id = m.service_id
end
set_layout_mt_c( layout_gce_adl_service_id , regist_mt_type(mt[1]) , 'ad_mt_gce_adl.service_id' , mt[1]);

return m;
