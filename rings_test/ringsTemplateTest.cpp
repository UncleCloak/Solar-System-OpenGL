#include <GL/freeglut.h>
#include <cmath>
#include <GL/glut.h>
#include <map>
#include <string>

struct coord
{
    GLfloat x, y, z;
    coord(GLfloat x, GLfloat y, GLfloat z)
    {
        this->x= x; this->y= y; this->z= z;
    }
};

struct color
{
    GLfloat r, g, b;
    color(GLint r, GLint g, GLint b)
    {
        this->r= r/255.0; this-> g= g/ 255.0; this->b= b/255.0;
    }
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
coord camPos = {-7.0, 2.7, -7.0};     // 0.0, 0.0, 3.0
coord camLook= {7.0, -2.7, 7.0};          // 0.0, 0.0, -1.0
coord camUp  = {0.0, 1.0, 0.0};         // 0.0, 1.0, 0.0

float angle= 0.0f;
float horizentalRotation = 0.0f;
float verticalRotation = 0.0f;    // 0.0f
bool firstMouse = true;
bool inMouseMode = false;
int lastMouseX = SCREEN_WIDTH / 2;
int lastMouseY = SCREEN_HEIGHT / 2;
int windowCenterX = SCREEN_WIDTH / 2;
int windowCenterY = SCREEN_HEIGHT / 2;

void init()
{
    glClearColor(0,0,0,0);
    glEnable(GL_DEPTH_TEST);
    // glEnable(GL_CULL_FACE);
    // glCullFace(GL_BACK);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(FOV, (GLdouble)SCREEN_WIDTH / (GLdouble)SCREEN_HEIGHT, Z_NEAR, Z_FAR);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

coord crossProduct(const coord &a, const coord &b)
{
    return coord(
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    );
}

void updateCamLook()
{
    float horizentalRotationRad = horizentalRotation * DEGREE_TO_RADIAN_CONVERTION_FACTOR;
    float verticalRotationRad = verticalRotation * DEGREE_TO_RADIAN_CONVERTION_FACTOR;

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
            lastMouseX = windowCenterX;
            lastMouseY = windowCenterY;
            firstMouse = false;
            glutWarpPointer(windowCenterX, windowCenterY);
            return;
        }

        horizentalRotation += (x - lastMouseX) * MOUSE_SENS;
        verticalRotation += (lastMouseY - y) * MOUSE_SENS;

        if (verticalRotation > VERTICAL_ROTATION_POS_LIMIT) verticalRotation = VERTICAL_ROTATION_POS_LIMIT;
        if (verticalRotation < VERTICAL_ROTATION_NEG_LIMIT) verticalRotation = VERTICAL_ROTATION_NEG_LIMIT;

        updateCamLook();
        glutWarpPointer(windowCenterX, windowCenterY);

        lastMouseX = windowCenterX;
        lastMouseY = windowCenterY;

        glutPostRedisplay();
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
                camPos.x += camLook.x * MOVE_SPEED;
                camPos.y += camLook.y * MOVE_SPEED;
                camPos.z += camLook.z * MOVE_SPEED;
            }
            else
            {
                camPos.y += MOVE_SPEED;
            }
            break;
        case GLUT_KEY_DOWN:
            if (inMouseMode)
            {
                camPos.x -= camLook.x * MOVE_SPEED;
                camPos.y -= camLook.y * MOVE_SPEED;
                camPos.z -= camLook.z * MOVE_SPEED;
            }
            else
            {
                camPos.y -= MOVE_SPEED;
            }
            break;
        case GLUT_KEY_RIGHT:
            if (inMouseMode)
            {
                camPos.x += vertMove.x * MOVE_SPEED;
                camPos.y += vertMove.y * MOVE_SPEED;
                camPos.z += vertMove.z * MOVE_SPEED;
            }
            else
            {
                camPos.x += MOVE_SPEED;
            }
            break;
        case GLUT_KEY_LEFT:
            if (inMouseMode)
            {
                camPos.x -= vertMove.x * MOVE_SPEED;
                camPos.y -= vertMove.y * MOVE_SPEED;
                camPos.z -= vertMove.z * MOVE_SPEED;
            }
            else
            {
                camPos.x -= MOVE_SPEED;
            }
            break;
    }

    glutPostRedisplay();
}

void handleKeys(unsigned char key, int x, int y)
{
    coord vertMove = crossProduct(camLook, camUp);

    switch(key)
    {
        case 'r':
        case 'R':
            inMouseMode = !inMouseMode;
            if (inMouseMode)
            {
                glutSetCursor(GLUT_CURSOR_NONE);
                firstMouse = true;
            }
            else
            {
                glutSetCursor(GLUT_CURSOR_LEFT_ARROW);
            }
            break;
        case 'w':
        case 'W':
            if (inMouseMode)
            {
                camPos.x += camLook.x * MOVE_SPEED;
                camPos.y += camLook.y * MOVE_SPEED;
                camPos.z += camLook.z * MOVE_SPEED;
            }
            else
            {
                camPos.y += MOVE_SPEED;
            }
            break;
        case 's':
        case 'S':
            if (inMouseMode)
            {
                camPos.x -= camLook.x * MOVE_SPEED;
                camPos.y -= camLook.y * MOVE_SPEED;
                camPos.z -= camLook.z * MOVE_SPEED;
            }
            else
            {
                camPos.y -= MOVE_SPEED;
            }
            break;
        case 'd':
        case 'D':
            if (inMouseMode)
            {
                camPos.x += vertMove.x * MOVE_SPEED;
                camPos.y += vertMove.y * MOVE_SPEED;
                camPos.z += vertMove.z * MOVE_SPEED;
            }
            else
            {
                camPos.x += MOVE_SPEED;
            }
            break;
        case 'a':
        case 'A':
            if (inMouseMode)
            {
                camPos.x -= vertMove.x * MOVE_SPEED;
                camPos.y -= vertMove.y * MOVE_SPEED;
                camPos.z -= vertMove.z * MOVE_SPEED;
            }
            else
            {
                camPos.x -= MOVE_SPEED;
            }
            break;
    }

    glutPostRedisplay();
}

void handleMouse(int button, int state, int x, int y)
{
    if (button == 3)
    {
        camPos.x += camLook.x * ZOOM_SPEED;
        camPos.y += camLook.y * ZOOM_SPEED;
        camPos.z += camLook.z * ZOOM_SPEED;
    }
    else if (button == 4)
    {
        camPos.x -= camLook.x * ZOOM_SPEED;
        camPos.y -= camLook.y * ZOOM_SPEED;
        camPos.z -= camLook.z * ZOOM_SPEED;
    }

    glutPostRedisplay();
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
Planet earth(0.08f, 15, color(20, 90, 45), color(31, 56, 111), 2.0f, 2.6f, 1.1f);
Ring earthRing(0.05f, 48, color(140, 130, 110), color(210, 200, 175), 0.4f, 1.0f, 0.03f, earth);

void display() 
{

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    gluLookAt(camPos.x, camPos.y, camPos.z,
            camPos.x + camLook.x, camPos.y + camLook.y, camPos.z + camLook.z,
            camUp.x, camUp.y, camUp.z);

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); 

    sun.draw();
    earth.draw();
    earthRing.draw();

    glutSwapBuffers();
}

void timer(int val)
{
    sun.update();
    earth.update();
    earthRing.update();

    glutPostRedisplay();
    glutTimerFunc(16, timer, 0);
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(SCREEN_WIDTH, SCREEN_HEIGHT);
    glutInitWindowPosition(SCREEN_X_OFFSET, SCREEN_Y_OFFSET);
    glutCreateWindow("Rings Testing");

    init();

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
