/*! \file    unmanaged.c
    \ingroup demos

    OpenGLUT client menu window example.
 
    Usage:
  
       unmanaged [standard GLUT options]
  
   Some suggested uses for menu windows:
  
       - Replacing built-in menus with your own.
       - Tool-tip windowlets.
       - Draggable icons between windows.
 
   Programming notes:
 
    - The menu items could be done with subwindows of the menu
      window.  That might be better.
    - No keyboard navigation at this time.  (Easy to add, though.)
    - Because menu windows do not get handled by the window
      manager, we have to force a reshape of our menu window.  The
      alternative is to just hard-code with the default OpenGL context.
      This is really an OpenGLUT bug (I think), but fixing it will affect
      the internal menus, I think, and for use in menus, it is workable.
      For now.
    - There's room in the menus for borders and more items.
      But this isn't supposed to be a polished menu system.  Rather,
      it is supposed to demonstrate that you *can* build workable
      menus out of client menu windows.

    \author  Copyright (C) 2004, the OpenGLUT project contributors.
 
    \image   html openglut_unmanaged.png OpenGLUT Client Menu Window Example
    \include demos/unmanaged/unmanaged.c
*/
#include <stdlib.h>

#include <GL/openglut.h>
#include <GL/openglut_exp.h>


int window_id;
int object;

#define MENU_WIDTH  100
#define MENU_HEIGHT 300
int menu_id;
int in_menu;

/*
 * menu_x and menu_y hold the menu window coordinates on-screen.
 * x and y are the menu coordinates relative to the parent window
 * This is a bit hokey, I know; the code evolved a bit and needs to
 * be consolidated.  Sorry.
 */
int x;
int y;
int menu_x;
int menu_y;
int menu_select;

enum {MENU_QUIT = 1, MENU_TEAPOT, MENU_CUBE, MENU_SPHERE, MENU_SPONGE};

GLfloat light_color[ 3 ] = { ( GLfloat )1, ( GLfloat )1, ( GLfloat ).9 };
GLfloat light_pos[ 3 ] =   { ( GLfloat )3, ( GLfloat )4, ( GLfloat )5 };

/*
 * A filter for glColor(); usurps with white the {item} is {menu_select}ed.
 *
 * Used to simplify the logic of coloring menu items when they are to be
 * highlighted.
 */
void set_menu_color( const int item, double r, double g, double b )
{
    if( item == menu_select )
        r = g = b = 1.0;
    glColor3d( ( GLdouble )r, ( GLdouble )g, ( GLdouble )b );
}

/*
 * Draws a rod, intended to be used for the borders of menus.
 */
void draw_border_rod( double len )
{
    glutSolidCylinder( ( GLdouble )3,
                       ( GLdouble )len, ( GLint )10, ( GLint )2 );
}

/*
 * Draws a ball of the same radius as the above rod.
 *
 * Used for capping rods with round tips.
 */
void draw_corner_ball( void )
{
    glutSolidSphere( ( GLdouble )3, ( GLint )10, ( GLint )10 );
}


void menu_display( void )
{
    static const GLdouble sponge_offset[ 3 ] =
        { ( GLdouble )0, ( GLdouble )0, ( GLdouble )0 };
    int w = glutGet( GLUT_WINDOW_WIDTH );
    int h = glutGet( GLUT_WINDOW_HEIGHT );

    glClearColor( ( GLclampf ).5, ( GLclampf ).5, ( GLclampf ).5,
                  ( GLclampf )0 );
    glClear( ( GLbitfield )( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT ) );

    glEnable( GL_LIGHTING );
    glPushMatrix( );
    glTranslated( ( GLdouble )3, ( GLdouble )3, ( GLdouble )-5 );
    glRotated( ( GLdouble )-90, ( GLdouble )1, ( GLdouble )0, ( GLdouble )0 );
    draw_border_rod( h - 6 );
    glTranslated( ( GLdouble )( w - 6 ), ( GLdouble )0, ( GLdouble )0 );
    draw_border_rod( h - 6 );
    glPopMatrix( );
    glPushMatrix( );
    glTranslated( ( GLdouble )3, ( GLdouble )3, ( GLdouble )-5 );
    glRotated( ( GLdouble )90, ( GLdouble )0, ( GLdouble )1, ( GLdouble )0 );
    draw_border_rod( w - 6 );
    glTranslated( ( GLdouble )0, ( GLdouble )( h - 6 ), ( GLdouble )0 );
    draw_border_rod( w - 6 );
    glPopMatrix( );

    glPushMatrix( );
    glTranslated( ( GLdouble )3, ( GLdouble )3, ( GLdouble )-5 );
    draw_corner_ball( );
    glTranslated( ( GLdouble )( w - 6 ), ( GLdouble )0, ( GLdouble )0 );
    draw_corner_ball( );
    glTranslated( ( GLdouble )0, ( GLdouble )( h - 6 ), ( GLdouble )0 );
    draw_corner_ball( );
    glTranslated( ( GLdouble )( 6 - w ), ( GLdouble )0, ( GLdouble )0 );
    draw_corner_ball( );
    glPopMatrix( );

    glDisable( GL_LIGHTING );
    set_menu_color( MENU_QUIT, 0, 0, 0 );
    glRasterPos3d( ( GLdouble )30, ( GLdouble )( h - 30 ), ( GLdouble )-5 );
    glutBitmapString( GLUT_BITMAP_TIMES_ROMAN_24, ( unsigned char *)"Quit" );

    glPushMatrix( );
    set_menu_color( MENU_TEAPOT, 1, 0, 0 );
    glTranslated( ( GLdouble )50, ( GLdouble )( h - 76 ), ( GLdouble )-50 );
    glutWireTeapot( ( GLdouble )25 );
    glPopMatrix( );

    glPushMatrix( );
    set_menu_color( MENU_CUBE, 1, 0, 0 );
    glTranslated( ( GLdouble )50, ( GLdouble )( h - 126 ), ( GLdouble )-50 );
    glutWireCube( ( GLdouble )25 );
    glPopMatrix( );
    
    glPushMatrix( );
    set_menu_color( MENU_SPHERE, 1, 0, 0 );
    glTranslated( ( GLdouble )50, ( GLdouble )( h - 176 ), ( GLdouble )-50 );
    glutWireSphere( ( GLdouble )25, ( GLint )8, ( GLint )8 );
    glPopMatrix( );
    
    glPushMatrix( );
    set_menu_color( MENU_SPONGE, 1, 0, 0 );
    glTranslated( ( GLdouble )50, ( GLdouble )( h - 226 ), ( GLdouble )-50 );
    glutWireSierpinskiSponge( 2, sponge_offset, ( GLdouble )25 );
    glPopMatrix( );

    glEnable( GL_LIGHTING );
    glutSwapBuffers( );
}

void menu_reshape( int w, int h )
{
    glEnable( GL_DEPTH_TEST );
    glEnable( GL_CULL_FACE );
    glEnable( GL_LIGHTING );
    glEnable( GL_LIGHT0 );
    glLightfv( GL_LIGHT0, GL_POSITION, light_pos );
    glLightfv( GL_LIGHT0, GL_DIFFUSE, light_color );
    glLightfv( GL_LIGHT0, GL_SPECULAR, light_color );
    glViewport( ( GLint )0, ( GLint )0, ( GLsizei )w, ( GLsizei )h );
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity( );
    glOrtho(
        ( GLdouble )0, ( GLdouble )w,
        ( GLdouble )0, ( GLdouble )h,
        ( GLdouble )1, ( GLdouble )100
    );
    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity( );
    glutPostRedisplay( );
}

void cb_motion( int x, int y );
void cb_mouse( int button, int state, int x, int y );
void sync_menu( void )
{
    if( in_menu )
    {
        if( !menu_id )
        {
            glutInitDisplayMode( GLUT_DEPTH | GLUT_DOUBLE );
            menu_id = glutCreateMenuWindow(
                window_id, x, y, MENU_WIDTH, MENU_HEIGHT
            );
            glutDisplayFunc( menu_display );
            glutReshapeFunc( menu_reshape );

            /* XXX Hack; maybe no longer necessary */
            menu_reshape( MENU_WIDTH, MENU_HEIGHT); /* XXX */
        }
    }
    else if( menu_id )
    {
        glutDestroyWindow( menu_id );
        menu_id = 0;
    }
}


void cb_display( void )
{
    int msecs = glutGet( GLUT_ELAPSED_TIME );
    GLdouble theta = ( GLdouble )( .01 * msecs );
    static const GLdouble sponge_offset[ 3 ] =
        { ( GLdouble )0, ( GLdouble )0, ( GLdouble )0 };
    
    glClear( ( GLbitfield )( GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT ) );
    glColor3d( ( GLdouble )1, ( GLdouble )1, ( GLdouble )1 );
    glPushMatrix( );
    glRotated( theta, ( GLdouble )0, ( GLdouble )1, ( GLdouble )0 );
    switch( object )
    {
    case 0:
        glPopMatrix( );
        glPushMatrix( );
        glNormal3d( ( GLdouble )0, ( GLdouble )0, ( GLdouble )1 );
        glColor3d( ( GLdouble )1, ( GLdouble )1, ( GLdouble )1 );
        glRasterPos2d( ( GLdouble )-1.9, ( GLdouble )0.3 );
        glDisable( GL_LIGHTING );
        glutBitmapString(
            GLUT_BITMAP_TIMES_ROMAN_24,
            ( unsigned char * )"Click on me for menus!"
        );
        glEnable( GL_LIGHTING );
        break;
    case MENU_TEAPOT:
        glutSolidTeapot( ( GLdouble )1 );
        break;
    case MENU_CUBE:
        glutSolidCube( ( GLdouble )1 );
        break;
    case MENU_SPHERE:
        glutSolidSphere( ( GLdouble )1, ( GLint )50, ( GLint )50 );
        break;
    case MENU_SPONGE:
        glutSolidSierpinskiSponge(
            abs( ( ( msecs / 400 ) % 15 ) - 7 ) , sponge_offset, ( GLdouble )1
        );
        break;
    default:
        break;
    }
    glPopMatrix( );
    glutSwapBuffers( );
}

void cb_motion( int _x, int _y )
{
    int x; /* = _x + glutGet( GLUT_WINDOW_X ) - menu_x; */
    int y; /* = _y + glutGet( GLUT_WINDOW_Y ) - menu_y; */
    glutSetWindow( window_id );
    x = _x + glutGet( GLUT_WINDOW_X ) - menu_x;
    y = _y + glutGet( GLUT_WINDOW_Y ) - menu_y;

    menu_select = 0;
    if( ( 6 < x ) && ( ( MENU_WIDTH - 6 ) > x ) &&
        ( 6 < y ) && ( ( MENU_HEIGHT - 6 ) > y ) )
    {
        if( 30 > y )
            menu_select = MENU_QUIT;
        else if( 100 > y )
            menu_select = MENU_TEAPOT;
        else if( 150 > y )
            menu_select = MENU_CUBE;
        else if( 200 > y )
            menu_select = MENU_SPHERE;
        else if( 250 > y )
            menu_select = 5;
        else if( 300 > y )
            menu_select = 6;
        else
            menu_select = 7;
        glutSetWindow( menu_id );
        glutPostRedisplay( );
    }
}



void cb_mouse( int button, int state, int _x, int _y )
{
    x = _x;
    y = _y;
    if( !in_menu && ( GLUT_DOWN == state ) )
    {
        in_menu = button+1;
        menu_x = x + glutGet( GLUT_WINDOW_X );
        menu_y = y + glutGet( GLUT_WINDOW_Y );
    }
    else if( ( ( in_menu - 1 ) == button ) && ( GLUT_UP == state ) )
    {
        if( MENU_QUIT == menu_select )
            exit( 0 );
        object = menu_select;
        glutPostRedisplay( );
        in_menu = 0;
    }
    sync_menu( );
}


void cb_idle (void )
{
    glutSetWindow( window_id );
    glutPostRedisplay( );
}

void cb_reshape( int w, int h )
{
    double ar = w / (1.0 * h);

    glViewport(
        ( GLint )0, ( GLint )0,
        ( GLsizei )w, ( GLsizei )h
    );
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity( );
    if( w > h )
        glFrustum(
            ( GLdouble )-ar, ( GLdouble )ar,
            ( GLdouble )-1,  ( GLdouble )1,
            ( GLdouble )1,   ( GLdouble )50
        );
    else
        glFrustum(
            ( GLdouble ) -1,       ( GLdouble )1,
            ( GLdouble )( -1/ar ), ( GLdouble )( 1/ar ),
            ( GLdouble )1,         ( GLdouble )50
        );
    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity( );
    glTranslated( ( GLdouble )0, ( GLdouble )0, ( GLdouble )-3 );

    glEnable( GL_DEPTH_TEST );
    glEnable( GL_CULL_FACE );
    glCullFace( GL_BACK );
    glEnable( GL_LIGHTING );
    glEnable( GL_LIGHT0 );
    glLightfv( GL_LIGHT0, GL_POSITION, light_pos );
    glLightfv( GL_LIGHT0, GL_DIFFUSE, light_color );
    glLightfv( GL_LIGHT0, GL_SPECULAR, light_color );
}


int main( int argc, char **argv )
{
    glutInit( &argc, argv );

    glutInitDisplayMode( GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH );
    glutInitWindowPosition( 100, 100 );
    window_id = glutCreateWindow( "Unmanaged window demo" );
    glutReshapeFunc( cb_reshape );
    glutDisplayFunc( cb_display );
    glutMouseFunc( cb_mouse );
    glutMotionFunc( cb_motion );

    glutIdleFunc( cb_idle );

    glutMainLoop( );

    return EXIT_SUCCESS;
}
