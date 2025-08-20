//
// Created by Dylan Beaumont on 20/1/2025.
//

#ifndef TEXTURELOADER_H
#define TEXTURELOADER_H
#include <map>
#include "glad/glad.h"


class TextureLoader {
public:
    std::map<std::string, unsigned int> texMap;
    unsigned int loadTexture(char const * path);
    TextureLoader();

};



#endif //TEXTURELOADER_H
