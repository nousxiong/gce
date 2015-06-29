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

regist_field('bigmsg_size');
regist_field('idle_period');

require'duration_adl'

local mt = {};

local m = {
  sn_option,
};

m.sn_option = function()
  local obj = {
    idle_period = ns.gce_adl.duration(),
    bigmsg_size = 0,
  };
  setmetatable(obj,mt[1]);
  return obj;
end

local layout_gce_asio_adl_sn_option = regist_layout_c(
  '\2\0\24\11\105\100\108\101\95\112\101\114\105\111\100\22\0\0\0\11\98\105\103\109\115\103\95\115\105\122\101\13\0\0\0',
  {fields.idle_period,fields.bigmsg_size,},
  {layouts.gce_adl_duration,});
layouts.gce_asio_adl_sn_option = layout_gce_asio_adl_sn_option

local mc = {};
mc = {
  adtype = function(o) return m.sn_option end,
  skip_read = function(o,buf) return skip_read_c( field_list , mt_type_list , buf , layout_gce_asio_adl_sn_option , o) end,
  size_of = function(o) return size_of_c( field_list , mt_type_list , layout_gce_asio_adl_sn_option , o) end,
  read = function(o,buf) return read_c( field_list , mt_type_list , buf , layout_gce_asio_adl_sn_option , o) end,
  write = function(o,buf) return write_c( field_list , mt_type_list , buf , layout_gce_asio_adl_sn_option , o) end,
};
mc.__index = mc;
mt[1] = mc;
if ns.gce_asio_adl == nil then
  ns.gce_asio_adl = m;
else
  ns.gce_asio_adl.sn_option = m.sn_option
end
set_layout_mt_c( layout_gce_asio_adl_sn_option , regist_mt_type(mt[1]) , 'ad_mt_gce_asio_adl.sn_option' , mt[1]);

return m;
