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

regist_field('baud_rate');
regist_field('character_size');
regist_field('flow_control');
regist_field('parity');
regist_field('stop_bits');


local mt = {};

local m = {
  spt_option,
};

m.spt_option = function()
  local obj = {
    baud_rate = -1,
    flow_control = -1,
    parity = -1,
    stop_bits = -1,
    character_size = -1,
  };
  setmetatable(obj,mt[1]);
  return obj;
end

local layout_gce_asio_adl_spt_option = regist_layout_c(
  '\5\0\55\9\98\97\117\100\95\114\97\116\101\13\0\0\0\12\102\108\111\119\95\99\111\110\116\114\111\108\13\0\0\0\6\112\97\114\105\116\121\13\0\0\0\9\115\116\111\112\95\98\105\116\115\13\0\0\0\14\99\104\97\114\97\99\116\101\114\95\115\105\122\101\13\0\0\0',
  {fields.baud_rate,fields.flow_control,fields.parity,fields.stop_bits,fields.character_size,},
  {});
layouts.gce_asio_adl_spt_option = layout_gce_asio_adl_spt_option

local mc = {};
mc = {
  adtype = function(o) return m.spt_option end,
  skip_read = function(o,buf) return skip_read_c( field_list , mt_type_list , buf , layout_gce_asio_adl_spt_option , o) end,
  size_of = function(o) return size_of_c( field_list , mt_type_list , layout_gce_asio_adl_spt_option , o) end,
  read = function(o,buf) return read_c( field_list , mt_type_list , buf , layout_gce_asio_adl_spt_option , o) end,
  write = function(o,buf) return write_c( field_list , mt_type_list , buf , layout_gce_asio_adl_spt_option , o) end,
};
mc.__index = mc;
mt[1] = mc;
if ns.gce_asio_adl == nil then
  ns.gce_asio_adl = m;
else
  ns.gce_asio_adl.spt_option = m.spt_option
end
set_layout_mt_c( layout_gce_asio_adl_spt_option , regist_mt_type(mt[1]) , 'ad_mt_gce_asio_adl.spt_option' , mt[1]);

return m;
