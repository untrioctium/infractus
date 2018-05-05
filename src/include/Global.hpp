#ifndef GLOBAL_HPP
#define GLOBAL_HPP

#include <string>
#include <sstream>
#include <exception>
#include <vector>
#include <typeinfo>
#include <stdio.h>
#include <stdlib.h>
#include <map>
#include <stack>
#include <set>

#include <boost/any.hpp>
#include <boost/foreach.hpp>
#include <boost/property_tree/ptree.hpp>

using boost::property_tree::ptree;

/// Points to argv[0]; some libraries and functions may need this.
extern char* path;

#endif
