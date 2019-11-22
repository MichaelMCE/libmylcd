/*! \file    lorenz.c
    \ingroup demos

    Lorenz Strange Attractor

    What it does:

    This program starts with two particles right next to each other.
    
    The particles move through a three-dimensional phase space governed by
    the following equations:
 
    -    \f[ {d \over dt} x = \sigma( y - x ) \f]
    -    \f[ {d \over dt} y = r x - y + x z \f]
    -    \f[ {d \over dt} z = x y + b z \f]
 
    These are the Lorenz equations and define the "Lorenz Attractor."  Any
    two particles arbitrarily close together will move apart as time
    increases, but their tracks are confined within a region of the space.
  
    Commands:
    
    - <tt>Arrow keys </tt>: Rotate the view
    - <tt>PgUp, PgDn </tt>: Zoom in and out
    - <tt>Mouse click</tt>: Pick point on a particle trajectory
    - <tt>'r'/'R'    </tt>: Reset the simulation
    - <tt>'m'/'M'    </tt>: Modify the Lorenz parameters (in the text window)
    - <tt>'s'/'S'    </tt>: Stop (the advancement in time)
    - <tt>'g'/'G'    </tt>: Go
    - <tt>&lt;spacebar&gt; </tt>: Single-step
    - <tt>&lt;Escape&gt;   </tt>: Quit

    \image   html openglut_lorenz.png OpenGLUT Lorenz Attractor Demonstration

    \author  Written by John F. Fay in July 2003

    \author  Portions Copyright (C) 2004, the OpenGLUT project contributors. <br>
             OpenGLUT branched from freeglut in February, 2004.
 
    \include demos/Lorenz/lorenz.c
*/

#include <GL/openglut.h>

#include <time.h>
#include <math.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef MSC_VER
#include <crtdbg.h>  /* DUMP MEMORY LEAKS */
#endif


/******************* Defined Constants ***************************************/
/* Number of points to draw in the curves */
#define NUM_POINTS    512

/* Angle to rotate when the user presses an arrow key */
#define ROTATION_ANGLE  5.0

/* Amount to scale bu when the user presses PgUp or PgDn */
#define SCALE_FACTOR     0.8


/******************** Global Variables ***************************************/
/* Lorenz Attractor variables */
double s0 = 10.0, r0 = 28.0, b0 = 8.0/3.0;   /* Default Lorenz parameters */
double time_step = 0.03;                     /* Time step in the simulation */
double sigma = 10.0, r = 28.0, b = 8.0/3.0;  /* Lorenz attactor parameters */
double red_position[ NUM_POINTS ][ 3 ];      /* Path of the red point */
double grn_position[ NUM_POINTS ][ 3 ];      /* Path of the green point */
int array_index;                             /* Latest point in *_position */
double distance = 0.0;                       /* Distance between red/green */

/* GLUT variables */
/* double yaw = 0.0, pit = 0.0; */            /* Viewing rotation */
/* double scale = 1.0;          */            /* Scale factor */
double xcen = 0.0, ycen = 0.0, zcen = 0.0;   /* Coords of the examined point */

int animate = 1;                             /* 0:stop, 1:go, 2:single-step */


/************************ Functions ******************************************/

/* The Lorenz Attractor */
void calc_deriv( double position[ 3 ], double deriv[ 3 ] )
{
    /* Calculate the Lorenz attractor derivatives */
    deriv[0] = sigma * ( position[ 1 ] - position[ 0 ] );
    deriv[1] =( r + position[ 2 ] ) * position[ 0 ] - position[ 1 ];
    deriv[2] = -position[ 0 ] * position[ 1 ] - b * position[ 2 ];
}

void advance_in_time(
    double time_step, double position[ 3 ], double new_position[ 3 ]
)
{
    /* Move a point along the Lorenz attractor */
    double deriv0[ 3 ], deriv1[ 3 ], deriv2[ 3 ], deriv3[ 3 ];
    int i;
    /* Save the present values */
    memcpy( new_position, position, 3 * sizeof(double ) );

    /* First pass in a Fourth-Order Runge-Kutta integration method */
    calc_deriv( position, deriv0 );
    for( i = 0; i < 3; i++ )
        new_position[ i ] = position[ i ] + 0.5 * time_step * deriv0[ i ];

    /* Second pass */
    calc_deriv( new_position, deriv1 );
    for( i = 0; i < 3; i++ )
        new_position[ i ] = position[ i ] + 0.5 * time_step * deriv1[ i ];

    /* Third pass */
    calc_deriv( position, deriv2 );
    for( i = 0; i < 3; i++ )
        new_position[ i ] = position[ i ] + time_step * deriv2[ i ];

    /* Second pass */
    calc_deriv( new_position, deriv3 );
    for( i = 0; i < 3; i++ )
        new_position[ i ] = position[ i ] + 0.1666666666666666667 * time_step *
            ( deriv0[ i ] + 2.0 *( deriv1[ i ] + deriv2[ i ] ) + deriv3[ i ] );
}

/* GLUT callbacks */

#define INPUT_LINE_LENGTH 80

void key_cb( unsigned char key, int x, int y )
{
    int i;
    char inputline [ INPUT_LINE_LENGTH ];

    switch( key )
    {
    case 'r':
    case 'R':  /* Reset the simulation */
        /* Reset the Lorenz parameters */
        sigma = s0;
        b = b0;
        r = r0;
        /* Set an initial position */
        red_position[ 0 ][ 0 ] = rand( ) / ( double )RAND_MAX;
        red_position[ 0 ][ 1 ] = rand( ) / ( double )RAND_MAX;
        red_position[ 0 ][ 2 ] = rand( ) / ( double )RAND_MAX;
        grn_position[ 0 ][ 0 ] = rand( ) / ( double )RAND_MAX;
        grn_position[ 0 ][ 1 ] = rand( ) / ( double )RAND_MAX;
        grn_position[ 0 ][ 2 ] = rand( ) / ( double )RAND_MAX;
        array_index = 0;
        /* Initialize the arrays */
        for( i = 1; i < NUM_POINTS; i++ )
        {
            memcpy( red_position[ i ], red_position[ 0 ], 3*sizeof( double ) );
            memcpy( grn_position[ i ], grn_position[ 0 ], 3*sizeof( double ) );
        }

        break;

    case 'm':
    case 'M':  /* Modify the Lorenz parameters */
        printf(
            "Please enter new value for <sigma> (default %f, currently %f): ",
            s0, sigma
        );
        fgets( inputline, INPUT_LINE_LENGTH- 1 , stdin );
        sscanf( inputline, "%lf", &sigma );

        printf(
            "Please enter new value for <b> (default %f, currently %f): ",
            b0, b
        );
        fgets( inputline, INPUT_LINE_LENGTH - 1, stdin );
        sscanf( inputline, "%lf", &b );

        printf(
            "Please enter new value for <r> (default %f, currently %f): ",
            r0, r
        );
        fgets( inputline, INPUT_LINE_LENGTH - 1, stdin );
        sscanf( inputline, "%lf", &r );

        break;

    case 's':
    case 'S':  /* Stop the animation */
        animate = 0;
        break;

    case 'g':
    case 'G':  /* Start the animation */
        animate = 1;
        break;

    case ' ':  /* Spacebar:  Single step */
        animate = 2;
        break;

    case 27:  /* Escape key */
        glutLeaveMainLoop( );
        break;
    }
}

void special_cb( int key, int x, int y )
{
    switch( key )
    {
    case GLUT_KEY_UP:  /* Rotate up a little */
        glRotated( ROTATION_ANGLE, 0.0, 1.0, 0.0 );
        break;

    case GLUT_KEY_DOWN:  /* Rotate down a little */
        glRotated( -ROTATION_ANGLE, 0.0, 1.0, 0.0 );
        break;

    case GLUT_KEY_LEFT:  /* Rotate left a little */
        glRotated( ROTATION_ANGLE, 0.0, 0.0, 1.0 );
        break;

    case GLUT_KEY_RIGHT:  /* Rotate right a little */
        glRotated( -ROTATION_ANGLE, 0.0, 0.0, 1.0 );
        break;

    case GLUT_KEY_PAGE_UP:  /* Zoom in a little */
        glScaled( 1.0 / SCALE_FACTOR, 1.0 / SCALE_FACTOR, 1.0 / SCALE_FACTOR );
        break;

    case GLUT_KEY_PAGE_DOWN:  /* Zoom out a little */
        glScaled( SCALE_FACTOR, SCALE_FACTOR, SCALE_FACTOR );
        break;
    }

    glutPostRedisplay( );
}

void mouse_cb( int button, int updown, int x, int y )
{
    double dist = 1.0e20;  /* A very large number */
    if( updown == GLUT_DOWN )
        dist = 0.0;  /* so we don't get "unused variable" compiler warning */
        /*
         * The idea here is that we pick the nearest point
         * to the mouse click position.  Unfortunately I don't have the time to
         * implement it at the moment.
         *
         * XXX So, is that the nearest of the points in the ``trail''?
         * YYY (Yes, it is, John says.)
         */
}

void draw_curve( int index, double position[ NUM_POINTS ][ 3 ] )
{
    int i = index;

    glBegin( GL_LINE_STRIP );
    do
    {
        i = ( i == NUM_POINTS - 1 ) ? 0 : i + 1;
        glVertex3dv( position[ i ] );
    }
    while( i != index );

    glEnd( );
}

void display_cb( void )
{
    char string [ 80 ];

    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    glColor3d( 1.0, 1.0, 1.0 );  /* White */
    /* Draw some axes */
    glBegin( GL_LINES );
    glVertex3d( 0.0, 0.0, 0.0 );
    glVertex3d( 2.0, 0.0, 0.0 );
    glVertex3d( 0.0, 0.0, 0.0 );
    glVertex3d( 0.0, 1.0, 0.0 );
    glVertex3d( 0.0, 0.0, 0.0 );
    glVertex3d( 0.0, 0.0, 1.0 );
    glEnd( );

    glColor3d( 1.0, 0.0, 0.0 );  /* Red */
    draw_curve( array_index, red_position );

    glColor3d( 0.0, 1.0, 0.0 );  /* Green */
    draw_curve( array_index, grn_position );

    /* Print the distance between the two points */
    glColor3d( 1.0, 1.0, 1.0 );  /* White */
    sprintf( string, "Distance: %10.6f", distance );
    glRasterPos2i( 10, 10 );
    glutBitmapString( GLUT_BITMAP_HELVETICA_12, string );

    glutSwapBuffers( );
}

void reshape_cb( int width, int height )
{
    double ar;
    glViewport( 0, 0, width, height );
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity( );
    ar =  width * 1.0 / height;
    if( ar > 1.0 )
	glFrustum( -ar, ar, -1.0, 1.0, 10.0, 100.0 );
    else
	glFrustum( -1.0, 1.0, -1/ar, 1/ar, 10.0, 100.0 );
    glMatrixMode( GL_MODELVIEW );
    xcen = 0.0;
    ycen = 0.0;
    zcen = 0.0;
    glTranslated( xcen, ycen, zcen - 50.0 );
}


void timer_cb( int value )
{
    /* Function called at intervals to update the positions of the points */
    double deltax, deltay, deltaz;
    int new_index = array_index + 1;

    /* Set the next timed callback */
    glutTimerFunc( 30, timer_cb, 0 );

    if( animate > 0 )
    {
        if( new_index == NUM_POINTS )
            new_index = 0;
        advance_in_time(
            time_step, red_position[ array_index ], red_position[ new_index ]
        );
        advance_in_time(
            time_step, grn_position[ array_index ], grn_position[ new_index ]
        );
        array_index = new_index;

        deltax =
            red_position[ new_index ][ 0 ] - grn_position[ new_index ][ 0 ];
        deltay =
            red_position[ new_index ][ 1 ] - grn_position[ new_index ][ 1 ];
        deltaz =
            red_position[ new_index ][ 2 ] - grn_position[ new_index ][ 2 ];
        distance = sqrt( deltax * deltax + deltay * deltay + deltaz * deltaz );

        if( animate == 2 )
            animate = 0;
    }

    glutPostRedisplay( );
}



/* The Main Program */

int main( int argc, char *argv[] )
{
    int pargc = argc;

    /* Initialize the random number generator */
    srand(( int )time( NULL ) );

    /* Set up the OpenGL parameters */
    glEnable( GL_DEPTH_TEST );
    glClearColor( 0.0, 0.0, 0.0, 0.0 );
    glClearDepth( 1.0 );

    /* Initialize GLUT */
    glutInitWindowSize( 600, 600 );
    glutInit( &pargc, argv );
    glutInitDisplayMode( GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH );

    /* Create the window */
    glutCreateWindow( "Lorenz Attractor" );
    glutKeyboardFunc( key_cb );
    glutMouseFunc( mouse_cb );
    glutSpecialFunc( special_cb );
    glutDisplayFunc( display_cb );
    glutReshapeFunc( reshape_cb );
    glutTimerFunc( 30, timer_cb, 0 );

    /*
     *  Initialize the attractor:
     *  The easiest way is to call the keyboard callback with an
     *  argument of 'r' for Reset.
     */
    key_cb( 'r', 0, 0 );

    /* Enter the GLUT main loop */
    glutMainLoop( );

#ifdef MSC_VER
    _CrtDumpMemoryLeaks( );  /* DUMP MEMORY LEAK INFORMATION */
#endif

    return EXIT_SUCCESS;
}

