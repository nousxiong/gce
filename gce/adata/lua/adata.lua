m = { 
	tablen,
}

-- table length
m.tablen = function(tab)
	if tab == nil then return 0 end
	local count = 0
  for _ in pairs(tab) do count = count + 1 end
  return count
end

return m
