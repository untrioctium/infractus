class 'VariationTable'

VariationTable.variations = {}
VariationTable.parameters = {}
VariationTable.variationIndexes = {}
VariationTable.parameterIndexes = {}

function VariationTable:__init()
end

function VariationTable.addVariation( name )
	table.insert(VariationTable.variations, name)
	VariationTable.variationIndexes[name] = #VariationTable.variations
end

function VariationTable.getVariationParameterNames( name )
	local ret = {}
	for k,v in pairs(VariationTable.parameters) do
		if v.owner == name then
			table.insert(ret, v.name)
		end
	end
	return ret
end

function VariationTable.addParameter( name, owner, min, max, integral )
	local insert = {}
	insert.owner = owner
	insert.name = name
	insert.min = min
	insert.max = max
	insert.integral = integral
	table.insert(VariationTable.parameters, insert)
	VariationTable.parameterIndexes[name] = #VariationTable.parameters
end

function VariationTable.randomVariation()
	return VariationTable.variations[math.random(1, #VariationTable.variations)]
end

function VariationTable.randomParameterValue( name )
	if VariationTable.parameterIndexes[name] == nil then return 0.0 end
	local paramInfo = VariationTable.parameters[VariationTable.parameterIndexes[name]]
	
	if paramInfo.integral then
		return math.random( paramInfo.min, paramInfo.max )
	else
		return math.random() * (paramInfo.max - paramInfo.min) + paramInfo.min
	end
end

function VariationTable.isVariation( name )
	return VariationTable.variationIndexes[name] ~= nil
end

function VariationTable.isParameter( name )
	return VariationTable.parameterIndexes[name] ~= nil
end

function VariationTable.getVariationIndex( name )
	if not VariationTable.isVariation(name) then return -1 end
	return VariationTable.variationIndexes[name]
end

function VariationTable.getParameterIndex( name )
	if not VariationTable.isParameter(name) then return -1 end
	return VariationTable.parameterIndexes[name]
end
