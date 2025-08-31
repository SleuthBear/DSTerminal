//
// Created by Dylan Beaumont on 22/8/2025.
//

#include "MenuScreen.h"

#include <thread>

#include "../locks.h"

unsigned int MenuScreen::tex = 0;
MenuScreen::MenuScreen(int* width, int* height, FileNode* node)
    : UI(width, height)
{
    this->node = node;
    buttons.push_back({
        { 0.4, 0.3 },
        { 0.6, 0.45 },
        { 0.325f, 0.0f },
        { 0.521f, 0.117f },
        [this]() { this->exited = true; },
    });
    buttons.push_back({
        { 0.4, 0.5 },
        { 0.6, 0.65 },
        { 0.522f, 0.0f },
        { 0.749f, 0.126f },
        [this]() { this->shouldClose = true; },
    });
    buttons.push_back({
        { 0.4, 0.7 },
        { 0.6, 0.8 },
        // todo make actual visual component
        { 0.338f, 0.137f },
        { 0.557f, 0.248f },
        [this]() { this->save(); } });
}

void MenuScreen::save()
{
    // Save the reverse path? That's kinda shit actually
    std::vector<std::string> names;
    names.push_back(node->name);
    FileNode* pos = node->parent;
    while (pos != nullptr) {
        names.push_back(pos->name);
        pos = pos->parent;
    }
    std::string path = "";
    for (int i = names.size() - 2; i >= 0; i--) {
        path += names[i] + "/";
    }
    std::ofstream file;
    // todo error handling
    file.open("../saves/path.txt");
    file << path;
    file.close();
    std::string locks;
    for (std::pair<std::string, bool> pair : LOCKS_OPENED) {
        locks += pair.first + " ";
    }
    file.open("../saves/locks.txt");
    file << locks;
    file.close();
    visuals.push_back({
        {0.0f, 0.0f},
        {0.2f, 0.1f},
        {0.343f, 0.259f},
        {0.589f, 0.407f},
    });
    // Get rid of the visual after 1 second.
    std::thread t1([this]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            this->visuals.clear();
        });
    t1.detach();
}
