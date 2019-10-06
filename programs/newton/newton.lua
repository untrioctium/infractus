NewtonFunction = {}
Newton = {}

function NewtonFunction.random( xMin, xMax, yMin, yMax, pMin, pMax )
	local func = {}
	
	func.roots = {}
	func.rootCount = math.random(2, 8)
	
	local cRad = math.random() * .5
	local cAng = math.random() * math.pi * 2
	
	func.coeff = vec2( cRad * math.cos(cAng) + 1, cRad * math.sin(cAng) )
	
	for i=1, func.rootCount do
		func.roots[i] = vec4( 
							math.random() * (xMax - xMin) + xMin, 
							math.random() * (yMax - yMin) + yMin,
							math.random() * (pMin - pMax) + pMin,
							math.random() * (pMin - pMax) + pMin )
	end
	
	for i=func.rootCount + 1, 8 do
		func.roots[i] = vec4( 0.0, 0.0, 0.0, 0.0 )
	end
	
	return func
end

function Newton:updateTransform( func, unbind )
	if unbind ~= false then self.transform:unbindEngine() end
	local source = io.open("programs/transform.frag"):read("*a")
	source = string.gsub(source, "%$TRANSFORM%$", func)
	self.transform:setProgramLocationMemoryString("GLSLComputeEngine", source)
	self.transform:bindEngine(self.engine)
end

function Newton:init( interactive )
	print("start init")
	self.gamma = vec3( 1.0, 1.0, 1.0 )

	self.maxIterations = 20
	self.precision = .1
	
	self.time = 0
	self.transTime = 30000
	self.holdTime = 0

	self.paused = false
	self.holding = false
	self.hueTimeOffset = 0
	self.blend = .75

	self.zoom = 0

	self.engine = PluginManager.instance():loadPlugin("GLSLComputeEngine"):toEngine()
	
	self.transform = Program()
	self.transform:load("programs/transform.program")
	self:updateTransform("z", false)
	
	self.calc = Program()
	self.calc:setWorkingDirectory(self:getWorkingDirectory())
	self.calc:load("newton.program")
	
	self.color = Program()
	self.color:setWorkingDirectory(self:getWorkingDirectory())
	self.color:load("newton.color.program")
	
	self.calc:bindEngine(self.engine)
	self.color:bindEngine(self.engine)
	
	self:setBufferTexture(GraphicsSystem.instance():createBufferTexture(0,0))
	
	local screenInfo = GraphicsSystem.instance():getScreenInfo()
	
	self.transform:allocateStorage( screenInfo.w, screenInfo.h, Program.output, 0 )
	self.calc:allocateStorage( screenInfo.w, screenInfo.h, Program.output, 0 )
	self.color:allocateStorage( screenInfo.w, screenInfo.h, Program.output, 0 )

	self.calc:setStorage( Program.input, 0, self.transform:getStorage(Program.output, 0))
	self.color:setStorage( Program.input, 0, self.calc:getStorage(Program.output, 0) )

	print("\tdone.")

	self.yMin = -1
	self.yMax = 1
	self.yLength = self.yMax - self.yMin
	self.xLength = self.yLength * screenInfo.w/screenInfo.h
	self.xMin = -self.xLength/2
	self.xMax = self.xLength/2
	self.xCenter = 0
	self.aspect = screenInfo.w/screenInfo.h

	self.pMin = 1
	self.pMax = 1.25

	self.sFunc = NewtonFunction.random( self.xMin, self.xMax, self.yMin, self.yMax, self.pMin, self.pMax )
	self.eFunc = NewtonFunction.random( self.xMin, self.xMax, self.yMin, self.yMax, self.pMin, self.pMax )
	
	self.smoothstep = function( weight, edge0, edge1 )
						weight = weight * weight * (3 - 2 * weight)
						return edge1 * (1 - weight) + edge0 * weight
						end
	
	self.transform:getParameter("X_MIN"):setFloat(self.xMin)
	self.transform:getParameter("X_MAX"):setFloat(self.xMax)
	self.transform:getParameter("Y_MIN"):setFloat(self.yMin)
	self.transform:getParameter("Y_MAX"):setFloat(self.yMax)

	print("done")

end

function Newton:updatePlane( xCenter, yMin, yMax )
	self.yMin = yMin
	self.yMax = yMax
	self.yLength = yMax - yMin
	
	self.xCenter = xCenter
	self.xLength = self.yLength * self.aspect
	self.xMin = xCenter - self.xLength/2
	self.xMax = xCenter + self.xLength/2
	
	local parameters = {}
	parameters["X_MIN"] = self.xMin
	parameters["X_MAX"] = self.xMax
	parameters["Y_MIN"] = self.yMin
	parameters["Y_MAX"] = self.yMax
	
	for param,value in pairs(parameters) do
		self.transform:getParameter( param ):setFloat(value)
	end
end

function Newton:input()

	local inputSystem = InputSystem.instance()
	local screenInfo = GraphicsSystem.instance():getScreenInfo()

	if inputSystem:getKeyState(InputSystem.K_SPACE) == InputSystem.released then
		self.paused = not self.paused
	end
	
	if inputSystem:getKeyState(InputSystem.K_r) == InputSystem.released then
		self.sFunc = NewtonFunction.random( self.xMin, self.xMax, self.yMin, self.yMax, self.pMin, self.pMax )
		self.eFunc = NewtonFunction.random( self.xMin, self.xMax, self.yMin, self.yMax, self.pMin, self.pMax )
		self.time = self.transTime
	end
		
	if inputSystem:getMouseButtonState(1) == InputSystem.down then
		local mouseMove = inputSystem:getMouseMotion()
		local dx = -self.xLength/screenInfo.w * mouseMove.x
		local dy = -self.yLength/screenInfo.h * mouseMove.y
		self:updatePlane( self.xCenter + dx, self.yMin + dy, self.yMax + dy )
	end

	if not self.ignoreZoom then
		local scroll = inputSystem:getMouseWheelScroll()

		if scroll ~= 0 then
			self.zoom = self.zoom + scroll * 7;
		end

		if self.zoom ~= 0 then
			local factor
			if self.zoom < 0 then factor = 1.005 else factor = .995 end
			factor = math.pow( factor, math.abs(self.zoom/7) )
			local yCenter = (self.yMax - self.yMin)/2 + self.yMin
			local yLength = factor * self.yLength
			local yMin = yCenter - yLength/2
			local yMax = yCenter + yLength/2
			self.zoom = self.zoom + -factor * math.abs(self.zoom)/self.zoom
			if math.abs(self.zoom) < 1 then self.zoom = 0 end
			self:updatePlane( self.xCenter, yMin, yMax )
		end
	end
end

function Newton:processTransitions(dt)
	self.time = self.time - dt
	
	if self.time <= 0 and not self.holding then
		self.sFunc = self.eFunc
		self.eFunc = NewtonFunction.random( self.xMin, self.xMax, self.yMin, self.yMax, self.pMin, self.pMax )
		
		self.holding = true
	end

	if self.time <= -self.holdTime then
		self.holding = false
		self.time = self.transTime
	end
	
	local maxSize = (#self.sFunc.roots < #self.eFunc.roots and #self.eFunc.roots) or #self.sFunc.roots
	self.calc:getParameter("functionSize"):setInt(maxSize)
	
	local weight = self.time/self.transTime
	if weight < 0 then weight = 1 end
	
	local func = self.calc:getParameter("function")
	for i=1,8 do
		local blend = vec4()
		blend.x = self.smoothstep( weight, self.sFunc.roots[i].x, self.eFunc.roots[i].x )
		blend.y = self.smoothstep( weight, self.sFunc.roots[i].y, self.eFunc.roots[i].y )
		blend.z = self.smoothstep( weight, self.sFunc.roots[i].z, self.eFunc.roots[i].z )
		blend.w = self.smoothstep( weight, self.sFunc.roots[i].w, self.eFunc.roots[i].w )
		func:at(i - 1):setVec4( blend )
	end
	
	local coeff = vec2()
	coeff.x = self.smoothstep( weight, self.sFunc.coeff.x, self.eFunc.coeff.x )
	coeff.y = self.smoothstep( weight, self.sFunc.coeff.y, self.eFunc.coeff.y )
	self.calc:getParameter("coeff"):setVec2(coeff)
end

function Newton:run(dt)

	self.delta = (self.paused and 0) or dt
	self:processTransitions(self.delta)
	
	self.calc:getParameter("requiredPrecision"):setFloat(self.precision)
	self.calc:getParameter("maxIterations"):setInt(self.maxIterations)
	self.calc:getParameter("t"):setFloat(0)
	
	self.color:getParameter("requiredPrecision"):setFloat(self.precision)
	self.color:getParameter("maxIterations"):setInt(self.maxIterations)
	self.color:getParameter("hueOffset"):setFloat(0)
	self.color:getParameter("blend"):setFloat(self.blend)
	self.color:getParameter("gamma"):setVec3(self.gamma)
	
	self.transform:run()
	self.calc:run()
	self.color:run()
end

function Newton:draw()
	local screenInfo = GraphicsSystem.instance():getScreenInfo()
	local graphicsSystem = GraphicsSystem.instance()

	graphicsSystem:useTexture(self.color:getStorageVal( Program.output, 0 ):toTexture())
	graphicsSystem:clearToColor( Color(0.0,0.0,0.0,1.0))
	graphicsSystem:drawRectangle( Point( 0, 0 ), Point( screenInfo.w, screenInfo.h ))
	graphicsSystem:useTexture(Texture())

	--local statusTable = {
	--	{"Mouse Coords", string.format("%.5f, %.5f", get_mouse_coords())},
	--	{"Max Iterations", maxIterations },
	--	{"Precision", precision },
	--	{"Blend", string.format("%.2f", blend) },
	--	{"Grabbed", (grabbedRoot == 0 and "none") or grabbedRoot }, 
	--	{"Period", string.format("%.5f", time/period)},
	--	{"Arg", string.format("%.5f", time/period * math.pi * 2) }, 
	--	{"Period Length", string.format("%.3f s", period/1000) }, 
	--	{"FPS", math.floor(graphicsSystem:getFPS()) }
	--	}

	--if orbitViewMode then
	--	mousePos = inputSystem:getMousePosition()
	--	x, y = pixel_to_coord( mousePos.x, mousePos.y )
	--	len, prevdelta, delta,count = draw_orbit(x,y)
	--	for k,v in pairs(orbitDataFuncs) do
	--		table.insert(statusTable, 1, {k,v()})
	--	end
	--	table.insert(statusTable, 1, {"Orbit Iters", count})
	--	table.insert(statusTable, 1, {"Orbit pDelta", prevdelta})
	--	table.insert(statusTable, 1, {"Orbit fDelta", delta})
	--	table.insert(statusTable, 1, {"Orbit Length", len})
	--end

	--draw_status( unpack(statusTable) )
end

return Newton