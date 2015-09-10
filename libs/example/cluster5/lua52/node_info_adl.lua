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

regist_field('ep_');
regist_field('list_');
regist_field('svcid_');

require'match_adl'
require'service_id_adl'

local mt = {};

local m = {
  node_info,
  node_info_list,
};

m.node_info = function()
  local obj = {
    svcid_ = ns.gce_adl.service_id(),
    ep_ = '',
  };
  setmetatable(obj,mt[1]);
  return obj;
end

m.node_info_list = function()
  local obj = {
    list_ = {},
  };
  setmetatable(obj,mt[2]);
  return obj;
end

local layout_cluster5_node_info = regist_layout_c(
  '\2\0\11\6\115\118\99\105\100\95\22\0\0\0\3\101\112\95\19\0\0\0',
  {fields.svcid_,fields.ep_,},
  {layouts.gce_adl_service_id,});
layouts.cluster5_node_info = layout_cluster5_node_info
local layout_cluster5_node_info_list = regist_layout_c(
  '\1\1\6\5\108\105\115\116\95\20\0\0\1\22\0',
  {fields.list_,},
  {layouts.cluster5_node_info,});
layouts.cluster5_node_info_list = layout_cluster5_node_info_list

local mc = {};
mc = {
  adtype = function(o) return m.node_info end,
  skip_read = function(o,buf) return skip_read_c( field_list , mt_type_list , buf , layout_cluster5_node_info , o) end,
  size_of = function(o) return size_of_c( field_list , mt_type_list , layout_cluster5_node_info , o) end,
  read = function(o,buf) return read_c( field_list , mt_type_list , buf , layout_cluster5_node_info , o) end,
  write = function(o,buf) return write_c( field_list , mt_type_list , buf , layout_cluster5_node_info , o) end,
};
mc.__index = mc;
mt[1] = mc;
mc = {
  adtype = function(o) return m.node_info_list end,
  skip_read = function(o,buf) return skip_read_c( field_list , mt_type_list , buf , layout_cluster5_node_info_list , o) end,
  size_of = function(o) return size_of_c( field_list , mt_type_list , layout_cluster5_node_info_list , o) end,
  read = function(o,buf) return read_c( field_list , mt_type_list , buf , layout_cluster5_node_info_list , o) end,
  write = function(o,buf) return write_c( field_list , mt_type_list , buf , layout_cluster5_node_info_list , o) end,
};
mc.__index = mc;
mt[2] = mc;
if ns.cluster5 == nil then
  ns.cluster5 = m;
else
  ns.cluster5.node_info = m.node_info
  ns.cluster5.node_info_list = m.node_info_list
end
set_layout_mt_c( layout_cluster5_node_info , regist_mt_type(mt[1]) , 'ad_mt_cluster5.node_info' , mt[1]);
set_layout_mt_c( layout_cluster5_node_info_list , regist_mt_type(mt[2]) , 'ad_mt_cluster5.node_info_list' , mt[2]);

return m;
