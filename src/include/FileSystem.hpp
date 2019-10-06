#include <experimental/filesystem>

namespace fs = std::experimental::filesystem;

class FileSystem
{
public:
	FileSystem() {}

	~FileSystem() {}

	void init();

	bool fileExists( const fs::path& filepath );
	std::vector<fs::path> getFileList( const fs::path& dir );
	std::vector<fs::path> findAll( const fs::path& dir, const std::string& filename );

private:

};
