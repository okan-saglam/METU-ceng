/*
    Ahmet Yasin Ocak - 2521797 
    Okan Sağlam - 2521904

    We have implemented the following features:
    - 3 different shapes: 3x3x3, 1x3x1, 3x1x3
    - Special block: 30% chance to spawn a special block that explodes and clears the blocks under it
    - Score system: 10 points for each block removed by the explosive block
    - Game area: 9x15x9 grid
    - Game speed: Speed increases over time, min speed is 0.25, max speed is 1.5
    - Pause: Press 'P' to pause the game
    - Game over: When a block reaches the bottom, game over screen is displayed
    - Blinking animation: When a layer is filled, it blinks for 6 times before disappearing
    - Key display: Shows the last key pressed for 1 second
    - Score display: Shows the current score
    - Game area display: Shows the game area grid
    - Game area color: Different colors for different blocks
    - Game area rotation: Rotates the game area 90 degrees to the right

    We have used the following keys:
    - 'W' to move forward
    - 'S' to move backward
    - 'A' to move left
    - 'D' to move right
    - 'P' to pause the game
    - 'H' to rotate the game area 90 degrees to the left
    - 'K' to rotate the game area 90 degrees to the right

*/

#include <algorithm>
#include <random>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include <map>
#define _USE_MATH_DEFINES
#include <math.h>
#include <GL/glew.h>
//#include <OpenGL/gl3.h>   // The GL Header File
#include <GLFW/glfw3.h> // The GLFW header
#include <glm/glm.hpp> // GL Math library header
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp> 
#include <ft2build.h>
#include FT_FREETYPE_H

#define BUFFER_OFFSET(i) ((char*)NULL + (i))

using namespace std;

GLuint gProgram[3];
int gWidth = 600, gHeight = 1000;
GLuint gVertexAttribBuffer, gTextVBO, gIndexBuffer;
GLuint gTex2D;
int gVertexDataSizeInBytes, gNormalDataSizeInBytes;
int gTriangleIndexDataSizeInBytes, gLineIndexDataSizeInBytes;

GLint modelingMatrixLoc[2];
GLint viewingMatrixLoc[2];
GLint projectionMatrixLoc[2];
GLint eyePosLoc[2];
GLint lightPosLoc[2];
GLint kdLoc[2];

glm::mat4 projectionMatrix;
glm::mat4 viewingMatrix;
glm::mat4 modelingMatrix = glm::translate(glm::mat4(1.f), glm::vec3(-0.5, -0.5, -0.5));
glm::vec3 eyePos = glm::vec3(0, 0, 24);
glm::vec3 lightPos = glm::vec3(0, 0, 7);

glm::vec3 kdGround(0.334, 0.288, 0.635); // this is the ground color in the demo
glm::vec3 kdCubes(0.86, 0.11, 0.31);

int activeProgramIndex = 0;

// Holds all state information relevant to a character as loaded using FreeType
struct Character {
    GLuint TextureID;   // ID handle of the glyph texture
    glm::ivec2 Size;    // Size of glyph
    glm::ivec2 Bearing;  // Offset from baseline to left/top of glyph
    GLuint Advance;    // Horizontal offset to advance to next glyph
};

std::map<GLchar, Character> Characters;


/*
-------------------------------------------------------------------------------
        OUR VARIABLES
*/

enum ShapeType { 
    SHAPE_3x3x3, 
    SHAPE_1x3x1, 
    SHAPE_3x1x3, 
    SPECIAL_EXPLOSIVE  // Explodes and clears surrounding blocks
};
ShapeType currentShape = SHAPE_3x3x3;
ShapeType explosiveShape = SHAPE_1x3x1;


glm::vec3 color;
glm::vec3 gsred(0.66, 0.01, 0.2);
glm::vec3 gsyellow(0.99, 0.73, 0.07);
glm::vec3 green(0.0, 1.0, 0.0);
glm::vec3 cyan(0.0, 1.0, 1.0);
glm::vec3 specialBlockColor(0.4, 0.0, 0.6); // Orange for special blocks

glm::mat4 rotationMatrix(1.0f);

const int GAMEAREA_WIDTH = 9;
const int GAMEAREA_HEIGHT = 15;
const int GAMEAREA_DEPTH = 9;
int gameArea[GAMEAREA_HEIGHT][GAMEAREA_WIDTH][GAMEAREA_DEPTH] = {0}; // 0: empty, 1: occupied
glm::vec3 gameAreaColor[GAMEAREA_HEIGHT][GAMEAREA_WIDTH][GAMEAREA_DEPTH];

float posX = -1.5f;
float posY = 5.0f;
float posZ = -1.5f;

int over = 0;

float gameRotationAngle = 0.0f;
const float ROTATION_SPEED = 110.0f; // degrees per second
bool isRotating = false;
float targetRotationAngle = 0.0f;
float currentRotationAngle = 0.0f;


const glm::vec3 DIRECTIONS[4] = {
    glm::vec3(1.0f, 0.0f, 0.0f),   // Front view - move +X
    glm::vec3(0.0f, 0.0f, 1.0f),  // Right view - move -Z
    glm::vec3(-1.0f, 0.0f, 0.0f),  // Back view - move -X
    glm::vec3(0.0f, 0.0f, -1.0f)    // Left view - move +Z
};


int score = 0;
std::string lastKeyPressed = "";
double lastKeyPressTime = 0.0;
const double KEY_DISPLAY_DURATION = 1.0;

float fallTime = 1.0f;
float fallSpeed = 1.0f;
const float MIN_FALL_SPEED = 0.25f;
const float MAX_FALL_SPEED = 1.5f; 
const float SPEED_CHANGE = 0.125f;

bool isPaused = false;

double currentTime = 0.0f;
float prePauseCurrentTime = 0.0f;


struct BlinkingLayer {
    std::vector<int> layerYs;  // Store multiple layer Y positions
    float blinkTimer;
    int blinkCount;
    bool isBlinking;
};
BlinkingLayer blinkingLayer = {{}, 0.0f, 0, false};
const float BLINK_DURATION = 0.3f;  // Duration for each blink
const int BLINK_COUNT = 6;

bool explosiveBlockPresent = false;
float explosiveBlockStartTime = 0.0f;
float explosiveBlockDuration = 0.0f;

/*
-------------------------------------------------------------------------------
*/

void drawCube()
{
	glUseProgram(gProgram[0]);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
}

void drawCubeEdges()
{
    glLineWidth(3);

	glUseProgram(gProgram[1]);

    for (int i = 0; i < 6; ++i)
    {
	    glDrawElements(GL_LINE_LOOP, 4, GL_UNSIGNED_INT, BUFFER_OFFSET(gTriangleIndexDataSizeInBytes + i * 4 * sizeof(GLuint)));
    }
}

/*
-------------------------------------------------------------------------------
        OUR FUNCTIONS
*/

void selectRandomShape() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(0, 99);
    static std::uniform_int_distribution<> shapeDis(0, 2);
    
    int random = dis(gen);
    if(random < 70) { // 70% normal blocks
        currentShape = static_cast<ShapeType>(shapeDis(gen));
    }
    else { // 30% explosive
        currentShape = SPECIAL_EXPLOSIVE;
        // Randomly select which shape to use for explosive block
        int shapeType = shapeDis(gen);
        switch(shapeType) {
            case 0: explosiveShape = SHAPE_3x3x3; break;
            case 1: explosiveShape = SHAPE_1x3x1; break;
            case 2: explosiveShape = SHAPE_3x1x3; break;
        }
    }
}

void handleSpecialBlockEffect(int x, int y, int z) {
    if(currentShape == SPECIAL_EXPLOSIVE) {
        int totalRemoved = 0;
        
        // Check only blocks sharing a face (6 directions)
        // const int directions[6][3] = {
        //     {1, 0, 0},  // right
        //     {-1, 0, 0}, // left
        //     {0, 1, 0},  // up
        //     {0, -1, 0}, // down
        //     {0, 0, 1},  // front
        //     {0, 0, -1}  // back
        // };

        const int directions[1][3] = {
            {0, -1, 0} // down
        };
        
        // Remove the block itself
        if(gameArea[y][x][z] == 1) {
            gameArea[y][x][z] = 0;
        }
        
        // Clear only adjacent blocks
        for(int i = 0; i < 1; i++) {
            int newX = x + directions[i][0];
            int newY = y + directions[i][1];
            int newZ = z + directions[i][2];
            
            if(newX >= 0 && newX < GAMEAREA_WIDTH &&
               newY >= 0 && newY < GAMEAREA_HEIGHT &&
               newZ >= 0 && newZ < GAMEAREA_DEPTH) {
                if(gameArea[newY][newX][newZ] == 1) {
                    gameArea[newY][newX][newZ] = 0;
                    totalRemoved++;
                }
            }
        }
        
        score += totalRemoved * 10;
    }
}

void setInitialPosition() {
    if(currentShape == SPECIAL_EXPLOSIVE) {
        // Use explosiveShape to determine position
        switch(explosiveShape) {
            case SHAPE_3x3x3:
                posX = -1.5f;
                posY = 5.0f;
                posZ = -1.5f;
                break;
            case SHAPE_1x3x1:
                posX = -0.5f;
                posY = 5.0f;
                posZ = -0.5f;
                break;
            case SHAPE_3x1x3:
                posX = -1.5f;
                posY = 5.0f;
                posZ = -1.5f;
                break;
        }
    }
    else {
        // Normal shape positioning
        switch(currentShape) {
            case SHAPE_3x3x3:
                posX = -1.5f;
                posY = 5.0f;
                posZ = -1.5f;
                break;
            case SHAPE_1x3x1:
                posX = -0.5f;
                posY = 5.0f;
                posZ = -0.5f;
                break;
            case SHAPE_3x1x3:
                posX = -1.5f;
                posY = 5.0f;
                posZ = -1.5f;
                break;
        }
    }
}

void drawGround()
{
    float startX = -4.5f;
    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 9; j++) {
            modelingMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(startX + j, -7.5, startX + i));
            modelingMatrix = modelingMatrix * glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, 0.5f, 1.0f));
            modelingMatrix = rotationMatrix * modelingMatrix;

            for (int prog = 0; prog < 2; prog++) {
                glUseProgram(gProgram[prog]);
                glUniformMatrix4fv(modelingMatrixLoc[prog], 1, GL_FALSE, glm::value_ptr(modelingMatrix));
                glUniform3fv(kdLoc[prog], 1, glm::value_ptr(gsyellow));
            }
            drawCube();
            drawCubeEdges();
        }
    }
}

void submitTheObject(){
    for (int y = 0; y < GAMEAREA_HEIGHT; y++) {
        for (int x = 0; x < GAMEAREA_WIDTH; x++) {
            for (int z = 0; z < GAMEAREA_DEPTH; z++) {
                if (gameArea[y][x][z]) {

                    if (blinkingLayer.isBlinking && (std::find(blinkingLayer.layerYs.begin(), blinkingLayer.layerYs.end(), y) != blinkingLayer.layerYs.end()) && (blinkingLayer.blinkCount % 2 == 1)) {
                        continue;
                    }
                 
                    float worldX = x - 4.5f;
                    float worldY = y - 7.0f;
                    float worldZ = z - 4.5f;
                    
                    modelingMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(worldX, worldY, worldZ));
                    glm::mat4 finalMatrix = modelingMatrix;
                    finalMatrix = rotationMatrix * finalMatrix;

                    for (int prog = 0; prog < 2; prog++) {
                        glUseProgram(gProgram[prog]);
                        glUniformMatrix4fv(modelingMatrixLoc[prog], 1, GL_FALSE, glm::value_ptr(finalMatrix));
                        glUniform3fv(kdLoc[prog], 1, glm::value_ptr(gameAreaColor[y][x][z]));
                    }

                    drawCube();
                    drawCubeEdges();
                }
            }
        }
    }
}

void createRotationMatrix(){
    if(isRotating)
    {
        float deltaTime = 1.0f/60.0f;
        float rotationStep = ROTATION_SPEED * deltaTime;
        
        float angleDiff = targetRotationAngle - currentRotationAngle;
        if (angleDiff > 180.0f) {
            angleDiff -= 360.0f;
        } else if (angleDiff < -180.0f) {
            angleDiff += 360.0f;
        }
        
        float step = (angleDiff > 0) ? rotationStep : -rotationStep;
        
        if (abs(angleDiff) <= rotationStep) {
            currentRotationAngle = targetRotationAngle;
            isRotating = false;
        } else {
            currentRotationAngle += step;
        }
        
        if (currentRotationAngle >= 360.0f) {
            currentRotationAngle -= 360.0f;
        } else if (currentRotationAngle < 0.0f) {
            currentRotationAngle += 360.0f;
        }

        rotationMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(currentRotationAngle), glm::vec3(0.0f, 1.0f, 0.0f));
    }
}

// void checkAndRemoveFilledLayers() {
//     for(int y = 0; y < GAMEAREA_HEIGHT; y++) {
//         bool layerFilled = true;
//         int blockCount = 0;
        
//         for(int x = 0; x < GAMEAREA_WIDTH && layerFilled; x++) {
//             for(int z = 0; z < GAMEAREA_DEPTH && layerFilled; z++) {
//                 if(gameArea[y][x][z] == 1) {
//                     blockCount++;
//                 } else {
//                     layerFilled = false;
//                 }
//             }
//         }

//         if(layerFilled) {
//             score += blockCount;

//             for(int moveY = y; moveY < GAMEAREA_HEIGHT - 1; moveY++) {
//                 for(int x = 0; x < GAMEAREA_WIDTH; x++) {
//                     for(int z = 0; z < GAMEAREA_DEPTH; z++) {
//                         gameArea[moveY][x][z] = gameArea[moveY + 1][x][z];
//                         gameAreaColor[moveY][x][z] = gameAreaColor[moveY + 1][x][z];
//                     }
//                 }
//             }

//             for(int x = 0; x < GAMEAREA_WIDTH; x++) {
//                 for(int z = 0; z < GAMEAREA_DEPTH; z++) {
//                     gameArea[GAMEAREA_HEIGHT - 1][x][z] = 0;
//                 }
//             }

//             y--;
//         }
//     }
// }

void checkAndRemoveFilledLayers() {
    if (blinkingLayer.isBlinking) return;

    std::vector<int> filledLayers;
    
    // Check all layers first
    for(int y = 0; y < GAMEAREA_HEIGHT; y++) {
        bool layerFilled = true;
        
        for(int x = 0; x < GAMEAREA_WIDTH && layerFilled; x++) {
            for(int z = 0; z < GAMEAREA_DEPTH && layerFilled; z++) {
                if(gameArea[y][x][z] == 0) {
                    layerFilled = false;
                }
            }
        }

        if(layerFilled) {
            filledLayers.push_back(y);
        }
    }

    // If any layers are filled, start blinking
    if(!filledLayers.empty()) {
        blinkingLayer.layerYs = filledLayers;
        blinkingLayer.blinkTimer = glfwGetTime();
        blinkingLayer.blinkCount = 0;
        blinkingLayer.isBlinking = true;
    }
}

void handleBlinkingAnimation(){
    if (blinkingLayer.isBlinking) {
        float currentTime = glfwGetTime();
        float timeDiff = currentTime - blinkingLayer.blinkTimer;
        
        if (timeDiff >= BLINK_DURATION) {
            blinkingLayer.blinkCount++;
            blinkingLayer.blinkTimer = currentTime;
            
            if (blinkingLayer.blinkCount >= BLINK_COUNT) {
                int totalBlocks = 0;
                
                std::sort(blinkingLayer.layerYs.begin(), blinkingLayer.layerYs.end(), std::greater<int>());
                
                for(int y : blinkingLayer.layerYs) {
                    for(int x = 0; x < GAMEAREA_WIDTH; x++) {
                        for(int z = 0; z < GAMEAREA_DEPTH; z++) {
                            if(gameArea[y][x][z] == 1) totalBlocks++;
                        }
                    }
                    
                    for(int moveY = y; moveY < GAMEAREA_HEIGHT - 1; moveY++) {
                        for(int x = 0; x < GAMEAREA_WIDTH; x++) {
                            for(int z = 0; z < GAMEAREA_DEPTH; z++) {
                                gameArea[moveY][x][z] = gameArea[moveY + 1][x][z];
                                gameAreaColor[moveY][x][z] = gameAreaColor[moveY + 1][x][z];
                            }
                        }
                    }
                    
                    for(int x = 0; x < GAMEAREA_WIDTH; x++) {
                        for(int z = 0; z < GAMEAREA_DEPTH; z++) {
                            gameArea[GAMEAREA_HEIGHT - 1][x][z] = 0;
                        }
                    }
                }
                
                score += totalBlocks;
                
                blinkingLayer.isBlinking = false;
                blinkingLayer.layerYs.clear();
                checkAndRemoveFilledLayers();
            }
        }
    }
}

bool collisionCheck(float x, float y, float z) {
    
    int mappedX =  static_cast<int>(x + 4.5f);
    int mappedY =  static_cast<int>(y + 7.0f);
    int mappedZ =  static_cast<int>(z + 4.5f);

    if (mappedX < 0 || mappedX >= GAMEAREA_WIDTH ||
        mappedY < 0 || mappedY >= GAMEAREA_HEIGHT ||
        mappedZ < 0 || mappedZ >= GAMEAREA_DEPTH) {
        return true;
    }

    if(gameArea[mappedY][mappedX][mappedZ] == 1){
        return true;
    }
    return false;

}

void moveObject3x3x3(){
    currentTime = glfwGetTime();
    bool flag = false;

    if (currentTime > fallTime && !isPaused) {
        fallTime = currentTime + fallSpeed;
        for (int i = 0 ; i < 3 ; i++) {
            for (int j = 0 ; j < 3 ; j++) {
                for (int k = 0 ; k < 3 ; k++) {
                    
                    if(collisionCheck(posX+i, posY -1 +j, posZ+k)){
                        flag = true;
                        break;
                    }
                }
            }
        }

        if(flag || posY <= -7.0f){
            for (int i = 0 ; i < 3 ; i++) {
                for (int j = 0 ; j < 3 ; j++) {
                    for (int k = 0 ; k < 3 ; k++) {
                        int mappedX =  static_cast<int>(posX + i + 4.5f);
                        int mappedY =  static_cast<int>(posY + j + 7.0f);
                        int mappedZ =  static_cast<int>(posZ + k + 4.5f);
                        
                        if (mappedX >= 0 && mappedX < GAMEAREA_WIDTH &&
                            mappedY >= 0 && mappedY < GAMEAREA_HEIGHT &&
                            mappedZ >= 0 && mappedZ < GAMEAREA_DEPTH){

                                if(currentShape == SPECIAL_EXPLOSIVE) {
                                    handleSpecialBlockEffect(mappedX, mappedY, mappedZ);
                                    explosiveBlockPresent = true;
                                    explosiveBlockStartTime = glfwGetTime();
                                }
                                else{
                                    gameArea[mappedY][mappedX][mappedZ] = 1;
                                    gameAreaColor[mappedY][mappedX][mappedZ] = gsred;
                                }
                        }
                    }
                }
            }

            checkAndRemoveFilledLayers();

            if(!over){
                selectRandomShape();
                setInitialPosition();
            }
        }
        else{
            posY -= 1;
        }
    }
}

void moveObject1x3x1() {
    currentTime = glfwGetTime();
    bool flag = false;

    if (currentTime > fallTime && !isPaused) {
        fallTime = currentTime + fallSpeed;

        for (int j = 0; j < 3; j++) {
            if(collisionCheck(posX, posY - 1 + j, posZ)){
                flag = true;
                break;
            }
        }

        if(flag || posY <= -7.0f){
            for (int j = 0; j < 3; j++) {
                int mappedX = static_cast<int>(posX + 4.5f);
                int mappedY = static_cast<int>(posY + j + 7.0f);
                int mappedZ = static_cast<int>(posZ + 4.5f);
                
                if (mappedX >= 0 && mappedX < GAMEAREA_WIDTH &&
                    mappedY >= 0 && mappedY < GAMEAREA_HEIGHT &&
                    mappedZ >= 0 && mappedZ < GAMEAREA_DEPTH) {
                        if(currentShape == SPECIAL_EXPLOSIVE) {
                            handleSpecialBlockEffect(mappedX, mappedY, mappedZ);
                            explosiveBlockPresent = true;
                            explosiveBlockStartTime = glfwGetTime();
                        }
                        else{
                            gameArea[mappedY][mappedX][mappedZ] = 1;
                            gameAreaColor[mappedY][mappedX][mappedZ] = green;
                        }
                }
            }

            checkAndRemoveFilledLayers();

            if(!over){
                selectRandomShape();
                setInitialPosition();
            }
        }
        else{
            posY -= 1;
        }
    }
}

void moveObject3x1x3() {
    currentTime = glfwGetTime();
    bool flag = false;

    if (currentTime > fallTime && !isPaused) {
        fallTime = currentTime + fallSpeed;
        
        for (int i = 0; i < 3; i++) {
            for (int k = 0; k < 3; k++) {
                if(collisionCheck(posX + k, posY - 1, posZ + i)){
                    flag = true;
                    break;
                }
            }
        }

        if(flag || posY <= -7.0f){
            for (int i = 0; i < 3; i++) {
                for (int k = 0; k < 3; k++) {
                    int mappedX = static_cast<int>(posX + k + 4.5f);
                    int mappedY = static_cast<int>(posY + 7.0f);
                    int mappedZ = static_cast<int>(posZ + i + 4.5f);
                    
                    if (mappedX >= 0 && mappedX < GAMEAREA_WIDTH &&
                        mappedY >= 0 && mappedY < GAMEAREA_HEIGHT &&
                        mappedZ >= 0 && mappedZ < GAMEAREA_DEPTH) {
                            if(currentShape == SPECIAL_EXPLOSIVE) {
                                handleSpecialBlockEffect(mappedX, mappedY, mappedZ);
                                explosiveBlockPresent = true;
                                explosiveBlockStartTime = glfwGetTime();
                            }
                            else{
                                gameArea[mappedY][mappedX][mappedZ] = 1;
                                gameAreaColor[mappedY][mappedX][mappedZ] = cyan;
                            }
                    }
                }
            }

            checkAndRemoveFilledLayers();

            if(!over){
                selectRandomShape();
                setInitialPosition();
            }
        }
        else{
            posY -= 1;
        }
    }
}

int isGameOver(){
    int checkHeight = 0;
    switch(currentShape) {
        case SHAPE_3x3x3:
            checkHeight = 3;
            break;
        case SHAPE_1x3x1:
            checkHeight = 3;
            break;
        case SHAPE_3x1x3:
            checkHeight = 1;
            break;
    }

    for(int i = 3; i < 6; i++){
        for(int j = 3; j < 6; j++){
            for(int k = GAMEAREA_HEIGHT - checkHeight; k < GAMEAREA_HEIGHT; k++){
                if(gameArea[k][i][j] == 1){
                    return 1;
                }
            }
        }
    }
    return 0;
}

void drawShape3x3x3()
{
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            for (int k = 0; k < 3; k++) {
                modelingMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(posX + k, posY + j, posZ + i));
                glm::mat4 finalMatrix = modelingMatrix;
                finalMatrix = rotationMatrix * finalMatrix;

                for (int prog = 0; prog < 2; prog++) {
                    glUseProgram(gProgram[prog]);
                    glUniformMatrix4fv(modelingMatrixLoc[prog], 1, GL_FALSE, glm::value_ptr(finalMatrix));
                    glUniform3fv(kdLoc[prog], 1, glm::value_ptr(color));
                }
                
                drawCube();
                drawCubeEdges();
            }
        }
    }
}

void drawShape1x3x1()
{
    for (int j = 0; j < 3; j++) {
        modelingMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(posX, posY + j, posZ));
        glm::mat4 finalMatrix = modelingMatrix;
        finalMatrix = rotationMatrix * finalMatrix;

        for (int prog = 0; prog < 2; prog++) {
            glUseProgram(gProgram[prog]);
            glUniformMatrix4fv(modelingMatrixLoc[prog], 1, GL_FALSE, glm::value_ptr(finalMatrix));
            glUniform3fv(kdLoc[prog], 1, glm::value_ptr(color));
        }
        
        drawCube();
        drawCubeEdges();
    }
}

void drawShape3x1x3()
{
    for (int i = 0; i < 3; i++) {
        for (int k = 0; k < 3; k++) {
            modelingMatrix = glm::translate(glm::mat4(1.0f), 
                glm::vec3(posX + k, posY, posZ + i));
            glm::mat4 finalMatrix = modelingMatrix;
            finalMatrix = rotationMatrix * finalMatrix;

            for (int prog = 0; prog < 2; prog++) {
                glUseProgram(gProgram[prog]);
                glUniformMatrix4fv(modelingMatrixLoc[prog], 1, GL_FALSE, 
                    glm::value_ptr(finalMatrix));
                glUniform3fv(kdLoc[prog], 1, glm::value_ptr(color));
            }
            
            drawCube();
            drawCubeEdges();
        }
    }
}

void drawObjects(){
    if(!over && !blinkingLayer.isBlinking){
        switch(currentShape) {
            case SHAPE_3x3x3:
                color = gsred;
                drawShape3x3x3();
                moveObject3x3x3();
                break;
            case SHAPE_1x3x1:
                color = green;
                drawShape1x3x1();
                moveObject1x3x1();
                break;
            case SHAPE_3x1x3:
                color = cyan;
                drawShape3x1x3();
                moveObject3x1x3();
                break;
            case SPECIAL_EXPLOSIVE:
                color = specialBlockColor;  // Orange color
                switch(explosiveShape) {
                    case SHAPE_3x3x3:
                        drawShape3x3x3();
                        moveObject3x3x3();
                        break;
                    case SHAPE_1x3x1:
                        drawShape1x3x1();
                        moveObject1x3x1();
                        break;
                    case SHAPE_3x1x3:
                        drawShape3x1x3();
                        moveObject3x1x3();
                        break;
                }
                break;
        }
    }
}

/*
-------------------------------------------------------------------------------
*/


// For reading GLSL files
bool ReadDataFromFile(
    const string& fileName, ///< [in]  Name of the shader file
    string&       data)     ///< [out] The contents of the file
{
    fstream myfile;

    // Open the input 
    myfile.open(fileName.c_str(), std::ios::in);

    if (myfile.is_open())
    {
        string curLine;

        while (getline(myfile, curLine))
        {
            data += curLine;
            if (!myfile.eof())
            {
                data += "\n";
            }
        }

        myfile.close();
    }
    else
    {
        return false;
    }

    return true;
}

GLuint createVS(const char* shaderName)
{
    string shaderSource;

    string filename(shaderName);
    if (!ReadDataFromFile(filename, shaderSource))
    {
        cout << "Cannot find file name: " + filename << endl;
        exit(-1);
    }

    GLint length = shaderSource.length();
    const GLchar* shader = (const GLchar*) shaderSource.c_str();

    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &shader, &length);
    glCompileShader(vs);

    char output[1024] = {0};
    glGetShaderInfoLog(vs, 1024, &length, output);
    printf("VS compile log: %s\n", output);

	return vs;
}

GLuint createFS(const char* shaderName)
{
    string shaderSource;

    string filename(shaderName);
    if (!ReadDataFromFile(filename, shaderSource))
    {
        cout << "Cannot find file name: " + filename << endl;
        exit(-1);
    }

    GLint length = shaderSource.length();
    const GLchar* shader = (const GLchar*) shaderSource.c_str();

    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &shader, &length);
    glCompileShader(fs);

    char output[1024] = {0};
    glGetShaderInfoLog(fs, 1024, &length, output);
    printf("FS compile log: %s\n", output);

	return fs;
}

void initFonts(int windowWidth, int windowHeight)
{
    // Set OpenGL options
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glm::mat4 projection = glm::ortho(0.0f, static_cast<GLfloat>(windowWidth), 0.0f, static_cast<GLfloat>(windowHeight));
    glUseProgram(gProgram[2]);
    glUniformMatrix4fv(glGetUniformLocation(gProgram[2], "projection"), 1, GL_FALSE, glm::value_ptr(projection));

    // FreeType
    FT_Library ft;
    // All functions return a value different than 0 whenever an error occurred
    if (FT_Init_FreeType(&ft))
    {
        std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
    }

    // Load font as face
    FT_Face face;
    if (FT_New_Face(ft, "C:/Windows/Fonts/arial.ttf", 0, &face))
    //if (FT_New_Face(ft, "/usr/share/fonts/truetype/gentium-basic/GenBkBasR.ttf", 0, &face)) // you can use different fonts
    {
        std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;
    }

    // Set size to load glyphs as
    FT_Set_Pixel_Sizes(face, 0, 48);

    // Disable byte-alignment restriction
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1); 

    // Load first 128 characters of ASCII set
    for (GLubyte c = 0; c < 128; c++)
    {
        // Load character glyph 
        if (FT_Load_Char(face, c, FT_LOAD_RENDER))
        {
            std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
            continue;
        }
        // Generate texture
        GLuint texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(
                GL_TEXTURE_2D,
                0,
                GL_RED,
                face->glyph->bitmap.width,
                face->glyph->bitmap.rows,
                0,
                GL_RED,
                GL_UNSIGNED_BYTE,
                face->glyph->bitmap.buffer
                );
        // Set texture options
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // Now store character for later use
        Character character = {
            texture,
            glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
            glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
            (GLuint) face->glyph->advance.x
        };
        Characters.insert(std::pair<GLchar, Character>(c, character));
    }

    glBindTexture(GL_TEXTURE_2D, 0);
    // Destroy FreeType once we're finished
    FT_Done_Face(face);
    FT_Done_FreeType(ft);

    //
    // Configure VBO for texture quads
    //
    glGenBuffers(1, &gTextVBO);
    glBindBuffer(GL_ARRAY_BUFFER, gTextVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 4, NULL, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void initShaders()
{
	// Create the programs

    gProgram[0] = glCreateProgram();
	gProgram[1] = glCreateProgram();
	gProgram[2] = glCreateProgram();

	// Create the shaders for both programs

    GLuint vs1 = createVS("vert.glsl"); // for cube shading
    GLuint fs1 = createFS("frag.glsl");

	GLuint vs2 = createVS("vert2.glsl"); // for border shading
	GLuint fs2 = createFS("frag2.glsl");

	GLuint vs3 = createVS("vert_text.glsl");  // for text shading
	GLuint fs3 = createFS("frag_text.glsl");

	// Attach the shaders to the programs

	glAttachShader(gProgram[0], vs1);
	glAttachShader(gProgram[0], fs1);

	glAttachShader(gProgram[1], vs2);
	glAttachShader(gProgram[1], fs2);

	glAttachShader(gProgram[2], vs3);
	glAttachShader(gProgram[2], fs3);

	// Link the programs

    for (int i = 0; i < 3; ++i)
    {
        glLinkProgram(gProgram[i]);
        GLint status;
        glGetProgramiv(gProgram[i], GL_LINK_STATUS, &status);

        if (status != GL_TRUE)
        {
            cout << "Program link failed: " << i << endl;
            exit(-1);
        }
    }


	// Get the locations of the uniform variables from both programs

	for (int i = 0; i < 2; ++i)
	{
		modelingMatrixLoc[i] = glGetUniformLocation(gProgram[i], "modelingMatrix");
		viewingMatrixLoc[i] = glGetUniformLocation(gProgram[i], "viewingMatrix");
		projectionMatrixLoc[i] = glGetUniformLocation(gProgram[i], "projectionMatrix");
		eyePosLoc[i] = glGetUniformLocation(gProgram[i], "eyePos");
		lightPosLoc[i] = glGetUniformLocation(gProgram[i], "lightPos");
		kdLoc[i] = glGetUniformLocation(gProgram[i], "kd");

        glUseProgram(gProgram[i]);
        glUniformMatrix4fv(modelingMatrixLoc[i], 1, GL_FALSE, glm::value_ptr(modelingMatrix));
        glUniform3fv(eyePosLoc[i], 1, glm::value_ptr(eyePos));
        glUniform3fv(lightPosLoc[i], 1, glm::value_ptr(lightPos));
        glUniform3fv(kdLoc[i], 1, glm::value_ptr(kdCubes));
	}
}

// VBO setup for drawing a cube and its borders
void initVBO()
{
    GLuint vao;
    glGenVertexArrays(1, &vao);
    assert(vao > 0);
    glBindVertexArray(vao);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	assert(glGetError() == GL_NONE);

	glGenBuffers(1, &gVertexAttribBuffer);
	glGenBuffers(1, &gIndexBuffer);

	assert(gVertexAttribBuffer > 0 && gIndexBuffer > 0);

	glBindBuffer(GL_ARRAY_BUFFER, gVertexAttribBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gIndexBuffer);

    GLuint indices[] = {
        0, 1, 2, // front
        3, 0, 2, // front
        4, 7, 6, // back
        5, 4, 6, // back
        0, 3, 4, // left
        3, 7, 4, // left
        2, 1, 5, // right
        6, 2, 5, // right
        3, 2, 7, // top
        2, 6, 7, // top
        0, 4, 1, // bottom
        4, 5, 1  // bottom
    };

    GLuint indicesLines[] = {
        7, 3, 2, 6, // top
        4, 5, 1, 0, // bottom
        2, 1, 5, 6, // right
        5, 4, 7, 6, // back
        0, 1, 2, 3, // front
        0, 3, 7, 4, // left
    };

    GLfloat vertexPos[] = {
        0, 0, 1, // 0: bottom-left-front
        1, 0, 1, // 1: bottom-right-front
        1, 1, 1, // 2: top-right-front
        0, 1, 1, // 3: top-left-front
        0, 0, 0, // 0: bottom-left-back
        1, 0, 0, // 1: bottom-right-back
        1, 1, 0, // 2: top-right-back
        0, 1, 0, // 3: top-left-back
    };

    GLfloat vertexNor[] = {
         1.0,  1.0,  1.0, // 0: unused
         0.0, -1.0,  0.0, // 1: bottom
         0.0,  0.0,  1.0, // 2: front
         1.0,  1.0,  1.0, // 3: unused
        -1.0,  0.0,  0.0, // 4: left
         1.0,  0.0,  0.0, // 5: right
         0.0,  0.0, -1.0, // 6: back 
         0.0,  1.0,  0.0, // 7: top
    };

	gVertexDataSizeInBytes = sizeof(vertexPos);
	gNormalDataSizeInBytes = sizeof(vertexNor);
    gTriangleIndexDataSizeInBytes = sizeof(indices);
    gLineIndexDataSizeInBytes = sizeof(indicesLines);
    int allIndexSize = gTriangleIndexDataSizeInBytes + gLineIndexDataSizeInBytes;

	glBufferData(GL_ARRAY_BUFFER, gVertexDataSizeInBytes + gNormalDataSizeInBytes, 0, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, gVertexDataSizeInBytes, vertexPos);
	glBufferSubData(GL_ARRAY_BUFFER, gVertexDataSizeInBytes, gNormalDataSizeInBytes, vertexNor);

	glBufferData(GL_ELEMENT_ARRAY_BUFFER, allIndexSize, 0, GL_STATIC_DRAW);
	glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, gTriangleIndexDataSizeInBytes, indices);
	glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, gTriangleIndexDataSizeInBytes, gLineIndexDataSizeInBytes, indicesLines);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(gVertexDataSizeInBytes));
}

void init() 
{
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    // polygon offset is used to prevent z-fighting between the cube and its borders
    glPolygonOffset(0.5, 0.5);
    glEnable(GL_POLYGON_OFFSET_FILL);

    initShaders();
    initVBO();
    initFonts(gWidth, gHeight);
}

void renderText(const std::string& text, GLfloat x, GLfloat y, GLfloat scale, glm::vec3 color)
{
    // Activate corresponding render state	
    glUseProgram(gProgram[2]);
    glUniform3f(glGetUniformLocation(gProgram[2], "textColor"), color.x, color.y, color.z);
    glActiveTexture(GL_TEXTURE0);

    // Iterate through all characters
    std::string::const_iterator c;
    for (c = text.begin(); c != text.end(); c++) 
    {
        Character ch = Characters[*c];

        GLfloat xpos = x + ch.Bearing.x * scale;
        GLfloat ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

        GLfloat w = ch.Size.x * scale;
        GLfloat h = ch.Size.y * scale;

        // Update VBO for each character
        GLfloat vertices[6][4] = {
            { xpos,     ypos + h,   0.0, 0.0 },            
            { xpos,     ypos,       0.0, 1.0 },
            { xpos + w, ypos,       1.0, 1.0 },

            { xpos,     ypos + h,   0.0, 0.0 },
            { xpos + w, ypos,       1.0, 1.0 },
            { xpos + w, ypos + h,   1.0, 0.0 }           
        };

        // Render glyph texture over quad
        glBindTexture(GL_TEXTURE_2D, ch.TextureID);

        // Update content of VBO memory
        glBindBuffer(GL_ARRAY_BUFFER, gTextVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); // Be sure to use glBufferSubData and not glBufferData

        //glBindBuffer(GL_ARRAY_BUFFER, 0);

        // Render quad
        glDrawArrays(GL_TRIANGLES, 0, 6);
        // Now advance cursors for next glyph (note that advance is number of 1/64 pixels)

        x += (ch.Advance >> 6) * scale; // Bitshift by 6 to get value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))
    }

    glBindTexture(GL_TEXTURE_2D, 0);
}

void display()
{
    glClearColor(0, 0, 0, 1);
    glClearDepth(1.0f);
    glClearStencil(0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    
    if(isGameOver()) over = 1;

    createRotationMatrix();

	drawGround();

    submitTheObject();

    handleBlinkingAnimation();

    drawObjects();

    // Show the game over text
    if(over == 1) {
        glm::mat4 textProjection = glm::ortho(0.0f, static_cast<float>(gWidth), 0.0f, static_cast<float>(gHeight));
        glUseProgram(gProgram[2]);
        glUniformMatrix4fv(glGetUniformLocation(gProgram[2], "projection"), 1, GL_FALSE, glm::value_ptr(textProjection));

        std::string gameOverText = "GAME OVER!";
        float textScale = 1.0f;
        float textWidth = 0.0f;
        
        for(char c : gameOverText) {
            Character ch = Characters[c];
            textWidth += (ch.Advance >> 6) * textScale;
        }
        
        float textX = (gWidth - textWidth) / 2.0f;
        float textY = gHeight / 2.0f;
        
        renderText(gameOverText, textX, textY, textScale, gsyellow);
    }
    // Show the game over text

    // Show the score
    glm::mat4 textProjection = glm::ortho(0.0f, static_cast<float>(gWidth), 0.0f, static_cast<float>(gHeight));
    glUseProgram(gProgram[2]);
    glUniformMatrix4fv(glGetUniformLocation(gProgram[2], "projection"), 1, GL_FALSE, glm::value_ptr(textProjection));

    std::string scoreText = "Score: " + std::to_string(score);
    float textScale = 0.5f;
    float textWidth = 0.0f;
    
    for(char c : scoreText) {
        Character ch = Characters[c];
        textWidth += (ch.Advance >> 6) * textScale;
    }
    
    float padding = 20.0f;
    float textX = gWidth - textWidth - padding;
    float textY = gHeight - padding;
    
    renderText(scoreText, textX, textY, textScale, gsyellow);
    // Show the score

    // Show the current view
    textProjection = glm::ortho(0.0f, static_cast<float>(gWidth), 0.0f, static_cast<float>(gHeight));
    glUseProgram(gProgram[2]);
    glUniformMatrix4fv(glGetUniformLocation(gProgram[2], "projection"), 1, GL_FALSE, glm::value_ptr(textProjection));

    std::string viewText;
    int viewIndex = ((int)(currentRotationAngle / 90.0f)) % 4;
    if (viewIndex < 0) viewIndex += 4;

    switch(viewIndex) {
        case 0: viewText = "View: Front"; break;
        case 1: viewText = "View: Left"; break;
        case 2: viewText = "View: Back";  break;
        case 3: viewText = "View: Right";  break;
    }

    padding = 20.0f;
    textX = padding;
    textY = gHeight - padding;
    textScale = 0.5f;

    renderText(viewText, textX, textY, textScale, gsyellow);
    // Show the current view

    // Show the last key pressed
    double currentTime = glfwGetTime();
    if (currentTime - lastKeyPressTime < KEY_DISPLAY_DURATION) {
        std::string keyText = lastKeyPressed;
        float keyTextX = padding;
        float keyTextY = gHeight - (2 * padding + 30.0f); 
        renderText(lastKeyPressed, keyTextX, keyTextY, textScale, gsred);
    }
    // Show the last key pressed

    // Show the explosive block warning  
    if(currentShape == SPECIAL_EXPLOSIVE) {
        // Show warning for current explosive block
        std::string warningText = "Explosive Block!";
        float textScale = 0.5f;
        float textWidth = 0.0f;
        
        for(char c : warningText) {
            Character ch = Characters[c];
            textWidth += (ch.Advance >> 6) * textScale;
        }
        
        float textX = (gWidth - textWidth) / 2.0f;
        float textY = gHeight - 50.0f;
        
        renderText(warningText, textX, textY, textScale, specialBlockColor);
    }
    // Show the explosive block warning

    assert(glGetError() == GL_NO_ERROR);
}

void reshape(GLFWwindow* window, int w, int h)
{
    w = w < 1 ? 1 : w;
    h = h < 1 ? 1 : h;

    gWidth = w;
    gHeight = h;

    glViewport(0, 0, w, h);

	// Use perspective projection

	float fovyRad = (float) (45.0 / 180.0) * M_PI;
	projectionMatrix = glm::perspective(fovyRad, gWidth / (float) gHeight, 1.0f, 100.0f);

    // always look toward (0, 0, 0)
	viewingMatrix = glm::lookAt(eyePos, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));

    for (int i = 0; i < 2; ++i)
    {
        glUseProgram(gProgram[i]);
        glUniformMatrix4fv(projectionMatrixLoc[i], 1, GL_FALSE, glm::value_ptr(projectionMatrix));
        glUniformMatrix4fv(viewingMatrixLoc[i], 1, GL_FALSE, glm::value_ptr(viewingMatrix));
    }
}

void keyboard(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if ((key == GLFW_KEY_Q || key == GLFW_KEY_ESCAPE) && action == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }

    if (action == GLFW_PRESS)
    {
        int viewIndex = ((int)(currentRotationAngle / 90.0f)) % 4;
        if (viewIndex < 0) viewIndex += 4;
        float rotationAmount;
        bool canMove;
        glm::vec3 moveDir;

        switch(key) {
            case GLFW_KEY_W:
                if(!over){
                    fallSpeed = std::min(MAX_FALL_SPEED, fallSpeed + SPEED_CHANGE);
                    // Calculate speed percentage (inverse because lower fallSpeed means faster game)
                    int speedPercent = static_cast<int>((MAX_FALL_SPEED - fallSpeed) / (MAX_FALL_SPEED - MIN_FALL_SPEED) * 100);
                    lastKeyPressed = "W - Speed: " + std::to_string(speedPercent) + "%";
                    lastKeyPressTime = glfwGetTime();
                }
                break;
                
            case GLFW_KEY_S:
                if(!over){
                    fallSpeed = std::max(MIN_FALL_SPEED, fallSpeed - SPEED_CHANGE);
                    // Calculate speed percentage (inverse because lower fallSpeed means faster game)
                    int speedPercent = static_cast<int>((MAX_FALL_SPEED - fallSpeed) / (MAX_FALL_SPEED - MIN_FALL_SPEED) * 100);
                    lastKeyPressed = "S - Speed: " + std::to_string(speedPercent) + "%";
                    lastKeyPressTime = glfwGetTime();
                }
                break;

            case GLFW_KEY_A:
                if(!over) {
                    lastKeyPressed = "A";
                    lastKeyPressTime = glfwGetTime();
                    moveDir = -DIRECTIONS[viewIndex];
                    canMove = true;

                    if(currentShape == SPECIAL_EXPLOSIVE) {
                        // Use explosiveShape to determine checks
                        switch(explosiveShape) {
                            case SHAPE_3x3x3:
                                for (int i = 0; i < 3 && canMove; i++) {
                                    for (int j = 0; j < 3 && canMove; j++) {
                                        for (int k = 0; k < 3 && canMove; k++) {
                                            float newX = posX + k + moveDir.x;
                                            float newZ = posZ + i + moveDir.z;
                                            if (collisionCheck(newX, posY + j, newZ)) {
                                                canMove = false;
                                            }
                                        }
                                    }
                                }
                                break;
                            case SHAPE_1x3x1:
                                for(int j = 0; j < 3 && canMove; j++) {
                                    float newX = posX + moveDir.x;
                                    float newZ = posZ + moveDir.z;
                                    if (collisionCheck(newX, posY + j, newZ)) {
                                        canMove = false;
                                    }
                                }
                                break;
                            case SHAPE_3x1x3:
                                for (int i = 0; i < 3 && canMove; i++) {
                                    for (int k = 0; k < 3 && canMove; k++) {
                                        float newX = posX + k + moveDir.x;
                                        float newZ = posZ + i + moveDir.z;
                                        if (collisionCheck(newX, posY, newZ)) {
                                            canMove = false;
                                        }
                                    }
                                }
                                break;
                        }
                    }
                    else{
                        if(currentShape == SHAPE_3x3x3) {
                            for (int i = 0; i < 3 && canMove; i++) {
                                for (int j = 0; j < 3 && canMove; j++) {
                                    for (int k = 0; k < 3 && canMove; k++) {
                                        float newX = posX + k + moveDir.x;
                                        float newZ = posZ + i + moveDir.z;
                                        if (collisionCheck(newX, posY + j, newZ)) {
                                            canMove = false;
                                        }
                                    }
                                }
                            }
                        } else if(currentShape == SHAPE_1x3x1) {
                            for(int j = 0; j < 3 && canMove; j++) {
                                float newX = posX + moveDir.x;
                                float newZ = posZ + moveDir.z;
                                if (collisionCheck(newX, posY + j, newZ)) {
                                    canMove = false;
                                }
                            }
                        } else if(currentShape == SHAPE_3x1x3) {
                            for (int i = 0; i < 3 && canMove; i++) {
                                for (int k = 0; k < 3 && canMove; k++) {
                                    float newX = posX + k + moveDir.x;
                                    float newZ = posZ + i + moveDir.z;
                                    if (collisionCheck(newX, posY, newZ)) {
                                        canMove = false;
                                    }
                                }
                            }
                        }
                    }

                    if (canMove) {
                        posX += moveDir.x;
                        posZ += moveDir.z;
                    }
                }
                break;

            case GLFW_KEY_D:
                if(!over) {
                    lastKeyPressed = "D";
                    lastKeyPressTime = glfwGetTime();
                    moveDir = DIRECTIONS[viewIndex];
                    canMove = true;

                    if(currentShape == SPECIAL_EXPLOSIVE) {
                        switch(explosiveShape) {
                            case SHAPE_3x3x3:
                                for (int i = 0; i < 3 && canMove; i++) {
                                    for (int j = 0; j < 3 && canMove; j++) {
                                        for (int k = 0; k < 3 && canMove; k++) {
                                            float newX = posX + k + moveDir.x;
                                            float newZ = posZ + i + moveDir.z;
                                            if (collisionCheck(newX, posY + j, newZ)) {
                                                canMove = false;
                                            }
                                        }
                                    }
                                }
                                break;
                            case SHAPE_1x3x1:
                                for(int j = 0; j < 3 && canMove; j++) {
                                    float newX = posX + moveDir.x;
                                    float newZ = posZ + moveDir.z;
                                    if (collisionCheck(newX, posY + j, newZ)) {
                                        canMove = false;
                                    }
                                }
                                break;
                            case SHAPE_3x1x3:
                                for (int i = 0; i < 3 && canMove; i++) {
                                    for (int k = 0; k < 3 && canMove; k++) {
                                        float newX = posX + k + moveDir.x;
                                        float newZ = posZ + i + moveDir.z;
                                        if (collisionCheck(newX, posY, newZ)) {
                                            canMove = false;
                                        }
                                    }
                                }
                                break;
                        }
                    }
                    else{
                        if(currentShape == SHAPE_3x3x3) {
                            for (int i = 0; i < 3 && canMove; i++) {
                                for (int j = 0; j < 3 && canMove; j++) {
                                    for (int k = 0; k < 3 && canMove; k++) {
                                        float newX = posX + k + moveDir.x;
                                        float newZ = posZ + i + moveDir.z;
                                        if (collisionCheck(newX, posY + j, newZ)) {
                                            canMove = false;
                                        }
                                    }
                                }
                            }
                        } else if(currentShape == SHAPE_1x3x1) {
                            for(int j = 0; j < 3 && canMove; j++) {
                                float newX = posX + moveDir.x;
                                float newZ = posZ + moveDir.z;
                                if (collisionCheck(newX, posY + j, newZ)) {
                                    canMove = false;
                                }
                            }
                        } else if(currentShape == SHAPE_3x1x3) {
                            for (int i = 0; i < 3 && canMove; i++) {
                                for (int k = 0; k < 3 && canMove; k++) {
                                    float newX = posX + k + moveDir.x;
                                    float newZ = posZ + i + moveDir.z;
                                    if (collisionCheck(newX, posY, newZ)) {
                                        canMove = false;
                                    }
                                }
                            }
                        }
                    }

                    if (canMove) {
                        posX += moveDir.x;
                        posZ += moveDir.z;
                    }
                }
                break;

            case GLFW_KEY_H:
                if(!isRotating){
                    lastKeyPressed = "H"; 
                    lastKeyPressTime = glfwGetTime();
                    rotationAmount = 90.0f;
                    targetRotationAngle = fmod(currentRotationAngle + rotationAmount, 360.0f);
                    if (targetRotationAngle < 0) {
                        targetRotationAngle += 360.0f;
                    }
                    isRotating = true;
                }
                break;

            case GLFW_KEY_K:
                if(!isRotating){
                    lastKeyPressed = "K";
                    lastKeyPressTime = glfwGetTime();
                    rotationAmount = -90.0f;
                    targetRotationAngle = fmod(currentRotationAngle + rotationAmount, 360.0f);
                    if (targetRotationAngle < 0) {
                        targetRotationAngle += 360.0f;
                    }
                    isRotating = true;
                }
                break;

            case GLFW_KEY_P:
                if (!over) {
                    isPaused = !isPaused;
                    lastKeyPressed = "P";
                    lastKeyPressTime = glfwGetTime();
                }
                break;

            default:
                break;
        }
    }
}

void mainLoop(GLFWwindow* window)
{
    while (!glfwWindowShouldClose(window))
    {
        display();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}

int main(int argc, char** argv)   // Create Main Function For Bringing It All Together
{
    GLFWwindow* window;
    if (!glfwInit())
    {
        exit(-1);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(gWidth, gHeight, "tetrisGL", NULL, NULL);

    if (!window)
    {
        glfwTerminate();
        exit(-1);
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    // Initialize GLEW to setup the OpenGL Function pointers
    if (GLEW_OK != glewInit())
    {
        std::cout << "Failed to initialize GLEW" << std::endl;
        return EXIT_FAILURE;
    }

    char rendererInfo[512] = {0};
    strcpy(rendererInfo, (const char*) glGetString(GL_RENDERER));
    strcat(rendererInfo, " - ");
    strcat(rendererInfo, (const char*) glGetString(GL_VERSION));
    glfwSetWindowTitle(window, rendererInfo);

    init();

    glfwSetKeyCallback(window, keyboard);
    glfwSetWindowSizeCallback(window, reshape);

    reshape(window, gWidth, gHeight); // need to call this once ourselves
    mainLoop(window); // this does not return unless the window is closed

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}