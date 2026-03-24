#include <GL/freeglut.h>
#include <cmath>

const float PI = 3.141592653589f;
const int SCREEN_SIZE = 800;

class Planet
{
    public:
        float radius;
        int sections;
        
        float axisAngle;
        float axisSpeed;

        float orbitAngle;
        float orbitSpeed;
        float orbitDistance; // How far from the Sun

        Planet(float radius, int sections, float axisSpd, float orbitSpd, float distance)
        {
            this->radius = radius;
            this->sections = sections;
            
            this->axisSpeed = axisSpd;
            this->axisAngle = 0.0f;

            this->orbitSpeed = orbitSpd;
            this->orbitAngle = 0.0f;
            this->orbitDistance = distance;
        }

        void update()
        {
            axisAngle += axisSpeed;
            if(axisAngle > 360.0f) axisAngle -= 360.0f;

            orbitAngle += orbitSpeed;
            if(orbitAngle > 360.0f) orbitAngle -= 360.0f;
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

                    glVertex3f(x1, y1, z1);
                    glVertex3f(x2, y2, z2);
                }
                glEnd();
            }
            
            glPopMatrix(); 
        }
};

Planet sun(0.3f, 20, 0.5f, 0.0f, 0.0f);   
Planet earth(0.1f, 15, 2.0f, 1.0f, 0.7f);

void display() 
{

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    glRotatef(30, 1, 0, 0); 

    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); 

    glColor3f(1.0f, 1.0f, 0.0f); 
    sun.draw();

    glColor3f(0.0f, 0.8f, 1.0f); 
    earth.draw();

    glutSwapBuffers();
}

void timer(int val)
{
    sun.update();
    earth.update();

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
    glutTimerFunc(0,timer,0);
    
    glutMainLoop();
    return 0;
}
