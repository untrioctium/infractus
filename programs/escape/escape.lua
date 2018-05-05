assert(loadfile("scripts/drawStatus.lua"))()

class 'Escape' (InfractusProgram)

function Escape:__init()
	InfractusProgram.__init(self)
end

function Escape:loadAllEscapes()
	self.escapes = {}

	local escapesPath = self:getWorkingDirectory() .. "escapes.xml"
	ConfigSystem.instance():loadConfig(escapesPath)
	local escapes = ConfigSystem.instance():getConfigPtree(escapesPath):getChild("escapes")
	
	for k,v in pairs(escapes:children()) do
		local escape = {}

		local dims = {"x", "y"}
		
		for k,f in pairs(dims) do
			escape[f] = v.second:get(f)
		end
		
		escape.name = v.second:get("<xmlattr>.name")
		escape.radius = tonumber(v.second:get("<xmlattr>.radius"))
		escape.maxIterations = tonumber(v.second:get("<xmlattr>.max"))
		
		escape.parameters = {}
		
		for i,p in pairs(v.second:getChild("parameters"):children()) do
			local info = p.second:getChild("<xmlattr>")
			local parameter = {}
			parameter.name = info:get("name")
			parameter.index = i-1
			parameter.min = info:get("min")
			parameter.max = info:get("max")
			escape.parameters[parameter.index] = parameter
		end
		
		self.escapes[escape.name] = escape
	end
end

function Escape:useEscape( name, unbind )
	local escape = self.escapes[name]
	local source = io.open(self:getWorkingDirectory() .. "escape.frag"):read("*a")
	local exp = {"x", "y"}

	if unbind ~= false then self.calc:unbindEngine() end
	
	for k,v in pairs(exp) do
		local sourceString = escape[v]
		for index,v in pairs(escape.parameters) do
			sourceString = string.gsub( sourceString, "%[" .. v.name .. "%]", 
					string.format("p[%d]", index))
		end
		
		for k,v in pairs(exp) do
			sourceString = string.gsub( sourceString, string.format("%%[%s%%]", v), 
				string.format("lastValue.%s", v))
		end
		print(v .. ": " .. sourceString)
		source = string.gsub( source, string.format("%%$%s%%$", v), sourceString)
	end
	
	source = string.gsub( source, "%[cx%]", "startValue.x")
	source = string.gsub( source, "%[cy%]", "startValue.y")
	
	self.parameters = {}
	for name,v in pairs(escape.parameters) do
		self.parameters[v.index] = {}
		self.parameters[v.index].endVal = math.random() * (v.max - v.min) + v.min
	end				

	self.activeEscape = escape
	self.maxIterations = escape.maxIterations
	self.minIterations = 0
	self.calc:setProgramLocationMemoryString("GLSLComputeEngine", source)
	self.calc:bindEngine(self.engine)
end

function Escape:processTransitions( dt )
	if self.time <= 0 then
		for k,v in pairs(self.activeEscape.parameters) do
			local param = self.parameters[k]
			param.startVal = param.endVal
			param.val = param.startVal
			param.endVal = math.random() * (v.max - v.min) + v.min
		end
		self.time = self.transTime
	else
		for k,v in pairs(self.parameters) do
			v.val = self.smoothstep( 1 - self.time/self.transTime, v.startVal, v.endVal )
		end
		self.time = self.time - dt
	end
end

function Escape:updateTransform( func, unbind )
	if unbind ~= false then self.transform:unbindEngine() end
	local source = io.open("programs/transform.frag"):read("*a")
	source = string.gsub(source, "%$TRANSFORM%$", func)
	self.transform:setProgramLocationMemoryString("GLSLComputeEngine", source)
	self.transform:bindEngine(self.engine)
end

function Escape:updatePlane( xCenter, yMin, yMax )
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

function Escape:init( usingTextureSource, width, height )

	local graphicsSystem = GraphicsSystem.instance()
	local screenInfo = graphicsSystem:getScreenInfo()

	self:loadAllEscapes()

	self.interactive = true
	self.engine = PluginManager.instance():loadPlugin("GLSLComputeEngine"):toEngine()
	
	self.transform = Program()
	self.transform:load("programs/transform.program")
	
	print("bind transform")
	self:updateTransform( "z", false )
	print("done")
		
	self.calc = Program()
	self.calc:setWorkingDirectory(self:getWorkingDirectory())
	self.calc:load("escape.program")
	
	print("bind escape")
	self:useEscape( "mandelbrot", false )
	print("done")
	
	self.color = Program()
	self.color:setWorkingDirectory( self:getWorkingDirectory() )
	self.color:load("escape.color.program")
	
	print("bind color")
	self.color:bindEngine( self.engine )
	print("done")
	
	self:setBufferTexture(GraphicsSystem.instance():createBufferTexture(0,0))
	
	self.transform:allocateStorage( screenInfo.w, screenInfo.h, Program.output, 0 )
	self.calc:allocateStorage( screenInfo.w, screenInfo.h, Program.output, 0 )
	self.color:allocateStorage( screenInfo.w, screenInfo.h, Program.output, 0 )
	self.outputTexture = self.color:getStorage( Program.output, 0 ):toTexture()
	
	self.calc:setStorage( Program.input, 0, self.transform:getStorage( Program.output, 0 ) )
	self.color:setStorage( Program.input, 0, self.calc:getStorage( Program.output, 0 ) )
	
	self.highlight = 0
	
	self.aspect = screenInfo.w/screenInfo.h
	self.zoom = 0
	self:updatePlane( 2, -4, 0 )
	
	self.paused = false
	self.time = 0
	self.transTime = 1000
	
	self.orbitRadius = 0
	self.orbitMax = 1
	self.orbitCycleTime = 5000
	self.orbitTime = 0
	
	self.hueTime = 0
	self.hueCycleTime = 5000
	
	self.iterSmooth = .01
	
	self.smoothstep = function( weight, edge0, edge1 )
						weight = weight * weight * (3 - 2 * weight)
						return edge0 * (1 - weight) + edge1 * weight
						end
end

function Escape:input()

	local inputSystem = InputSystem.instance()
	local screenInfo = GraphicsSystem.instance():getScreenInfo()

	if inputSystem:getKeyState( InputSystem.K_SPACE ) == InputSystem.released then
		self.paused = not self.paused
	end

	if inputSystem:getMouseButtonState(2) == InputSystem.down then
		local mouseMove = inputSystem:getMouseMotion()
		local dx = -self.xLength/screenInfo.w * mouseMove.x
		local dy = -self.yLength/screenInfo.h * mouseMove.y
		self:updatePlane( self.xCenter + dx, self.yMin + dy, self.yMax + dy )
	end
	
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

function Escape:run( dt, scale )
	
	if not self.paused then self:processTransitions(dt) end
	
	self.hueTime = self.hueTime + dt
	if self.hueTime > self.hueCycleTime then self.hueTime = self.hueTime - self.hueCycleTime end
	
	local p = self.calc:getParameter("p")
	for index,v in pairs(self.parameters) do
		p:at(index):setFloat(v.val)
	end
	
	self.calc:getParameter("maxIterations"):setInt( self.activeEscape.maxIterations )
	self.calc:getParameter("escapeRadius"):setFloat( self.activeEscape.radius )
	self.transform:run()
	self.calc:run()
	
	--local reduce = self.engine:reduce( self.calc:getStorage( Program.output, 0 ), Engine.Maximum )
	--self.frameMinIter = self.activeEscape.maxIterations - reduce.y
	--self.frameMaxIter = reduce.x
	
	--self.maxIterations = self.maxIterations * (1 - self.iterSmooth) + self.frameMaxIter * self.iterSmooth
	--self.minIterations = self.minIterations * (1 - self.iterSmooth) + self.frameMinIter * self.iterSmooth
	
	self.color:getParameter("highlightNonConverge"):setInt( self.highlight )
	self.color:getParameter("maxIterations"):setInt( self.activeEscape.maxIterations )
	--self.color:getParameter("actualMaxIterations"):setFloat( self.maxIterations )
	--self.color:getParameter("actualMinIterations"):setFloat( self.minIterations )
	self.color:getParameter("hueOffset"):setFloat( self.hueTime/self.hueCycleTime )
	self.color:run()

end

function Escape:draw()
	local graphicsSystem = GraphicsSystem.instance()
	local screenInfo = graphicsSystem:getScreenInfo()
	
	graphicsSystem:clearToColor( Color( 0.0, 0.0, 0.0, 0.0 ) )
	graphicsSystem:useTexture(self.outputTexture)
	graphicsSystem:drawRectangle(Point(0,0), Point(screenInfo.w, screenInfo.h))
	graphicsSystem:useTexture(Texture())

	--if self.interactive then
	--	local status = {}
	--	for index,v in pairs(self.parameters) do
	--		table.insert(status, {self.activeEscape.parameters[index].name, string.format("%.5f", v.val)})
	--	end
--
	--	table.insert(status, {"fMaxIter", self.frameMaxIter})
	--	table.insert(status, {"fMinIter", self.frameMinIter})
	--	table.insert(status, {"maxIter", self.maxIterations})
	--	table.insert(status, {"minIter", self.minIterations})
	--	table.insert(status, {"time", self.time})
	--	table.insert(status, {"FPS", math.floor(graphicsSystem:getFPS())} )
	--	drawStatus( Color( 1.0, 1.0, 1.0, 1.0 ), Color( 1.9, 0.0, 0.0, 1.0 ), unpack(status) )
	--end
end

function getProgram()
	self = Escape()
	return self
end
