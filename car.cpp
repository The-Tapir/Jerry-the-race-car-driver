//program: car.cpp
//author:  Gordon Griesel
//Modified by: Moss, David, Alex, Nathan
//date:    Fall 2023
//Framework for group attempting a 3D game.

#include "global.h"
#include "common.h"

typedef float Flt;
typedef Flt Vec[3];
typedef Flt	Matrix[4][4];

//Variables added by Nathan
int timed = 0;
bool show_stats = false;
//Variables added by David
int selectedOption = 0;// 0 represents "Play" as the default selection
enum GameState { MENU, PLAY, CONTROLS, EXIT};
GameState gameState = MENU;
extern float Pos1;
extern float Pos2;
extern float Pos3;
extern float theta;

void init();
void init_opengl();
void check_mouse(XEvent *e);
int check_keys(XEvent *e);
void physics();
void drawStreet();
void render();

//----
bool pointInTriangle(float, int, int);

class X11_wrapper {
	private:
		Display *dpy;
		Window win;
		GLXContext glc;
	public:
		X11_wrapper() {
			//Look here for information on XVisualInfo parameters.
			//http://www.talisman.org/opengl-1.1/Reference/glXChooseVisual.html
			Window root;
			GLint att[] = { GLX_RGBA,
				GLX_STENCIL_SIZE, 2,
				GLX_DEPTH_SIZE, 24,
				GLX_DOUBLEBUFFER, None };
			//GLint att[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None };
			//GLint att[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, None };
			//XVisualInfo *vi;
			Colormap cmap;
			XSetWindowAttributes swa;
			setup_screen_res(640, 480);
			dpy = XOpenDisplay(NULL);
			if (dpy == NULL) {
				printf("\n\tcannot connect to X server\n\n");
				exit(EXIT_FAILURE);
			}
			root = DefaultRootWindow(dpy);
			XVisualInfo *vi = glXChooseVisual(dpy, 0, att);
			if (vi == NULL) {
				printf("\n\tno appropriate visual found\n\n");
				exit(EXIT_FAILURE);
			} 
			cmap = XCreateColormap(dpy, root, vi->visual, AllocNone);
			swa.colormap = cmap;
			swa.event_mask = ExposureMask | KeyPressMask | KeyReleaseMask |
				StructureNotifyMask | SubstructureNotifyMask;
			win = XCreateWindow(dpy, root, 0, 0, g.xres, g.yres, 0,
					vi->depth, InputOutput, vi->visual,
					CWColormap | CWEventMask, &swa);
			set_title();
			glc = glXCreateContext(dpy, vi, NULL, GL_TRUE);
			glXMakeCurrent(dpy, win, glc);
		}
		~X11_wrapper() {
			XDestroyWindow(dpy, win);
			XCloseDisplay(dpy);
		}
		void setup_screen_res(const int w, const int h) {
			g.xres = w;
			g.yres = h;
			g.aspectRatio = (GLfloat)g.xres / (GLfloat)g.yres;
		}
		void check_resize(XEvent *e) {
			//The ConfigureNotify is sent by the
			//server if the window is resized.
			if (e->type != ConfigureNotify)
				return;
			XConfigureEvent xce = e->xconfigure;
			if (xce.width != g.xres || xce.height != g.yres) {
				//Window size did change.
				reshape_window(xce.width, xce.height);
			}
		}
		void reshape_window(int width, int height) {
			//window has been resized.
			setup_screen_res(width, height);
			//
			glViewport(0, 0, (GLint)width, (GLint)height);
			glMatrixMode(GL_PROJECTION); glLoadIdentity();
			glMatrixMode(GL_MODELVIEW); glLoadIdentity();
			glOrtho(0, g.xres, 0, g.yres, -1, 1);
			set_title();
		}
		void set_title() {
			//Set the window title bar.
			XMapWindow(dpy, win);
			XStoreName(dpy, win, "Jerry The Race Car Driver");
		}
		bool getXPending() {
			return XPending(dpy);
		}
		XEvent getXNextEvent() {
			XEvent e;
			XNextEvent(dpy, &e);
			return e;
		}
		void swapBuffers() {
			glXSwapBuffers(dpy, win);
		}
} x11;


int main()
{
	init_opengl();
	init();
	int done = 0;
	total_running_time(false);
	PlaySound("./sounds/background_music.wav");
	while ( (!done) && (gameState != EXIT) ) {
		while (x11.getXPending()) {
			XEvent e = x11.getXNextEvent();
			x11.check_resize(&e);
			check_mouse(&e);
			done = check_keys(&e);
		}
		physics();
		render();
		x11.swapBuffers();
	}
	cleanup_fonts();
	cleanupAL();
	return 0;
}

void init()
{
	alutInit(0, NULL);
}

void init_opengl()
{
	//OpenGL initialization
	glDepthFunc(GL_LESS);
	glEnable(GL_DEPTH_TEST);
	glShadeModel(GL_SMOOTH);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0f, g.aspectRatio, 0.1f, 100.0f);
	glMatrixMode(GL_MODELVIEW);
	//Enable this so material colors are the same as vert colors.
	glEnable(GL_COLOR_MATERIAL);
	glEnable(GL_LIGHTING);
	//Turn on a light
	glLightfv(GL_LIGHT0, GL_POSITION, g.lightPosition);
	glEnable(GL_LIGHT0);
	//Do this to allow fonts
	glEnable(GL_TEXTURE_2D);
	initialize_fonts();
	//init_textures();
}

Flt vecNormalize(Vec vec) {
	Flt len = vec[0]*vec[0] + vec[1]*vec[1] + vec[2]*vec[2];
	if (len == 0.0) {
		MakeVector(0.0,0.0,1.0,vec);
		return 1.0;
	}
	len = sqrt(len);
	Flt tlen = 1.0 / len;
	vec[0] *= tlen;
	vec[1] *= tlen;
	vec[2] *= tlen;
	return len;
}

void check_mouse(XEvent *e)
{
	//Did the mouse move?
	//Was a mouse button clicked?
	static int savex = 0;
	static int savey = 0;
	//
	if (e->type == ButtonRelease) {
		return;
	}
	if (e->type == ButtonPress) {
		if (e->xbutton.button==1) {
			//Left button is down
		}
		if (e->xbutton.button==3) {
			//Right button is down
		}
	}
	if (savex != e->xbutton.x || savey != e->xbutton.y) {
		//Mouse moved
		time_since_mouse_moved(false);

		savex = e->xbutton.x;
		savey = e->xbutton.y;
	}
	mouseMovement(e, false);    
}


int check_keys(XEvent *e)
{
	//Was there input from the keyboard?
	Vec dir;
	if (e->type == KeyPress) {

		key_presses(false);
		//look up what library XLookupKeysym(&e->xkey, 0)
		int key = (XLookupKeysym(&e->xkey, 0) & 0x0000ffff);

		//---------------------------------------------
		//Flt cameraAngle;
		//cameraAngle = PI * (PI * 0.5);
		//---------------------------------------------
		switch(key) {
			case XK_Up:
				if(selectedOption > 0) {
					selectedOption--;
				}

				// Move the car forward in the calculated direction
				if (gameState == PLAY) 
					CarMovement("up");
				break;
			case XK_Down:
				if(selectedOption < 2) {
					selectedOption++;
				}
				// Move the car backwards in the calculated direction
				if (gameState == PLAY) 
					CarMovement("down");         
				break;
			case XK_Return:
				if(selectedOption == 0) {
					gameState = PLAY;
					drawStreet();
					drawTrack();
				} else if(selectedOption == 1) {
					gameState = CONTROLS;
				} else if(selectedOption == 2) {
					gameState = EXIT;
				}
				break;
			case XK_1:
				break;
			case XK_Right:
				g.cameraPosition[0] += 0.1;
				break;
			case XK_Left:
				g.cameraPosition[0] -= 0.1;
				break;
			case XK_q:
				if (gameState == PLAY) 
					CarMovement("q");		
				break;
			case XK_e:
				if (gameState == PLAY) 
					CarMovement("e");
				/*
				   Pos3 -= 1.0f;
				   theta -= 0.05f;
				   g.cameraPosition[2] += 1.0f;
				   g.cameraAngle += 0.05f;
				   */
				break;
			case XK_b:
				break;
			case XK_t:
				show_stats = !show_stats;
				std::cout << g.cameraPosition[0] << "," << g.cameraPosition[1] << "," << g.cameraPosition[2];
				break;
			case XK_a:
				//Steer car to the left
				g.cameraAngle -= 0.05;
				break;

			case XK_d:
				//Steer car to the right
				g.cameraAngle += 0.05;
				break;

			case XK_w:
				//move car forward
				PlaySound("./sounds/car_sound.wav");
				dir[0] = cos(g.cameraAngle);
				dir[2] = sin(g.cameraAngle);
				dir[1] = 0.0;

				//g.cameraPosition[0] += car_acceleration(true);
				//g.cameraPosition[1] += car_acceleration(true);
				//g.cameraPosition[2] += car_acceleration(true);

				//	dir[0] *= car_acceleration(true);
				//	dir[2] *= car_acceleration(true);


				VecAdd(g.cameraPosition, dir, g.cameraPosition);
				break;

			case XK_s:
				//move car backwards
				PlaySound("./sounds/brake.wav");

				dir[0] = cos(g.cameraAngle);
				dir[2] = sin(g.cameraAngle);
				dir[1] = 0.0;


				//dir[0] *= car_slow_down();
				//	dir[2] *= car_slow_down();

				VecSub(g.cameraPosition, dir, g.cameraPosition);
				break;

			case XK_Tab:
				timed = timer();
				break;
				//-----------------------------------------------------
			case XK_Escape:
				if (gameState != MENU) {
					gameState = MENU;
				}
				else {
					return 1;
				}
				break;
		}
	}
	return 0;
}



void box(float w1, float h1, float d1)
{
	float w=w1*0.5;
	float d=d1*0.5;
	float h=h1*0.5;
	//notice the normals being set
	glBegin(GL_QUADS);
	//top
	glNormal3f( 0.0f, 1.0f, 0.0f);
	glVertex3f( w, h,-d);
	glVertex3f(-w, h,-d);
	glVertex3f(-w, h, d);
	glVertex3f( w, h, d);
	// bottom
	glNormal3f( 0.0f, -1.0f, 0.0f);
	glVertex3f( w,-h, d);
	glVertex3f(-w,-h, d);
	glVertex3f(-w,-h,-d);
	glVertex3f( w,-h,-d);
	// front
	glNormal3f( 0.0f, 0.0f, 1.0f);
	glVertex3f( w, h, d);
	glVertex3f(-w, h, d);
	glVertex3f(-w,-h, d);
	glVertex3f( w,-h, d);
	// back
	glNormal3f( 0.0f, 0.0f, -1.0f);
	glVertex3f( w,-h,-d);
	glVertex3f(-w,-h,-d);
	glVertex3f(-w, h,-d);
	glVertex3f( w, h,-d);
	// left side
	glNormal3f(-1.0f, 0.0f, 0.0f);
	glVertex3f(-w, h, d);
	glVertex3f(-w, h,-d);
	glVertex3f(-w,-h,-d);
	glVertex3f(-w,-h, d);
	// Right side
	glNormal3f( 1.0f, 0.0f, 0.0f);
	glVertex3f( w, h,-d);
	glVertex3f( w, h, d);
	glVertex3f( w,-h, d);
	glVertex3f( w,-h,-d);
	glEnd();
	glEnd();
}


void identity33(Matrix m)
{
	m[0][0] = m[1][1] = m[2][2] = 1.0f;
	m[0][1] = m[0][2] = m[1][0] = m[1][2] = m[2][0] = m[2][1] = 0.0f;
}

void yy_transform(const Vec rotate, Matrix a)
{
	//This function applies a rotation to a matrix.
	//Call this function first, then call trans_vector() to apply the
	//rotations to an object or vertex.
	//
	if (rotate[0] != 0.0f) {
		Flt ct = cos(rotate[0]), st = sin(rotate[0]);
		Flt t10 = ct*a[1][0] - st*a[2][0];
		Flt t11 = ct*a[1][1] - st*a[2][1];
		Flt t12 = ct*a[1][2] - st*a[2][2];
		Flt t20 = st*a[1][0] + ct*a[2][0];
		Flt t21 = st*a[1][1] + ct*a[2][1];
		Flt t22 = st*a[1][2] + ct*a[2][2];
		a[1][0] = t10;
		a[1][1] = t11;
		a[1][2] = t12;
		a[2][0] = t20;
		a[2][1] = t21;
		a[2][2] = t22;
		return;
	}
	if (rotate[1] != 0.0f) {
		Flt ct = cos(rotate[1]), st = sin(rotate[1]);
		Flt t00 = ct*a[0][0] - st*a[2][0];
		Flt t01 = ct*a[0][1] - st*a[2][1];
		Flt t02 = ct*a[0][2] - st*a[2][2];
		Flt t20 = st*a[0][0] + ct*a[2][0];
		Flt t21 = st*a[0][1] + ct*a[2][1];
		Flt t22 = st*a[0][2] + ct*a[2][2];
		a[0][0] = t00;
		a[0][1] = t01;
		a[0][2] = t02;
		a[2][0] = t20;
		a[2][1] = t21;
		a[2][2] = t22;
		return;
	}
	if (rotate[2] != 0.0f) {
		Flt ct = cos(rotate[2]), st = sin(rotate[2]);
		Flt t00 = ct*a[0][0] - st*a[1][0];
		Flt t01 = ct*a[0][1] - st*a[1][1];
		Flt t02 = ct*a[0][2] - st*a[1][2];
		Flt t10 = st*a[0][0] + ct*a[1][0];
		Flt t11 = st*a[0][1] + ct*a[1][1];
		Flt t12 = st*a[0][2] + ct*a[1][2];
		a[0][0] = t00;
		a[0][1] = t01;
		a[0][2] = t02;
		a[1][0] = t10;
		a[1][1] = t11;
		a[1][2] = t12;
		return;
	}
}

void trans_vector(Matrix mat, const Vec in, Vec out)
{
	Flt f0 = mat[0][0] * in[0] + mat[1][0] * in[1] + mat[2][0] * in[2];
	Flt f1 = mat[0][1] * in[0] + mat[1][1] * in[1] + mat[2][1] * in[2];
	Flt f2 = mat[0][2] * in[0] + mat[1][2] * in[1] + mat[2][2] * in[2];
	out[0] = f0;
	out[1] = f1;
	out[2] = f2;
}

void drawStreet()
{
	glClearColor(0.0f, 0.4f, 0.5f, 0.0f);
	glClearDepth(1.0);
	glPushMatrix();
	glColor3f(0.2f, 0.2f, 0.2f);
	//float w = 5.0;
	//float d = 100.0;
	//float h = 0.0;
	glTranslatef(0.0f, 0.0f, 0.0f);
	glBegin(GL_QUADS);
	//top
	/*
	   glNormal3f( 0.0f, 1.0f, 0.0f);
	   glVertex3f( w, h,-d);
	   glVertex3f(-w, h,-d);
	   glVertex3f(-w, h, d);
	   glVertex3f( w, h, d);
	   glEnd();
	   glPopMatrix();
	   */
	//double yellow line
	glColor3f(0.8f, 0.8f, 0.2f);
	//w = 0.1;
	//d = 100.0;
	//h = 0.01;
	glPushMatrix();
	glTranslatef(-0.15f, 0.0f, 0.0f);
	glBegin(GL_QUADS);
	//top
	/*glNormal3f( 0.0f, 1.0f, 0.0f);
	  glVertex3f( w, h,-d);
	  glVertex3f(-w, h,-d);
	  glVertex3f(-w, h, d);
	  glVertex3f( w, h, d);
	  glEnd();
	  glPopMatrix();
	  glPushMatrix();
	  glTranslatef(0.15f, 0.0f, 0.0f);
	  glBegin(GL_QUADS);
	//top
	glNormal3f( 0.0f, 1.0f, 0.0f);
	glVertex3f( w, h,-d);
	glVertex3f(-w, h,-d);
	glVertex3f(-w, h, d);
	glVertex3f( w, h, d);
	glEnd();
	glPopMatrix();
	*/
	//guard rails
	glColor3f(1.0f, 1.0f, 1.0f);
	for (int i=0; i<40; i++) {
		glPushMatrix();
		glTranslatef(6.0f, -0.5f, (float)-i*2.5);
		box(0.2, 5.0, 0.2);
		glPopMatrix();
		glPushMatrix();
		glTranslatef(-6.0f, -0.5f, (float)-i*2.5);
		box(0.2, 5.0, 0.2);
		glPopMatrix();
	}
}

void physics()
{   
	//Makes camera go down the road
	//	g.cameraPosition[2] -= 0.1;
	//Makes camera sway left and right
	//	g.cameraPosition[0] = 1.0 + sin(g.cameraPosition[2]*0.3);


	//


	Vec dir;
	car_acceleration(false);

	if (car_acceleration(false) > 0) {
		dir[0] = cos(g.cameraAngle);
		dir[2] = sin(g.cameraAngle);
		dir[1] = 0.0;

		VecAdd(g.cameraPosition, dir, g.cameraPosition);
	}
	total_physics_function_calls(false);
}

void render()
{
	total_render_function_calls(false);
	Rect r;
	glClearColor(0.00f, 0.0f, 0.0f, 1.0f);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	//
	//3D mode
	glMatrixMode(GL_PROJECTION); glLoadIdentity();
	gluPerspective(45.0f, g.aspectRatio, 0.1f, 100.0f);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	//for documentation...
	Vec up = {0,1,0};
	//old
	/*
	   gluLookAt(
	   g.cameraPosition[0], g.cameraPosition[1], g.cameraPosition[2],
	   g.cameraPosition[0], g.cameraPosition[1], g.cameraPosition[2]-1.0,
	   up[0], up[1], up[2]);
	   */
	//new
	Vec spot;
	spot[0] = cos(g.cameraAngle);
	spot[2] = sin(g.cameraAngle);
	spot[1] = 0.0;

	VecAdd(spot, g.cameraPosition, spot);

	gluLookAt(
			g.cameraPosition[0], g.cameraPosition[1], g.cameraPosition[2],
			spot[0], spot[1], spot[2],
			up[0], up[1], up[2]);

	glLightfv(GL_LIGHT0, GL_POSITION, g.lightPosition);
	//
	if (gameState == MENU) {
		drawMenu(g.xres, g.yres, selectedOption);
	} else if (gameState == PLAY) {
		drawTrack();
		drawTerrain();
		drawTree3D();
		createImages();
		physics();
		drawScene();
		drawAzuritaCar(Pos1, Pos2, Pos3, theta);

	} else if (gameState == CONTROLS) {
		drawControls(g.xres, g.yres, selectedOption );
	}


	//
	//
	//
	//switch to 2D mode
	//
	glViewport(0, 0, g.xres, g.yres);
	glMatrixMode(GL_MODELVIEW);   glLoadIdentity();
	glMatrixMode (GL_PROJECTION); glLoadIdentity();
	gluOrtho2D(0, g.xres, 0, g.yres);
	glPushAttrib(GL_ENABLE_BIT);
	glDisable(GL_LIGHTING);
	//glDisable(GL_DEPTH_TEST);
	//glDisable(GL_CULL_FACE);
	r.bot = g.yres - 20;
	r.left = 10;
	r.center = 0;
	
	//Putting Stopwatch
	ggprint8b(&r, 16, 0x00887766, "Jerry Berry");
//	std::string text = "Time was: ";
//	text += std::to_string(timed);

//	ggprint8b(&r, 16, 0x00887766, text.c_str()); 
	
	if (show_stats) {
		ggprint13(&r, 16, 0x00ffff00, "sec running time %i",
				total_running_time(true));
		ggprint13(&r, 16, 0x00ffff00, "render calls: %i",
				total_render_function_calls(true));
		ggprint13(&r, 16, 0x00ffff00, "physics calls: %i",
				total_physics_function_calls(true));

		ggprint13(&r, 16, 0x00ffff00, "key presses: %i",
				key_presses(true));

		ggprint13(&r, 16, 0x00ffff00, "mouse distance: %f",
				//time_since_mouse_moved(false);
				mouseMovement(NULL, true));


	}   
	glPopAttrib();
}

bool pointInTriangle(float tri[3][3], int x, int y)
{
	//is point <x,y> inside triangle rn?
	//
	//return true - point x,y is inside the triangle
	//return false - not inside
	//
	float v0[3], v1[3], v2[3];
	int sum = 0;
	for (int i = 0; i < 3; i++) {
		int j = (i+1) % 3;
		//Build a vector from triangle vertex 0 to 1.
		// (subtract vertex0 from vertex 1)
		// Name it v0
		v0[0] = tri[j][0] - tri[i][0];
		v0[1] = tri[j][1] - tri[i][1];
		//2. Build a vector perpendicular to v0. Name it v1.
		v1[0] = v0[1];
		v1[1] = -v0[0];
		//3. Build a vector from triangle vertex 0 to your point
		// Name it v2.
		v2[0] = (Flt)x - tri[i][0];
		v2[1] = (Flt)y - tri[i][1];
		//4. Find the dot-product of v1, v2. Name it dot
		Flt dot = v1[0]*v2[0] + v1[1]*v2[1];
		//5. Add the sign of dot to a sum (negative 1 or positive 1)
		int sign = (dot >= 0.0) ? 1:-1;
		sum += sign;
		//6. Repeat from step 1 for the two other triangle sides
	}
	//7. If the absolute-value of sum is 3
	//the point is inside the triangle.
	return (abs(sum) == 3);
}


