/** @author morenvino@xxxxxxxx.xxx */

#ifndef UCARE_CONFIG_H
#define UCARE_CONFIG_H

/* ===========================================================================
 * Header
 * ===========================================================================*/

#include <fstream>
#include <sstream>
#include <string>
#include <map>

namespace ucare {

/* ===========================================================================
 * Class
 * ===========================================================================*/

/** A simple config reader. */
class Config {
	/** Default config filename. */
	constexpr static const char * DEFAULT_CONFIG_FILE = "config.ini";

public:
	/** Read config. */
	Config(const char *filename = DEFAULT_CONFIG_FILE) {
		using std::getline;
		
		std::ifstream in(filename);
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
}; // class Config

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

} // namespace ucare

#endif // UCARE_CONFIG_H
