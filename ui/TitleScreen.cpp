//
// Created by Dylan Beaumont on 22/8/2025.
//

#include "TitleScreen.h"

#include "glm/ext/matrix_clip_space.hpp"
#include "glm/gtc/type_ptr.inl"
unsigned int TitleScreen::tex = 0;
TitleScreen::TitleScreen(int* width, int* height) : UI(width, height) {
    buttons.push_back({
        {0.4, 0.5},
        {0.6, 0.65},
        {0.325f, 0.0f},
        {0.521f, 0.117f},
        [this](){this->exited = true;},
        });
    visuals.push_back({
            {0.25, 0.1},
            {0.75, 0.35},
            {0.0f, 0.0f},
            {0.318f, 0.24f},
    });
}



// void TitleScreen::renderVisuals() {
//     std::vector<float> vertices;
//     float lx = 0.25**scrWidth, rx = 0.75**scrWidth;
//     float uy = *scrHeight - 0.1**scrHeight, dy = *scrHeight - 0.35**scrHeight;
//     vertices.emplace_back(lx), vertices.emplace_back(uy), vertices.emplace_back(0.0), vertices.emplace_back(0.0),
//     vertices.emplace_back(lx), vertices.emplace_back(dy), vertices.emplace_back(0.0), vertices.emplace_back(0.24),
//     vertices.emplace_back(rx), vertices.emplace_back(uy), vertices.emplace_back(0.318), vertices.emplace_back(0.0),
//
//     vertices.emplace_back(rx), vertices.emplace_back(uy), vertices.emplace_back(0.318), vertices.emplace_back(0.0),
//     vertices.emplace_back(lx), vertices.emplace_back(dy), vertices.emplace_back(0.0), vertices.emplace_back(0.24),
//     vertices.emplace_back(rx), vertices.emplace_back(dy), vertices.emplace_back(0.318), vertices.emplace_back(0.24);
//
//     glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(*scrWidth), 0.0f, static_cast<float>(*scrHeight));
//     shader->use();
//     glUniformMatrix4fv(glGetUniformLocation(shader->ID, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
//     glUniform3f(glGetUniformLocation(shader->ID, "spriteColor"), 1.0f, 1.0f, 1.0f);
//     glBindTexture(GL_TEXTURE_2D, tex);
//     glBindVertexArray(VAO);
//     glBindBuffer(GL_ARRAY_BUFFER, VBO);
//     glEnableVertexAttribArray(0);
//     glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *) 0);
//     glBufferData(GL_ARRAY_BUFFER, vertices.size()*sizeof(float), vertices.data(), GL_STATIC_DRAW);
//     glDrawArrays(GL_TRIANGLES, 0, vertices.size()/4);
// }

