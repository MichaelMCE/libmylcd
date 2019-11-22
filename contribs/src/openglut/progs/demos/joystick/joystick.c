/*! \file    joystick.c
    \ingroup demos

    This program is a joystick test program.

    \author  Copyright (C) 2004, the OpenGLUT project contributors.
 
    \image   html openglut_joystick.png OpenGLUT Joystick Demonstration

    \include demos/joystick/joystick.c
*/

#include <GL/openglut.h>

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_AXES 3

static float joystickAxis[MAX_AXES];
static int   joystickButtons;

/* GLUT callback Handlers */

static void
resize(int width, int height)
{
    const float ar = (float) width / (float) height;
    const float sf = 0.8f;

    glViewport(0, 0, width, height);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glFrustum(-ar*sf, ar*sf, -sf, sf, 2.0, 8.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity() ;
}

static void
drawButton(int button, int down)
{
    glPushMatrix();
        glTranslated(-0.9+0.2*button,0.05,0.8);
        glScaled(0.16,0.6,0.16);
        if ( down )
            glColor3d(1.0,0.0,0.0);
        else
            glColor3d(0.4,0.3,0.3);
        glutSolidCube(1.0);
   glPopMatrix();
}

static void
display(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_LIGHTING);

    glPushMatrix();
        glTranslated(0.0,-0.5,-4.5);
        glRotated(30,1.0,0.0,0.0);

        glPushMatrix();
            glScaled(2.0,0.5,2.0);
            glColor3d(0.6,0.6,0.5);
            glutSolidCube(1.0);
        glPopMatrix();

        glPushMatrix();
            glRotated(-35.0f*joystickAxis[0],0.0,0.0,1.0);
            glRotated( 35.0f*joystickAxis[1],1.0,0.0,0.0);
            glTranslated(0.0,1.0,0.0);
            glScaled(0.5,2.0,0.5);
            if (joystickAxis[0]!=0.0f || joystickAxis[1]!=0.0f)
                glColor3d(1.0,0.0,0.0);
            else
                glColor3d(0.3,0.3,0.2);
            glutSolidCube(1.0);
        glPopMatrix();

        glPushMatrix();
            glTranslated(0.9*joystickAxis[2],0.0,1.1);
            glScaled(0.2,0.5,0.2);
            glColor3d(0.5,0.9,0.5);
            glutSolidCube(1.0);                        
        glPopMatrix();

        drawButton( 0, joystickButtons&0x0001 );
        drawButton( 1, joystickButtons&0x0002 );
        drawButton( 2, joystickButtons&0x0004 );
        drawButton( 3, joystickButtons&0x0008 );
        drawButton( 4, joystickButtons&0x0010 );
        drawButton( 5, joystickButtons&0x0020 );
        drawButton( 6, joystickButtons&0x0040 );
        drawButton( 7, joystickButtons&0x0080 );
        drawButton( 8, joystickButtons&0x0100 );
        drawButton( 9, joystickButtons&0x0200 );
            
    glPopMatrix();

    glutSwapBuffers();
}

static void
joystick(unsigned int buttons, int xaxis, int yaxis, int zaxis)
{
    joystickAxis[0] = xaxis/1000.0f;
    joystickAxis[1] = yaxis/1000.0f;
    joystickAxis[2] = zaxis/1000.0f;

    joystickButtons = buttons;

    glutPostRedisplay();
}

static void
key(unsigned char key, int x, int y)
{
    switch (key)
    {
    case 27 :
    case 'Q':
    case 'q': exit(0); 

    default:
        break;
    }

    glutPostRedisplay();
}

const GLfloat light_ambient[]  = { 0.0f, 0.0f, 0.0f, 1.0f };
const GLfloat light_diffuse[]  = { 1.0f, 1.0f, 1.0f, 1.0f };
const GLfloat light_specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
const GLfloat light_position[] = { 2.0f, 5.0f, 5.0f, 0.0f };

const GLfloat mat_ambient[]    = { 0.7f, 0.7f, 0.7f, 1.0f };
const GLfloat mat_diffuse[]    = { 0.8f, 0.8f, 0.8f, 1.0f };
const GLfloat mat_specular[]   = { 1.0f, 1.0f, 1.0f, 1.0f };
const GLfloat high_shininess[] = { 100.0f };

/* Program entry point */

int
main(int argc, char *argv[])
{
    int i;

    glutInitWindowSize(640,480);
    glutInitWindowPosition(40,40);
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);

    glutCreateWindow("OpenGLUT Joystick");

    glutReshapeFunc(resize);
    glutDisplayFunc(display);
    glutJoystickFunc(joystick,50);
    glutKeyboardFunc(key);

    glClearColor(1,1,1,1);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    glEnable(GL_LIGHT0);
    glEnable(GL_NORMALIZE);
    glEnable(GL_COLOR_MATERIAL);

    glLightfv(GL_LIGHT0, GL_AMBIENT,  light_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE,  light_diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);

    glMaterialfv(GL_FRONT, GL_AMBIENT,   mat_ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE,   mat_diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR,  mat_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, high_shininess);

    if( glutDeviceGet(GLUT_HAS_JOYSTICK) )
    {
        printf("Joystick detected.\n");
        printf("%d joystick axes.\n",glutDeviceGet(GLUT_JOYSTICK_AXES));
        printf("Joystick poll rate is %d.\n",glutDeviceGet(GLUT_JOYSTICK_POLL_RATE));
        printf("Joystick %savailable.\n",glutDeviceGet(GLUT_OWNS_JOYSTICK) ? "" : "un");
    }
    else
        printf("Joystick not detected.\n");

    for (i=0; i<MAX_AXES; ++i)
        joystickAxis[i] = 0.0f;

    joystickButtons = 0;

    glutMainLoop();

    return EXIT_SUCCESS;
}
