#include "Global.hpp"
#include "LoggingSystem.hpp"
#include "FileSystem.hpp"

void FileSystem::init()
{
}

bool FileSystem::fileExists( const fs::path& filename )
{
	return fs::exists( filename );
}

std::vector<fs::path> FileSystem::getFileList( const fs::path& dir )
{
	std::vector<fs::path> listing;
	
	fs::directory_iterator endIter;
	
	for( fs::directory_iterator iter( dir ); iter != endIter; iter++ )
		listing.push_back( iter->path() );
		
	return listing;
}

std::vector<fs::path> FileSystem::findAll( const fs::path& dir, const std::string& filename )
{
	std::vector<fs::path> paths;
	fs::directory_iterator endIter;
	
	for( fs::directory_iterator iter( dir ); iter != endIter; iter++ )
	{
		if( fs::is_directory( iter->status() ) )
		{
			std::vector<fs::path> dirPaths = findAll( iter->path(), filename );
			paths.insert( paths.begin(), dirPaths.begin(), dirPaths.end() );
		}
		else if( iter->path().filename() == filename )
		{
			paths.push_back( iter->path() );
		}
	}
	
	return paths;
}








