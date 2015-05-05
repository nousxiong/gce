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

regist_field('heartbeat_count');
regist_field('heartbeat_period');
regist_field('init_reconn_period');
regist_field('init_reconn_try');
regist_field('is_router');
regist_field('rebind_period');
regist_field('rebind_try');
regist_field('reconn_period');
regist_field('reconn_try');

require'duration_adl'

local mt = {};

local m = {
  net_option,
};

m.net_option = function()
  local obj = {
    is_router = 0,
    heartbeat_period = ns.gce_adl.duration(),
    heartbeat_count = 0,
    init_reconn_period = ns.gce_adl.duration(),
    init_reconn_try = 0,
    reconn_period = ns.gce_adl.duration(),
    reconn_try = 0,
    rebind_period = ns.gce_adl.duration(),
    rebind_try = 0,
  };
  setmetatable(obj,mt[1]);
  return obj;
end

local layout_gce_adl_net_option = regist_layout_c(
  '\9\0\128\128\9\105\115\95\114\111\117\116\101\114\9\0\0\0\16\104\101\97\114\116\98\101\97\116\95\112\101\114\105\111\100\22\0\0\0\15\104\101\97\114\116\98\101\97\116\95\99\111\117\110\116\13\0\0\0\18\105\110\105\116\95\114\101\99\111\110\110\95\112\101\114\105\111\100\22\0\0\0\15\105\110\105\116\95\114\101\99\111\110\110\95\116\114\121\13\0\0\0\13\114\101\99\111\110\110\95\112\101\114\105\111\100\22\0\0\0\10\114\101\99\111\110\110\95\116\114\121\13\0\0\0\13\114\101\98\105\110\100\95\112\101\114\105\111\100\22\0\0\0\10\114\101\98\105\110\100\95\116\114\121\13\0\0\0',
  {fields.is_router,fields.heartbeat_period,fields.heartbeat_count,fields.init_reconn_period,fields.init_reconn_try,fields.reconn_period,fields.reconn_try,fields.rebind_period,fields.rebind_try,},
  {layouts.gce_adl_duration,layouts.gce_adl_duration,layouts.gce_adl_duration,layouts.gce_adl_duration,});
layouts.gce_adl_net_option = layout_gce_adl_net_option

local mc = {};
mc = {
  adtype = function(o) return m.net_option end,
  skip_read = function(o,buf) return skip_read_c( field_list , mt_type_list , buf , layout_gce_adl_net_option , o) end,
  size_of = function(o) return size_of_c( field_list , mt_type_list , layout_gce_adl_net_option , o) end,
  read = function(o,buf) return read_c( field_list , mt_type_list , buf , layout_gce_adl_net_option , o) end,
  write = function(o,buf) return write_c( field_list , mt_type_list , buf , layout_gce_adl_net_option , o) end,
};
mc.__index = mc;
mt[1] = mc;
if ns.gce_adl == nil then
  ns.gce_adl = m;
else
  ns.gce_adl.net_option = m.net_option
end
set_layout_mt_c( layout_gce_adl_net_option , regist_mt_type(mt[1]) , 'ad_mt_gce_adl.net_option' , mt[1]);

return m;
