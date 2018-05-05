#include "libcompute.hpp"

using namespace libcompute;

std::map<std::string, Engine::DataStorage::DataType> Engine::DataStorage::dataTypeNameTable_ = Engine::DataStorage::initDataTypeNameTable();
