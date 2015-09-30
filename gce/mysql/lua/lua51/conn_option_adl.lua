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

regist_field('client_compress');
regist_field('client_found_rows');
regist_field('client_ignore_sigpipe');
regist_field('client_ignore_space');
regist_field('client_multi_results');
regist_field('client_multi_statements');
regist_field('compress');
regist_field('connect_timeout');
regist_field('init_command');
regist_field('read_default_file');
regist_field('read_default_group');
regist_field('read_timeout');
regist_field('reconnect');
regist_field('set_charset_name');
regist_field('write_timeout');


local mt = {};

local m = {
  conn_option,
};

m.conn_option = function()
  local obj = {
    init_command = '',
    compress = -1,
    connect_timeout = -1,
    read_timeout = -1,
    reconnect = -1,
    write_timeout = -1,
    read_default_file = '',
    read_default_group = '',
    set_charset_name = '',
    client_compress = -1,
    client_found_rows = -1,
    client_ignore_sigpipe = -1,
    client_ignore_space = -1,
    client_multi_results = -1,
    client_multi_statements = -1,
  };
  setmetatable(obj,mt[1]);
  return obj;
end

local layout_gce_mysql_adl_conn_option = regist_layout_c(
  '\15\0\128\250\12\105\110\105\116\95\99\111\109\109\97\110\100\19\0\0\0\8\99\111\109\112\114\101\115\115\9\0\0\0\15\99\111\110\110\101\99\116\95\116\105\109\101\111\117\116\13\0\0\0\12\114\101\97\100\95\116\105\109\101\111\117\116\13\0\0\0\9\114\101\99\111\110\110\101\99\116\9\0\0\0\13\119\114\105\116\101\95\116\105\109\101\111\117\116\13\0\0\0\17\114\101\97\100\95\100\101\102\97\117\108\116\95\102\105\108\101\19\0\0\0\18\114\101\97\100\95\100\101\102\97\117\108\116\95\103\114\111\117\112\19\0\0\0\16\115\101\116\95\99\104\97\114\115\101\116\95\110\97\109\101\19\0\0\0\15\99\108\105\101\110\116\95\99\111\109\112\114\101\115\115\9\0\0\0\17\99\108\105\101\110\116\95\102\111\117\110\100\95\114\111\119\115\9\0\0\0\21\99\108\105\101\110\116\95\105\103\110\111\114\101\95\115\105\103\112\105\112\101\9\0\0\0\19\99\108\105\101\110\116\95\105\103\110\111\114\101\95\115\112\97\99\101\9\0\0\0\20\99\108\105\101\110\116\95\109\117\108\116\105\95\114\101\115\117\108\116\115\9\0\0\0\23\99\108\105\101\110\116\95\109\117\108\116\105\95\115\116\97\116\101\109\101\110\116\115\9\0\0\0',
  {fields.init_command,fields.compress,fields.connect_timeout,fields.read_timeout,fields.reconnect,fields.write_timeout,fields.read_default_file,fields.read_default_group,fields.set_charset_name,fields.client_compress,fields.client_found_rows,fields.client_ignore_sigpipe,fields.client_ignore_space,fields.client_multi_results,fields.client_multi_statements,},
  {});
layouts.gce_mysql_adl_conn_option = layout_gce_mysql_adl_conn_option

local mc = {};
mc = {
  adtype = function(o) return m.conn_option end,
  skip_read = function(o,buf) return skip_read_c( field_list , mt_type_list , buf , layout_gce_mysql_adl_conn_option , o) end,
  size_of = function(o) return size_of_c( field_list , mt_type_list , layout_gce_mysql_adl_conn_option , o) end,
  read = function(o,buf) return read_c( field_list , mt_type_list , buf , layout_gce_mysql_adl_conn_option , o) end,
  write = function(o,buf) return write_c( field_list , mt_type_list , buf , layout_gce_mysql_adl_conn_option , o) end,
};
mc.__index = mc;
mt[1] = mc;
if ns.gce_mysql_adl == nil then
  ns.gce_mysql_adl = m;
else
  ns.gce_mysql_adl.conn_option = m.conn_option
end
set_layout_mt_c( layout_gce_mysql_adl_conn_option , regist_mt_type(mt[1]) , 'ad_mt_gce_mysql_adl.conn_option' , mt[1]);

return m;
