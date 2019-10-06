class 'PlaneBase' (InfractusProgram)

function PlaneBase:__init()
	InfractusProgram.__init(self)
end

function PlaneBase:

inputSystem = InputSystem.instance()
graphicsSystem = GraphicsSystem.instance()
screenInfo = graphicsSystem:getScreenInfo()
aspect = screenInfo.w/screenInfo.h

transformProgram = 0
calcProgram = 0
colorProgram = 0
convoveProgram = 0

calcParameters = {}
colorParameters = {}
transformParameters = {}
convolveParameters = {}

convolveKernel = { 0, 0, 0, 0, 1, 0, 0, 0, 0 }

activeEngine = 0

ymin = -1
ymax = 1
ylength = ymax - ymin
xlength = ylength * aspect
xcenter = 0
xmin = xcenter - xlength/2
xmax = xcenter + xlength/2

zoom = 0
ignoreZoom = false
ignoreMotion = false
motionButton = 1

function update_transform( func )
	transformProgram:unbindEngine()
	source = io.open("programs/transform.frag"):read("*a")
	source = string.gsub(source, "%$TRANSFORM%$", func)
	transformProgram:setProgramLocationMemoryString("GLSLComputeEngine", source)
	transformProgram:bindEngine(activeEngine)
end

function cache_parameters( program, cache )
	local names = program:getParameterNames()
	for i=0,names:size()-1 do
		paramName = names:at(i)
		cache[paramName] = program:getParameter(paramName)
	end
end

function init(engine)
	activeEngine = engine
	setup()
	
	transformProgram = Program("programs/transform.program")
	source = io.open("programs/transform.frag"):read("*a")
	source = string.gsub(source, "%$TRANSFORM%$", "z")
	transformProgram:setProgramLocationMemoryString("GLSLComputeEngine", source)
	convolveProgram = Program("programs/convolve.program")

	cache_parameters( transformProgram, transformParameters )
	cache_parameters( calcProgram, calcParameters )
	cache_parameters( colorProgram, colorParameters )
	cache_parameters( convolveProgram, convolveParameters )
	
	transformParameters["X_MIN"]:setFloat(xmin)
	transformParameters["X_MAX"]:setFloat(xmax)
	transformParameters["Y_MIN"]:setFloat(ymin)
	transformParameters["Y_MAX"]:setFloat(ymax)

	print("bind transform")
	transformProgram:bindEngine(engine)
	
	print("bind calc")
	calcProgram:bindEngine(engine)
	
	print("bind color")
	colorProgram:bindEngine(engine)
	
	print("bind convolve")
	convolveProgram:bindEngine(engine)

	print("done bind")

	transformProgram:allocateStorage( screenInfo.w, screenInfo.h, Program.output, 0 )
	calcProgram:allocateStorage( screenInfo.w, screenInfo.h, Program.output, 0 )
	colorProgram:allocateStorage( screenInfo.w, screenInfo.h, Program.output, 0 )
	convolveProgram:allocateStorage( screenInfo.w, screenInfo.h, Program.output, 0 )

	calcProgram:setStorage( Program.input, 0, transformProgram:getStorage(Program.output, 0))
	colorProgram:setStorage( Program.input, 0, calcProgram:getStorage(Program.output, 0) )
	convolveProgram:setStorage( Program.input, 0, colorProgram:getStorage(Program.output, 0) )
end

function run()

	if ignoreMotion ~= true and inputSystem:getMouseButtonState(motionButton) == InputSystem.down then
		mouseMove = inputSystem:getMouseMotion()
		dx = -xlength/screenInfo.w * mouseMove.x
		dy = ylength/screenInfo.h * mouseMove.y
		update_screen( xmin + dx, xmax + dx, ymin + dy, ymax + dy )
	end

	if ignoreZoom ~= true then
		scroll = inputSystem:getMouseWheelScroll()

		if scroll ~= 0 then
			zoom = zoom + scroll * 7;
		end
	
		if zoom ~= 0 then
			if zoom < 0 then factor = 1.005 else factor = .995 end
			factor = math.pow( factor, math.abs(zoom/7) )
			xcenter = xmin + xlength/2
			ycenter = ymin + ylength/2
			xlength = factor * xlength
			ylength = factor * ylength
			xmin = xcenter - xlength/2
			xmax = xcenter + xlength/2
			ymin = ycenter - ylength/2
			ymax = ycenter + ylength/2
			zoom = zoom + -factor * math.abs(zoom)/zoom
			if math.abs(zoom) < 1 then zoom = 0 end
			update_screen( xmin, xmax, ymin, ymax )
		end
	end

	do_frame()

	transformProgram:run()
	calcProgram:run()
	colorProgram:run()
	
	for i=1,9 do
		convolveParameters["convolveKernel"]:at(i-1):setFloat(convolveKernel[i])
	end
	
	convolveProgram:run()
end

function draw_status( ... )
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
	
	graphicsSystem:setDrawColor( Color(0,0,0,.75) )
	graphicsSystem:drawRectangle( Point(0, starty), Point( maxFinalLength * textw, starty + total * texth) )
	graphicsSystem:resetDrawColor()
	
	for index,value in ipairs(arg) do
		graphicsSystem:drawText( value[1], Point(0, starty) )
		starty = starty + texth
	end
end

function update_screen( nxmin, nxmax, nymin, nymax )
	xmin = nxmin
	xmax = nxmax
	ymin = nymin
	ymax = nymax	

	xlength = xmax - xmin
	ylength = ymax - ymin

	transformParameters["X_MIN"]:setFloat(xmin)
	transformParameters["X_MAX"]:setFloat(xmax)
	transformParameters["Y_MIN"]:setFloat(ymin)
	transformParameters["Y_MAX"]:setFloat(ymax)

end

function coord_to_pixel( x, y )
	return (x - xmin)/xlength * screenInfo.w, (ymax - y)/ylength * screenInfo.h
end

function pixel_to_coord( x, y )
	return xmin + x/screenInfo.w * xlength, ymax - y/screenInfo.h * ylength
end

function get_output_array()
	return colorProgram:getStorage(Program.output)
end
