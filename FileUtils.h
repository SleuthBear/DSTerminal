//
// Created by Dylan Beaumont on 9/8/2025.
//

#include <string>

#include "json.h"
#include "Terminal.h"
using json = nlohmann::json;

FileNode* getNode(json data);
FileNode* readSystem(const std::string& filePath);


FileNode* readSystem(const std::string& filePath) {
    FileNode root = {};
    std::ifstream f(filePath);
    json data = json::parse(f);
    return getNode(data);
}

FileNode* getNode(json data) {
    if (data["name"].empty()) return nullptr;
    FileNode* newNode = new FileNode{data["name"], data["type"], nullptr, nullptr, 0};
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