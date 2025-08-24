//
// Created by Dylan Beaumont on 9/8/2025.
//
#pragma once

#include <codecvt>
#include <string>

#include "../include/json.h"
#include "KeyState.h"

using json = nlohmann::json;

struct FileNode {
    std::string name;
    int type;
    FileNode *parent;
    FileNode **children;
    int nChildren;
    std::string fileRef;
};

FileNode* getNode(json data);
FileNode* readSystem(const std::string& filePath);
FileNode *findChild(FileNode* pos, std::string_view name);
std::string_view trimString(std::string_view str);
FileNode *followPath(FileNode *pos, std::string_view path);
FileNode *findChild(FileNode* pos, const std::string_view name);

inline FileNode* readSystem(const std::string& filePath) {
    FileNode root = {};
    std::ifstream f(filePath);
    json data = json::parse(f);
    return getNode(data);
}

inline FileNode* getNode(json data) {
    if (data["name"].empty()) return nullptr;
    FileNode* newNode = new FileNode{data["name"], data["type"], nullptr, nullptr, 0};
    if (newNode->type == DS_FILE) {
        newNode->fileRef = data["file_ref"];
    }
    // DO NOT stack allocate this recursively (for obvious reasons)
    int capacity = 4;
    FileNode **children = (FileNode**) malloc(sizeof(FileNode*) * capacity);
    int count = 0;
    for (const json& child : data["children"]) {
        if (count >= capacity) {
            capacity *= 2;
            children = (FileNode**) realloc(children, capacity*sizeof(FileNode*));
            if (!children) {
                fprintf(stderr, "Failed to allocate memory for child nodes");
                exit(1);
            }
        }
        FileNode* childNode = getNode(child);
        if (childNode) {
            childNode->parent = newNode;
            children[count] = childNode;
            count++;
        }
    }
    newNode->children = children;
    newNode->nChildren = count;
    return newNode;
}

// I hate this function, it is so ugly. But it mostly works...
inline FileNode *followPath(FileNode *pos, std::string_view path) {
    int i = 0;
    int start = 0;
    path = trimString(path);
    while (i < path.length()) {
        if (path[i] == '/' || path[i] == '\\') {
            if (i != start) {
                pos = findChild(pos, path.substr(start, i-start));
                // skip around the slash
                start = i+1;
                if (pos == nullptr) {
                    return nullptr;
                }
                if (pos->name.find(".lock") != -1) {
                    // We have encountered a lock.
                    return pos;
                }
            }
        }
        i++;
    }
    if (i != start) {
        pos = findChild(pos, path.substr(start, i-start));
        if (pos == nullptr) {
            return nullptr;
        }
    }
    return pos;
}

inline FileNode *findChild(FileNode* pos, const std::string_view name) {
    if (name == "..") {
        return pos->parent;
    }
    for(int i = 0; i < pos->nChildren; i++) {
        FileNode *child = pos->children[i];
        if (child->name == name) {
            return child;
        }
    }
    return nullptr;
}

inline std::string_view trimString(std::string_view str) {
    int start = -1;
    for (int i = 0; i < str.length(); i++) {
        if (start == -1 && str[i] != ' ') start = i;
        else if (start != -1 && str[i] == ' ') {
            return str.substr(start, i-start);
        }
    }
    return str.substr(std::max(0,start), str.length()-start);
}