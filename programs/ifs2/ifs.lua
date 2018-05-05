assert(loadfile("scripts/drawStatus.lua"))()
assert(loadfile("programs/ifs2/VariationTable.lua"))()
assert(loadfile("programs/ifs2/AffineMatrix.lua"))()
assert(loadfile("programs/ifs2/Flame.lua"))()
assert(loadfile("programs/ifs2/FlameGenerator.lua"))()
assert(loadfile("programs/ifs2/FlameAnimator.lua"))()

class 'IFS' (InfractusProgram)

function IFS:__init()
	InfractusProgram.__init(self)
end

function ss()
	GraphicsSystem.instance():saveTexture( self.downsample.out:toTexture(), "./" .. math.random(100000, 999999))
end

function IFS:init(interactive)

	-- controls how many samples there are in each direction
	-- 2 is 2x2 oversampling (i.e. 2x antialiasing)
	self.oversample =1
	
	-- weight of the new color in the flame algorithm
	self.colWeight = .75
	
	-- the number of particles to use to create the flame is this squared
	self.size = 2048
	
	-- the number of iterations that "seed" each particle
	self.seedIterCount = 15
	
	-- the number of drawing iterations to perform
	-- the number of particles drawn is size * size * (drawIterCount + 1)
	self.drawIterCount = 1
	
	-- minimum variation probability
	self.minChance = .005
	
	-- maximum variation probability
	self.maxChance = .0075
	
	-- minimum value for each affine coefficient
	self.affineMin = .3
	
	-- maximum value for each affine coefficient
	self.affineMax = 1
	
	-- gamma applied before tonemapping
	self.preGamma = .8
	
	-- gamma applied after tonemapping
	self.postGamma = 2.2

	-- chance that a function only has a linear variation
	self.linearOnlyChance = .2

	-- chance of the linear variation
	self.linearChance = .10
	
	-- minimum weight for each variation (pre normalization)
	self.varMin = 0
	
	self.exposure = 1.5
	
	-- maximum variations per flame
	-- lowering this allows for more total xforms
	-- making it too low can mess with flame interpolation
	-- if they have many unique variations
	self.maximumVariations = 12
	
	self.clearColor = Color( 0.0, 0.0, 0.0, .5 )
	self.rot = Point( 0, 0, 0 )
	self.trans = Point( 0,0, -1.85 )
	self.scale = Point( 1, 1, 1 )
	
	self.lockSpin = false
	
	-- get the shader source
	local source = io.open(self:getWorkingDirectory() .. "ifs.frag"):read("*a")

	-- load up and process all the variations
	local variationsPath = self:getWorkingDirectory() .. "variations.xml"
	ConfigSystem.instance():loadConfig(variationsPath)
	local variations = ConfigSystem.instance():getConfigPtree(variationsPath):getChild("variations")
	
	-- holds the source for each variation
	local varSource = {}
	
	-- load up variations and parameters into VariationTable
	for k,v in pairs(variations:children()) do
		local varName = v.second:get("<xmlattr>.name")
		VariationTable.addVariation(varName)
		if v.second:count("parameters") > 0 then
			for k,v in pairs(v.second:getChild("parameters"):children()) do
				local info = v.second:getChild("<xmlattr>")
				VariationTable.addParameter( info:get("name"), varName, tonumber(info:get("min")), tonumber(info:get("max")), info:get("integral") == "true" )
			end
		end
		varSource[varName] = v.second:get("code")
	end
	
	self.maxFunctions = 12
	self.numVariations = #VariationTable.variations
	self.numParameters = #VariationTable.parameters
	self.numParamMats = math.ceil(self.numParameters/16)

	self.maxPerSwitch = 16
	self.numSwitch = math.ceil(self.numVariations/self.maxPerSwitch)
	
	local buildBlock = function(code, idx, start)
		if start == nil then start = "" end
		return start .. " if( idx < " .. idx .. ")\n{\nswitch(idx){\n" .. code .. "\n}\n} "
	end
	
	local matPosFromIdx = function(idx)
		local mat = math.floor(idx/16)
		idx = idx - mat * 16
		local row = math.floor(idx/4)
		idx = idx - row * 4
		return string.format("varParam[funcIdx * %d + %d][%d][%d]", self.numParamMats, mat, row, idx)
	end
	
	-- build the shader source
	local compiledStruct = ""
	local compiledVars = ""
	for k,varName in pairs(VariationTable.variations) do
		local sIdx = k - 1
		local parameters = VariationTable.getVariationParameterNames(varName)
		
		if #parameters > 0 then
			for k,paramName in pairs(parameters) do
				local matPos = matPosFromIdx(VariationTable.getParameterIndex(paramName) - 1)
				varSource[varName] = string.gsub(varSource[varName], paramName, matPos)
			end
		end
		if sIdx ~= 0 and sIdx % self.maxPerSwitch == 0 then
			local start = (sIdx == self.maxPerSwitch and "") or "else"
			compiledStruct = compiledStruct .. "\n" .. buildBlock(compiledVars, sIdx, start)
			compiledVars = ""
		end

		compiledVars = compiledVars .. string.format("// variation %d: %s\ncase %d:\n{%s\ncontinue;}", 
			sIdx, varName, sIdx, varSource[varName])
	end
	
	if compiledVars ~= "" then
		compiledStruct = compiledStruct .. "\n" .. "else{\nswitch(idx){\n" .. compiledVars .. "\n}\n}"
	end
	print(compiledStruct)
	
	-- replace affine coefficients
	local affineCoeff = {aa="mat[0].x", ab="mat[0].y", ac="mat[0].z", ad="mat[1].x", ae="mat[1].y", af="mat[1].z"}
	compiledVars = string.gsub(compiledStruct, "(%w+)", affineCoeff)
	
	-- place the variation source in the shader
	source = string.gsub( source, "%$VARSOURCE%$", compiledVars)
	
	self.ifs = Program()
	self.ifs:setWorkingDirectory(self:getWorkingDirectory())
	self.ifs:load("ifs.program")
	self.ifs:setProgramLocationMemoryString("GLSLComputeEngine", source)
	self.ifs:addParameterArray("mats", Parameter.Mat4, self.maxFunctions)
	self.ifs:addParameterArray("vars", Parameter.Vec2, self.maxFunctions * self.maximumVariations)
	self.ifs:addParameterArray("varOffsets", Parameter.Int, self.maxFunctions)
	self.ifs:addParameterArray("varParam", Parameter.Mat4, self.maxFunctions * self.numParamMats)
	self.ifs:addParameterArray("colSpeeds", Parameter.Float, self.maxFunctions)
	
	self.engine = PluginManager.instance():loadPlugin("GLSLComputeEngine"):toEngine()
	
	local graphicsSystem = GraphicsSystem.instance()
	local screenInfo = graphicsSystem:getScreenInfo()
		self.bufferWidth = screenInfo.w * self.oversample
	self.bufferHeight = screenInfo.h * self.oversample
	self:setBufferTexture(graphicsSystem:createBufferTexture(self.bufferWidth, self.bufferHeight))
	self.bufferStorage = self.engine:fromTexture( self:getBufferTexture() )
	self.ifs:bindEngine(self.engine)
	
	self.paused = false
	self.drawStatus = true
	
	self.tonemap = Program();
	self.tonemap.out = self.engine:fromTexture( graphicsSystem:createBufferTexture(self.bufferWidth, self.bufferHeight) )
	self.tonemap:setWorkingDirectory("./programs/")
	self.tonemap:load("tonemap.program")
	self.tonemap:bindEngine(self.engine)
	self.tonemap:setStorage( Program.input, 0, self.bufferStorage )
	self.tonemap:setStorage( Program.output, 0, self.tonemap.out )	
	
	self.convolve = Program()
	self.convolve.out = self.engine:fromTexture( graphicsSystem:createBufferTexture(self.bufferWidth, self.bufferHeight) )
	self.convolve:setWorkingDirectory("./programs/")
	self.convolve:load("convolve.program");
	self.convolve:bindEngine(self.engine);
	self.convolve:setStorage( Program.input, 0, self.tonemap.out )
	self.convolve:setStorage( Program.output, 0, self.convolve.out )
	self.convolve.kernel = {1,2,1,2,4,2,1,2,1} -- gaussian kernel
	
	self.downsample = Program();
	self.downsample.out = self.engine:fromTexture( graphicsSystem:createBufferTexture(0,0) )
	self.downsample:setWorkingDirectory("./programs/")
	self.downsample:load("downsample.program")
	self.downsample:bindEngine(self.engine)
	self.downsample:setStorage( Program.input, 0, self.convolve.out )
	self.downsample:setStorage( Program.output, 0, self.downsample.out )	
	self.downsample:getParameter("samples"):setInt(self.oversample)
	self.downsample:getParameter("inOff"):setVec2( vec2( 1/self.bufferWidth, 1/self.bufferHeight ) )
	
	self.ifs:allocateStorage( self.size, self.size, Program.input, 0 )
	self.ifs:allocateStorage( 1, 512, Program.input, 1 )
	self.ifs:allocateStorage( self.size, self.size, Program.output, 0 )
	self.ifs:allocateStorage( self.size, self.size, Program.output, 1 )
	
	self.blurBuffer = graphicsSystem:createBufferTexture(0,0)
	self.blurFactor = 1
	
	graphicsSystem:drawToTexture( self.blurBuffer  )
	graphicsSystem:clearToColor( self.clearColor )
	graphicsSystem:drawToTexture( self:getBufferTexture()  )
	graphicsSystem:clearToColor( self.clearColor )
	graphicsSystem:drawToTexture( Texture() )
	
	self.sFlame = FlameGenerator.random()
	self.eFlame = FlameGenerator.random()
	self.iFlame = nil
	
	self.time = 0
	
	self.TRANS_TIME = 100000
	self.HOLD_TIME = 30000
	self.holding = false
	self.ROT_SPEED = 750000
	
	self.paused = false
	
	self.frameTime = 0
	self.pushTime = 0
	self.runTime = 0
	self.drawCalcTime = 0
	self.drawPointTime = 0
	self.postTime = 0
end

function IFS:resizeInput(size)
	self.size = size
	
	self.ifs:allocateStorage( self.size, self.size, Program.input, 0 )
	self.ifs:allocateStorage( self.size, self.size, Program.input, 1 )
	self.ifs:allocateStorage( self.size, self.size, Program.output, 0 )
	self.ifs:allocateStorage( self.size, self.size, Program.output, 1 )
end

function IFS:run( dt )
	self.frameTime = ticks()
	self.pushTime = ticks()
	if self.paused then dt = 0 end
	self.time = self.time - dt
	local rot = dt * 360/self.ROT_SPEED
	for i=1,self.sFlame:getXformCount() do
		local xform = self.sFlame:getXform(i)
		if xform:getColorSpeed() > 0 then
			xform:getPostMatrix():rotate(rot)
			xform:getAffineMatrix():rotate(-rot)
			rot = rot * -1
		end
	end 
	
	rot = math.abs(rot)
	for i=1,self.eFlame:getXformCount() do
		local xform = self.eFlame:getXform(i)
		if xform:getColorSpeed() > 0 then
			xform:getPostMatrix():rotate(rot)
			xform:getAffineMatrix():rotate(-rot)
			rot = rot * -1
		end
	end 
	
	if self.time <= -self.HOLD_TIME and self.lockSpin == false then
		self.holding = false
		self.sFlame = self.eFlame
		self.eFlame = FlameGenerator.random()
		self.time = self.TRANS_TIME
		self.iFlame = self.sFlame
	elseif self.holding then
		self.iFlame = self.eFlame
	elseif self.time <= 0 then
		self.holding = true
		self.iFlame = self.eFlame
	else
		local weight = self.time/self.TRANS_TIME
		weight = weight * weight * (3 - 2 * weight)
		self.iFlame = self.sFlame:interpolate( self.eFlame, weight )
	end
		self.pushTime = ticks() - self.pushTime
		self.ifsTime = ticks()
	self.iFlame:pushToProgram( self.ifs, self.numVarMats, self.numParamMats )
	self.ifsTime = ticks() - self.ifsTime
	
	self.randTime = math.random(100, 30592059)
	self.ifs:getParameter("time"):setInt(self.randTime)
	self.ifs:getParameter("iterCount"):setInt(self.seedIterCount)
	self.ifs:getParameter("seed"):setInt(1)
	self.ifs:getParameter("colWeight"):setFloat(self.colWeight)
	
	self.runTime = ticks()
	self.ifs:run()
	self.runTime = ticks() - self.runTime
end

function IFS:draw()
local graphicsSystem = GraphicsSystem.instance()
	local screenInfo = graphicsSystem:getScreenInfo()
	
	graphicsSystem:clearToColor(self.clearColor)
	
	graphicsSystem:pushDrawingMode()
	graphicsSystem:setDrawingMode( GraphicsSystem.ThreeD )

	graphicsSystem:scaleView( self.scale )
	local transVec = self.iFlame:getTrans()
	graphicsSystem:translateView( Point(transVec.x, transVec.y, transVec.z) )
	graphicsSystem:rotateView( self.rot.x, Point(1,0,0))
	graphicsSystem:rotateView( self.rot.y, Point(0,1,0))
	graphicsSystem:rotateView( -self.rot.z, Point(0,0,1))
	
	self.ifs:getParameter("seed"):setInt(0);
	self.ifs:getParameter("iterCount"):setInt(1);
	graphicsSystem:setBlendingMode( GraphicsSystem.Additive )
	self.drawCalcTime = 0
	self.drawPointTime = 0
	for i=0,self.drawIterCount do
		self.ifs:getParameter("time"):setInt( self.randTime * (i + 1))

		local drawTime = ticks()
		graphicsSystem:drawPointArray( self.ifs:getStorageVal( Program.output, 0 ):toTexture(), 
										self.ifs:getStorageVal( Program.output, 1 ):toTexture() )
		self.drawPointTime = self.drawPointTime + ticks() - drawTime
		if i~=self.drawIterCount then
			self.ifs:swapInputOutput(0)
			local calcTime = ticks()
			self.ifs:run()
			self.drawCalcTime = self.drawCalcTime + ticks() - calcTime
		end
	end
	graphicsSystem:setBlendingMode( GraphicsSystem.Normal )
	graphicsSystem:popDrawingMode()
end

function IFS:getOutput()
	self.postTime = ticks()
	local graphicsSystem = GraphicsSystem.instance()
	local screenInfo = graphicsSystem:getScreenInfo()
	
	self.tonemap:getParameter("preGamma"):setFloat(self.preGamma)
	self.tonemap:getParameter("postGamma"):setFloat(self.postGamma)
	self.tonemap:getParameter("exposure"):setFloat(self.exposure)
	self.tonemap:getParameter("alpha"):setFloat(1.0)
	self.tonemap:getParameter("useAlpha"):setInt(1)
	self.tonemap:run()
	
	convolveKernel = self.convolve:getParameter("convolveKernel")
	for i=1,9 do
		convolveKernel:at(i-1):setFloat(self.convolve.kernel[i])
	end

	self.convolve:run()
	
	-- for oversampling less than 3, use the built in linear downsampling
	-- provided by opengl
	if self.oversample <= 2 then
		graphicsSystem:drawToTexture( self.downsample.out:toTexture()  )
		graphicsSystem:clearToColor(self.clearColor)
		graphicsSystem:useTexture(self.convolve.out:toTexture() )
		graphicsSystem:drawRectangle( Point(0,screenInfo.h), Point(screenInfo.w, 0) )
		graphicsSystem:useTexture( Texture() )
		graphicsSystem:drawToTexture( Texture() )
	else
		self.downsample:run()
	end
	
	graphicsSystem:drawToTexture( self.downsample.out:toTexture()  )
	graphicsSystem:useTexture(self.blurBuffer)
	graphicsSystem:setDrawColor( Color( 1.0, 1.0, 1.0,1- math.pow(10, -self.blurFactor)) )
	graphicsSystem:drawRectangle( Point(0,screenInfo.h), Point(screenInfo.w, 0) )
	graphicsSystem:useTexture( Texture() )
	graphicsSystem:drawToTexture( Texture() )
	
	graphicsSystem:drawToTexture(self.blurBuffer)
	graphicsSystem:useTexture(self.downsample.out:toTexture())
	graphicsSystem:resetDrawColor()
	graphicsSystem:drawRectangle( Point(0,screenInfo.h), Point(screenInfo.w, 0) )
	graphicsSystem:useTexture( Texture() )
	graphicsSystem:drawToTexture( Texture() )

	local status = {}
	local fps = graphicsSystem:getFPS()
	self.postTime = ticks() - self.postTime
	self.frameTime = ticks() - self.frameTime
	
	table.insert(status, {"F", self.frameTime})
	table.insert(status, {"P", string.format("%d (%.1f%%)", self.pushTime, self.pushTime/self.frameTime * 100)})
	table.insert(status, {"PIFS", string.format("%d (%.1f%%)", self.ifsTime, self.ifsTime/self.frameTime * 100)})
	table.insert(status, {"R", string.format("%d (%.1f%%)", self.runTime, self.runTime/self.frameTime * 100)})
	table.insert(status, {"DC", string.format("%d (%.1f%%)", self.drawCalcTime, self.drawCalcTime/self.frameTime * 100)})
	table.insert(status, {"DCA", string.format("%.3f", self.drawCalcTime/self.drawIterCount)})
	table.insert(status, {"DCAP", string.format("%f", self.drawCalcTime/(self.drawIterCount * self.size * self.size))})
	table.insert(status, {"DP", string.format("%d (%.1f%%)", self.drawPointTime, self.drawPointTime/self.frameTime * 100)})
	table.insert(status, {"P", string.format("%d (%.1f%%)", self.postTime, self.postTime/self.frameTime * 100)})
	local otherTime = self.frameTime - self.pushTime - self.runTime - self.drawCalcTime - self.drawPointTime - self.postTime - self.ifsTime
	table.insert(status, {"O", string.format("%d (%.1f%%)", otherTime, otherTime/self.frameTime * 100)})
	table.insert(status, {"IPS", string.format("%.5f mil", fps * self.size * self.size * (self.drawIterCount + self.seedIterCount)/1000000.0)})
	table.insert(status, {"Drawn", string.format("%.5f mil", self.size * self.size * (self.drawIterCount + 1)/1000000.0)})
	table.insert(status, {"Rpt", (self.lockSpin and "true") or "false"} )
	table.insert(status, {"FPS", math.floor(fps)})
	
	--graphicsSystem:useTexture(Texture())
	if self.drawStatus then
		graphicsSystem:drawToTexture(self.downsample.out:toTexture())
		drawStatus( Color( 0.0, 0.0, 0.0, 1.0 ), Color( 1.0, 1.0, 1.0, 1.0 ), unpack(status))
		graphicsSystem:drawToTexture(Texture())
	end

	return self.downsample.out:toTexture()
end

function IFS:input()
	local inputSystem = InputSystem.instance()
	
	if inputSystem:getKeyState(InputSystem.K_r) == InputSystem.released then
			self.sFlame = FlameGenerator.random()
	self.eFlame = FlameGenerator.random()
	self.holding = true
		self.time = 0
	end
	
	if inputSystem:getKeyState(InputSystem.K_SPACE) == InputSystem.released then
		self.paused = not self.paused
	end
	
	if inputSystem:getKeyState(InputSystem.K_a) == InputSystem.released then
		self.eFlame:randomizeAffine()
	end
	
	if inputSystem:getKeyState(InputSystem.K_s) == InputSystem.released then
		self.drawStatus = not self.drawStatus
	end
	
	if inputSystem:getKeyState(InputSystem.K_c) == InputSystem.released then
		self.eFlame:randomizePallette()
	end
	
	if inputSystem:getKeyState(InputSystem.K_v) == InputSystem.released then
		self.eFlame:randomizeParameters()
	end
	
	if inputSystem:getKeyState(InputSystem.K_t) == InputSystem.released then
		self.eFlame:randomizeTrans()
	end
	
	if inputSystem:getKeyState(InputSystem.K_l) == InputSystem.released then
		self.lockSpin = not self.lockSpin
	end
end

function getProgram()
	self = IFS()
	return self
end
