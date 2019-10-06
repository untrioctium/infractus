#include "libcompute.hpp"

#include <boost/property_tree/xml_parser.hpp> 	
#include <boost/foreach.hpp>

using namespace libcompute;

/*Program::Program(const std::string& str)
{
	this->storage_[0] = this->storage_[1] = NULL;
	this->boundEngine_ = NULL;
	load(str);
}*/

Program::~Program()
{
	if( storage_[0] != NULL )
		delete storage_[0];

	if( storage_[1] != NULL )
		delete storage_[1];

	if( boundEngine_ != NULL )
		boundEngine_->unbindProgram(this);
}

/*Program::Program(const Program& parent, const std::string& fileLocation)
{
	error_ = 0;

	this->supportedEngines_[parent.getActiveEngine()->pluginName()] = fileLocation;
	this->parameters_ = parent.parameters_;	

	this->bindEngine( parent.getActiveEngine() );

	this->engineInternalStorage_ = parent.engineInternalStorage_;
	this->inputStorage_ = parent.inputStorage_;
	this->outputStorage_ = parent.outputStorage_;
	this->inputDataType_ = parent.inputDataType_;
	this->outputDataType_ = parent.outputDataType_;
}*/

void Program::load( const std::string& path )
{
	try
	{
		std::string fullPath = workingDirectory_ + path;
		boost::property_tree::ptree config;
		read_xml( fullPath, config );

		for( auto &v: config.get_child( "program.engines" ))
		{
			boost::property_tree::ptree engine = v.second;

			supportedEngines_[engine.get<std::string>("name")] = std::pair<ProgramLocation, std::any>( File, engine.get<std::string>("file") );
		}	

		storageDataTypes_[Input].type = Engine::DataStorage::typeFromName(config.get<std::string>( "program.input.type" ));
		storageDataTypes_[Output].type = Engine::DataStorage::typeFromName(config.get<std::string>( "program.output.type" ));
		storageDataTypes_[Input].size = config.get<unsigned int>("program.input.size", 0);
		storageDataTypes_[Output].size = config.get<unsigned int>("program.output.size", 0);
		storageCount_[Input] = config.get<unsigned int>("program.input.count", 1);
		storageCount_[Output] = config.get<unsigned int>("program.output.count", 1);

		storage_[Input] = new Engine::DataStorage::Ptr*[storageCount_[Input]];
		for( unsigned int i = 0; i < storageCount_[Input]; i++ )
			storage_[Input][i] = NULL;
			
		storage_[Output] = new Engine::DataStorage::Ptr*[storageCount_[Output]];
		for( unsigned int i = 0; i < storageCount_[Output]; i++ )
			storage_[Output][i] = NULL;
			
		for( auto &v: config.get_child( "program.input.parameters" ) )
		{
			boost::property_tree::ptree parameter = v.second;
			std::string paramName = parameter.get<std::string>("<xmlattr>.name");
			parameters_[paramName] = Parameter(paramName, Parameter::typeFromName(parameter.get<std::string>("<xmlattr>.type")),
										   parameter.get<unsigned int>("<xmlattr>.size", 1));
		}

	}
	catch ( std::exception e )
	{
		printf("f-- %s\n", e.what());
	}
}

Parameter& Program::getParameter( const std::string& name )
{
	return parameters_[name];
}

void Program::addParameter( const std::string& name, Parameter::ParameterType type ) 
{
	parameters_[name] = Parameter(name, type, 1);
}

void Program::addParameterArray( const std::string& name, Parameter::ParameterType type, unsigned int size )
{
	parameters_[name] = Parameter(name, type, size);
}

std::vector<std::string> Program::getParameterNames()
{
	std::vector<std::string> retVal;
	std::pair<std::string, Parameter> iter;
	BOOST_FOREACH( iter, parameters_ )
		retVal.push_back(iter.first);

	return retVal;
}

void* Program::getActiveProgram()
{
	return this->activeProgram_;
}

void Program::run()
{
	boundEngine_->runProgram(this);
}

void Program::allocateStorage( unsigned int width, unsigned int height, StorageLocation location, unsigned int index )
{
	if( this->storage_[location][index] != NULL )
		delete this->storage_[location][index];

	this->storage_[location][index] = new Engine::DataStorage::Ptr(
		boundEngine_->allocateStorage( storageDataTypes_[location], width, height ) );
}

Program::ProgramLocation Program::getProgramLocationType( const std::string& engine ) const
{
	return supportedEngines_.find(engine)->second.first;
}

void Program::setProgramLocationFile( const std::string& engine, const std::string& path )
{
	supportedEngines_[engine].first = File;
	supportedEngines_[engine].second = path;
}

void Program::setProgramLocationMemory( const std::string& engine, std::any location )
{
	supportedEngines_[engine].first = Memory;
	supportedEngines_[engine].second = location;
}

std::string Program::getProgramLocationFile( const std::string& engine )
{
	return workingDirectory_ + std::any_cast<std::string>( supportedEngines_[engine].second );
}
std::any Program::getProgramLocationMemory( const std::string& engine )
{
	return supportedEngines_[engine].second;
}

Engine::DataStorage::Ptr Program::getStorage( StorageLocation location, unsigned int index )
{
	if( location == Input )
		return *(storage_[0][index]);

	else return *(storage_[1][index]);
}

void Program::setStorage( StorageLocation location, unsigned int index, Engine::DataStorage::Ptr storage )
{
	if( storage_[location] != NULL )
		delete storage_[location][index];

	storage_[location][index] = new Engine::DataStorage::Ptr( storage );
}

void Program::swapInputOutput( unsigned int index )
{
	Engine::DataStorage::Ptr* temp = storage_[0][index];
	storage_[0][index] = storage_[1][index];
	storage_[1][index] = temp;
}

void Program::bindEngine( Engine* engine )
{
	boundEngine_ = engine;
	this->activeProgram_ = engine->bindProgram( this );
}

Engine::DataStorage::Info Program::getStorageInfo( StorageLocation location )
{
	return storageDataTypes_[location];
}
