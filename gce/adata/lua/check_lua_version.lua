local _, _, lua_v1, lua_v2 = string.find(_VERSION, 'Lua (%d+)%.(%d+)');

local check_version = function(v1,v2)
  if tonumber(lua_v1) < v1 or tonumber(lua_v2) < v2 then error('please use lua '..v1..'.'..v2..' at least.') end;
end

return check_version;