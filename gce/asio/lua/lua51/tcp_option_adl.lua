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

regist_field('backlog');
regist_field('enable_connection_aborted');
regist_field('keep_alive');
regist_field('no_delay');
regist_field('receive_buffer_size');
regist_field('reuse_address');
regist_field('send_buffer_size');


local mt = {};

local m = {
  tcp_option,
};

m.tcp_option = function()
  local obj = {
    backlog = -1,
    reuse_address = -1,
    receive_buffer_size = -1,
    send_buffer_size = -1,
    no_delay = -1,
    keep_alive = -1,
    enable_connection_aborted = -1,
  };
  setmetatable(obj,mt[1]);
  return obj;
end

local layout_gce_asio_adl_tcp_option = regist_layout_c(
  '\7\0\105\7\98\97\99\107\108\111\103\13\0\0\0\13\114\101\117\115\101\95\97\100\100\114\101\115\115\9\0\0\0\19\114\101\99\101\105\118\101\95\98\117\102\102\101\114\95\115\105\122\101\13\0\0\0\16\115\101\110\100\95\98\117\102\102\101\114\95\115\105\122\101\13\0\0\0\8\110\111\95\100\101\108\97\121\9\0\0\0\10\107\101\101\112\95\97\108\105\118\101\9\0\0\0\25\101\110\97\98\108\101\95\99\111\110\110\101\99\116\105\111\110\95\97\98\111\114\116\101\100\9\0\0\0',
  {fields.backlog,fields.reuse_address,fields.receive_buffer_size,fields.send_buffer_size,fields.no_delay,fields.keep_alive,fields.enable_connection_aborted,},
  {});
layouts.gce_asio_adl_tcp_option = layout_gce_asio_adl_tcp_option

local mc = {};
mc = {
  adtype = function(o) return m.tcp_option end,
  skip_read = function(o,buf) return skip_read_c( field_list , mt_type_list , buf , layout_gce_asio_adl_tcp_option , o) end,
  size_of = function(o) return size_of_c( field_list , mt_type_list , layout_gce_asio_adl_tcp_option , o) end,
  read = function(o,buf) return read_c( field_list , mt_type_list , buf , layout_gce_asio_adl_tcp_option , o) end,
  write = function(o,buf) return write_c( field_list , mt_type_list , buf , layout_gce_asio_adl_tcp_option , o) end,
};
mc.__index = mc;
mt[1] = mc;
if ns.gce_asio_adl == nil then
  ns.gce_asio_adl = m;
else
  ns.gce_asio_adl.tcp_option = m.tcp_option
end
set_layout_mt_c( layout_gce_asio_adl_tcp_option , regist_mt_type(mt[1]) , 'ad_mt_gce_asio_adl.tcp_option' , mt[1]);

return m;
