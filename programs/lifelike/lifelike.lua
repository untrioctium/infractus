class 'LifeLike' (InfractusProgram)

function LifeLike:__init()
	InfractusProgram.__init(self)
end

function LifeLike:setRule( name )
	self.rules.birth = self:computeRule(self.ruleTable[name].birth)
	self.rules.live = self:computeRule(self.ruleTable[name].live)
	self.rules.deathStates = self.ruleTable[name].deathStates
	self.rules.goodRandom = self.ruleTable[name].goodRandom
end

function LifeLike:init(interactive, width, height)
	self.engine = PluginManager.instance():loadPlugin("GLSLComputeEngine"):toEngine()
	print("in init")
	self.compute = Program()
	self.compute:setWorkingDirectory(self:getWorkingDirectory())
	self.compute:load("life.program")
	self.compute:bindEngine(self.engine)
	
	self.color = Program()
	self.color:setWorkingDirectory(self:getWorkingDirectory())
	self.color:load("life.color.program")
	self.color:bindEngine(self.engine)
	self.hueSpacing = 255
	
	self:setBufferTexture(GraphicsSystem.instance():createBufferTexture(2560,1440))
	self.bufferStorage = self.engine:fromTexture( self:getBufferTexture() )
	
	self.convolve = Program()
	self.convolve.out = self.engine:fromTexture( GraphicsSystem.instance():createBufferTexture(2560,1440) )
	self.convolve:setWorkingDirectory("./programs/")
	self.convolve:load("convolve.program");
	self.convolve:bindEngine(self.engine);
	self.convolve:setStorage( Program.input, 0, self.bufferStorage )
	self.convolve:setStorage( Program.output, 0, self.convolve.out )
	self.kernel = {1, 1, 1, 1, 1, 1, 1, 1, 1}
	
	self.ruleTable = {}
	self.ruleTable["coral"] = {{3}, {4,5,6,7,8}, 0}
	self.ruleTable["stains"] = {{3,6,7,8}, {2,3,5,6,7,8}, 0}
	self.ruleTable["coagulations"] = {{3,7,8}, {2,3,5,6,7,8}, 0}
	self.ruleTable["life"] = {{3}, {2,3}, 0}
	self.ruleTable["starwars"] = {{2}, {3,4,5}, 4}
	
	self.rules = {}

	self.fade = 0
	self.fadeTime = 0
	
	self.paused = false

	local screenInfo = GraphicsSystem.instance():getScreenInfo()

	self.w = 2560
	self.h = 1440
	
	self.compute:allocateStorage( self.w, self.h, Program.input, 0 )
	self.compute:allocateStorage( self.w, self.h, Program.output, 0 )
	self.color:allocateStorage( self.w, self.h, Program.output, 0 )
	
	local boardSize = self.w * self.h * 4
	self.localBoard = Array1Dfloat( boardSize )
	local setTime = ticks()
	self.localBoard:setAll(0)
	print(string.format("Took %d ms to clear board to zero.", ticks() - setTime))
	
	local rulesPath = self:getWorkingDirectory() .. "rules.xml"
	ConfigSystem.instance():loadConfig(rulesPath)
	rules = ConfigSystem.instance():getConfigPtree(rulesPath):getChild("rules")
	
	for k,v in pairs(rules:children()) do
		local rule = {}
		rule.live = v.second:get("<xmlattr>.survival")
		rule.birth = v.second:get("<xmlattr>.birth")
		rule.deathStates = tonumber(v.second:get("<xmlattr>.death_states"))
		rule.goodRandom = tonumber(v.second:get("<xmlattr>.good_random"))
		self.ruleTable[v.second:get("<xmlattr>.name")] = rule
	end
	
	self:setRule("coral")
	self:randomize( self.rules.goodRandom, 1, 1)
end

function LifeLike:randomize( genChance, valMin, valMax )

	local genTime = ticks()
	self.localBoard:setAll(0)
	local randCount = math.floor(self.w * self.h * genChance)
	local x,y, offset
	for i=1,randCount do
		x = math.random(0, self.w-1)
		y = math.random(0, self.h-1)
		self.localBoard:set( (y * self.w + x) * 4, math.random() * (valMax - valMin) + valMin )
	end
	genTime = ticks() - genTime
	local copyTime = ticks()
	self.compute:getStorageVal( Program.input, 0 ):copyFromArray( self.localBoard:array() )
	copyTime = ticks() - copyTime
	
	print(string.format("Took %d ms to generate, %d ms to copy to storage.", genTime, copyTime))
end

function LifeLike:draw()
	local graphicsSystem = GraphicsSystem.instance()
	local screenInfo = graphicsSystem:getScreenInfo()
	
	graphicsSystem:clearToColor( Color( 0.0, 0.0, 0.0, 0.0 ) )
	graphicsSystem:useTexture( self.color:getStorage(Program.output, 0):toTexture() )
	graphicsSystem:drawRectangle( Point(0,0), Point(screenInfo.w, screenInfo.h) )
	graphicsSystem:useTexture( Texture() )
end

function LifeLike:getOutput()
	self.convolve:setStorage( Program.input, 0, self.bufferStorage )
	convolveKernel = self.convolve:getParameter("convolveKernel")
	for i=1,9 do
		convolveKernel:at(i-1):setFloat(self.kernel[i])
	end

	self.convolve:run()
	return self.convolve.out:toTexture()
end

function LifeLike:input()
	local inputSystem = InputSystem.instance()
	
	if inputSystem:getKeyState( InputSystem.K_SPACE ) == InputSystem.released then self.paused = not self.paused end
	if inputSystem:getKeyState( InputSystem.K_r ) == InputSystem.released then self:randomize( self.rules.goodRandom, 1, 1 ) end
end

function LifeLike:computeRule( rule )
	local ret = 0
	for v in string.gmatch(rule, "%d") do
		ret = ret + math.ldexp( 1, tonumber(v) )
	end
	return ret
end

function LifeLike:run(dt)

	if self.paused then return end
	if self.fade > 0 then 
		self.fade = self.fade - dt
		return
	end
	
	self.fade = self.fadeTime
	
	self.compute:getParameter("liveRules"):setInt(self.rules.live)
	self.compute:getParameter("birthRules"):setInt(self.rules.birth)
	self.compute:getParameter("deathStates"):setInt(self.rules.deathStates)
	self.color:getParameter("deathStates"):setInt(self.rules.deathStates)
	self.color:getParameter("hueSpacing"):setInt(self.hueSpacing)
	
	self.compute:run()
	self.color:setStorage( Program.input, 0, self.compute:getStorage( Program.output, 0 ) )
	self.color:run()
	
	self.compute:swapInputOutput(0)
end

function getProgram()
	self = LifeLike()
	return self
end

