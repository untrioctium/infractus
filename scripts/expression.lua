functionTable = {}
Expression = {}

function explode( str, sep )
	local retTable, lastMatch
	retTable = {}
	lastMatch = 0
	
	while true do
		curMatch = string.find( str, sep, lastMatch + 1, true )
		if curMatch ~= nil then
			table.insert( retTable, string.sub( str, lastMatch, curMatch - 1 ) )
			lastMatch = curMatch + 1
		else
			table.insert( retTable, string.sub( str, lastMatch ) )
			break
		end
	end

	return retTable
end

function replace_table( from, to )
	for k,v in pairs(from) do from[k] = nil end
	for k,v in pairs(to) do from[k] = to[k] end
end

-- creates a constant leaf with a specified value
function Expression:createConstant( value )
	o = { type = "constant"; value = value }
	setmetatable(o, self)
	self.__index = self
	return o
end

-- creates a variable leaf with the specified name
function Expression:createVariable( name )
	o = { type = "variable"; name = name }
	setmetatable(o, self)
	self.__index = self
	return o
end

-- creates a function leaf with the specified name and arguments
function Expression:createFunction( name, ... )
	arg.type = "function"
	arg.name = name
	arg.def = functionTable[name]
	setmetatable(arg, self)
	self.__index = self
	return arg
end

function Expression:replaceVariable( replace, var )
	if self.type == "variable" and self.name == var then
		replace_table( self, replace )
	end
	if self.type == "function" then
		for k,v in ipairs(self) do
			self[k]:replaceVariable( replace, var )
		end
	end
end 

function Expression:addDerivative( var )
	local dself = {}

	dself[1] = self[1]:derivative( var )
	dself[2] = self[2]:derivative( var )

	return Expression:createFunction("+", dself[1], dself[2] )
end

function Expression:subDerivative( var )
	local dself = {}

	dself[1] = self[1]:derivative( var )
	dself[2] = self[2]:derivative( var )

	return Expression:createFunction("-", dself[1], dself[2] )
end

function Expression:mulDerivative( var )
	local dself = {}

	dself[1] = self[1]:derivative( var )
	dself[2] = self[2]:derivative( var )

	return Expression:createFunction("+", 
			Expression:createFunction("*", self[1], dself[2]),
			Expression:createFunction("*", self[2], dself[1]))
end

function Expression:divDerivative( var )
	local dself = {}

	dself[1] = self[1]:derivative( var )
	dself[2] = self[2]:derivative( var )

	return Expression:createFunction("/",
			Expression:createFunction( "-", 
				Expression:createFunction("*", self[2], dself[1] ),
				Expression:createFunction("*", self[1], dself[2] )
			),
			Expression:createFunction( "^", self[2], Expression:createConstant(2))
		)
end

function pow_derivative( self, var )
	
end

function Expression:chainDerivative( var )
	dself = Expression:fromPostfix( self.def.d )
	dArg = self[1]:derivative(var)
	dself:replaceVariable( self[1], "_1" ) 	

	return Expression:createFunction( "*", dArg, dself )
end

function Expression:derivative( var )

	if self == nil then return nil end

	if self.type == "variable" and self.name == var then
		return Expression:createConstant(1)
	end

	if self.type == "function" then
		if type(self.def.d) == "string" then
			return self:chainDerivative( var )
		else
			return self.def.d( self, var )
		end
	end

	return Expression:createConstant(0)
end

function Expression:equals( expression )
	for k,v in pairs(self) do
		if type(v) == "table" then
			return v:equals( self[k] )
		end
		return v == self[k]
	end
end

function Expression:print( depth )
	if depth == nil then depth = "" end

	if self.type == "constant" then print(depth .. tostring(self.value))
	elseif self.type == "variable" then print(depth .. self.name)
	elseif self.type == "function" then
		print(depth .. self.name)
		for k,v in ipairs(self) do
			v:print(depth .. "  ")
		end
	end
end

function Expression:toPostfix()
	if self.type == "constant" then return tostring(self.value) end
	if self.type == "variable" then return self.name end
	if self.type == "function" then
		postfix = ""
		for k,v in ipairs(self) do
			postfix = postfix .. v:toPostfix() .. " "
		end
		postfix = string.sub(postfix, 0, postfix:len() - 1)
		if self.def.argc == 1 then
			return postfix .. " " .. self.name
		else
			return postfix .. string.rep(" " .. self.name, self.n - 1 ) 
		end
	end
end

function Expression:toInfix(parent)
	if self.type == "constant" then return tostring(self.value) end
	if self.type == "variable" then return self.name end
	if self.type == "function" then
		if self.def.argc == 1 then
			return string.format("%s(%s)", self.name, self[1]:toInfix())
		else
			local out = self[1]:toInfix( self ) .. self.name .. self[2]:toInfix( self )

			for i=3,self.n do
				out = out .. self.name .. self[i]:toInfix( self[i], self )
			end

			if parent ~= nil then
				if self.def.traits.precedence < parent.def.traits.precedence then
					out = "(" .. out .. ")"
				end
			end

			return out
		end
	end
end

function Expression:toPrefix()
	if self.type == "constant" then return tostring(self.value) end
	if self.type == "variable" then return self.name end
	if self.type == "function" then
		if self.def.argc == 1 then
			return string.format("%s(%s)", self.name, self[1]:toPrefix())
		else
			local out = self.name .. "(" .. self[1]:toPrefix() .. "," .. self[2]:toPrefix() .. ")";
			return out
		end
	end
end

-- optimizes a self for least execution time
function Expression:optimize()
	if self.type == "function" then
		-- optimize the children
		for i=1,self.n do
			self[i]:optimize()
		end

		if self.def.traits ~= nil then
			-- pull up any commutative children
			if self.def.traits.commutative == true then
				local pulledChildren = {}
				local size = self.n
				for i=1,self.n do
					if self[i].type == "function" then
						if self[i].name ~= self.name then break end
						for k,v in ipairs(self[i]) do
							table.insert(self,v)
							size = size + 1
						end
						table.insert(pulledChildren,i)
					end
				end
				local offset = 0
				for k,v in ipairs(pulledChildren) do
					table.remove(self,v - offset)
					size = size - 1
					offset = offset + 1
				end
				self.n = size
			end

			-- perform constant optimization on leaves
			if self.def.traits.constantOptimization == true then
				local constantValues = {}
				local pos = 1
				while pos <= self.n do
					if self[pos].type == "constant" then 
						table.insert(constantValues, self[pos].value)
						table.remove(self,pos)
						self.n = self.n - 1
					else pos = pos + 1 end
				end
				if #constantValues ~= 0 then
					local value = constantValues[1]
					for i=2,#constantValues do
						value = self.def.evalFunc( value, constantValues[i] )
					end
					value = Expression:createConstant(value)
					if self.n == 0 then replace_table(self,value) else
						table.insert(self, value)
						self.n = self.n + 1
					end
				end
			end
		end

		if self.def ~= nil and self.def.optimizeFunc ~= nil then
			self.def.optimizeFunc(self)
		end
	end
end

function Expression:fromPostfix( func )

	-- extract the rightmost token
	spacePos = string.find( string.reverse(func), " ", 0, true )

	-- if there was no space, then do some basic checks
	if spacePos == nil then
		if functionTable[func] ~= nil then return nil end
		
		funcv = tonumber(func)
		if funcv ~= nil then return Expression:createConstant( funcv ) end
		return Expression:createVariable( func )
					
	end

	spacePos = func:len() - spacePos + 1
	rightFunc = string.sub(func, spacePos + 1)
	arg = func:sub( 0, spacePos - 1 )

	if functionTable[rightFunc].argc == 1 then
		return Expression:createFunction( rightFunc, Expression:fromPostfix(arg) )
	else
		requiredArgs = 1
		argTokens = explode( arg, " " )
		current = #argTokens + 1

		while requiredArgs > 0 do
			current = current - 1
			if current == 0 then return nil end
			if functionTable[argTokens[current]] ~= nil then
				requiredArgs = requiredArgs + functionTable[argTokens[current]].argc
			end
			
			requiredArgs = requiredArgs - 1
		end		
		local firstArg, secondArg
		firstArg = table.concat( argTokens, " ", 1, current - 1)
		secondArg = table.concat( argTokens," ", current ) 
		return Expression:createFunction( rightFunc, Expression:fromPostfix( firstArg ), Expression:fromPostfix( secondArg ) )
	end
end

function Expression:evaluate( vars, complex )
	if complex == nil then complex = false end
	if self.type == "constant" then return (complex and type(self.value) == "number" and {self.value, 0}) or self.value end
	if self.type == "variable" then 
		if vars[self.name] == nil then 
			return nil
		else return vars[self.name] end
	end
	if self.type == "function" then
		local evalFunc = (complex and self.def.complexEval) or self.def.evalFunc
		
		local args = {}
		for key,value in ipairs(self) do
			args[key] = value:evaluate(vars, complex)
			if args[key] == nil then return nil end
		end
		if self.n < 3 then return evalFunc( unpack(args) ) end
		
		local value = evalFunc( args[1], args[2] )
		for i=3,self.n do value = evalFunc(value, args[i]) end
		return value
			
	end
end

function add_function( name, argc, d, evalFunc, complexEval, optimizeFunc, traits )
	functionTable[name] = { name = name; argc = argc; d = d; evalFunc = evalFunc; complexEval = complexEval; optimizeFunc = optimizeFunc; traits = traits }
end

function Expression:addOptimize()
	-- remove zero valued constant leaves
	for k,v in ipairs(self) do
		if v.type == "constant" and v.value == 0 then 
			table.remove( self, k )
			self.n = self.n - 1
		end
	end
	if self.n == 1 then replace_table(self, self[1]) end
end
add_function("+", 2, Expression.addDerivative, 
		function(lhs,rhs) return lhs+rhs end, 
		function(lhs,rhs) return {lhs[1] + rhs[1], lhs[2] + rhs[2]} end,
		Expression.addOptimize, { constantOptimization = true; precedence = 0; commutative = false } )
add_function("-", 2, Expression.subDerivative, function(lhs,rhs) return lhs-rhs end, function(lhs,rhs) return {lhs[1] - rhs[1], lhs[2] - rhs[2]} end, nil, { constantOptimization = true; precedence = 0 } )

function Expression:mulOptimize()
	-- remove one valued constant leaves
	-- return a constant leaf of zero if any constant leaves are zero
	for k,v in ipairs(self) do
		if v.type == "constant" then
			if v.value == 0 then replace_table(self, Expression:createConstant(0)); return end
			if v.value == 1 then
				table.remove( self, k )
				self.n = self.n - 1
			end
		end
	end
	if self.n == 1 then replace_table(self, self[1]) end
end

add_function("*", 2, Expression.mulDerivative, 
		function(lhs,rhs) return lhs*rhs end, 
		function(lhs,rhs) return {lhs[1] * rhs[1] - lhs[2] * rhs[2], lhs[1] * rhs[2] + lhs[2] * rhs[1] } end, Expression.mulOptimize, { constantOptimization = true; precedence = 1; commutative = false } )
add_function("/", 2, Expression.divDerivative, function(lhs,rhs) return lhs/rhs end, 
		function(lhs, rhs)
			local denom = rhs[1] * rhs[1] + rhs[2] * rhs[2]
			return {(lhs[1] * rhs[1] + lhs[2] * rhs[2])/denom, (lhs[1] * rhs[2] - lhs[2] * rhs[1])/denom } end, nil, { constantOptimization = true; precedence = 1 } )
add_function("^", 2, Expression.powDerivative, math.pow, nil, { precedence = 2 } )
add_function("sin", 1, "_1 cos", math.sin, function(z) return {math.sin(z[1]) * math.cosh(z[2]), math.cos(z[1]) * math.sinh(z[2])} end, nil, {} )
add_function("cos", 1, "_1 sin -1 *", math.cos, function(z) return {math.cos(z[1]) * math.cosh(z[2]), -math.sin(z[1]) * math.sinh(z[2])} end,  nil, {} )
add_function("exp", 1, "_1 exp", math.exp, nil, nil, {} )
