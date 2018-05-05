#include "LuaProgramLoader.hpp"
#include "ProgramManager.hpp"

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

using namespace libcompute;
using namespace boost::property_tree;

InfractusProgram* ProgramManager::createInstance( const std::string& programName )
{
	if( avaliablePrograms_.count(programName) == 0 ) return NULL;
	
	ProgramInfo& info = avaliablePrograms_[programName];
	
	ProgramLoader* loader = (ProgramLoader*) &Singleton<LuaProgramLoader>::instance();
	
	InfractusProgram* instance = loader->createInstance( info.getInfoField("path") + info.getInfoField("file") );
	instance->workingDirectory_ = info.getInfoField("path");
	return instance;
}

std::vector<std::string> ProgramManager::getAvaliablePrograms()
{
	std::pair<std::string, ProgramInfo> val;
	std::vector<std::string> ret;
	
	BOOST_FOREACH( val, avaliablePrograms_ )
		ret.push_back(val.first);
	
	return ret;
}

ProgramInfo ProgramManager::getProgramInfo( const std::string& programName )
{
	if( avaliablePrograms_.count(programName) == 0 ) return ProgramInfo();
	return avaliablePrograms_[programName];
}

void ProgramManager::loadAllInfo()
{
	std::vector<fs::path> programInfos = Singleton<FileSystem>::instance().findAll( programsDir_, "program.xml" );
	
	BOOST_FOREACH( fs::path path, programInfos )
	{
		ProgramInfo info;
		ptree xml;
		read_xml( path.string(), xml );
		
		BOOST_FOREACH( ptree::value_type& v, xml.get_child("program") )
		{
			std::string name = v.first;
			if( name == "attributes" )
			{
				BOOST_FOREACH( ptree::value_type& a, v.second )
					info.attributes_.insert( a.first );
			}
			else info.info_[v.first] = v.second.data();
		}
		info.info_["path"] = path.remove_filename().string() + "/";
		avaliablePrograms_[info.info_["name"]] = info;
	}
}
