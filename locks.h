//
// Created by Dylan Beaumont on 26/8/2025.
//

#ifndef DSTERMINAL_LOCKS_H
#define DSTERMINAL_LOCKS_H
#include <map>

struct LockInfo {
    std::string hint;
    std::string answer;
    bool fuzzy;
};

static std::map<std::string, LockInfo> LOCK_MAP = {
    {"Backup_755012715.lock", {"brother in law", "Jacob", false}},
    {"Code_163903526.lock", {"first BDOWDC champion", "Leighton Rees", false}},
    {"Logs_1807696380.lock", {"my zodiac element + symbol", "metal dragon", true}}
};

extern std::map<std::string, bool> LOCKS_OPENED;

#endif //DSTERMINAL_LOCKS_H