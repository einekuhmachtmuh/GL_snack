#include <windows.h>

#include <stdio.h>	// printf
#include <math.h>	// floorf

#include <GL/gl.h>
#include <GL/glext.h>	// wglext.h when use wchar

#define cellx 35
#define celly 27

#define scale 5
#define rect (2 * scale)

#define Wndwidth ( 2 * scale * (cellx - 1) ) 
#define Wndheight ( 2 * scale * (celly - 1) ) 

#define interval_x (cellx - 1) 
#define interval_y (celly - 1) 

#define max_x (interval_x) / 2
#define max_y (interval_y) / 2

#define bound_x ( max_x + 1 ) 
#define bound_y ( max_y + 1 ) 



typedef struct _node{
	int pos_x;			// w/(2 * cell) as unit, easy to calculate.
	int pos_y;			// Beware, pos of rec center.
	GLubyte is_turn;	// avoid unnecessary collision test.
	struct _node* next;
	struct _node* pre;
} node;

// Windows, registeration -> resize windows rect -> create windows 
WNDCLASS wndClass;							// wndclass, descriptor of windows "format"
HWND hWnd = NULL;							// Handle of real windows
char *className = "OpenGL"; 				// wndclass identifier for registeration
char *windowName = "Snake"; 				// windows title
RECT primaryDisplaySize;					// get by SystemParametersInfo
RECT WndSize = { 0, 0, Wndwidth, Wndheight };			// left, top, right, bottom, { 0, 0, W, H }
DWORD WNDstyle =  WS_CAPTION | WS_SYSMENU | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;	// windows style

// OpenGL, hDc -> pixel format -> hGLRC
HDC hDC;					// Device context of hWnd
HGLRC hGLRC;				// OpenGL rendering context of hWnd
PIXELFORMATDESCRIPTOR pfd =	// price of choosing pixel formats
{
	sizeof(PIXELFORMATDESCRIPTOR),	// nSize;
	1,								// nVersion;
	PFD_SUPPORT_OPENGL |
	PFD_DRAW_TO_WINDOW |
	PFD_DOUBLEBUFFER,				// dwFlags;
	PFD_TYPE_RGBA,					// iPixelType;
	32,		// cColorBits; can get with GetDeviceCaps(hDC, BITSPIXEL);
	0, 0,	// cRedBits, cRedShift
	0, 0,	// cGreenBits, cGreenShift
	0, 0,	// cBlueBits, cBlueShift
	8, 0,	// cAlphaBits, cAlphaShift
	0,		// cAccumBits;
	0, 0, 0, 0,	// cAccumRedBits, cAccumGreenBits, cAccumBlueBits, cAccumAlphaBits
	24,	// cDepthBits;
	8,	// cStencilBits;
	0,	// cAuxBuffers;
	PFD_MAIN_PLANE,	// iLayerType;
	0,	// bReserved;
	0,	// dwLayerMask;
	0,	// dwVisibleMask;
	0,	// dwDamageMask;
};

// WM_TIMER
UINT idTimer = 1;	// timer ID
UINT Period = 102;	// time interval per frame in ms, no fps.

// color definition with {R, G, B}
GLfloat red[3] = {1, 0, 0};
GLfloat grey[3] = {0.588235f, 0.588235f, 0.588235f};
GLfloat white[3] = {1.0f, 1.0f, 1.0f};

//Game variables
float trans[2] = { 1 / ( (float)(max_x) + 0.5 ), 1 / ( (float)(max_y) + 0.5 )};
node snake[200];	// we believe no one can reach this score! 
node* head = NULL;
node* tail = NULL;
GLint snake_len = 3;

int berry[2] = { 31, 23 };

GLubyte dir_x = 0;	// 00 nothing, 01 negitive direction, 10 positive direction
GLubyte dir_y = 2;
GLuint turn = 0;	// avoid unnecessary collision test. 
GLuint pause = 0;
GLubyte lost = 0;

inline void
draw()		// just according to pos
{		
	// **Beware, draw with normalized coordinate, i.e. center of windows is ( 0, 0 ), and top right corner is ( 1, 1 ). (DIY "software" projection)
	
	node* temp = head->next;
	
	glPointSize( rect );	// in pixel
	glClear(GL_COLOR_BUFFER_BIT);	// blanking screen
	
	glBegin(GL_POINTS);	// head -> body -> berry

		glColor3fv( grey );
		glVertex2f( (float)( head->pos_x ) * trans[0] ,  (float)( head->pos_y ) * trans[1] );
		glColor3fv( white );
		do{
			glVertex2f( (float)( temp->pos_x ) * trans[0]  ,  (float)( temp->pos_y  ) * trans[1] );
			temp = temp->next;
		}while( temp );
		glColor3fv( red );
		glVertex2f( (float)( berry[0] ) * trans[0]  ,  (float)( berry[1] ) * trans[1] );
	
	glEnd();
	SwapBuffers(hDC);	//swap to screen
}

inline void
spawn_berry()	// gen berry pos
{
	float f1, f2;
	int i, j;
	
	do{
		f1 = (float)( rand() ) / (float) RAND_MAX;
		f2 = (float)( rand() ) / (float) RAND_MAX;
		
		i = floorf( f1 * interval_x );
		j = floorf( f2 * interval_y );
		
	}while( ( i == head->pos_x ) && ( j == head->pos_y ) );
	
	berry[0] = i - max_x;
	berry[1] = j - max_y;
}

inline void
game_init()	//init whole snake, turn number, lost, pause, dirction, and then spawn berry.
{
	snake[0].pos_x = 0;
	snake[0].pos_y = 2;
	snake[0].next = snake + 1;
	snake[0].pre = NULL;
	snake[0].is_turn = 0;
	
	snake[1].pos_x = 0;
	snake[1].pos_y = 0;
	snake[1].next = snake + 2;
	snake[1].pre = snake;
	snake[1].is_turn = 0;
	
	snake[2].pos_x = 0;
	snake[2].pos_y = - 2;
	snake[2].next = NULL;
	snake[2].pre = snake + 1;
	snake[2].is_turn = 0;
	
	head = snake;	
	tail = snake + 2;
	
	snake_len = 3;
	
	turn = 0;
	pause = 0;
	lost = 0; 
	
	GLubyte dir_x = 0;
	GLubyte dir_y = 2;
	
	spawn_berry();
}

inline void
game_update()	// collision test -> snake update -> draw
{
	
	int t_pos[2];
	node* t_node = head;
	
	t_pos[0] = head->pos_x + ((dir_x >> 1) - (dir_x & 1));
	
	t_pos[1] = head->pos_y + ((dir_y >> 1) - (dir_y & 1));
	
	if( abs( t_pos[0] ) == bound_x ){ // lost when reach left/right wall
		lost = 1;
		KillTimer(hWnd, idTimer);
		return;
	}
	
	if( abs( t_pos[1] ) == bound_y ){ // lost when reach top/bottom wall
		lost = 1;
		KillTimer(hWnd, idTimer);
		return;
	}
	
	
	if(turn > 2){ // lost as a "poly-rectangle"
		do{
			t_node = t_node->next;
			if( ( t_node->pos_x == head->pos_x  ) && ( t_node->pos_y == head->pos_y ) ){
				lost = 1;
				return;
			}
		}while(t_node->next);
		t_node = head;
	}
	
	
	if( ( t_pos[0] == berry[0] ) && ( t_pos[1] == berry[1] ) ){	// eat berry
		
		head = snake + snake_len;	//"alloc" new head
		snake_len ++;
		head->pos_x = t_pos[0];
		head->pos_y = t_pos[1];
		spawn_berry();
		if(snake_len < 41){	// level up
			KillTimer(hWnd, idTimer);
			Period -= 2;
			SetTimer(hWnd, idTimer, Period, NULL);
		}
		
	}else{	// just use popped node as new head
		
		if(tail->is_turn){
			turn--;
			tail->is_turn = 0;
		}
		head = tail;
		tail = tail->pre;
		tail-> next = NULL;
		head->pos_x = t_pos[0];
		head->pos_y = t_pos[1];	
	}
	
	t_node->pre = head;
	head->next = t_node;
	head->pre = NULL;
	
	draw();
	
}

LRESULT APIENTRY
WndProc(
	HWND hWnd,
	UINT message,
	WPARAM wParam,
	LPARAM lParam)
{
	switch (message) {
		case WM_CREATE:
			SetTimer(hWnd, idTimer, Period, NULL);
			return 0;
		case WM_TIMER:
			game_update();
			return 0;
		case WM_DESTROY:
			KillTimer(hWnd, idTimer);
			PostQuitMessage(0);
			return 0;
		case WM_PAINT:
		/*
		** Thanks to silly wnd, we need repaint the windows.
		*/
			if (hGLRC) {
				PAINTSTRUCT ps;
				BeginPaint(hWnd, &ps);
				EndPaint(hWnd, &ps);
				return 0;
			}
			return 0;
		case WM_CHAR:
			if (wParam == VK_SPACE) {
				if( !lost ){
					if( !pause ){
						KillTimer(hWnd, idTimer);
						pause = 1;
					} else{
						pause = 0;
						SetTimer(hWnd, idTimer, Period, NULL);
					}
				}
				return 0;
			}else if( (wParam == 'r' || wParam == 'R') && lost){
				game_init();
				SetTimer(hWnd, idTimer, 102, NULL);
				return 0;
			}
			return 0;
		case WM_KEYUP:
			if(pause){
				return 0;
			}		
			if (dir_x == 0 ){ 
				if (wParam == VK_RIGHT){
					dir_x = 2;
					dir_y = 0;
					turn ++;
					head->is_turn =1;
				} 
					
				if (wParam == VK_LEFT){
					dir_x = 1;
					dir_y = 0;
					turn ++;
					head->is_turn =1;
				} 
					
			} 
			if (dir_y == 0){ 
				if (wParam == VK_UP){
					dir_x = 0;
					dir_y = 2;
					turn ++;
					head->is_turn =1;
				}
					
				if (wParam == VK_DOWN){
					dir_x = 0;
					dir_y = 1;
					turn ++;
					head->is_turn =1;
				} 
					
			}
			return 0;
		default:
			break;
	}
	return DefWindowProc( hWnd, message, wParam, lParam );
}

int APIENTRY
WinMain(
	HINSTANCE hCurrentInst,
	HINSTANCE hPreviousInst,
	LPSTR lpszCmdLine,
	int nCmdShow)
{
	wndClass.style = CS_HREDRAW | CS_VREDRAW;
	wndClass.lpfnWndProc = WndProc;
	wndClass.cbClsExtra = 0;
	wndClass.cbWndExtra = 0;
	wndClass.hInstance = hCurrentInst;
	wndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndClass.hbrBackground = GetStockObject(WHITE_BRUSH);
	wndClass.lpszMenuName = NULL;
	wndClass.lpszClassName = className;
	
	if(!RegisterClass(&wndClass)){
		printf( "Failed to Register wndClass, Error no.0x%X\n", GetLastError() );
		return 0;
	}
	
	AdjustWindowRect( &WndSize, WNDstyle, 0 );
	SystemParametersInfo( SPI_GETWORKAREA, 0, &primaryDisplaySize, 0 );

	/* Create a window of the previously defined class */
	hWnd = CreateWindow(
	className,		/* Window class's name */
	windowName,		/* Title bar text */
	WNDstyle,
	(  primaryDisplaySize.right - primaryDisplaySize.left - WndSize.right + WndSize.left ) >> 1,
	(  primaryDisplaySize.bottom - primaryDisplaySize.top - WndSize.bottom + WndSize.top) >> 1,		/* Position of top */
	WndSize.right - WndSize.left, WndSize.bottom - WndSize.top,												/* Size */
	NULL,			/* Parent window's handle */
	NULL,			/* Menu handle */
	hCurrentInst,		/* Instance handle */
	NULL);			/* No additional data */

	if(!hWnd){
		printf( "Failed to create hWnd, Error no.0x%X\n", GetLastError() );
		return 0;	
	}

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	hDC = GetDC(hWnd);
	
	int SelectedPixelFormat =  ChoosePixelFormat(hDC, &pfd);
	
	if ( SelectedPixelFormat == 0 ) {
		printf( "Failed to choose pixel format, Error no.0x%X\n", GetLastError() );
		return 0;
	}

	if ( !SetPixelFormat(hDC, SelectedPixelFormat, &pfd) ) {
		printf( "Failed to select pixel format, Error no.0x%X\n", GetLastError() );
		return 0;
	}
	
	if ( DescribePixelFormat(hDC, SelectedPixelFormat, sizeof(PIXELFORMATDESCRIPTOR), &pfd) == 0 ){
		printf( "Failed to describe pixel format, Error no.0x%X\n", GetLastError() );
	} 
	
	
	hGLRC = wglCreateContext( hDC );
	wglMakeCurrent( hDC, hGLRC );
	
	glEnable( GL_PROGRAM_POINT_SIZE );
	game_init();
	
	MSG msg;

	while ( GetMessage(&msg, NULL, 0, 0) > 0 ) {
		TranslateMessage( &msg );
		DispatchMessage( &msg );
	}

	if ( hGLRC ) {
		wglMakeCurrent( NULL, NULL );
		wglDeleteContext( hGLRC );
	}
	ReleaseDC( hWnd, hDC );
	DestroyWindow( hWnd );
	UnregisterClass( className, hCurrentInst );

	return msg.wParam;
}
