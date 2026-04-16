#define STB_IMAGE_IMPLEMENTATION
#include "..//include//stb_image.h"

#ifdef _WIN32
#  include <windows.h>
#endif
#include <GL/freeglut.h>
#include <cmath>
#include <deque>
#include <GL/glut.h>
#include <map>
#include <string>
#include <math.h>
#include <stdio.h>

#ifndef GL_CLAMP_TO_EDGE
#define GL_CLAMP_TO_EDGE 0x812F
#endif
/*
* github Handle of students working on this project: ELementaleLord, Faust, uncleCloak
*/
struct coord
{
    GLfloat x, y, z;
    coord(){}
    coord(GLfloat x, GLfloat y, GLfloat z)
    {
        this->x= x; this->y= y; this->z= z;
    }
};

struct color
{
    GLfloat r, g, b;
    color(){}
    color(GLint r, GLint g, GLint b)
    {
        this->r= r/255.0; this-> g= g/ 255.0; this->b= b/255.0;
    }
};

//~ the below are mainly to be used as human reference 
//~ use the integers straight up when filtering for certain actions
const std::map<int, std::string> mouseKeys= 
{
    {0, "leftClick"     },
    {1, "middleClick"   },
    {2, "rightClick"    },
    {3, "scrollUp"      },
    {4, "scrollDown"    },
};// int clickType in handle mouse
const std::map<int, std::string> modKeys= 
{
    {0, "none"              },
    {1, "shift"             },
    {2, "ctrl"              },
    {3, "shift+ ctrl"       },
    {4, "alt"               },
    {5, "alt+ shift"        },
    {6, "alt+ ctrl"         },
    {7, "alt+ ctrl+ shift"  },
}; // glutGetModifiers();

const std::map<std::string, color> colors= 
{
    {"black",   {  0,   0,   0}},
    {"white",   {255, 255, 255}},
    {"red",     {255,   0,   0}},
    {"green",   {  0, 255,   0}},
    {"blue",    {  0,   0, 255}},
    {"yellow",  {255, 255,   0}},
    {"cyan",    {  0, 255, 255}},
    {"magenta", {255,   0, 255}},
    {"orange",  {255, 128,   0}},
    {"lime",    {128, 255,   0}},
    {"purple",  {128,   0, 255}},
    {"sun",     {241,  93,  34}},
    {"mercury", {191, 189, 188}},
    {"venus",   {218, 178, 146}},
    {"earth",   { 31,  56, 111}},
    {"mars",    {242, 124,  95}},
    {"jupiter", {191, 176, 156}},
    {"saturn",  {218, 183, 120}},
    {"uranus",  {207, 236, 240}},
    {"neptune", {120, 158, 191}},
};

static const char *faceFiles[6] = {
    "..\\assets\\galaxy.png", // right
    "..\\assets\\galaxy.png", // left
    "..\\assets\\galaxy.png", // top
    "..\\assets\\galaxy.png", // bottom
    "..\\assets\\galaxy.png", // front
    "..\\assets\\galaxy.png", // back
};

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define SCREEN_X_OFFSET 300
#define SCREEN_Y_OFFSET 100
#define VIEWPORT_X 0
#define VIEWPORT_Y 0
#define VERTICAL_ROTATION_POS_LIMIT 89.0f
#define VERTICAL_ROTATION_NEG_LIMIT -89.0f
#define FOV 60
#define MOVE_SPEED 0.5f
#define ZOOM_SPEED 0.3f
#define Z_NEAR 0.1f
#define Z_FAR 500.0f
#define MOUSE_SENS 0.15f
#define PI 3.14159265358979323846f
#define DEGREE_TO_RADIAN_CONVERTION_FACTOR PI / 180.0f
#define starTotal 1000
#define starDistMin 2
#define starDistRange 100
#define starPointScale 1.5
#define ORBIT_LINE_ALPHA 0.45f
#define ORBIT_LINE_WIDTH 2.0f
#define TRAIL_MAX_POINTS 100
#define MOON_TRAIL_MAX_POINTS (TRAIL_MAX_POINTS / 10)

coord camPos = {-7.0, 2.7, -7.0};
coord camLook= {7.0, -2.7, 7.0};
coord camUp  = {0.0, 1.0, 0.0};

float angle= 0.0f;
float horizentalRotation = 0.0f;
float verticalRotation = 0.0f;
bool firstMouse = true;
bool inMouseMode = false;
int lastMouseX = SCREEN_WIDTH / 2;
int lastMouseY = SCREEN_HEIGHT / 2;
int windowCenterX = SCREEN_WIDTH / 2;
int windowCenterY = SCREEN_HEIGHT / 2;
static GLuint skyTextures[6];

coord crossProduct(const coord &a, const coord &b)
{
    return coord(
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    );
}

//~ returns the textures from given files
static GLuint loadTexture(const char *filename)
{
    int w, h, channels;
    unsigned char *data = stbi_load(filename, &w, &h, &channels, 4);
    if (!data) {
        fprintf(stderr, "ERROR: could not load '%s': %s\n", filename, stbi_failure_reason());
        return 0;
    }

    GLuint id;
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);

    //# No mipmaps so we must clamp to edge so seams dont show
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, data);

    stbi_image_free(data);
    return id;
}

//~ calculation to move the cameraLook vector based on 2D rotation
void updateCamLook()
{
    //# convert horizentalRotation and verticalRotation from degree to radians
    float horizentalRotationRad = horizentalRotation * DEGREE_TO_RADIAN_CONVERTION_FACTOR;
    float verticalRotationRad = verticalRotation * DEGREE_TO_RADIAN_CONVERTION_FACTOR;

    //# do some angle related math (i got this from the internet)
    camLook.x = std::cos(verticalRotationRad) * std::cos(horizentalRotationRad);
    camLook.y = std::sin(verticalRotationRad);
    camLook.z = std::cos(verticalRotationRad) * std::sin(horizentalRotationRad);
}

void handlePassiveMotion(int x, int y)
{
    if (inMouseMode)
    {
        if (firstMouse)
        {
            //# move the cursor to the center of the widow and modify appropriate values
            lastMouseX = windowCenterX;
            lastMouseY = windowCenterY;
            firstMouse = false;
            glutWarpPointer(windowCenterX, windowCenterY);
            return;
        }

        //# calculate offset from last position
        horizentalRotation += (x - lastMouseX) * MOUSE_SENS;
        verticalRotation += (lastMouseY - y) * MOUSE_SENS;

        if (verticalRotation > VERTICAL_ROTATION_POS_LIMIT) verticalRotation = VERTICAL_ROTATION_POS_LIMIT;
        if (verticalRotation < VERTICAL_ROTATION_NEG_LIMIT) verticalRotation = VERTICAL_ROTATION_NEG_LIMIT;

        updateCamLook();
        //# centralize cursor to prevent continuous movement crossing the window boundary
        glutWarpPointer(windowCenterX, windowCenterY);//# moves cursor to the given x, y position

        //# save the last (for next call) mouse position
        lastMouseX = windowCenterX;
        lastMouseY = windowCenterY;

        glutPostRedisplay();//# call to reDisplay stuff after moving camera
        // printf("camPos= {%d, %d, %d} camLook= {%d, %d, %d} camUp= {%d, %d, %d}\n", camPos.x, camPos.y, camPos.z, camLook.x, camLook.y, camLook.z, camUp.x, camUp.y, camUp.z);
    }
}

void handleSpecialKeys(int key, int x, int y)
{
    coord vertMove = crossProduct(camLook, camUp);

    switch(key)
    {
        case GLUT_KEY_UP:
            if (inMouseMode)
            {
                //# move camera forward
                camPos.x += camLook.x * MOVE_SPEED;
                camPos.y += camLook.y * MOVE_SPEED;
                camPos.z += camLook.z * MOVE_SPEED;
            }
            else 
            {
                //# move whole camera up along y axis
                camPos.y += MOVE_SPEED;
            }
            break;
        case GLUT_KEY_DOWN:
            if (inMouseMode){
                //# move camera backward
                camPos.x -= camLook.x * MOVE_SPEED;
                camPos.y -= camLook.y * MOVE_SPEED;
                camPos.z -= camLook.z * MOVE_SPEED;
            }
            else
            {
                //# move whole camera down along y axis
                camPos.y -= MOVE_SPEED;
            }
            break;
        case GLUT_KEY_RIGHT:
            if (inMouseMode)
            {
                //# move camera rightward
                camPos.x += vertMove.x * MOVE_SPEED;
                camPos.y += vertMove.y * MOVE_SPEED;
                camPos.z += vertMove.z * MOVE_SPEED;
            }
            else 
            {
                //# move whole camera right along x axis
                camPos.x += MOVE_SPEED;
            }
            break;
        case GLUT_KEY_LEFT:
            if (inMouseMode)
            {
                //# move camera leftward
                camPos.x -= vertMove.x * MOVE_SPEED;
                camPos.y -= vertMove.y * MOVE_SPEED;
                camPos.z -= vertMove.z * MOVE_SPEED;
            } 
            else 
            {
                //# move whole camera left along x axis
                camPos.x -= MOVE_SPEED;
            }
            break;
    }
    glutPostRedisplay();//# call to reDisplay stuff after moving camera
}

void handleKeys(unsigned char key, int x, int y)
{
    coord vertMove = crossProduct(camLook, camUp);

    switch(key){
        case 'r':
        case 'R':
            inMouseMode = !inMouseMode;
            if (inMouseMode){
                glutSetCursor(GLUT_CURSOR_NONE);//# hide the cursor
                firstMouse = true;
                //# reset so when we swap to mouseMode we start with the mouse in the center of the screen
            } else {
                glutSetCursor(GLUT_CURSOR_LEFT_ARROW);//# show the "default" cursor
            }
            break;
        case 'w':
        case 'W':
            if (inMouseMode)
            {
                //# move camera forward
                camPos.x += camLook.x * MOVE_SPEED;
                camPos.y += camLook.y * MOVE_SPEED;
                camPos.z += camLook.z * MOVE_SPEED;
            }
            else 
            {
                //# move whole camera up along y axis
                camPos.y += MOVE_SPEED;
            }
            break;
        case 's':
        case 'S':
            if (inMouseMode)
            {
                //# move camera backward
                camPos.x -= camLook.x * MOVE_SPEED;
                camPos.y -= camLook.y * MOVE_SPEED;
                camPos.z -= camLook.z * MOVE_SPEED;
            }
            else
            {
                //# move whole camera down along y axis
                camPos.y -= MOVE_SPEED;
            }
            break;
        case 'd':
        case 'D':
            if (inMouseMode)
            {
                //# move camera rightward
                camPos.x += vertMove.x * MOVE_SPEED;
                camPos.y += vertMove.y * MOVE_SPEED;
                camPos.z += vertMove.z * MOVE_SPEED;
            } 
            else 
            {
                //# move whole camera right along x
                camPos.x += MOVE_SPEED;
            }
            break;
        case 'a':
        case 'A':
            if (inMouseMode)
            {
                //# move camera leftward
                camPos.x -= vertMove.x * MOVE_SPEED;
                camPos.y -= vertMove.y * MOVE_SPEED;
                camPos.z -= vertMove.z * MOVE_SPEED;
            }
            else 
            {
                //# move whole camera left along x
                camPos.x -= MOVE_SPEED;
            }
            break;
    }
    glutPostRedisplay();//# call to reDisplay stuff after moving camera
}

void handleMouse(int button, int state, int x, int y)
{

    if (button == 3)
    {
        //# move camera forward on scroolUp
        camPos.x += camLook.x * ZOOM_SPEED;
        camPos.y += camLook.y * ZOOM_SPEED;
        camPos.z += camLook.z * ZOOM_SPEED;
    }
    else if (button == 4) 
    {
        //# move camera backward on scroolDown
        camPos.x -= camLook.x * ZOOM_SPEED;
        camPos.y -= camLook.y * ZOOM_SPEED;
        camPos.z -= camLook.z * ZOOM_SPEED;
    }

    glutPostRedisplay();//# call to reDisplay stuff after moving camera
}

void handleReshape(int width, int height)
{
    if (height == 0) height = 1;
    glViewport(VIEWPORT_X, VIEWPORT_Y, width, height);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(FOV, (GLdouble)width / (GLdouble)height, Z_NEAR, Z_FAR);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

class Planet
{
    public:
        float radius;
        int sections;
        color bottomColor;
        color topColor;
        
        float axisAngle;
        float axisSpeed;

        float orbitAngle;
        float orbitSpeed;
        float orbitDistance; // How far from the Sun
        std::deque<coord> orbitTrail;

        Planet(float radius, int sections, const color& bottomColor, const color& topColor, float axisSpd, float orbitSpd, float distance)
            : radius(radius),
              sections(sections),
              bottomColor(bottomColor),
              topColor(topColor),
              axisAngle(0.0f),
              axisSpeed(axisSpd),
              orbitAngle(0.0f),
              orbitSpeed(orbitSpd),
              orbitDistance(distance)
        {
        }

        void update()
        {
            axisAngle += axisSpeed;
            if(axisAngle > 360.0f) axisAngle -= 360.0f;

            orbitAngle += orbitSpeed;
            if(orbitAngle > 360.0f) orbitAngle -= 360.0f;
            
            if (orbitDistance > 0.0f)
            {
                //! push_back add the current position of the planet so the trail starts rendering
                orbitTrail.push_back(getOrbitPosition());
                if (orbitTrail.size() > TRAIL_MAX_POINTS)
                {
                    //! pop_front removes the the oldest points to give the dafing out effect
                    orbitTrail.pop_front();
                }
            }
        }

        coord getOrbitPosition() const
        {
            float orbitAngleRadians = orbitAngle * DEGREE_TO_RADIAN_CONVERTION_FACTOR;
            return coord(
                orbitDistance * std::cos(orbitAngleRadians),    // x coord
                0.0f,                                           // y coord
                -orbitDistance * std::sin(orbitAngleRadians)    // z coord
            );
        }

        void draw()
        {
            glPushMatrix(); 

            glRotatef(orbitAngle, 0.0f, 1.0f, 0.0f);
            glTranslatef(orbitDistance, 0.0f, 0.0f);
            glRotatef(axisAngle, 0.0f, 1.0f, 0.0f);

            for(int i = 0; i < sections; ++i)
            {
                float phi1 = PI * (-0.5f + (float) i / sections);
                float phi2 = PI * (-0.5f + (float) (i + 1) / sections);

                glBegin(GL_QUAD_STRIP);
                for(int j = 0; j <= sections; j++)
                {
                    float theta = 2 * PI * (float) j / sections;

                    float x1 = radius * std::cos(phi1) * std::cos(theta);
                    float y1 = radius * sin(phi1);
                    float z1 = radius * std::cos(phi1) * sin(theta);

                    float x2 = radius * std::cos(phi2) * std::cos(theta);
                    float y2 = radius * sin(phi2);
                    float z2 = radius * std::cos(phi2) * sin(theta);

                    float t1 = (y1 + radius) / (2.0f * radius);
                    float t2 = (y2 + radius) / (2.0f * radius);

                    glColor3f(
                        bottomColor.r + t1 * (topColor.r - bottomColor.r),
                        bottomColor.g + t1 * (topColor.g - bottomColor.g),
                        bottomColor.b + t1 * (topColor.b - bottomColor.b)
                    );
                    glVertex3f(x1, y1, z1);

                    glColor3f(
                        bottomColor.r + t2 * (topColor.r - bottomColor.r),
                        bottomColor.g + t2 * (topColor.g - bottomColor.g),
                        bottomColor.b + t2 * (topColor.b - bottomColor.b)
                    );
                    glVertex3f(x2, y2, z2);
                }
                glEnd();
            }
            
            glPopMatrix(); 
        }

        void drawOrbitTrail() const
        {
            if (orbitTrail.size() < 2) return;

            glBegin(GL_LINE_STRIP);
            for (size_t i = 0; i < orbitTrail.size(); ++i)
            {
                float fade = static_cast<float>(i + 1) / static_cast<float>(orbitTrail.size());
                fade *= fade;
                glColor4f(topColor.r, topColor.g, topColor.b, ORBIT_LINE_ALPHA * fade);
                glVertex3f(orbitTrail[i].x, orbitTrail[i].y, orbitTrail[i].z);
            }
            glEnd();
        }
};

class Moon : public Planet
{
    public:
        Planet& parentPlanet;

        Moon(float radius, int sections, const color& bottomColor, const color& topColor, float axisSpd, float orbitSpd, float distance, Planet& parent)
            : Planet(radius, sections, bottomColor, topColor, axisSpd, orbitSpd, distance),
              parentPlanet(parent)
        {
            orbitTrail.clear();
            if (orbitDistance > 0.0f)
            {
                orbitTrail.push_back(getOrbitPosition());
            }
        }

        void update()
        {
            axisAngle += axisSpeed;
            if(axisAngle > 360.0f) axisAngle -= 360.0f;

            orbitAngle += orbitSpeed;
            if(orbitAngle > 360.0f) orbitAngle -= 360.0f;

            if (orbitDistance > 0.0f)
            {
                orbitTrail.push_back(getOrbitPosition());
                if (orbitTrail.size() > MOON_TRAIL_MAX_POINTS)
                {
                    orbitTrail.pop_front();
                }
            }
        }

        coord getOrbitPosition() const
        {
            float orbitAngleRadians = orbitAngle * DEGREE_TO_RADIAN_CONVERTION_FACTOR;
            coord parentPosition = parentPlanet.getOrbitPosition();

            return coord(
                parentPosition.x + orbitDistance * std::cos(orbitAngleRadians),
                parentPosition.y,
                parentPosition.z - orbitDistance * std::sin(orbitAngleRadians)
            );
        }

        void draw()
        {
            coord parentPosition = parentPlanet.getOrbitPosition();

            glPushMatrix();

            glTranslatef(parentPosition.x, parentPosition.y, parentPosition.z);
            glRotatef(orbitAngle, 0.0f, 1.0f, 0.0f);
            glTranslatef(orbitDistance, 0.0f, 0.0f);
            glRotatef(axisAngle, 0.0f, 1.0f, 0.0f);

            for(int i = 0; i < sections; ++i)
            {
                float phi1 = PI * (-0.5f + (float) i / sections);
                float phi2 = PI * (-0.5f + (float) (i + 1) / sections);

                glBegin(GL_QUAD_STRIP);
                for(int j = 0; j <= sections; j++)
                {
                    float theta = 2 * PI * (float) j / sections;

                    float x1 = radius * std::cos(phi1) * std::cos(theta);
                    float y1 = radius * sin(phi1);
                    float z1 = radius * std::cos(phi1) * sin(theta);

                    float x2 = radius * std::cos(phi2) * std::cos(theta);
                    float y2 = radius * sin(phi2);
                    float z2 = radius * std::cos(phi2) * sin(theta);

                    float t1 = (y1 + radius) / (2.0f * radius);
                    float t2 = (y2 + radius) / (2.0f * radius);

                    glColor3f(
                        bottomColor.r + t1 * (topColor.r - bottomColor.r),
                        bottomColor.g + t1 * (topColor.g - bottomColor.g),
                        bottomColor.b + t1 * (topColor.b - bottomColor.b)
                    );
                    glVertex3f(x1, y1, z1);

                    glColor3f(
                        bottomColor.r + t2 * (topColor.r - bottomColor.r),
                        bottomColor.g + t2 * (topColor.g - bottomColor.g),
                        bottomColor.b + t2 * (topColor.b - bottomColor.b)
                    );
                    glVertex3f(x2, y2, z2);
                }
                glEnd();
            }

            glPopMatrix();
        }
};

class Ring : public Planet
{
    public:
        Planet& parentPlanet;

        Ring(float radius, int sections, const color& bottomColor, const color& topColor, float axisSpd, float orbitSpd, float distance, Planet& parent)
            : Planet(radius, sections, bottomColor, topColor, axisSpd, orbitSpd, distance),
              parentPlanet(parent)
        {}

        void update()
        {
            axisAngle += axisSpeed;
            if(axisAngle > 360.0f) axisAngle -= 360.0f;

            orbitAngle += orbitSpeed;
            if(orbitAngle > 360.0f) orbitAngle -= 360.0f;
        }

        coord getOrbitPosition() const
        {
            return parentPlanet.getOrbitPosition();
        }

        void draw()
        {
            coord parentPosition = parentPlanet.getOrbitPosition();
            float innerRadius = parentPlanet.radius + orbitDistance;
            float outerRadius = innerRadius + radius;

            glPushMatrix();

            glTranslatef(parentPosition.x, parentPosition.y, parentPosition.z);
            glRotatef(parentPlanet.axisAngle, 0.0f, 1.0f, 0.0f);
            glRotatef(orbitAngle, 0.0f, 1.0f, 0.0f);
            glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
            glRotatef(axisAngle, 0.0f, 0.0f, 1.0f);

            glBegin(GL_QUAD_STRIP);
            for(int i = 0; i <= sections; ++i)
            {
                float theta = 2.0f * PI * (float)i / sections;
                float cosTheta = std::cos(theta);
                float sinTheta = std::sin(theta);
                float t = (float)i / sections;

                glColor3f(
                    bottomColor.r + t * (topColor.r - bottomColor.r),
                    bottomColor.g + t * (topColor.g - bottomColor.g),
                    bottomColor.b + t * (topColor.b - bottomColor.b)
                );
                glVertex3f(innerRadius * cosTheta, innerRadius * sinTheta, 0.0f);

                glColor3f(
                    topColor.r,
                    topColor.g,
                    topColor.b
                );
                glVertex3f(outerRadius * cosTheta, outerRadius * sinTheta, 0.0f);
            }
            glEnd();

            glPopMatrix();
        }
};

Planet sun(0.35f, 24, color(201, 78, 20), color(255, 195, 80), 0.4f, 0.0f, 0.0f);
Planet mercury(0.04f, 12, color(120, 118, 117), color(191, 189, 188), 1.2f, 4.2f, 0.55f);
Planet venus(0.07f, 14, color(160, 120, 88), color(218, 178, 146), 0.8f, 3.2f, 0.8f);
Planet earth(0.08f, 15, color(20, 90, 45), color(31, 56, 111), 2.0f, 2.6f, 1.1f);
Planet mars(0.06f, 13, color(166, 72, 52), color(242, 124, 95), 1.8f, 2.1f, 1.45f);
Planet jupiter(0.18f, 18, color(140, 120, 98), color(191, 176, 156), 2.8f, 1.1f, 2.1f);
Planet saturn(0.15f, 18, color(168, 138, 78), color(218, 183, 120), 2.4f, 0.85f, 2.7f);
Planet uranus(0.11f, 16, color(140, 190, 198), color(207, 236, 240), 1.9f, 0.55f, 3.2f);
Planet neptune(0.11f, 16, color(60, 95, 150), color(120, 158, 191), 1.7f, 0.4f, 3.7f);

Moon earthMoon(0.025f, 12, color(140, 140, 140), color(210, 210, 210), 1.1f, 8.0f, 0.18f, earth);
Moon marsMoon1(0.014f, 10, color(110, 102, 94), color(170, 160, 145), 1.4f, 10.0f, 0.12f, mars);
Moon marsMoon2(0.01f, 10, color(125, 116, 108), color(186, 176, 166), 0.9f, 7.0f, 0.18f, mars);

Ring jupiterRing(0.035f, 64, color(105, 96, 88), color(156, 146, 132), 0.3f, 0.8f, 0.02f, jupiter);
Ring saturnRing(0.16f, 96, color(150, 132, 104), color(223, 210, 181), 0.45f, 0.9f, 0.03f, saturn);
Ring uranusRing(0.05f, 72, color(82, 98, 105), color(152, 170, 176), 1.1f, 0.45f, 0.025f, uranus);
Ring neptuneRing(0.03f, 72, color(78, 96, 118), color(120, 138, 160), 0.9f, 0.35f, 0.04f, neptune);

struct star{
    coord p;
    color c;
    star(){ this->c= colors.at("white"); }
};
star starPosArr[starTotal];

void generateStars()
{
    for (int i= 0; i< starTotal; i++ ){

        float theta=    ((float)rand() / RAND_MAX) * 2.0f * PI;
        float phi=      acosf(1.0f - 2.0f * ((float)rand() / RAND_MAX));// vertical angle
        float radius=   starDistMin + starDistRange * ((float)rand() / RAND_MAX);

        starPosArr[i].p.x= radius * sinf(phi) * cosf(theta);
        starPosArr[i].p.y= radius * cosf(phi);
        starPosArr[i].p.z= radius * sinf(phi) * sinf(theta);
        // printf("x= %f, y=%f, z=%f\n", starPosArr[i].p.x, starPosArr[i].p.y, starPosArr[i].p.z);
    }
}

void drawStars()
{
    glPointSize(starPointScale);
    glBegin(GL_POINTS);
    for (int i= 0; i< starTotal; i++){
        glColor3f(starPosArr[i].c.r, starPosArr[i].c.g, starPosArr[i].c.b);
        glVertex3f(starPosArr[i].p.x, starPosArr[i].p.y, starPosArr[i].p.z);
    }
    glEnd();
}

static void drawSkybox(void)
{
    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    glDisable(GL_LIGHTING);
    glEnable(GL_TEXTURE_2D);

    //# Right
    glBindTexture(GL_TEXTURE_2D, skyTextures[0]);
    glBegin(GL_QUADS);
        glTexCoord2f(0,1); glVertex3f( 1,-1, 1);
        glTexCoord2f(1,1); glVertex3f( 1,-1,-1);
        glTexCoord2f(1,0); glVertex3f( 1, 1,-1);
        glTexCoord2f(0,0); glVertex3f( 1, 1, 1);
    glEnd();

    //# Left
    glBindTexture(GL_TEXTURE_2D, skyTextures[1]);
    glBegin(GL_QUADS);
        glTexCoord2f(0,1); glVertex3f(-1,-1,-1);
        glTexCoord2f(1,1); glVertex3f(-1,-1, 1);
        glTexCoord2f(1,0); glVertex3f(-1, 1, 1);
        glTexCoord2f(0,0); glVertex3f(-1, 1,-1);
    glEnd();

    //# Top
    glBindTexture(GL_TEXTURE_2D, skyTextures[2]);
    glBegin(GL_QUADS);
        glTexCoord2f(0,1); glVertex3f(-1, 1, 1);
        glTexCoord2f(1,1); glVertex3f( 1, 1, 1);
        glTexCoord2f(1,0); glVertex3f( 1, 1,-1);
        glTexCoord2f(0,0); glVertex3f(-1, 1,-1);
    glEnd();

    //# Bottom
    glBindTexture(GL_TEXTURE_2D, skyTextures[3]);
    glBegin(GL_QUADS);
        glTexCoord2f(0,1); glVertex3f(-1,-1,-1);
        glTexCoord2f(1,1); glVertex3f( 1,-1,-1);
        glTexCoord2f(1,0); glVertex3f( 1,-1, 1);
        glTexCoord2f(0,0); glVertex3f(-1,-1, 1);
    glEnd();

    //# Front
    glBindTexture(GL_TEXTURE_2D, skyTextures[4]);
    glBegin(GL_QUADS);
        glTexCoord2f(0,1); glVertex3f(-1,-1, 1);
        glTexCoord2f(1,1); glVertex3f( 1,-1, 1);
        glTexCoord2f(1,0); glVertex3f( 1, 1, 1);
        glTexCoord2f(0,0); glVertex3f(-1, 1, 1);
    glEnd();

    //# Back
    glBindTexture(GL_TEXTURE_2D, skyTextures[5]);
    glBegin(GL_QUADS);
        glTexCoord2f(0,1); glVertex3f( 1,-1,-1);
        glTexCoord2f(1,1); glVertex3f(-1,-1,-1);
        glTexCoord2f(1,0); glVertex3f(-1, 1,-1);
        glTexCoord2f(0,0); glVertex3f( 1, 1,-1);
    glEnd();

    //# Reset to "default"
    glDepthMask(GL_TRUE);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_TEXTURE_2D);
}

void display() 
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glRotatef(verticalRotation, 1, 0, 0);
    glRotatef(horizentalRotation,   0, 1, 0);
    drawSkybox();

    glLoadIdentity();
    gluLookAt(camPos.x, camPos.y, camPos.z,
            camPos.x + camLook.x, camPos.y + camLook.y, camPos.z + camLook.z,
            camUp.x, camUp.y, camUp.z);

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); 
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glLineWidth(ORBIT_LINE_WIDTH);
    glDepthMask(GL_FALSE);

    mercury.drawOrbitTrail();
    venus.drawOrbitTrail();
    earth.drawOrbitTrail();
    mars.drawOrbitTrail();
    jupiter.drawOrbitTrail();
    saturn.drawOrbitTrail();
    uranus.drawOrbitTrail();
    neptune.drawOrbitTrail();
    
    earthMoon.drawOrbitTrail();
    marsMoon1.drawOrbitTrail();
    marsMoon2.drawOrbitTrail();

    glDepthMask(GL_TRUE);
    glDisable(GL_LINE_SMOOTH);
    glDisable(GL_BLEND);

    sun.draw();
    mercury.draw();
    venus.draw();
    earth.draw();
    mars.draw();
    jupiter.draw();
    saturn.draw();
    uranus.draw();
    neptune.draw();
    
    earthMoon.draw();
    marsMoon1.draw();
    marsMoon2.draw();

    jupiterRing.draw();
    saturnRing.draw();
    uranusRing.draw();
    neptuneRing.draw();

    drawStars();

    glutSwapBuffers();
}

void timer(int val)
{
    sun.update();
    mercury.update();
    venus.update();
    earth.update();
    mars.update();
    jupiter.update();
    saturn.update();
    uranus.update();
    neptune.update();
    
    earthMoon.update();
    marsMoon1.update();
    marsMoon2.update();
    
    jupiterRing.update();
    saturnRing.update();
    uranusRing.update();
    neptuneRing.update();

    glutPostRedisplay();
    glutTimerFunc(16, timer, 0);
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(SCREEN_WIDTH, SCREEN_HEIGHT);
    glutInitWindowPosition(SCREEN_X_OFFSET, SCREEN_Y_OFFSET);
    glutCreateWindow("OpenGL Solar System");

    //init
    glClearColor(0,0,0,0);
    glEnable(GL_DEPTH_TEST);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(FOV, (GLdouble)SCREEN_WIDTH / (GLdouble)SCREEN_HEIGHT, Z_NEAR, Z_FAR);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    generateStars();
    int allLoaded = 1;
    for (int i = 0; i < 6; i++) {
        skyTextures[i] = loadTexture(faceFiles[i]);
        if (skyTextures[i] == 0) allLoaded = 0;
    }
    if (!allLoaded) {
        fprintf(stderr, "One or more textures failed to load. Check filenames.\n");
        /* We continue anyway so you can see the window; missing faces = black */
    }
    //init

    glEnable(GL_DEPTH_TEST);
    glutDisplayFunc(display);
    glutTimerFunc(0,timer,0);

    glutKeyboardFunc(handleKeys);
    glutSpecialFunc(handleSpecialKeys);
    glutMouseFunc(handleMouse);
    glutReshapeFunc(handleReshape);
    glutPassiveMotionFunc(handlePassiveMotion);
    
    glutMainLoop();
    return 0;
}
