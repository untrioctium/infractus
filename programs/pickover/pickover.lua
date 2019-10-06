Pickover = {}

function draw_status( ... )
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
	
	graphicsSystem:setDrawColor( Color(.5,.5,.5, 1.0) )
	graphicsSystem:drawRectangle( Point(0, starty), Point( maxFinalLength * textw, starty + total * texth) )
	graphicsSystem:resetDrawColor()
	
	for index,value in ipairs(arg) do
		graphicsSystem:drawText( value[1], Point(0, starty) )
		starty = starty + texth
	end
end

--this = 0

function Pickover:loadAttractors()
	self.attractors = {}

	local attractorsPath = self:getWorkingDirectory() .. "attractors.xml"
	ConfigSystem.instance():loadConfig(attractorsPath)
	attractors = ConfigSystem.instance():getConfigPtree(attractorsPath):getChild("attractors")
	
	for k,v in pairs(attractors:children()) do
		local attractor = {}
		
		attractor.code = v:get("code")
		
		attractor.parameters = {}
		
		for i,p in pairs(v:getChild("parameters"):children()) do
			local info = p:getChild("<xmlattr>")
			local parameter = {}
			parameter.name = info:get("name")
			parameter.index = i-1
			parameter.min = info:get("min")
			parameter.max = info:get("max")
			attractor.parameters[parameter.name] = parameter
		end
		
		self.attractors[v:get("<xmlattr>.name")] = attractor
	end
end

function Pickover:useAttractor( name, unbind )

	local attractor = self.attractors[name]
	local source = io.open(self:getWorkingDirectory() .. "pickover.frag"):read("*a")
	local exp = {"x", "y", "z"}

	if unbind ~= false then self.pickover:unbindEngine() end
	
	local sourceString = attractor.code
	for k,v in pairs(attractor.parameters) do
		sourceString = string.gsub( sourceString, "%[" .. k .. "%]", 
				string.format("param[%d]", v.index))
	end
		
	sourceString = string.gsub( sourceString, "%[(%w+)%]", self.subtable)
	print(sourceString)
	source = string.gsub( source, "%$SOURCE%$", sourceString)
	
	self.parameters = {}
	for k,v in pairs(attractor.parameters) do
		self.parameters[k] = {}
		self.parameters[k].endVal = math.random() * (v.max - v.min) + v.min
	end				

	self.activeAttractor = name
	self.pickover:setProgramLocationMemoryString("GLSLComputeEngine", source)
	self.pickover:bindEngine(self.engine)
end

function Pickover:processTransitions( dt )

	if self.time <= 0 then
		local paramChance = math.random()
		if paramChance < .2 then paramChance = .2 end
		
		local randParameters = {}
		local constParameters = {}
	
		for k,v in pairs(self.attractors[self.activeAttractor].parameters) do
			if math.random() <= paramChance then
				table.insert(randParameters, k)
			else
				table.insert(constParameters, k)
			end
		end
		if #randParameters == 0 then
			self:processTransitions(dt)
			return
		end
		
		for k,v in pairs(randParameters) do
			local def = self.attractors[self.activeAttractor].parameters[v]
			local param = self.parameters[v]
			param.startVal = param.endVal
			param.val = param.startVal
			param.endVal = math.random() * (def.max - def.min) + def.min
		end
		for k,v in pairs(constParameters) do
			local param = self.parameters[v]
			param.startVal = param.endVal
			param.val = param.endVal
			param.endVal = param.endVal
		end
		self.time = self.TRANS_TIME
	else
		for k,v in pairs(self.parameters) do
			v.val = self.smoothstep( 1 - self.time/self.TRANS_TIME, v.startVal, v.endVal )
		end
		self.time = self.time - dt
	end
end

function Pickover:init( interactive )

	local graphicsSystem = GraphicsSystem.instance()
	graphicsSystem:setPointSize(1)

	self.interactive = interactive
	self.engine = PluginManager.instance():loadPlugin("GLSLComputeEngine"):toEngine()

	print("in init")
	self.pickover = Program()
	self.pickover:setWorkingDirectory(self:getWorkingDirectory());
	self.pickover:load("pickover.program");
	--self.pickover:bindEngine(self.engine)
	print("done bind")
	
	self.paused = false
	self.doConvolve = true
	self.doTonemap = true
	self.calcAverage = false
	self.drawGrid = false
	self.showStatus = false
	self.size = 2048
	self.iterCount = 20
	self.escapeRadius = 10
	self.preGamma = 1.75
	self.postGamma = 1
	self.alpha = .25
	self.exposure = 1
	self.blitAlpha = 1
	self.lenPow = 1
	self.anglePow = .75
	self.clearColor = Color( 0.0, 0.0, 0.0, 1.0 )
	self.TRANS_TIME = 20000
	self.ROT_SPEED = 60000
	self.time = 0

	self.smoothstep = function( weight, edge0, edge1 )
						weight = weight * weight * (3 - 2 * weight)
						return edge0 * (1 - weight) + edge1 * weight
						end
	
	self.subtable = { nx="next.x", ny="next.y", nz="next.z", nv = "next", x="cur.x", y="cur.y", z="cur.z", v="cur" }
	
	self.center = Point( 0, 0 )
	self.centerWeight =.02
	
	self:loadAttractors()
	self:useAttractor("pickover", false)
	
	self.hueTime = 0
	self.hueCycleTime = 10000
	self:setBufferTexture(graphicsSystem:createBufferTexture(0,0))
	self.bufferStorage = self.engine:fromTexture( self:getBufferTexture() )
	
	self.convolve = Program()
	self.convolve_out = self.engine:fromTexture( graphicsSystem:createBufferTexture(0,0) )
	self.convolve:setWorkingDirectory("./programs/")
	self.convolve:load("convolve.program");
	self.convolve:bindEngine(self.engine);
	self.convolve:setStorage( Program.input, 0, self.bufferStorage )
	self.convolve:setStorage( Program.output, 0, self.convolve_out )
	self.convolve_kernel = {1,2,1,2,4,2,1,2,1} -- gaussian kernel
	
	self.tonemap = Program();
	self.tonemap_out = self.engine:fromTexture( graphicsSystem:createBufferTexture(0,0) )
	self.tonemap:setWorkingDirectory("./programs/")
	self.tonemap:load("tonemap.program")
	self.tonemap:bindEngine(self.engine)
	self.tonemap:setStorage( Program.input, 0, self.bufferStorage )
	self.tonemap:setStorage( Program.output, 0, self.tonemap_out )	
	self.brightAvg = 5
	self.brightWeight = .005
	
	self.blurBuffer = graphicsSystem:createBufferTexture(0,0)
	self.blurFactor = 2
	
	graphicsSystem:drawToTexture(self.blurBuffer)
	graphicsSystem:clearToColor(self.clearColor)
	graphicsSystem:drawToTexture(Texture())
	
	local screenInfo = graphicsSystem:getScreenInfo()
	
	self.rot = Point( 0, 0, 0 )
	self.trans = Point( 0, 0, -6 )
	self.scale = Point( screenInfo.w/screenInfo.h, 1, 1 )
	
	self.pickover:allocateStorage( self.size, self.size, Program.output, 0 )
	self.pickover:allocateStorage( self.size, self.size, Program.output, 1 )
	
	print("done init")
end

function Pickover:draw()
	local graphicsSystem = GraphicsSystem.instance()
	local screenInfo = graphicsSystem:getScreenInfo()
	
	graphicsSystem:clearToColor(self.clearColor)
	
	graphicsSystem:pushDrawingMode()
	graphicsSystem:setDrawingMode( GraphicsSystem.ThreeD )

	if self.calcAverage then
		self.trans.x = -self.center.x
		self.trans.y = -self.center.y
	else
		self.trans.x = 0
		self.trans.y = 0
	end

	graphicsSystem:translateView( self.trans )
	graphicsSystem:scaleView( self.scale )
	graphicsSystem:rotateView( self.rot.x, Point(1,0,0))
	graphicsSystem:rotateView( self.rot.y, Point(0,1,0))
	graphicsSystem:rotateView( self.rot.z, Point(0,0,1))
	
	graphicsSystem:setBlendingMode( GraphicsSystem.Additive )
	graphicsSystem:drawPointArray( self.pickover:getStorageVal( Program.output, 0 ):toTexture(), 
								self.pickover:getStorageVal( Program.output, 1 ):toTexture() )
	graphicsSystem:setBlendingMode( GraphicsSystem.Normal )
	
	if self.drawGrid then
		for i=-3,3 do
			for j=-3,3 do
				if math.abs(j) < 3 then
				graphicsSystem:drawLine( Point( i, j, -2 ), Point( i, j, 2 ))
				graphicsSystem:drawLine( Point( i, -3, j ), Point( i, 3, j ))
				graphicsSystem:drawLine( Point( -3, i, j ), Point( 3, i, j ))
				end
			end 
		end
	
		-- draw reference axises
		graphicsSystem:setDrawColor( Color( 1.0, 0.0, 0.0, 1.0 ) )
		graphicsSystem:drawLine( Point( self.center.x, self.center.y, -1 ), Point( self.center.x, self.center.y, 1 ))
		graphicsSystem:setDrawColor( Color( 0.0, 1.0, 0.0, 1.0 ) )
		graphicsSystem:drawLine( Point( self.center.x, -2, 0 ), Point( self.center.x, 2, 0 ))
		graphicsSystem:setDrawColor( Color( 0.0, 0.0, 1.0, 1.0 ) )
		graphicsSystem:drawLine( Point( -2, self.center.y, 0 ), Point( 2, self.center.y, 0 ))
		graphicsSystem:resetDrawColor()
	end
	
	graphicsSystem:popDrawingMode()
end

function Pickover:input()
	local inputSystem = InputSystem.instance()
	
	if inputSystem:getKeyState( InputSystem.K_SPACE ) == InputSystem.released then self.paused = not self.paused end
	if inputSystem:getKeyState( InputSystem.K_c ) == InputSystem.released then self.doConvolve = not self.doConvolve end
	if inputSystem:getKeyState( InputSystem.K_t ) == InputSystem.released then self.doTonemap = not self.doTonemap end
	if inputSystem:getKeyState( InputSystem.K_r ) == InputSystem.released then self.calcAverage = not self.calcAverage end
	if inputSystem:getKeyState( InputSystem.K_g ) == InputSystem.released then self.drawGrid = not self.drawGrid end
	if inputSystem:getKeyState( InputSystem.K_s ) == InputSystem.released then self.showStatus = not self.showStatus end
	
	if inputSystem:getMouseButtonState(3) == InputSystem.down then
		motion = inputSystem:getMouseMotion()
		self.rot.y = self.rot.y + motion.x
		if self.rot.y > 360 then self.rot.y = self.rot.y - 360 end
		
		self.rot.x = self.rot.x + motion.y
		if self.rot.x > 360 then self.rot.x = self.rot.x - 360 end
	end
	
	local scroll = inputSystem:getMouseWheelScroll()
	self.trans.z = self.trans.z + scroll
end

function Pickover:run(dt)
	
	local drot = dt * 360/self.ROT_SPEED
	self.rot.y = self.rot.y + drot
	if self.rot.y > 360 then self.rot.y = 0 end
	self.rot.x = self.rot.x + drot
	if self.rot.x > 360 then self.rot.x = 0 end
	
	if not self.paused then self:processTransitions(dt) end
	
	local p = self.pickover:getParameter("param")
	for k,v in pairs(self.parameters) do
		p:at(self.attractors[self.activeAttractor].parameters[k].index):setFloat(v.val)
	end
	
	self.pickover:getParameter("escapeRadius"):setFloat(self.escapeRadius);
	self.pickover:getParameter("alpha"):setFloat(self.alpha);
	self.pickover:getParameter("lenPow"):setFloat(self.lenPow);
	self.pickover:getParameter("anglePow"):setFloat(self.anglePow);
	self.pickover:getParameter("iterCount"):setInt(self.iterCount);
	self.pickover:getParameter("hueOffset"):setFloat((self.hueTime % self.hueCycleTime)/self.hueCycleTime)
	self.hueTime = self.hueTime + dt
	self.pickover:getParameter("time"):setInt(self.hueTime)

	self.pickover:run()
	
	if self.calcAverage then
		local coordMin, coordMax, coordCenter
		coordMin = self.engine:reduce( self.pickover:getStorage( Program.output, 0 ), Engine.Minimum )
		coordMax = self.engine:reduce( self.pickover:getStorage( Program.output, 0 ), Engine.Maximum )
		coordCenter = Point( (coordMax.x - coordMin.x)/2 + coordMin.x, (coordMax.y - coordMin.y)/2 + coordMin.y )

		self.center.x = self.center.x * (1 - self.centerWeight) + self.centerWeight * coordCenter.x
		self.center.y = self.center.y * (1 - self.centerWeight) + self.centerWeight * coordCenter.y
	end
end

function Pickover:getOutput()
	local graphicsSystem = GraphicsSystem.instance()
	local screenInfo = graphicsSystem:getScreenInfo()

	local status = {}
	for k,v in pairs(self.parameters) do
		table.insert(status, {k, string.format("%.5f", v.val)})
	end

	table.insert(status, {"time", self.time})
	table.insert(status, {"iters per sec", string.format("%.5f mil", self.size * self.size * self.iterCount/1000000) } )
	table.insert(status, {"FPS", math.floor(graphicsSystem:getFPS())} )
	
	if self.doConvolve then
	
		-- perform convolution on the framebuffer
		self.convolve:setStorage( Program.input, 0, self.bufferStorage )
		convolveKernel = self.convolve:getParameter("convolveKernel")
		for i=1,9 do
			convolveKernel:at(i-1):setFloat(self.convolve_kernel[i])
		end

		self.convolve:run()

	
		-- blit the convolved result back to the framebuffer
		graphicsSystem:drawToTexture( self:getBufferTexture()  )
		graphicsSystem:setDrawColor( Color( 1.0, 1.0, 1.0, self.blitAlpha ) )
		graphicsSystem:setBlendingMode( GraphicsSystem.Additive )
		graphicsSystem:useTexture( self.convolve_out:toTexture() )
		graphicsSystem:drawRectangle( Point(0,screenInfo.h), Point(screenInfo.w, 0) )
		graphicsSystem:useTexture( Texture() )
		graphicsSystem:setBlendingMode( GraphicsSystem.Normal )
		graphicsSystem:resetDrawColor()
		graphicsSystem:drawToTexture( Texture() )
	end
	
	if self.doTonemap then
		--local maxBright = self.engine:reduce( self.bufferStorage, Engine.Maximum)
		--if tostring(maxBright.w) ~= "inf" and maxBright.w > 1 then
		--	self.brightAvg = self.brightAvg * (1 - self.brightWeight) + self.brightWeight * math.log(maxBright.w)
		--end
		--table.insert(status, 1, {"curbright", string.format("%.3f", math.log(maxBright.w))})
		--table.insert(status, 1, {"bright", string.format("%.3f", self.brightAvg)})
		-- perform tonemapping on the framebuffer
		--self.tonemap:getParameter("gamma"):setFloat(self.gamma)
		--self.tonemap:getParameter("exposure"):setFloat(self.exposure)
		--self.tonemap:getParameter("denom"):setFloat(math.exp(self.brightAvg))
		--self.tonemap:getParameter("alpha"):setFloat(1.0)
		--self.tonemap:getParameter("useAlpha"):setInt(0)
		--self.tonemap:run()
		self.tonemap:getParameter("preGamma"):setFloat(self.preGamma)
	self.tonemap:getParameter("postGamma"):setFloat(self.postGamma)
	self.tonemap:getParameter("exposure"):setFloat(self.exposure)
	self.tonemap:getParameter("alpha"):setFloat(1.0)
	self.tonemap:getParameter("useAlpha"):setInt(1)
	self.tonemap:run()
		-- overwrite the framebuffer with the tonemapped result
		graphicsSystem:drawToTexture( self:getBufferTexture()  )
		graphicsSystem:clearToColor(Color(0.0, 0.0, 0.0, 0.0))
		graphicsSystem:resetDrawColor()
		graphicsSystem:useTexture( self.tonemap_out:toTexture() )
		graphicsSystem:drawRectangle( Point(0,screenInfo.h), Point(screenInfo.w, 0) )
		graphicsSystem:useTexture( Texture() )
		graphicsSystem:drawToTexture( Texture() )
		
	end
	
	if self.calcAverage then
		table.insert( status, 1, {"center", string.format("(%.4f, %.4f)", self.center.x, self.center.y)})
	end
	
	--graphicsSystem:drawToTexture( self:getBufferTexture() )
	--graphicsSystem:useTexture(self.blurBuffer)
	--graphicsSystem:setDrawColor( Color( 1.0, 1.0, 1.0, 1 - math.pow(10, -self.blurFactor)) )
	--graphicsSystem:drawRectangle( Point(0,screenInfo.h), Point(screenInfo.w, 0) )
	--graphicsSystem:useTexture( Texture() )
	--graphicsSystem:drawToTexture( Texture() )
	
	--graphicsSystem:drawToTexture(self.blurBuffer)
	--graphicsSystem:useTexture(self:getBufferTexture())
	--graphicsSystem:resetDrawColor()
	--graphicsSystem:drawRectangle( Point(0,screenInfo.h), Point(screenInfo.w, 0) )
	--graphicsSystem:useTexture( Texture() )
	--graphicsSystem:drawToTexture( Texture() )
	
	--if self.showStatus then
	--	graphicsSystem:drawToTexture( self:getBufferTexture() )
	--	--draw_status(unpack(status))
	--	graphicsSystem:drawToTexture( Texture() )
	--end
	
	return self:getBufferTexture()
end

return Pickover