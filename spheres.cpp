#include <vector>
#include <iostream>
#include <GL/freeglut.h>
#include <cmath>

const float PI = 3.141592653589f;
const int SCREEN_SIZE = 800;

class Sphere
{
    public:
        float radius;
        int sections;
        float rotationAngle;
        float rotationSpeed;

        Sphere(float radius, int sections,float speed)
        {
            this->radius = radius;
            this->sections = sections;
            this->rotationSpeed = speed;
            rotationAngle = 0.0f;
        }

        void update()
        {
            rotationAngle += rotationSpeed;
            if(rotationAngle > 360.0f)
            {
                rotationAngle -= 360.0f;
            }
        }

        void drawSphere()
        {
            glPushMatrix();

            glRotatef(rotationAngle,0.0f, 1.0f, 0.0f);
            for(int i = 0; i < sections; ++i)
            {
                float phi1 = PI * (-0.5f + (float) i / sections);
                float phi2 = PI * (-0.5f + (float) (i + 1) / sections);

                glBegin(GL_QUAD_STRIP);
                for(int j = 0; j <= sections; j++)
                {
                    float theta = 2 * PI * (float) j / sections;

                    float x1 = radius * std::cos(phi1) * std::cos(theta);
                    float y1 = radius * std::sin(phi1);
                    float z1 = radius * std::cos(phi1) * std::sin(theta);

                    float x2 = radius * std::cos(phi2) * std::cos(theta);
                    float y2 = radius * std::sin(phi2);
                    float z2 = radius * std::cos(phi2) * std::sin(theta);

                    glVertex3f(x1, y1, z1);
                    glVertex3f(x2, y2, z2);
                }
                glEnd();
            }
            glPopMatrix();
        }
};

Sphere test(0.7, 12, 0.5f);
void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    glRotatef(30, 1, 1, 0); 
    glColor3f(30.0f,0.0f,0.0f);


    glColor3f(0.0f, 1.0f, 0.5f); // Seafoam green
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); 

    test.drawSphere();
    glutSwapBuffers();
}

void timer(int val)
{
    test.update();
    glutPostRedisplay();
    glutTimerFunc(16, timer, 0);
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    glutCreateWindow("OpenGL Sphere Math Test");

    glEnable(GL_DEPTH_TEST);
    glutDisplayFunc(display);

    glutTimerFunc(0, timer, 0);
    
    glutMainLoop();
    return 0;
}
