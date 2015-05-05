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

regist_field('certificate');
regist_field('certificate_authority');
regist_field('certificate_chain');
regist_field('certificate_chain_file');
regist_field('certificate_file');
regist_field('certificate_format');
regist_field('default_verify_paths');
regist_field('default_workarounds');
regist_field('no_compression');
regist_field('no_sslv2');
regist_field('no_sslv3');
regist_field('no_tlsv1');
regist_field('private_key');
regist_field('private_key_file');
regist_field('private_key_format');
regist_field('rsa_private_key');
regist_field('rsa_private_key_file');
regist_field('rsa_private_key_format');
regist_field('single_dh_use');
regist_field('tmp_dh');
regist_field('tmp_dh_file');
regist_field('verify_client_once');
regist_field('verify_depth');
regist_field('verify_fail_if_no_peer_cert');
regist_field('verify_file');
regist_field('verify_none');
regist_field('verify_paths');
regist_field('verify_peer');


local mt = {};

local m = {
  ssl_option,
};

m.ssl_option = function()
  local obj = {
    verify_paths = {},
    default_verify_paths = -1,
    certificate_authority = '',
    verify_file = '',
    default_workarounds = -1,
    single_dh_use = -1,
    no_sslv2 = -1,
    no_sslv3 = -1,
    no_tlsv1 = -1,
    no_compression = -1,
    verify_depth = -1,
    verify_none = -1,
    verify_peer = -1,
    verify_fail_if_no_peer_cert = -1,
    verify_client_once = -1,
    certificate = '',
    certificate_file = '',
    certificate_format = 1,
    certificate_chain = '',
    certificate_chain_file = '',
    private_key = '',
    private_key_file = '',
    private_key_format = 1,
    rsa_private_key = '',
    rsa_private_key_file = '',
    rsa_private_key_format = 1,
    tmp_dh = '',
    tmp_dh_file = '',
  };
  setmetatable(obj,mt[1]);
  return obj;
end

local layout_gce_asio_adl_ssl_option = regist_layout_c(
  '\28\1\129\188\1\12\118\101\114\105\102\121\95\112\97\116\104\115\20\0\0\1\19\0\20\100\101\102\97\117\108\116\95\118\101\114\105\102\121\95\112\97\116\104\115\9\0\0\0\21\99\101\114\116\105\102\105\99\97\116\101\95\97\117\116\104\111\114\105\116\121\19\0\0\0\11\118\101\114\105\102\121\95\102\105\108\101\19\0\0\0\19\100\101\102\97\117\108\116\95\119\111\114\107\97\114\111\117\110\100\115\9\0\0\0\13\115\105\110\103\108\101\95\100\104\95\117\115\101\9\0\0\0\8\110\111\95\115\115\108\118\50\9\0\0\0\8\110\111\95\115\115\108\118\51\9\0\0\0\8\110\111\95\116\108\115\118\49\9\0\0\0\14\110\111\95\99\111\109\112\114\101\115\115\105\111\110\9\0\0\0\12\118\101\114\105\102\121\95\100\101\112\116\104\13\0\0\0\11\118\101\114\105\102\121\95\110\111\110\101\9\0\0\0\11\118\101\114\105\102\121\95\112\101\101\114\9\0\0\0\27\118\101\114\105\102\121\95\102\97\105\108\95\105\102\95\110\111\95\112\101\101\114\95\99\101\114\116\9\0\0\0\18\118\101\114\105\102\121\95\99\108\105\101\110\116\95\111\110\99\101\9\0\0\0\11\99\101\114\116\105\102\105\99\97\116\101\19\0\0\0\16\99\101\114\116\105\102\105\99\97\116\101\95\102\105\108\101\19\0\0\0\18\99\101\114\116\105\102\105\99\97\116\101\95\102\111\114\109\97\116\9\0\0\0\17\99\101\114\116\105\102\105\99\97\116\101\95\99\104\97\105\110\19\0\0\0\22\99\101\114\116\105\102\105\99\97\116\101\95\99\104\97\105\110\95\102\105\108\101\19\0\0\0\11\112\114\105\118\97\116\101\95\107\101\121\19\0\0\0\16\112\114\105\118\97\116\101\95\107\101\121\95\102\105\108\101\19\0\0\0\18\112\114\105\118\97\116\101\95\107\101\121\95\102\111\114\109\97\116\9\0\0\0\15\114\115\97\95\112\114\105\118\97\116\101\95\107\101\121\19\0\0\0\20\114\115\97\95\112\114\105\118\97\116\101\95\107\101\121\95\102\105\108\101\19\0\0\0\22\114\115\97\95\112\114\105\118\97\116\101\95\107\101\121\95\102\111\114\109\97\116\9\0\0\0\6\116\109\112\95\100\104\19\0\0\0\11\116\109\112\95\100\104\95\102\105\108\101\19\0\0\0',
  {fields.verify_paths,fields.default_verify_paths,fields.certificate_authority,fields.verify_file,fields.default_workarounds,fields.single_dh_use,fields.no_sslv2,fields.no_sslv3,fields.no_tlsv1,fields.no_compression,fields.verify_depth,fields.verify_none,fields.verify_peer,fields.verify_fail_if_no_peer_cert,fields.verify_client_once,fields.certificate,fields.certificate_file,fields.certificate_format,fields.certificate_chain,fields.certificate_chain_file,fields.private_key,fields.private_key_file,fields.private_key_format,fields.rsa_private_key,fields.rsa_private_key_file,fields.rsa_private_key_format,fields.tmp_dh,fields.tmp_dh_file,},
  {});
layouts.gce_asio_adl_ssl_option = layout_gce_asio_adl_ssl_option

local mc = {};
mc = {
  adtype = function(o) return m.ssl_option end,
  skip_read = function(o,buf) return skip_read_c( field_list , mt_type_list , buf , layout_gce_asio_adl_ssl_option , o) end,
  size_of = function(o) return size_of_c( field_list , mt_type_list , layout_gce_asio_adl_ssl_option , o) end,
  read = function(o,buf) return read_c( field_list , mt_type_list , buf , layout_gce_asio_adl_ssl_option , o) end,
  write = function(o,buf) return write_c( field_list , mt_type_list , buf , layout_gce_asio_adl_ssl_option , o) end,
};
mc.__index = mc;
mt[1] = mc;
if ns.gce_asio_adl == nil then
  ns.gce_asio_adl = m;
else
  ns.gce_asio_adl.ssl_option = m.ssl_option
end
set_layout_mt_c( layout_gce_asio_adl_ssl_option , regist_mt_type(mt[1]) , 'ad_mt_gce_asio_adl.ssl_option' , mt[1]);

return m;
