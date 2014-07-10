/** @author morenvino@uchicago.edu */

#ifndef MV_CONFIG_H
#define MV_CONFIG_H

/* ===========================================================================
 * Headers
 * ===========================================================================*/

#include <fstream>
#include <sstream>
#include <string>
#include <map>
#include <stdexcept>
//#include <cstdlib>
//#include <iostream>

namespace mv {

/** Default configuration filename. */
#define CONFIG_FILENAME "config.ini"

/* ===========================================================================
 * Classes
 * ===========================================================================*/

/** A simple config reader. */
class Config {
public:
	/** Read config. */
	Config(const char *filename = CONFIG_FILENAME) {
		using std::getline;
		
		std::ifstream in(filename);
		//in.exceptions(std::ifstream::failbit);

		std::string line;
		while (getline(in, line)) {
			std::istringstream is(line);
		  std::string key, value;
  		getline(is, key, '=');
    	getline(is, value, ' ');
      config[key] = value;
  	}
	}

	/** @return a value of key as type T. */
	template<typename T = const char *> 
	inline T get(const char *key) {
			return config[key].c_str();
	}

private:
	std::map<std::string, std::string> config; /// Config data
}; // class Matrix

/** @return an int value of key. */
template<> inline int Config::get<int>(const char *key) {
	return atoi(get(key)); 	
}
		
/** @return a double value of key. */
template<> inline double Config::get<double>(const char *key) {
	return atof(get(key)); 	
}

/** @return a string value of key. */
template<> inline std::string Config::get<std::string>(const char *key) {
	return std::string(get(key)); 	
}

/** @return a bool value of key. */
template<> inline bool Config::get<bool>(const char *key) {
	return get<int>(key); 	
}

} // namespace mv

#endif // MV_CONFIG_H
