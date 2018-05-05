class 'FlameGenerator'


function HSVtoRGB( hsv )
	if hsv.x == 1 then hsv.x = 0 end
	
	x = r
	y = g
	z = b
	
	local i = math.floor(hsv.x * 6)
	local f = hsv.x * 6 - i;
	local p = hsv.z * (1 - hsv.y)
	local q = hsv.z * (1 - f * hsv.y)
	local t = hsv.z * (1 - (1 - f) * hsv.y)
	local v = hsv.z
	
	if i == 0 then
		return vec3(v,t,p)
	elseif i == 1 then
		return vec3(q,v,p)
	elseif i == 2 then
		return vec3(p,v,t)
	elseif i == 3 then
		return vec3(p,q,v)
	elseif i == 4 then
		return vec3(t,p,v)
	elseif i == 5 then
		return vec3(v,q,p)
	end
end

function FlameGenerator:__init()
end

function FlameGenerator.random( numXforms )
	local flame = Flame()
	
	if numXforms == nil then
		local xformDist = {2, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4, 5, 5, 6}
		numXforms = xformDist[math.random(1, #xformDist)]
	end
	
	for i=1,numXforms do
		print("xform: " .. i)
		local xform = Xform()
		local varCount = 1
		while varCount < 5 and math.random() <.25 do varCount = varCount + 1 end
		
		xform:setAffineMatrix( AffineMatrix.random( 0, 2, true ) )
		--if math.random() < .15 then
		--xform:getPostMatrix().matrix[1][3] = math.random() - .5;
		--xform:getPostMatrix().matrix[2][3] = math.random() - .5;
		--end
		xform:setDensity( math.random() )
		xform:setColor( math.random() )
			
		for j=1, varCount do
			local randVar
			repeat
				randVar = VariationTable.randomVariation()
			until not xform:hasVariation(randVar)
		
			xform:setVariationWeight( randVar, math.random())
			print(string.format("%d: %s", VariationTable.getVariationIndex(randVar), randVar))
		end
		
		for k,v in pairs(VariationTable.parameters) do
			xform:setParameter( v.name, VariationTable.randomParameterValue(v.name) )
		end
		
		xform:normalizeVariations()
		flame:addXform(xform)
	end
	
	--flame:normalizeDensities()
	flame:randomizePallette()
	flame:addSymmetry(math.random(-4,5))
	return flame
end
