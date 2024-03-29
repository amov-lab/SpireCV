#include "sv_util.h"
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <ctime>
#include <chrono>
#include <fstream>
#include <dirent.h>
#include <opencv2/opencv.hpp>
#include <unordered_map>


using namespace std;

namespace sv {


std::string _get_home()
{
    struct passwd *pw = getpwuid(getuid());
    const char *homedir = pw->pw_dir;
    return std::string(homedir);
}

bool _is_file_exist(std::string& fn)
{
    std::ifstream f(fn);
    return f.good();
}

bool _comp_str_greater(const std::string& a, const std::string& b)
{
    return a > b;
}

bool _comp_str_lesser(const std::string& a, const std::string& b)
{
    return a < b;
}

void _get_sys_time(TimeInfo& t_info)
{
    time_t tt = time(NULL);
    tm* t = localtime(&tt);
    t_info.year = t->tm_year + 1900;
    t_info.mon = t->tm_mon + 1;
    t_info.day = t->tm_mday;
    t_info.hour = t->tm_hour;
    t_info.min = t->tm_min;
    t_info.sec = t->tm_sec;
}

std::string _get_time_str()
{
    std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
    std::chrono::system_clock::duration tp = now.time_since_epoch();
    tp -= std::chrono::duration_cast<std::chrono::seconds>(tp);

    std::time_t tt = std::chrono::system_clock::to_time_t(now);
    tm t = *std::localtime(&tt);

    char buf[128];
    sprintf(buf, "%4d-%02d-%02d_%02d-%02d-%02d_%03u", t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec, static_cast<unsigned>(tp / std::chrono::milliseconds(1)));

    return std::string(buf);
}

vector<string> _split(const string& srcstr, const string& delimeter)
{
    vector<string> ret(0); //use ret save the spilted reault
    if (srcstr.empty())    //judge the arguments
    {
        return ret;
    }
    string::size_type pos_begin = srcstr.find_first_not_of(delimeter); //find first element of srcstr

    string::size_type dlm_pos; //the delimeter postion
    string temp;               //use third-party temp to save splited element
    while (pos_begin != string::npos) //if not a next of end, continue spliting
    {
        dlm_pos = srcstr.find(delimeter, pos_begin); //find the delimeter symbol
        if (dlm_pos != string::npos)
        {
            temp = srcstr.substr(pos_begin, dlm_pos - pos_begin);
            pos_begin = dlm_pos + delimeter.length();
        }
        else
        {
            temp = srcstr.substr(pos_begin);
            pos_begin = dlm_pos;
        }
        if (!temp.empty())
            ret.push_back(temp);
    }
    return ret;
}

bool _startswith(const std::string& str, const std::string& start)
{
    size_t srclen = str.size();
    size_t startlen = start.size();
    if (srclen >= startlen)
    {
        string temp = str.substr(0, startlen);
        if (temp == start)
            return true;
    }

    return false;
}

bool _endswith(const std::string& str, const std::string& end)
{
    size_t srclen = str.size();
    size_t endlen = end.size();
    if (srclen >= endlen)
    {
        string temp = str.substr(srclen - endlen, endlen);
        if (temp == end)
            return true;
    }

    return false;
}

string _trim(const std::string& str)
{
    string ret;
    // find the first position of not start with space or '\t'
    string::size_type pos_begin = str.find_first_not_of(" \t");
    if (pos_begin == string::npos)
        return str;

    // find the last position of end with space or '\t'
    string::size_type pos_end = str.find_last_not_of(" \t");
    if (pos_end == string::npos)
        return str;

    ret = str.substr(pos_begin, pos_end - pos_begin);

    return ret;
}

int _comp_str_idx(const std::string& in_str, const std::string* str_list, int len) {
    for (int i = 0; i < len; ++i) {
        if (in_str.compare(str_list[i]) == 0) return i;
    }
    return -1;
}


void _list_dir(std::string dir, std::vector<std::string>& files, std::string suffixs, std::string prefix, bool r) {
    // assert(_endswith(dir, "/") || _endswith(dir, "\\"));

    DIR *pdir;
    struct dirent *ent;
    string childpath;
    string absolutepath;
    pdir = opendir(dir.c_str());
    assert(pdir != NULL);

    vector<string> suffixd(0);
    if (!suffixs.empty() && suffixs != "") {
        suffixd = _split(suffixs, "|");
    }

    while ((ent = readdir(pdir)) != NULL) {
        if (ent->d_type & DT_DIR) {
            if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0) {
                continue;
            }
            if (r) { // If need to traverse subdirectories
                childpath = dir + ent->d_name;
                _list_dir(childpath, files);
            }
        }
        else {
            if (suffixd.size() > 0) {
                bool can_push = false, cancan_push = true;
                for (int i = 0; i < (int)suffixd.size(); i++) {
                    if (_endswith(ent->d_name, suffixd[i]))
                        can_push = true;
                }
                if (prefix.size() > 0) {
                    if (!_startswith(ent->d_name, prefix))
                        cancan_push = false;
                }
                if (can_push && cancan_push) {
                    absolutepath = dir + ent->d_name;
                    files.push_back(ent->d_name); // filepath
                }
            }
            else {
                absolutepath = dir + ent->d_name;
                files.push_back(ent->d_name); // filepath
            }
        }
    }
    sort(files.begin(), files.end()); //sort names
}


}
