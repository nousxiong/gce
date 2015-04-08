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

regist_field('ctxid_');
regist_field('in_pool_');
regist_field('sid_');
regist_field('svc_');
regist_field('svc_id_');
regist_field('timestamp_');
regist_field('type_');
regist_field('uintptr_');

require'match_adl'
require'service_id_adl'

local mt = {};

local m = {
  actor_id,
};

m.actor_id = function()
  local obj = {
    ctxid_ = ns.gce_adl.match(),
    timestamp_ = 0,
    uintptr_ = 0,
    svc_id_ = 0,
    type_ = 0,
    in_pool_ = 0,
    sid_ = 0,
    svc_ = ns.gce_adl.service_id(),
  };
  setmetatable(obj,mt[1]);
  return obj;
end

local layout_gce_adl_actor_id = regist_layout_c(
  '\8\0\60\6\99\116\120\105\100\95\22\0\0\0\10\116\105\109\101\115\116\97\109\112\95\16\0\0\0\8\117\105\110\116\112\116\114\95\16\0\0\0\7\115\118\99\95\105\100\95\16\0\0\0\5\116\121\112\101\95\10\0\0\0\8\105\110\95\112\111\111\108\95\10\0\0\0\4\115\105\100\95\14\0\0\0\4\115\118\99\95\22\0\0\0',
  {fields.ctxid_,fields.timestamp_,fields.uintptr_,fields.svc_id_,fields.type_,fields.in_pool_,fields.sid_,fields.svc_,},
  {layouts.gce_adl_match,layouts.gce_adl_service_id,});
layouts.gce_adl_actor_id = layout_gce_adl_actor_id

local mc = {};
mc = {
  adtype = function(o) return m.actor_id end,
  skip_read = function(o,buf) return skip_read_c( field_list , mt_type_list , buf , layout_gce_adl_actor_id , o) end,
  size_of = function(o) return size_of_c( field_list , mt_type_list , layout_gce_adl_actor_id , o) end,
  read = function(o,buf) return read_c( field_list , mt_type_list , buf , layout_gce_adl_actor_id , o) end,
  write = function(o,buf) return write_c( field_list , mt_type_list , buf , layout_gce_adl_actor_id , o) end,
};
mc.__index = mc;
mt[1] = mc;
if ns.gce_adl == nil then
  ns.gce_adl = m;
else
  ns.gce_adl.actor_id = m.actor_id
end
set_layout_mt_c( layout_gce_adl_actor_id , regist_mt_type(mt[1]) , 'ad_mt_gce_adl.actor_id' , mt[1]);

return m;
