function drawStatus( foreground, background, ... )
	local graphicsSystem = GraphicsSystem.instance()
	local screenInfo = graphicsSystem:getScreenInfo()
	
	local total = #arg
	local maxFieldLength = 0
	for index,value in ipairs(arg) do
		if value == nil then break end
		if value[1]:len() > maxFieldLength then maxFieldLength = value[1]:len() end
	end
	
	local maxFinalLength = 0
	for index,value in ipairs(arg) do
		value[1] = string.format("%s%s: %s", string.rep( " ", maxFieldLength - value[1]:len() ), value[1], tostring(value[2]))
		if value[1]:len() > maxFinalLength then maxFinalLength = value[1]:len() end
	end
	
	local texth = graphicsSystem:getFontHeight()
	local textw = graphicsSystem:getFontWidth()
	local starty = screenInfo.h - total * texth
	
	graphicsSystem:setDrawColor( background )
	graphicsSystem:drawRectangle( Point(0, starty), Point( maxFinalLength * textw, starty + total * texth) )
	
	graphicsSystem:setDrawColor( foreground )
	for index,value in ipairs(arg) do
		graphicsSystem:drawText( value[1], Point(0, starty) )
		starty = starty + texth
	end
	graphicsSystem:resetDrawColor()
end

