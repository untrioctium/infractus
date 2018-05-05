class 'AffineMatrix'

function AffineMatrix:__init()
	self.matrix = { {1, 0, 0}, {0, 1, 0} }
end

function AffineMatrix.random( minVal, maxVal, allowNeg )
	local randParam = 
		function()
			local val = math.random() * (maxVal - minVal) + minVal
			if allowNeg then if math.random() < .5 then val = -val end end
			return val
		end
	
	local mat = AffineMatrix()
	
	mat.matrix[1][1] = randParam()
	mat.matrix[1][2] = randParam()
	mat.matrix[1][3] = randParam()
	mat.matrix[2][1] = randParam()
	mat.matrix[2][2] = randParam()
	mat.matrix[2][3] = randParam()
	
	return mat
end

function AffineMatrix:rotate(deg)
	local rad = math.rad(deg)
	local rotSin = math.sin(rad)
	local rotCos = math.cos(rad)
	
	for i=1,3 do
		local temp = self.matrix[1][i]
		self.matrix[1][i] = temp * rotCos - self.matrix[2][i] * rotSin
		self.matrix[2][i] = temp * rotSin + self.matrix[2][i] * rotCos
	end
end

function AffineMatrix:__mul(v)
	local mat = AffineMatrix()

	mat.matrix[1][1] = self.matrix[1][1] * v
	mat.matrix[1][2] = self.matrix[1][2] * v
	mat.matrix[1][3] = self.matrix[1][3] * v
	mat.matrix[2][1] = self.matrix[2][1] * v
	mat.matrix[2][2] = self.matrix[2][2] * v
	mat.matrix[2][3] = self.matrix[2][3] * v
	
	return mat
end

function AffineMatrix:__add(v)
	local mat = AffineMatrix()
	
	for r=1,2 do
		for c=1,3 do
			mat.matrix[r][c] = self.matrix[r][c] + v.matrix[r][c]
		end
	end
	
	return mat
end

function AffineMatrix.genRotMatrix( deg )
	local rad = math.rad(deg)
	local rotSin = math.sin(rad)
	local rotCos = math.cos(rad)

	local mat = AffineMatrix()
	
	mat.matrix[2][2] = rotCos
	mat.matrix[1][1] = rotCos
	mat.matrix[1][2] = rotSin
	mat.matrix[2][1] = -rotSin
	
	return mat
end

function AffineMatrix.genFlipMatrix( flipX, flipY )
	local mat = AffineMatrix()
	if flipX then mat.matrix[1][1] = -1 end
	if flipY then mat.matrix[2][2] = -1 end
	return mat
end

function AffineMatrix:toMat4()
	local retMat = mat4()
	
	retMat:set(0,0, self.matrix[1][1])
	retMat:set(0,1, self.matrix[1][2])
	retMat:set(0,2, self.matrix[1][3])
	retMat:set(1,0, self.matrix[2][1])
	retMat:set(1,1, self.matrix[2][2])
	retMat:set(1,2, self.matrix[2][3])
	
	return retMat
end
