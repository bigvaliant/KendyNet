battle = {
	avatars={}, --地图上所有的avatar
}

function battle:new()
    local o = {}
    self.__index = self
    setmetatable(o, self)
    return o
end

function battle:init()
	
	return self
end

--地图定时器函数
function battle:tick(now)

end


