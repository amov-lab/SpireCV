#ifndef __SV_UTIL__
#define __SV_UTIL__

#include <string>
#include <vector>
#include <utility>


namespace sv {


struct TimeInfo
{
    int year, mon, day, hour, min, sec;
};

/*************    time-related functions     *************/
void _get_sys_time(TimeInfo& t_info);
std::string _get_time_str();

/************* std::string-related functions *************/
std::vector<std::string> _split(const std::string& srcstr, const std::string& delimeter);
bool _startswith(const std::string& str, const std::string& start);
bool _endswith(const std::string& str, const std::string& end);
std::string _trim(const std::string& str);
int _comp_str_idx(const std::string& in_str, const std::string* str_list, int len);

/*************    file-related functions   ***************/
std::string _get_home();
bool _is_file_exist(std::string& fn);

}

#endif // __SV_UTIL__
