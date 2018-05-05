class 'Xform'
function Xform:__init()	
	self.affine = AffineMatrix()
	self.post = AffineMatrix()
	self.variations = {}
	self.parameters = {}
	self.opacity = 1.0
	self.color = .5
	self.colorSpeed = math.random()
	self.density = 0
	self.palletteId = math.random()
end

function Xform:getAffineMatrix()
	return self.affine
end

function Xform:setAffineMatrix( matrix )
	self.affine = matrix
end

function Xform:getPostMatrix()
	return self.post
end

function Xform:setPostMatrix( matrix )
	self.post = matrix
end

function Xform:getVariationWeight( name )
	if self.variations[name] == nil then return 0.0 end
	return self.variations[name]
end

function Xform:setVariationWeight( name, weight )
	self.variations[name] = weight
end

function Xform:hasVariation( name )
	return self.variations[name] ~= nil and self.variations[name] ~= 0.0
end

function Xform:getAllVariations()
	return self.variations
end

function Xform:getColor()
	return self.color
end

function Xform:setColor( color )
	self.color = color
end

function Xform:getColorSpeed()
	return self.colorSpeed
end

function Xform:setColorSpeed( speed )
	self.colorSpeed = speed
end

function Xform:getOpacity()
	return self.opacity
end

function Xform:setOpacity( opacity )
	self.opacity = opacity
end

function Xform:getParameter( name )
	if self.parameters[name] == nil then return 0.0 end
	return self.parameters[name]
end

function Xform:setParameter( name, value )
	self.parameters[name] = value
end

function Xform:hasParameter( name )
	return self.parameters[name] ~= nil
end

function Xform:getAllParameters()
	return self.parameters
end

function Xform:getDensity()
	return self.density
end

function Xform:setDensity( density )
	self.density = density
end

function Xform:normalizeVariations()
	local varSum = 0
	for k,v in pairs(self.variations) do
		varSum = varSum + v
	end
	
	for k,v in pairs(self.variations) do
		self.variations[k] = self.variations[k]/varSum
	end
end

function Xform:clone()
	local clone = Xform()
	
	for k,v in pairs(self.variations) do
		clone.variations[k] = v
	end
	
	for k,v in pairs(self.parameters) do
		clone.parameters[k] = v
	end
	
	clone.opacity = self.opacity
	clone.colorSpeed = self.colorSpeed
	clone.density = self.density
	clone.color = self.color
	
	for r=1,2 do
		for c=1,3 do
			clone.affine.matrix[r][c] = self.affine.matrix[r][c]
			clone.post.matrix[r][c] = self.post.matrix[r][c]
		end
	end
	
	return clone
end

class 'Flame'

function Flame:__init()
	self.xforms = {}
	self.trans = vec3( math.random() - .5, math.random() - .5, -(math.random(1,3) + .85))
	self.id = tostring(math.random(10000,99999)) .. tostring(math.random(10000,99999)) .. tostring(math.random(10000,99999)) .. tostring(math.random(10000,99999))
	self.pallette = {}
	self.palletteUpdate = true
end

function Flame:getXformCount()
	return #self.xforms
end

function Flame:getTrans()
	return self.trans
end

function Flame:setTrans(trans)
	self.trans = trans
end

function Flame:addXform( xform )
	table.insert(self.xforms, xform)
end

function Flame:getXform( idx )
	if idx > self:getXformCount() then return nil end
	return self.xforms[idx]
end

function Flame:removeXform( idx )
	table.remove(self.xforms, idx)
end

function Flame:normalizeDensities( to )
	if to == nil then to = 1.0 end
	
	local densitySum = 0
	for idx,xform in pairs(self.xforms) do
		densitySum = densitySum + xform:getDensity()
	end
	
	densitySum = densitySum/ to
	
	for idx,xform in pairs(self.xforms) do
		xform:setDensity( xform:getDensity()/densitySum )
	end
end

function Flame:randomizeAffine()
	for k,xform in pairs(self.xforms) do
		if xform:getColorSpeed() > 0 then
		
			xform:setAffineMatrix( AffineMatrix.random( .2, .9, true ) )
		end
	end
end

function Flame:randomizeParameters()
	for k,xform in pairs(self.xforms) do
		for k,v in pairs(VariationTable.parameters) do
			xform:setParameter( v.name, VariationTable.randomParameterValue(v.name) )
		end
	end
end

function Flame:randomizePallette()
	local cpCount = math.random(50, 150)
	local cps = {}
	
	self.pallette = {}
	
	cps[1] = 1
	cps[512] = 1
	self.pallette[1] = { math.random(), math.random(), math.random(), 1.0 }
	self.pallette[512] = { math.random(), math.random(), math.random(), 1.0 }
	for i=1,cpCount do
		local pos
		repeat
			pos = math.random(1,256)
		until cps[pos] == nil
		cps[pos] = 1

		self.pallette[pos] = { math.random(), math.random(), math.random(), 1.0 }
	end
	
	local lcp = 1
	for i=1,512 do
		for j=i + 1,512 do
			if cps[j] ~= nil then ncp = j end
		end
		
		local weight = (i - lcp)/(ncp - lcp)
		if self.pallette[i] == nil then
			self.pallette[i] = {}
			for j=1,3 do
				self.pallette[i][j] = weight * self.pallette[ncp][j] + (1 - weight) * self.pallette[lcp][j]
			end
			self.pallette[i][4] = 1.0
		else
			lcp = i
		end
	end
	
	for i=1,#self.xforms do
		if self.xforms[i]:getColorSpeed() > 0 then
			self.xforms[i]:setColorSpeed(math.random() * .75)
			self.xforms[i]:setColor(math.random())
		end
	end
	
	self.palletteUpdate = true
	self.palletteId = math.random()
end

function Flame:randomizeTrans()
	self.trans = vec3( math.random() - .5, math.random() - .5, -(math.random(1,3) + .85))
end

function Flame:addSymmetry( sym )
	if sym == 1 then sym = 0 end
	local symMats = (sym > 1 and sym - 1) or math.abs(sym)
	
	--local symchance= 1/(symMats + 1)
	
	self:normalizeDensities(1 )
	
	if symMats ~= 0 then
		local dRot = 360/math.abs(sym)
		local rot = dRot
		if math.abs(sym) > 1 then
			for i=1, math.abs(sym) do
				local xform = Xform()
				xform:setAffineMatrix( AffineMatrix.genRotMatrix(rot) )
				xform:setDensity( 1 )
				xform:setColorSpeed(0)
				xform:setVariationWeight("linear", 1.0)
				self:addXform(xform)
				rot = rot + dRot
				if rot >= 360 then break end
			end
		end
		
		if sym < 0 then
			local xform = Xform()
			xform:setAffineMatrix( AffineMatrix.genFlipMatrix(true, false) )
			xform:setDensity( 1 )
			xform:setColorSpeed(0)
			xform:setVariationWeight("linear", 1.0)
			self:addXform(xform)
		end
	end
end

function Flame:getDensitySum()
	local densitySum = 0
	for idx,xform in pairs(self.xforms) do
		densitySum = densitySum + xform:getDensity()
	end
	return densitySum
end

function Flame:interpolate( other, weight )
	local flame = Flame()
	flame:randomizePallette()
	
	local xformCount = self:getXformCount()
	if other:getXformCount() > xformCount then xformCount = other:getXformCount() end
	
	for i=1, xformCount do
		local sXform, eXform
		local iXform = Xform()
		
		if i > self:getXformCount() then
			sXform = Xform()
		else sXform = self:getXform(i) end
		
		if i > other:getXformCount() then
			eXform = Xform()
		else eXform = other:getXform(i) end
		
		for k,v in pairs(VariationTable.variations) do
			if sXform:hasVariation(v) or eXform:hasVariation(v) then
				iXform:setVariationWeight(v, sXform:getVariationWeight(v) * weight + eXform:getVariationWeight(v) * (1 - weight))
			end
		end
		
		for k,v in pairs(VariationTable.parameters) do
			iXform:setParameter(v.name, sXform:getParameter(v.name) * weight + eXform:getParameter(v.name) * (1- weight))
		end
		
		local absS =sXform:getDensity()
		local absE =eXform:getDensity()
		
		iXform:setDensity( absS * weight + absE * (1 - weight) )
		iXform:setColor( sXform:getColor() * weight + eXform:getColor() * (1 - weight) )
		iXform:setAffineMatrix( sXform:getAffineMatrix() * weight + eXform:getAffineMatrix() * (1 - weight) )
		iXform:setPostMatrix( sXform:getPostMatrix() * weight + eXform:getPostMatrix() * (1 - weight) )
		iXform:setColorSpeed( sXform:getColorSpeed() * weight + eXform:getColorSpeed() * (1-weight) )
		flame:addXform(iXform)
	end
	
	for i=1,512 do
		for j=1,4 do
			flame.pallette[i][j] = self.pallette[i][j] * weight + other.pallette[i][j] * (1 - weight)
		end
	end
	
	flame:setTrans( self:getTrans() * weight + other:getTrans() * (1 - weight) )
	
	return flame
end

function Flame:mutate()
	local mutation = self:clone()
	
	local mutateMode = math.random()
	if mutateMode < .1 then mutateMode = "all_vars"
	elseif mutateMode < .5 then mutateMode = "one_xform_coefs"
	elseif mutateMode < .8 and mutation:getXformCount() > 2 then mutateMode = "delete_xform"
	else mutateMode = "all_coefs" end
	
	print(mutateMode)
	
	if mutateMode == "all_vars" then
		local rand = FlameGenerator.random( self:getXformCount() )
		for i=1,self:getXformCount() do
			local randX = rand:getXform(i)
			for k,v in pairs(VariationTable.variations) do
				if randX.variations[v] ~= mutation.xforms[i].variations[v] then
					mutation.xforms[i].variations[v] = randX.variations[v]
					
					for k,v in pairs(VariationTable.getVariationParameterNames(v)) do
						mutation.xforms[i].parameters[v] = randX.parameters[v]
					end
				end
			end
		end
	elseif mutateMode == "one_xform_coefs" then
		mutation:getXform(math.random(1, mutation:getXformCount())):setAffineMatrix(AffineMatrix.random(.2, .9, true))
	elseif mutateMode == "delete_xform" then
		mutation:removeXform(math.random(1, mutation:getXformCount()))
	else
		for i=1,mutation:getXformCount() do
			mutation:getXform(i):setAffineMatrix(AffineMatrix.random(.2, .9, true))
		end
	end
	
	return mutation
			
end

function Flame:clone()
	local clone = Flame()
	
	clone.trans.x = self.trans.x
	clone.trans.y = self.trans.y
	clone.trans.z = self.trans.z
	
	for i=1,self:getXformCount() do
		clone:addXform( self:getXform(i):clone() )
	end
	
	return clone
end

function Flame:cross( other )
	local crossMode = math.random()
	local cross
	
	if crossMode < .1 then crossMode = "union"
	else crossMode = "alternate" end
	
	if crossMode == "union" and self:getXformCount() + other:getXformCount() < 12 then
		cross = self:clone()
		for i=1, other:getXformCount() do
			cross:addXform( other:getXform(i):clone() )
		end
	else
		local father = other:clone()
		
		-- self is mother; after all, this is the flame that
		-- actually produces the cross :P
		local mother = self:clone()
		
		cross = Flame()
		
		local numFather, numMother
		repeat
			numFather = math.random(1, father:getXformCount())
			numMother = math.random(1, mother:getXformCount())
		until numFather + numMother < 12
		for i=1, numFather do
			local xformIdx = math.random(1, father:getXformCount())
			local xform = father:getXform( xformIdx )
			cross:addXform(xform)
			father:removeXform(xformIdx)
		end
		
		for i=1, numMother do
			local xformIdx = math.random(1, mother:getXformCount())
			local xform = mother:getXform( xformIdx )
			cross:addXform(xform)
			mother:removeXform(xformIdx)
		end
	end
	
	return cross
end

function Flame:pushToProgram( program, numVarMats, numParamMats )
	
	program:getParameter("matCount"):setInt(#self.xforms)
	
	local varOffset = 0
	
	program:getParameter("densitySum"):setFloat(self:getDensitySum())

	local palpush = ticks()	
	local pArray = Array1Dfloat(512 * 4)
	for i=1,512 do
		for j=1,4 do
			if self.pallette[i][j] < 0 then self.pallette[i][j] = 0.0 end
			if self.pallette[i][j] > 1 then self.pallette[i][j] = 1.0 end
			pArray:set( (i - 1) * 4 + j - 1, self.pallette[i][j])
		end
	end

	if program.palletteIndex == nil or program.palletteIndex ~= self.palletteId then
		program:getStorageVal(Program.input, 1):copyFromArray(pArray:array())
		program.palletteIndex = self.palletteId
		print("pushed pallette")
	end
	
	for idx,xform in pairs(self.xforms) do
		local xmat = xform:getAffineMatrix():toMat4()
		local xcol = xform:getColor()
		local pmat = xform:getPostMatrix():toMat4()
		
		xmat:set(2, 0, pmat:get(0,0))
		xmat:set(2, 1, pmat:get(0,1))
		xmat:set(2, 2, pmat:get(0,2))
		xmat:set(3, 0, pmat:get(1,0))
		xmat:set(3, 1, pmat:get(1,1))
		xmat:set(3, 2, pmat:get(1,2))
		xmat:set(0, 3, xcol)
		xmat:set(3, 3, xform:getDensity())
		
		program:getParameter("mats"):at(idx - 1):setMat4(xmat)
		program:getParameter("colSpeeds"):at(idx - 1):setFloat(xform:getColorSpeed())

		if xform:getVariationWeight("linear") == 1.0 then
			program:getParameter("varOffsets"):at(idx - 1):setInt( -1 )
		else
			program:getParameter("varOffsets"):at(idx - 1):setInt( varOffset )

			for k,v in pairs(VariationTable.variations) do
				if xform:hasVariation(v) then
					program:getParameter("vars"):at(varOffset):setVec2( vec2(k - 1, xform:getVariationWeight(v)))
					varOffset = varOffset + 1
				end
			end
		
			program:getParameter("vars"):at(varOffset):setVec2( vec2(-1,-1) )
			--print(string.format("vars[%d] = vec2(%d,%f)", varOffset, -1, -1))
			varOffset = varOffset + 1
		end
		
		local xpmat = mat4()
		mat = 0
		row = 0
		col = 0
		for k,v in pairs(VariationTable.parameters) do
			local val = xform:getParameter(v.name)
			xpmat:set(row, col, val)
			
			col = col + 1
			if col == 4 then
				col = 0
				row = row + 1
			end
			
			if row == 4 then
				row = 0
				program:getParameter("varParam"):at( numParamMats * (idx - 1) + mat ):setMat4(xpmat)
				mat = mat + 1
			end
		end
		
		if mat ~= numParamMats then program:getParameter("varParam"):at( numParamMats * (idx - 1) + mat ):setMat4(xpmat) end
	end
end
