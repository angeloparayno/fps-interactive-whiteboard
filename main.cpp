#pragma region Declarations

#define _CRT_SECURE_NO_WARNINGS

// STL Header
#include <string>
#include <iostream>
#include <stdio.h>
#include <Windows.h>
#include <fcntl.h>
#include <io.h>
#include <fstream>

//// OpenCV Header
#include <cv.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

// NiTE Header
#include <NiTE.h>
#include <OpenNI.h>

// namespace
using namespace std;
using namespace openni;
using namespace nite;
using namespace cv;

// Color Number + HSV Values
int cRed[7] = { 1,169,179,198,255,170,255 };
int cGreen[7] = {2,30,75,165,255,64,255};
int cBlue[7] = { 3,115,130,188,255,170,255 };
int cCyan[7] = { 4,75,115,100,255,147,241 };
int cYellow[7] = {5,0,15,100,250,131,255 };
int cMagenta[7] = { 6,138,171,232,255,188,255};

// userID variable for display
int uIDDisplay;
// desktop resolution
int desktopWidth;
int desktopHeight;
// max number of objects to be detected in frame
const int MAX_NUM_OBJECTS = 50;
// max number of users
const int MAX_USERS = 2;
const int MAX_TRACKED = 10;
// default capture width and height
const int FRAME_WIDTH = 640;
const int FRAME_HEIGHT = 480;
// minimum and maximum object area
const int MIN_OBJECT_AREA = 20 * 20;
const int MAX_OBJECT_AREA = FRAME_HEIGHT*FRAME_WIDTH / 1.5;
// clock time interval 
const double NUM_SECONDS = 5;
const double NUM_SECONDS2 = 1;
// get desktop window resolution
const HWND hDesktop = GetDesktopWindow();
// maximum mumber of lines the output console should have
static const WORD MAX_CONSOLE_LINES = 500;
// touch injection pointer contact
POINTER_TOUCH_INFO contact_[2];
// user han	d prefeference (default is right hand for both users)
bool rHand1 = true,rHand2 = true;
// cursor preference
bool curGDIShapes = false;
bool curGDIText = false;
bool curBitBlt = false;
// profileAlgo
bool profileAlgo = true;
// pop-up graphics text
int xPopText1 = 0;
int xPopText2 = 0;
int yPopText = 0;
HDC dcPopText = GetDC(NULL);
// test mode
bool testMode = false;

#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif

bool g_visibleUsers[MAX_TRACKED] = {false};
nite::SkeletonState g_skeletonStates[MAX_TRACKED] = {nite::SKELETON_NONE};
#pragma endregion

#pragma region Display User Number Text 
void displayUserNText(int curX,int curY,int user,int cursorLimitLeft,int cursorLimitTop)
{
	if(user == 0)
	{
		// Corner
		if(curX < 24 + cursorLimitLeft && curY < 45 + cursorLimitTop)	
			TextOut(dcPopText,(int)curX + 29,curY + 35,TEXT("USER 1"),6);
		// Max Left
		else if(curX < 24 + cursorLimitLeft)
			TextOut(dcPopText,(int)curX + 29,curY - 10,TEXT("USER 1"),6); 
		// Max Top
		else if(curY < 45 + cursorLimitTop)
			TextOut(dcPopText,(int)curX - 24,curY + 35,TEXT("USER 1"),6); 
		// Normal
		else
			TextOut(dcPopText,(int)curX - 24,curY - 45,TEXT("USER 1"),6); 
	}
	else if(user == 1)
	{
		// Corner
		if(curX < 24 + cursorLimitLeft && curY < 45 + + cursorLimitTop)	
			TextOut(dcPopText,(int)curX + 29,curY + 35,TEXT("USER 2"),6);
		// Max Left
		else if(curX < 24 + cursorLimitLeft)
			TextOut(dcPopText,(int)curX + 29,curY - 10,TEXT("USER 2"),6); 
		// Max Top
		else if(curY < 45 + + cursorLimitTop)
			TextOut(dcPopText,(int)curX - 24,curY + 35,TEXT("USER 2"),6); 
		// Normal
		else
			TextOut(dcPopText,(int)curX - 24,curY - 45,TEXT("USER 2"),6);	
	}		
}
#pragma endregion

#pragma region Display User State Function
void updateUserState(const nite::UserData& user,bool helpKeyPressed)
{
	if(!helpKeyPressed)
	{
		if(uIDDisplay == 1)
			TextOut(dcPopText,110,5,TEXT("USER 1"),6);
		else if(uIDDisplay == 2)
			TextOut(dcPopText,desktopWidth - 330,5,TEXT("USER 2"),6);
	}

	if (user.isNew())
	{	
		if(uIDDisplay == 1)
		{
			printf("User %d:\t New\n",uIDDisplay);
			TextOut(dcPopText,xPopText1,yPopText,TEXT(" - NEW                  "),24);
		}
		else if(uIDDisplay == 2)
		{
			printf("\t\t\t\tUser %d:\t New\n",uIDDisplay);
			TextOut(dcPopText,xPopText2,yPopText,TEXT(" - NEW                  "),24);
		}
	}
	else if (user.isVisible() && !g_visibleUsers[user.getId()])
	{
		if(uIDDisplay == 1)
		{ 
			printf("User %d:\t Visible\n",uIDDisplay);
			TextOut(dcPopText,xPopText1,yPopText,TEXT(" - VISIBLE              "),24);
		}
		else if(uIDDisplay == 2)
		{
			printf("\t\t\t\tUser %d:\t Visible\n",uIDDisplay);
			TextOut(dcPopText,xPopText2,yPopText,TEXT(" - VISIBLE		       "),24);
		}
	}
	else if (!user.isVisible() && g_visibleUsers[user.getId()])
	{
		if(uIDDisplay == 1)
		{
			printf("User %d:\t Out of Scene\n",uIDDisplay);
			TextOut(dcPopText,xPopText1,yPopText,TEXT(" - OUT OF SCENE	        "),24);
		}
		else if(uIDDisplay == 2)
		{
			printf("\t\t\t\tUser %d:\t Out of Scene\n",uIDDisplay);
			TextOut(dcPopText,xPopText2,yPopText,TEXT(" - OUT OF SCENE	        "),24);
		}
	}
	else if (user.isLost())
	{
		if(uIDDisplay == 1)
		{
			printf("User %d:\t Lost\n",uIDDisplay);
			TextOut(dcPopText,xPopText1,yPopText,TEXT(" - LOST                 "),24);
		}
		else if(uIDDisplay == 2)
		{
			printf("\t\t\t\tUser %d:\t Lost\n",uIDDisplay);
			TextOut(dcPopText,xPopText2,yPopText,TEXT(" - LOST                 "),24);
		}
	}

	g_visibleUsers[user.getId()] = user.isVisible();

	if(g_skeletonStates[user.getId()] != user.getSkeleton().getState())
	{
		switch(g_skeletonStates[user.getId()] = user.getSkeleton().getState())
		{
		case nite::SKELETON_NONE:
			if(uIDDisplay == 1)
			{
				("User %d:\t Stopped Tracking\n",uIDDisplay);
				TextOut(dcPopText,xPopText1,yPopText,TEXT(" - STOPPED TRACKING     "),24);
			}
			else if(uIDDisplay == 2)
			{
				printf("\t\t\t\tUser %d:\t Stopped Tracking\n",uIDDisplay);
				TextOut(dcPopText,xPopText2,yPopText,TEXT(" - STOPPED TRACKING     "),24);
			}
			break;
		case nite::SKELETON_CALIBRATING:
			if(uIDDisplay == 1)
			{
				("User %d:\t Calibrating...\n",uIDDisplay);
				TextOut(dcPopText,xPopText1,yPopText,TEXT(" - CALIBRATING...       "),24);
			}
			else if(uIDDisplay == 2)
			{
				printf("\t\t\t\tUser %d:\t Calibrating...\n",uIDDisplay);
				TextOut(dcPopText,xPopText2,yPopText,TEXT(" - CALIBRATING...       "),24);
			}
			break;
		case nite::SKELETON_TRACKED:
			if(uIDDisplay == 1)
			{
				("User %d:\t Tracking!\n",uIDDisplay);
				TextOut(dcPopText,xPopText1,yPopText,TEXT(" - TRACKING!            "),24);
			}
			else if(uIDDisplay == 2)
			{
				printf("\t\t\t\tUser %d:\t Tracking!\n",uIDDisplay);
				TextOut(dcPopText,xPopText2,yPopText,TEXT(" - TRACKING!            "),24);
			}
			break;
		case nite::SKELETON_CALIBRATION_ERROR_NOT_IN_POSE:
		case nite::SKELETON_CALIBRATION_ERROR_HANDS:
		case nite::SKELETON_CALIBRATION_ERROR_LEGS:
		case nite::SKELETON_CALIBRATION_ERROR_HEAD:
		case nite::SKELETON_CALIBRATION_ERROR_TORSO:
			if(uIDDisplay == 1)
			{
				("User %d:\t Calibration Failed...\n",uIDDisplay);
				TextOut(dcPopText,xPopText1,yPopText,TEXT(" - Calibration Failed..."),24);
			}
			else if(uIDDisplay == 2)
			{
				printf("\t\t\t\tUser %d:\t Calibration Failed...\n",uIDDisplay);
				TextOut(dcPopText,xPopText2,yPopText,TEXT(" - Calibration Failed..."),24);
			}
			break;
		}
	}
}
#pragma endregion

#pragma region Display Console Function
	void RedirectIOToConsole()
	{
	int hConHandle;
	long lStdHandle;
	CONSOLE_SCREEN_BUFFER_INFO coninfo;
	FILE *fp;

	// allocate a console for this app
	AllocConsole();

	// set the screen buffer to be big enough to let us scroll text
	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &coninfo);
	coninfo.dwSize.Y = MAX_CONSOLE_LINES;
	SetConsoleScreenBufferSize(GetStdHandle(STD_OUTPUT_HANDLE), coninfo.dwSize);

	// redirect unbuffered STDOUT to the console
	lStdHandle = (long)GetStdHandle(STD_OUTPUT_HANDLE);
	hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);

	fp = _fdopen(hConHandle, "w");
	*stdout = *fp;
	setvbuf(stdout, NULL, _IONBF, 0);

	// redirect unbuffered STDIN to the console

	lStdHandle = (long)GetStdHandle(STD_INPUT_HANDLE);
	hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);

	fp = _fdopen(hConHandle, "r");
	*stdin = *fp;
	setvbuf(stdin, NULL, _IONBF, 0);

	// redirect unbuffered STDERR to the console
	lStdHandle = (long)GetStdHandle(STD_ERROR_HANDLE);
	hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);

	fp = _fdopen(hConHandle, "w");

	*stderr = *fp;

	setvbuf(stderr, NULL, _IONBF, 0);

	// make cout, wcout, cin, wcin, wcerr, cerr, wclog and clog
	// point to console as well
	ios::sync_with_stdio();
	}
#pragma endregion

#pragma region OpenCV Functions
	void printColorFound(int color)
	{
	if (color == 1)
		cout << " Color RED is found " << endl;
	else if (color == 2)
		cout << " Color GREEN is found " << endl;
	else if (color == 3)
		cout << " Color BLUE is found " << endl;
	else if (color == 4)
		cout << " Color CYAN is found " << endl;
	else if (color == 5)
		cout << " Color YELLOW is found " << endl;
	else if (color == 6)
		cout << " Color MAGENTA is found " << endl;
	}

	bool trackFilteredObject(int color[], Mat hsv)
	{
	Mat imgThresholded;

	// detect certain color
	inRange(hsv, Scalar(color[1], color[3], color[5]), Scalar(color[2], color[4], color[6]), imgThresholded);

	// morphological opening (removes small objects from the foreground)
	erode(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));
	//dilate(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));

	// morphological closing (removes small holes from the foreground)
	//dilate(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));
	//erode(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));

	Mat temp;
	imgThresholded.copyTo(temp);

	// these two vectors needed for output of findContours
	vector< vector<Point> > contours;
	vector<Vec4i> hierarchy;
	// find contours of filtered image using openCV findContours function
	findContours(temp, contours, hierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE);

	// use moments method to find our filtered object
	double refArea = 0;
	bool objectFound = false;
	if (hierarchy.size() > 0)
	{
		int numObjects = hierarchy.size();
		// if number of objects greater than MAX_NUM_OBJECTS we have a noisy filter
		if (numObjects<MAX_NUM_OBJECTS)
		{
			for (int index = 0; index >= 0; index = hierarchy[index][0])
			{
				Moments moment = moments((cv::Mat)contours[index]);
				double area = moment.m00;

				/*
				if the area is less than 20 px by 20px then it is probably just noise
				if the area is the same as the 3/2 of the image size, probably just a bad filter
				we only want the object with the largest area so we safe a reference area each
				iteration and compare it to the area in the next iteration.
				*/
				if (area>MIN_OBJECT_AREA && area<MAX_OBJECT_AREA && area>refArea)
				{
					/*
					x = moment.m10/area;
					y = moment.m01/area;
					*/
					refArea = area;

					//color found, return color value
					printColorFound(color[0]);
					return true;
				}
			}
		}
	}
	//no color found, return value 0
	return false;
	}
#pragma endregion 

#pragma region TouchInjector Functions
void mousePressed(POINTER_TOUCH_INFO &contact, int x, int y) 
{
contact.pointerInfo.ptPixelLocation.x = x;
contact.pointerInfo.ptPixelLocation.y = y;

contact.rcContact.top = contact.pointerInfo.ptPixelLocation.y - 2;
contact.rcContact.bottom = contact.pointerInfo.ptPixelLocation.y + 2;
contact.rcContact.left = contact.pointerInfo.ptPixelLocation.x - 2;
contact.rcContact.right = contact.pointerInfo.ptPixelLocation.x + 2;

contact.pointerInfo.pointerFlags = POINTER_FLAG_INCONTACT | POINTER_FLAG_INRANGE | POINTER_FLAG_DOWN;
}

void mouseReleased(POINTER_TOUCH_INFO &contact, int x, int y) 
{
contact.pointerInfo.ptPixelLocation.x = x;
contact.pointerInfo.ptPixelLocation.y = y;

contact.rcContact.top = contact.pointerInfo.ptPixelLocation.y - 2;
contact.rcContact.bottom = contact.pointerInfo.ptPixelLocation.y + 2;
contact.rcContact.left = contact.pointerInfo.ptPixelLocation.x - 2;
contact.rcContact.right = contact.pointerInfo.ptPixelLocation.x + 2;

contact.pointerInfo.pointerFlags = POINTER_FLAG_INRANGE | POINTER_FLAG_UP;
}

void mouseMotion(POINTER_TOUCH_INFO &contact, int x, int y,bool isDragged) 
{
	
contact.pointerInfo.ptPixelLocation.x = x;
contact.pointerInfo.ptPixelLocation.y = y;

contact.rcContact.top = contact.pointerInfo.ptPixelLocation.y - 2;
contact.rcContact.bottom = contact.pointerInfo.ptPixelLocation.y + 2;
contact.rcContact.left = contact.pointerInfo.ptPixelLocation.x - 2;
contact.rcContact.right = contact.pointerInfo.ptPixelLocation.x + 2;

if(isDragged)
{
	contact.pointerInfo.pointerFlags = POINTER_FLAG_INCONTACT | POINTER_FLAG_INRANGE | POINTER_FLAG_UPDATE;
}
else
{
	contact.pointerInfo.pointerFlags = POINTER_FLAG_INRANGE | POINTER_FLAG_UPDATE;
}
}

#pragma endregion

#pragma region Prompt Menu
bool promptMenu()
{
	#pragma region Diffused RGB LED HSV Values
	int c1Red[7] = { 1,169,179,198,255,170,255 };
	int c1Green[7] = {2,30,75,165,255,64,255};
	int c1Blue[7] = { 3,115,130,188,255,170,255 };
	int c1Cyan[7] = { 4,75,115,100,255,147,241 };
	int c1Yellow[7] = {5,0,15,100,250,131,255 };
	int c1Magenta[7] = { 6,138,171,232,255,188,255};
	#pragma endregion
	#pragma region Lenovo A500 Display HSV Values
	int c2Red[7] = { 1,161,179,250,255,209,255 };
	int c2Green[7] = { 2,40,65,75,150,248,255 };
	int c2Blue[7] = { 3,107,141,240,255,252,255 };
	int c2Cyan[7] = { 4,72,92,15,98,235,255 };
	int c2Yellow[7] = { 5,28,30,56,145,250,255 };
	int c2Magenta[7] = { 6,143,157,201,255,28,255 };
	#pragma endregion

	char cMenu = 'x', cHand = 'x', cDevice = 'x', cCursor = 'x';
	String msgUser1 = "User 1 = Right Hand";
	String msgUser2 = "User 2 = Right Hand";
	String msgDevice = "Diffused RGB LED";
	String msgCursor = "Windows 8 Touch Cursor";

	cout << "[A] Start Tracking" << endl;
	cout << "[B] Hand Tracking Preference (" << msgUser1 << " | " << msgUser2 << ")"  << endl;
	cout << "[C] Color Source Preference (" << msgDevice << ")" << endl;
	cout << "[D] Cursor Preference (" << msgCursor << ")" << endl;
	cout << "[E] Speed Test Mode" << endl;
	cout << "[F] Exit" << endl;
	do
	{
		cout << "Input:";
		cin >> cMenu;
		switch(cMenu)
		{
			case 'A':
			case 'a':
				cout << endl;
				cout << "-------------------------------------------------------------------"<< endl;
				cout << "		WELCOME TO FPS INTERACTIVE WHITEBOARD						"<< endl;
				cout << "-------------------------------------------------------------------"<< endl;
				cout << "UTILITY KEYS"														 << endl;
				cout << "[ P ] to open Paint program"										 << endl;
				cout << "[ H ] to open Help graphics"										 << endl;
				cout << "[ L ] to provide locked border for the cursor area"				 << endl;
				cout << "Note: Press arrow keys to adjust the border (up/down)"				 << endl;
				cout << "[SPC] to close all utilities(paint / help graphics / locked border)"<< endl;
				cout << "-------------------------------------------------------------------"<< endl;
				cout << "HELP KEYS"															 << endl;
				cout << "[ 1 ] to reset User 1 Cursor Position"								 << endl;
				cout << "[ 2 ] to reset User 1 Cursor Position"								 << endl;
				cout << "[ESC] to exit the running program"									 << endl;
				cout 																		 << endl;

				printf("Desktop Resoulution: %d x %d \n",desktopWidth,desktopHeight);
				cout << "Starting.." << endl;
				return true;
				break;
			case 'B':
			case 'b':
				cout << "[A] User 1 - Right Hand" << endl;
				cout << "[B] User 1 - Left Hand" << endl;
				cout << "[C] User 2 - Right Hand" << endl;
				cout << "[D] user 2 - Left Hand" << endl;
				cout << "[E] Cancel" << endl;
				do
				{
					cout << "input:";
					cin >> cHand;
					switch(cHand)
					{
						case 'A':
						case 'a':
							msgUser1 = "User 1 = Right Hand";
							rHand1 = true;
							break;
						case 'B':
						case 'b':
							msgUser1 = "User 1 = Left Hand";
							rHand1 = false;
							break;
						case 'C':
						case 'c':
							msgUser2 = "User 2 = Right Hand";
							rHand2 = true;
							break;
						case 'D':
						case 'd':
							msgUser2 = "User 2 = Left Hand";
							rHand2 = false;
							break;
						case 'E':
						case 'e':
							break;
						default:
							cHand = 'x';
							cout << "Invalid input, please try again!" << endl;
							break;
					}
				}
				while(cHand == 'x');
				cout << "[A] Start Tracking" << endl;
				cout << "[B] Hand Tracking Preference (" << msgUser1 << " | " << msgUser2 << ")"  << endl;
				cout << "[C] Color Source Preference (" << msgDevice << ")" << endl;
				cout << "[D] Cursor Preference (" << msgCursor << ")" << endl;
				cout << "[E] Speed Test Mode" << endl;
				cout << "[F] Exit" << endl;
				cHand = 'x';
				break;
			case 'C':
			case 'c':
				cout << "[A] Diffused RGB LED" << endl;
				cout << "[B] Mobile Device Display" << endl;
				cout << "[C] Cancel" << endl;
				do
				{
					cout << "input:";
					cin >> cDevice;
					switch(cDevice)
					{
						case 'A':
						case 'a':
							for(int i = 0; i < 7; i++)
							{
								cRed[i] = c1Red[i];
								cGreen[i] = c1Green[i];
								cBlue[i] = c1Blue[i];
								cCyan[i] = c1Cyan[i];
								cYellow[i] = c1Yellow[i];
								cMagenta[i] = c1Magenta[i];
							}
							msgDevice = "Diffused RGB LED";
							break;
						case 'B':
						case 'b':
							for(int i = 0; i < 7; i++)
							{
								cRed[i] = c2Red[i];
								cGreen[i] = c2Green[i];
								cBlue[i] = c2Blue[i];
								cCyan[i] = c2Cyan[i];
								cYellow[i] = c2Yellow[i];
								cMagenta[i] = c2Magenta[i];
							}
							msgDevice = "Mobile Device Display";
							break;
						case 'C':
						case 'c':
							break;
						default:
							cDevice = 'x';
							cout << "Invalid input, please try again!" << endl;
							break;
					}
				}
				while(cDevice == 'x');
				cout << "[A] Start Tracking" << endl;
				cout << "[B] Hand Tracking Preference (" << msgUser1 << " | " << msgUser2 << ")"  << endl;
				cout << "[C] Color Source Preference (" << msgDevice << ")" << endl;
				cout << "[D] Cursor Preference (" << msgCursor << ")" << endl;
				cout << "[E] Speed Test Mode" << endl;
				cout << "[F] Exit" << endl;
				cDevice = 'x';
				break;
			case 'D':
			case 'd':
				cout << "[A] Windows 8 Touch Cursor" << endl;
			cout << "-----------------------------------------------"	<< endl;
			cout << "[B] GDI Shapes only"								<< endl;						
			cout << "[C] GDI Text only"									<< endl;
			cout << "-----------------------------------------------"	<< endl;
			cout << "[D] GDI Shapes Cursor + BitBlt Function"			<< endl;
			cout << "[E] GDI Text Cursor   + BitBlt Function"			<< endl;
			cout << "-----------------------------------------------"	<< endl;
			cout << "[F] GDI Shapes Cursor + Windows 8 Touch Cursor"	<< endl;
			cout << "[G] GDI Text Cursor   + Windows 8 Touch Cursor"	<< endl;
			cout << "-----------------------------------------------"	<< endl;
			cout << "[H] GDI Shapes Cur + W8 Touch Cur + BitBlt Func"	<< endl;
			cout << "[I] GDI Text Cur   + W8 Touch Cur + BltBlt Func"	<< endl;
			cout << "-----------------------------------------------"	<< endl;
			cout << "[J] Cancel"										<< endl;
			do
			{
				cout << "input:";
				cin >> cCursor;
				switch(cCursor)
				{
					case 'A':
					case 'a':
						msgCursor = "Windows 8 Touch Cursor";
						InitializeTouchInjection(2, TOUCH_FEEDBACK_INDIRECT);
						curGDIShapes = false;
						curGDIText = false;
						curBitBlt = false;
						break;
					case 'B':
					case 'b':
						msgCursor = "GDI Shapes only";
						InitializeTouchInjection(2, TOUCH_FEEDBACK_NONE);
						curGDIShapes = true;
						curGDIText = false;
						curBitBlt = false;
						break;
					case 'C':
					case 'c':
						msgCursor = "GDI Text only";
						InitializeTouchInjection(2, TOUCH_FEEDBACK_NONE);
						curGDIShapes = false;
						curGDIText = true;
						curBitBlt = false;
						break;
					case 'D':
					case 'd':
						msgCursor = "GDI Shapes + BitBtl Function";
						InitializeTouchInjection(2, TOUCH_FEEDBACK_NONE);
						curGDIShapes = true;
						curGDIText = false;
						curBitBlt = true;
						break;
					case 'E':
					case 'e':
						msgCursor = "GDI Text + BitBtl Function";
						InitializeTouchInjection(2, TOUCH_FEEDBACK_NONE);
						curGDIShapes = false;
						curGDIText = true;
						curBitBlt = true;
						break;
					case 'F':
					case 'f':
						msgCursor = "GDI Shapes Cursor + Windows 8 Touch Cursor";
						InitializeTouchInjection(2, TOUCH_FEEDBACK_INDIRECT);
						curGDIShapes = true;
						curGDIText = false;
						curBitBlt = false;
						break;
					case 'G':
					case 'g':
						msgCursor = "GDI Text Cursor + Windows 8 Touch Cursor";
						InitializeTouchInjection(2, TOUCH_FEEDBACK_INDIRECT);
						curGDIShapes = false;
						curGDIText = true;
						curBitBlt = false;
						break;
					case 'H':
					case 'h':
						msgCursor = "GDI Shapes Cur + W8 Touch Cur + BitBlt Func";
						InitializeTouchInjection(2, TOUCH_FEEDBACK_INDIRECT);
						curGDIShapes = true;
						curGDIText = false;
						curBitBlt = true;
						break;
					case 'I':
					case 'i':
						msgCursor = "GDI Text Cur + W8 Touch Cur + BltBlt Func";
						InitializeTouchInjection(2, TOUCH_FEEDBACK_INDIRECT);
						curGDIShapes = false;
						curGDIText = true;
						curBitBlt = true;
						break;
					case 'J':
					case 'j':
						break;
					default:
						cCursor = 'x';
						cout << "Invalid input, please try again!" << endl;
						break;
				}
			}
			while(cCursor == 'x');
			cout << "[A] Start Tracking" << endl;
			cout << "[B] Hand Tracking Preference (" << msgUser1 << " | " << msgUser2 << ")"  << endl;
			cout << "[C] Color Source Preference (" << msgDevice << ")" << endl;
			cout << "[D] Cursor Preference (" << msgCursor << ")" << endl;
			cout << "[E] Speed Test Mode" << endl;
			cout << "[F] Exit" << endl;
			cCursor = 'x';
			break;
			case 'E':
			case 'e':
				cout << "\nSpeed Test Mode..\n" << msgCursor << endl << endl;
				testMode = true;
				return true;
				break;
			case 'F':
			case 'f':
				cout << "..Exiting the program" << endl;
				return false;
				break;
			default:
				cout << "Invalid input, please try again!" << endl;
				break;
		}
	}
	while(cMenu != 'A' && cMenu != 'a');
}
#pragma endregion 

//-----------------------------------MAIN-----------------------------------//
int WINAPI WinMain
(
	_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPSTR lpCmdLine,
	_In_ int nShowCmd
)
{
#pragma region Initialization
	// OpenCV Video Stream
	cv::Mat mImageBGR;
	cv::Mat mImageHSV;
	//  Desktop Resolution
	RECT desktopResolution;
	GetWindowRect(hDesktop, &desktopResolution);

	// Cursor Speed
	int cursorSpeedDivisor = 3;
	// Confidence Level
	float nConfidence = .5;
	// Resolution Ratio
	double rRatio = 0.17;

	// TIMERS
	double time_counter = 0;
	clock_t this_time = clock();
	clock_t last_time = this_time;

	// TEST TIMER
	double time_counterT = 0;
	clock_t this_timeT = clock();
	clock_t last_timeT = this_timeT;
	int iterationCounter = 0;
	double loopCounter = 0;
	double loopAverage = 0;
	double speedAverage = 0;

	// DISPLAY USER TEXT
	double time_counter0 = 0;
	clock_t this_time0 = clock();
	clock_t last_time0 = this_time0;
	double time_counter1 = 0;
	clock_t this_time1 = clock();
	clock_t last_time1 = this_time1;
	bool displayUserNTextLock0 = false;
	bool displayUserNTextLock1 = false;

	// user profile variable (0 = user1, 1 = user2)
	int n[2] = {0,1};
	// Cursor Coordinates Reference
	float startPosX[2] = { -1,-1 };
	float startPosY[2] = { -1,-1 };
	// color track counter
	bool wasDrag1Tracked = false;
	bool wasDrag2Tracked = false;
	// Stream Capturing Perspective
	bool bMirror = false;
	// Dragged Cursor Status
	bool isDragged[2] = {false,false};
	// User slot status
	bool isUserSlot1Occupied = false;
	bool isUserSlot2Occupied = false;
	// Cursor Coordinates
	int curX[2] = {desktopResolution.right * 1/4, desktopResolution.right * 3/4};
	int curY[2] = {desktopResolution.bottom/2, desktopResolution.bottom/2};
	// Cursor Limitations
	int cursorLimitLeft = desktopResolution.left + 10;
	int cursorLimitRight = desktopResolution.right - 10;
	int cursorLimitTop = desktopResolution.top + 10;
	int cursorLimitBottom = desktopResolution.bottom - 10;
	// COLOR INDICATORS
	int	indicatorBoxSize = 70;
	int indicatorGap = 5;
	int indicatorTotalSize = indicatorBoxSize + indicatorGap;
	// INSTRUCTIONS
	int instructionGap = 10;
	int instructionLabel = 20;
	int instructionBorderSize = 2;
	int instructionBoxSize = 50;
	int instructionPosition = (int) desktopResolution.bottom * rRatio;
	// BORDER COUNTER
	int gapSizeUp = 100;
	int gapSizeDown = 100;
	// BORDER
	int borderTopMax = desktopResolution.bottom/2 - gapSizeUp;
	int borderTopLimit = (int) desktopResolution.bottom * rRatio;
	int borderBottomMax = desktopResolution.bottom/2 + gapSizeDown;
	int borderBottomLimit = desktopResolution.bottom - indicatorTotalSize - 10;
	int borderGap = indicatorGap + 3;
	// BORDER PEN SIZE
	int screenBorderSize = 6;
	// Keyboard Counter
	bool paintKeyPressed = false;
	bool lockKeyPressed = false;
	bool updateLock = false;
	bool helpKeyPressed = false;

#pragma region Auto Adjust Resolution
	if(desktopResolution.right == 1920)
		rRatio = 0.14;
	else if(desktopResolution.right == 1366)
		rRatio = 0.17;
	else
		rRatio = 0.17;
	
	borderTopLimit = (int) desktopResolution.bottom * rRatio;
#pragma endregion

#pragma region Pop Text Coordinates
	xPopText1 = 160;
	xPopText2 = desktopResolution.right - 280;
	yPopText = 5;

	//yPopText = instructionPosition - instructionBoxSize - instructionGap - instructionLabel;
	//xPopText1 = borderGap + 80;
	//xPopText2 = desktopResolution.right - borderGap - 300 + 99;
#pragma endregion

#pragma region Graphics
HDC dcRed = GetDC(NULL);
HDC dcGreen = GetDC(NULL);
HDC dcBlue = GetDC(NULL);
HDC dcCyan = GetDC(NULL);
HDC dcYellow = GetDC(NULL);
HDC dcMagenta = GetDC(NULL);
HDC dcText = GetDC(NULL);

HBRUSH hRedBrush = CreateSolidBrush(RGB(224,0,0));
HBRUSH hGreenBrush = CreateSolidBrush(RGB(0,224,0));
HBRUSH hBlueBrush = CreateSolidBrush(RGB(0,0,224));
HBRUSH hCyanBrush = CreateSolidBrush(RGB(0,224,224));
HBRUSH hYellowBrush = CreateSolidBrush(RGB(224,224,0));
HBRUSH hMagentaBrush = CreateSolidBrush(RGB(224,0,224));

HPEN hGrayPen = CreatePen(PS_SOLID, instructionBorderSize, RGB(64, 64,  64));

// Instruction Graphics
SelectObject(dcRed,hRedBrush);
SelectObject(dcGreen,hGreenBrush);
SelectObject(dcBlue,hBlueBrush);
SelectObject(dcCyan,hCyanBrush);
SelectObject(dcYellow,hYellowBrush);
SelectObject(dcMagenta,hMagentaBrush);

SelectObject(dcRed,hGrayPen);
SelectObject(dcGreen,hGrayPen);
SelectObject(dcBlue,hGrayPen);
SelectObject(dcCyan,hGrayPen);
SelectObject(dcYellow,hGrayPen);
SelectObject(dcMagenta,hGrayPen);
	
// Border and Cover Graphics	
HDC dcBorder = GetDC(NULL);
HDC dcCover = GetDC(NULL);

HPEN hBorderPen = CreatePen(PS_SOLID,  screenBorderSize, RGB(64, 64, 64));
HPEN hCoverPen = CreatePen(PS_SOLID,  screenBorderSize, RGB(192, 192, 192));

SelectObject(dcBorder,hBorderPen);
SelectObject(dcCover,hCoverPen);
#pragma endregion 

#pragma region Touch Injector
InitializeTouchInjection(2, TOUCH_FEEDBACK_INDIRECT);

for (int i = 0; i < 2; i++)
{
	POINTER_TOUCH_INFO &contact = contact_[i];
	memset(&contact, 0, sizeof(POINTER_TOUCH_INFO));
	contact.pointerInfo.pointerType = PT_TOUCH;
	contact.pointerInfo.pointerId = i;         
	contact.pointerInfo.ptPixelLocation.y = 0;
	contact.pointerInfo.ptPixelLocation.x = 0;

	contact.touchFlags = TOUCH_FLAG_NONE;
	contact.touchMask = TOUCH_MASK_CONTACTAREA | TOUCH_MASK_ORIENTATION | TOUCH_MASK_PRESSURE;
	contact.orientation = 90; 
	contact.pressure = 32000;

	// defining contact area (I have taken area of 4 x 4 pixel)
	contact.rcContact.top = contact.pointerInfo.ptPixelLocation.y - 2;
	contact.rcContact.bottom = contact.pointerInfo.ptPixelLocation.y + 2;
	contact.rcContact.left = contact.pointerInfo.ptPixelLocation.x - 2;
	contact.rcContact.right = contact.pointerInfo.ptPixelLocation.x + 2;

	contact.pointerInfo.pointerFlags = POINTER_FLAG_UPDATE | POINTER_FLAG_INRANGE;//| POINTER_FLAG_INCONTACT;
}
#pragma endregion

#pragma region OpenNI Part
// Initial OpenNI
if (OpenNI::initialize() != openni::STATUS_OK)
{
	cerr << "OpenNI Initial Error: " << OpenNI::getExtendedError() << endl;
	MessageBox(NULL,(LPCWSTR)L"Please connect/reconnect Kinect device!",(LPCWSTR)L"FPS Interactive Whiteboard",MB_ICONERROR | MB_OK | MB_DEFBUTTON1);
	return -1;
}
// Open Device
Device	devDevice;
if (devDevice.open(ANY_DEVICE) != openni::STATUS_OK)
{
	cerr << "Can't Open Device: " << OpenNI::getExtendedError() << endl;
	MessageBox(NULL,(LPCWSTR)L"Please connect/reconnect Kinect device!",(LPCWSTR)L"FPS Interactive Whiteboard",MB_ICONERROR | MB_OK | MB_DEFBUTTON1);
	return -1;
}
// create depth stream
VideoStream vsDepthStream;
if (vsDepthStream.create(devDevice, SENSOR_DEPTH) == openni::STATUS_OK)
{
	// set video mode
	VideoMode mMode;
	mMode.setResolution(640,480);
	mMode.setFps(30);
	mMode.setPixelFormat(PIXEL_FORMAT_DEPTH_1_MM);
	if (vsDepthStream.setVideoMode(mMode) != openni::STATUS_OK)
	{
		cout << "Can't apply VideoMode: " << OpenNI::getExtendedError() << endl;
	}
	vsDepthStream.setMirroringEnabled(bMirror);
}

// Create color stream
VideoStream vsColorStream;
if (vsColorStream.create(devDevice, SENSOR_COLOR) == openni::STATUS_OK)
{
	// set video mode
	VideoMode mMode;
	mMode.setResolution(FRAME_WIDTH, FRAME_HEIGHT);
	mMode.setFps(30);
	mMode.setPixelFormat(PIXEL_FORMAT_RGB888);

	if (vsColorStream.setVideoMode(mMode) != openni::STATUS_OK)
	{
		cout << "Can't apply VideoMode: " << OpenNI::getExtendedError() << endl;
	}

	// image registration
	if (devDevice.isImageRegistrationModeSupported(IMAGE_REGISTRATION_DEPTH_TO_COLOR))
	{
		devDevice.setImageRegistrationMode(IMAGE_REGISTRATION_DEPTH_TO_COLOR);
	}
	
	vsColorStream.setMirroringEnabled(bMirror);
}
else
{
	cerr << "Can't create color stream on device: " << OpenNI::getExtendedError() << endl;
	MessageBox(NULL,(LPCWSTR)L"Please connect/reconnect Kinect device!",(LPCWSTR)L"FPS Interactive Whiteboard",MB_ICONERROR | MB_OK | MB_DEFBUTTON1);
	return -1;
}
#pragma endregion

#pragma region NiTE Part
// Initial NiTE
if (NiTE::initialize() != nite::STATUS_OK)
{
	cerr << "NiTE initial error" << endl;
	MessageBox(NULL,(LPCWSTR)L"Please connect/reconnect Kinect device!",(LPCWSTR)L"FPS Interactive Whiteboard",MB_ICONERROR | MB_OK | MB_DEFBUTTON1);
	return -1;
}
// create user tracker
UserTracker mUserTracker;
if (mUserTracker.create(&devDevice) != nite::STATUS_OK)
{
	cerr << "Can't create user tracker" << endl;
	MessageBox(NULL,(LPCWSTR)L"Please connect/reconnect Kinect device!",(LPCWSTR)L"FPS Interactive Whiteboard",MB_ICONERROR | MB_OK | MB_DEFBUTTON1);
	return -1;
}

#pragma endregion

#pragma endregion
	//-----------------------------------START-----------------------------------//
	RedirectIOToConsole();

	vsColorStream.start();
	vsDepthStream.start();

	desktopWidth = desktopResolution.right;
	desktopHeight = desktopResolution.bottom;

	if(!promptMenu())
		goto end;

	nite::Status status = nite::STATUS_OK;
	//-----------------------------------LOOP START-----------------------------------//
	while (true)
	{
		if(testMode)
		{
			this_timeT = clock();
			time_counterT += (double)(this_timeT - last_timeT);
			last_timeT = this_timeT;
			loopCounter ++;
		}

		mouseMotion(contact_[0],curX[0],curY[0],isDragged[0]);
		mouseMotion(contact_[1],curX[1],curY[1],isDragged[1]);
		InjectTouchInput(2, contact_);

	#pragma region Display User N Text Clock Timer
	if(displayUserNTextLock0)
	{
		this_time0 = clock();
		time_counter0 += (double)(this_time0 - last_time0);
		last_time0 = this_time0;

		if(time_counter0 > (double)(NUM_SECONDS2 * CLOCKS_PER_SEC))
		{
			time_counter0 = 0;
			displayUserNTextLock0 = false;
		}
	}
	if(displayUserNTextLock1)
	{
		this_time1 = clock();
		time_counter1 += (double)(this_time1 - last_time1);
		last_time1 = this_time1;

		if(time_counter1 > (double)(NUM_SECONDS2 * CLOCKS_PER_SEC))
		{
			time_counter1 = 0;
			displayUserNTextLock1 = false;
		}
	}
	#pragma endregion

	#pragma region Lock Key Clock Timer
	if(lockKeyPressed)
	{
		this_time = clock();
		time_counter += (double)(this_time - last_time);
		last_time = this_time;

		if(time_counter > (double)(NUM_SECONDS * CLOCKS_PER_SEC) || updateLock)
		{
			time_counter -= (double)(NUM_SECONDS * CLOCKS_PER_SEC);

			MoveToEx(dcBorder,borderGap,borderTopMax, NULL);	
			LineTo(dcBorder, desktopResolution.right - borderGap, borderTopMax);
			LineTo(dcBorder, desktopResolution.right - borderGap, borderBottomMax);
			LineTo(dcBorder,borderGap,borderBottomMax);
			LineTo(dcBorder,borderGap,borderTopMax);

			cursorLimitLeft = borderGap + 30;
			cursorLimitRight = desktopResolution.right - borderGap - 30;
			cursorLimitTop = borderTopMax + 30;
			cursorLimitBottom = borderBottomMax -30;

			updateLock = false;
		}
	}
	#pragma endregion 
		
		// get user frame
		UserTrackerFrameRef	mUserFrame;
		if (mUserTracker.readFrame(&mUserFrame) == nite::STATUS_OK)
		{
			// get color frame
			VideoFrameRef vfColorFrame;
			if (vsColorStream.readFrame(&vfColorFrame) == openni::STATUS_OK)
			{
				// convert data to OpenCV format
				const cv::Mat mImageRGB(vfColorFrame.getHeight(), vfColorFrame.getWidth(), CV_8UC3, const_cast<void*>(vfColorFrame.getData()));
				// convert form RGB to BGR
				cv::cvtColor(mImageRGB, mImageBGR, CV_RGB2BGR);
				//Convert the captured frame from BGR to HSV
				cvtColor(mImageBGR, mImageHSV, COLOR_BGR2HSV);

			//-----------------------------------COLOR TRACKING-----------------------------------//
			#pragma region Color Tracking Part
			// First User (RGB)
			if(trackFilteredObject(cRed, mImageHSV))
			{
				Rectangle(dcRed,indicatorGap,desktopResolution.bottom - indicatorGap, indicatorTotalSize,desktopResolution.bottom -  indicatorTotalSize);

				if(wasDrag1Tracked)
				{
					mouseReleased(contact_[0], curX[0], curY[0]);
					InjectTouchInput(2, contact_);
					Sleep(20);
				}
				mousePressed(contact_[0], curX[0], curY[0]);
				InjectTouchInput(2, contact_);
				Sleep(20);
				mouseReleased(contact_[0], curX[0], curY[0]);
				InjectTouchInput(2, contact_);
				Sleep(20);

				isDragged[0] = false;
				wasDrag1Tracked = false;
					
			}
			else if(trackFilteredObject(cBlue, mImageHSV))
			{
				Rectangle(dcBlue,indicatorGap,desktopResolution.bottom - indicatorGap, indicatorTotalSize,desktopResolution.bottom -  indicatorTotalSize);
				if(!wasDrag1Tracked)
				{
					mousePressed(contact_[0], curX[0], curY[0]);
					InjectTouchInput(2, contact_);
					Sleep(20);

					isDragged[0] = true;
					wasDrag1Tracked = true;
				}
			}
			else if(trackFilteredObject(cGreen, mImageHSV))
			{
				Rectangle(dcGreen,indicatorGap,desktopResolution.bottom - indicatorGap, indicatorTotalSize,desktopResolution.bottom -  indicatorTotalSize);
				if(!displayUserNTextLock0)
				{
					displayUserNText(curX[0],curY[0],0,cursorLimitLeft,cursorLimitTop);
					displayUserNTextLock0 = true;
				}
				if(wasDrag1Tracked)
				{
					mouseReleased(contact_[0], curX[0], curY[0]);
					InjectTouchInput(2, contact_);
					Sleep(20);
				}
				isDragged[0] = false;
				wasDrag1Tracked = false;
			}

			// Second User (CYM)
			if(trackFilteredObject(cYellow, mImageHSV))
			{
				Rectangle(dcYellow,desktopResolution.right - indicatorGap,desktopResolution.bottom - indicatorGap ,desktopResolution.right -  indicatorTotalSize,desktopResolution.bottom -  indicatorTotalSize);
				if(wasDrag2Tracked)
				{
					mouseReleased(contact_[1], curX[1], curY[1]);
					InjectTouchInput(2, contact_);
					Sleep(20);
				}
				mousePressed(contact_[1], curX[1], curY[1]);
				InjectTouchInput(2, contact_);
				Sleep(20);
				mouseReleased(contact_[1], curX[1], curY[1]);
				InjectTouchInput(2, contact_);
				Sleep(20);

				wasDrag2Tracked = false;
				isDragged[1] = false;
			}
			else if(trackFilteredObject(cMagenta, mImageHSV))
			{
				Rectangle(dcMagenta,desktopResolution.right - indicatorGap,desktopResolution.bottom - indicatorGap ,desktopResolution.right -  indicatorTotalSize,desktopResolution.bottom -  indicatorTotalSize);
				if(!wasDrag2Tracked)
				{
					mousePressed(contact_[1], curX[1], curY[1]);
					InjectTouchInput(2, contact_);
					Sleep(20);

					isDragged[1] = true;
					wasDrag2Tracked = true;
				}
			}
			else if(trackFilteredObject(cCyan, mImageHSV))
			{	
				Rectangle(dcCyan,desktopResolution.right - indicatorGap,desktopResolution.bottom - indicatorGap ,desktopResolution.right -  indicatorTotalSize,desktopResolution.bottom -  indicatorTotalSize);
				if(!displayUserNTextLock1)
				{
					displayUserNText(curX[1],curY[1],1,cursorLimitLeft,cursorLimitTop);
					displayUserNTextLock1 = true;
				}
				if(wasDrag2Tracked)
				{
					mouseReleased(contact_[1], curX[1], curY[1]);
					InjectTouchInput(2, contact_);
					Sleep(20);
				}
				isDragged[1] = false;
				wasDrag2Tracked = false;
			}
			#pragma endregion
				
				vfColorFrame.release();
			}
			else
			{
				cerr << "Can't get color frame" << endl;
			}

			mouseMotion(contact_[0],curX[0],curY[0],isDragged[0]);
			mouseMotion(contact_[1],curX[1],curY[1],isDragged[1]);
			//-----------------------------------TRACK JOINTS and CURSOR-----------------------------------//
			#pragma region Track Joints and Cursor Part

			const nite::Array<nite::UserData>& mUsers = mUserFrame.getUsers();
			
			for (int i = 0; i < mUsers.getSize() && i < MAX_USERS; ++i)
			{
				const nite::UserData& user = mUsers[i];
				const nite::SkeletonJoint& nHandR = user.getSkeleton().getJoint(nite::JOINT_RIGHT_HAND);
				const nite::SkeletonJoint& nHandL = user.getSkeleton().getJoint(nite::JOINT_LEFT_HAND);

				uIDDisplay = n[i] + 1;
				updateUserState(user,helpKeyPressed);

				if (user.isNew())
				{
					mUserTracker.startSkeletonTracking(user.getId());
					//if(uIDDisplay == 2)
						//printf("\t\t\t\t");
					//printf("----- REGISTER USER %d -----\n",n[i] + 1);

					mouseMotion(contact_[n[i]],curX[n[i]],curY[n[i]],isDragged[n[i]]);
					
					#pragma region retain user profile algo - ADD
					if(profileAlgo)
					{
						if(!isUserSlot2Occupied)			// check if slot 2 is not occupied / else is full
						{
							if(!isUserSlot1Occupied)		// check if slot 1 is not occupied
							{
								n[0] = 0;					// slot 1 will accomodate the user 1
								isUserSlot1Occupied = true;
							}
							else							
							{
								if(n[0] == 0)				// slot 1 is occupied for user 1 / slot 2 will will retain user 2 profile
									n[1] = 1;				
								else						// slot 1 is occupied for user 2 / slot 2 will will retain user 1 profile
									n[1] = 0;					

								isUserSlot2Occupied = true;
							}
						}
					}
					#pragma endregion
					
				}
				else if (user.getSkeleton().getState() == nite::SKELETON_TRACKED)
				{
					#pragma region For User 1/Right Hand	
					if(n[i] == 0 && rHand1)
					{
						if (nHandR.getPositionConfidence() > nConfidence)
						{
							if (!(startPosX[n[i]] >= 0 && startPosY[n[i]] >= 0))
							{
								mUserTracker.convertJointCoordinatesToDepth(
									nHandR.getPosition().x,
									nHandR.getPosition().y,
									nHandR.getPosition().z,
									&startPosX[n[i]], &startPosY[n[i]]);

								//printf("Starting Cursor X: %4f | Cursor Y: %4f\n", startPosX[n[i]], startPosY[n[i]]);

								mouseMotion(contact_[n[i]],curX[n[i]],curY[n[i]],isDragged[n[i]]);
							}
							else if (startPosX[n[i]] >= 0 && startPosY[n[i]] >= 0)
							{
								float posX[MAX_USERS], posY[MAX_USERS];
							
								mUserTracker.convertJointCoordinatesToDepth(
									nHandR.getPosition().x,
									nHandR.getPosition().y,
									nHandR.getPosition().z,
									&posX[n[i]], &posY[n[i]]);

								if (abs(int(posX[n[i]] - startPosX[n[i]])) > 10)
									curX[n[i]] += int(((posX[n[i]] - startPosX[n[i]]) - 10) / cursorSpeedDivisor);
								if (abs(int(posY[n[i]] - startPosY[n[i]])) > 10)
									curY[n[i]] += int(((posY[n[i]] - startPosY[n[i]]) - 10) / cursorSpeedDivisor);
								curX[n[i]] = min(curX[n[i]], cursorLimitRight);
								curX[n[i]] = max(curX[n[i]], cursorLimitLeft);
								curY[n[i]] = min(curY[n[i]], cursorLimitBottom);
								curY[n[i]] = max(curY[n[i]], cursorLimitTop);

								//printf("User %d | Cursor X: %4d | Cursor Y: %4d\n", user.getId(), curX[n[i]], curY[n[i]]);
							
								mouseMotion(contact_[n[i]],curX[n[i]],curY[n[i]],isDragged[n[i]]);
							}
							else
								mouseMotion(contact_[n[i]],curX[n[i]],curY[n[i]],isDragged[n[i]]);
						}
						else
							mouseMotion(contact_[n[i]],curX[n[i]],curY[n[i]],isDragged[n[i]]);
					}
					#pragma endregion
					#pragma region For User 1/Left Hand
					else if(n[i] == 0 && !rHand1)
					{
						if (nHandL.getPositionConfidence() > nConfidence)
						{
							if (!(startPosX[n[i]] >= 0 && startPosY[n[i]] >= 0))
							{
								mUserTracker.convertJointCoordinatesToDepth(
									nHandL.getPosition().x,
									nHandL.getPosition().y,
									nHandL.getPosition().z,
									&startPosX[n[i]], &startPosY[n[i]]);

								//printf("Starting Cursor X: %4f | Cursor Y: %4f\n", startPosX[n[i]], startPosY[n[i]]);

								mouseMotion(contact_[n[i]],curX[n[i]],curY[n[i]],isDragged[n[i]]);
							}
							else if (startPosX[n[i]] >= 0 && startPosY[n[i]] >= 0)
							{
								float posX[MAX_USERS], posY[MAX_USERS];
							
								mUserTracker.convertJointCoordinatesToDepth(
									nHandL.getPosition().x,
									nHandL.getPosition().y,
									nHandL.getPosition().z,
									&posX[n[i]], &posY[n[i]]);

								if (abs(int(posX[n[i]] - startPosX[n[i]])) > 10)
									curX[n[i]] += int(((posX[n[i]] - startPosX[n[i]]) - 10) / cursorSpeedDivisor);
								if (abs(int(posY[n[i]] - startPosY[n[i]])) > 10)
									curY[n[i]] += int(((posY[n[i]] - startPosY[n[i]]) - 10) / cursorSpeedDivisor);
								curX[n[i]] = min(curX[n[i]], cursorLimitRight);
								curX[n[i]] = max(curX[n[i]], cursorLimitLeft);
								curY[n[i]] = min(curY[n[i]], cursorLimitBottom);
								curY[n[i]] = max(curY[n[i]], cursorLimitTop);

								//printf("User %d | Cursor X: %4d | Cursor Y: %4d\n", user.getId(), curX[n[i]], curY[n[i]]);
							
								mouseMotion(contact_[n[i]],curX[n[i]],curY[n[i]],isDragged[n[i]]);
							}
							else
								mouseMotion(contact_[n[i]],curX[n[i]],curY[n[i]],isDragged[n[i]]);
						}
						else
							mouseMotion(contact_[n[i]],curX[n[i]],curY[n[i]],isDragged[n[i]]);
					}
					#pragma endregion
					#pragma region For User 2/Right Hand
					else if(n[i] == 1 && rHand2)
					{
						if (nHandR.getPositionConfidence() > nConfidence)
						{
							if (!(startPosX[n[i]] >= 0 && startPosY[i] >= 0))
							{
								mUserTracker.convertJointCoordinatesToDepth(
									nHandR.getPosition().x,
									nHandR.getPosition().y,
									nHandR.getPosition().z,
									&startPosX[n[i]], &startPosY[n[i]]);

								//printf("Starting Cursor X: %4f | Cursor Y: %4f\n", startPosX[n[i]], startPosY[n[i]]);

								mouseMotion(contact_[n[i]],curX[n[i]],curY[n[i]],isDragged[n[i]]);
							}
							else if (startPosX[n[i]] >= 0 && startPosY[n[i]] >= 0)
							{
								float posX[MAX_USERS], posY[MAX_USERS];
							
								mUserTracker.convertJointCoordinatesToDepth(
									nHandR.getPosition().x,
									nHandR.getPosition().y,
									nHandR.getPosition().z,
									&posX[n[i]], &posY[n[i]]);

								if (abs(int(posX[n[i]] - startPosX[n[i]])) > 10)
									curX[n[i]] += int(((posX[n[i]] - startPosX[n[i]]) - 10) / cursorSpeedDivisor);
								if (abs(int(posY[n[i]] - startPosY[n[i]])) > 10)
									curY[n[i]] += int(((posY[n[i]] - startPosY[n[i]]) - 10) / cursorSpeedDivisor);
								curX[n[i]] = min(curX[n[i]], cursorLimitRight);
								curX[n[i]] = max(curX[n[i]], cursorLimitLeft);
								curY[n[i]] = min(curY[n[i]], cursorLimitBottom);
								curY[n[i]] = max(curY[n[i]], cursorLimitTop);

								//printf("User %d | Cursor X: %4d | Cursor Y: %4d\n", user.getId(), curX[n[i]], curY[n[i]]);
							
								mouseMotion(contact_[n[i]],curX[n[i]],curY[n[i]],isDragged[n[i]]);
							}
							else
								mouseMotion(contact_[n[i]],curX[n[i]],curY[n[i]],isDragged[n[i]]);
						}
						else
							mouseMotion(contact_[n[i]],curX[n[i]],curY[n[i]],isDragged[n[i]]);
					}
					#pragma endregion
					#pragma region For User 2/Right Hand
					else if(n[i] == 1 && !rHand2)
					{
						if (nHandL.getPositionConfidence() > nConfidence)
						{
							if (!(startPosX[n[i]] >= 0 && startPosY[n[i]] >= 0))
							{
								mUserTracker.convertJointCoordinatesToDepth(
									nHandL.getPosition().x,
									nHandL.getPosition().y,
									nHandL.getPosition().z,
									&startPosX[n[i]], &startPosY[n[i]]);

								//printf("Starting Cursor X: %4f | Cursor Y: %4f\n", startPosX[n[i]], startPosY[n[i]]);

								mouseMotion(contact_[n[i]],curX[n[i]],curY[n[i]],isDragged[n[i]]);
							}
							else if (startPosX[n[i]] >= 0 && startPosY[n[i]] >= 0)
							{
								float posX[MAX_USERS], posY[MAX_USERS];
							
								mUserTracker.convertJointCoordinatesToDepth(
									nHandL.getPosition().x,
									nHandL.getPosition().y,
									nHandL.getPosition().z,
									&posX[n[i]], &posY[n[i]]);

								if (abs(int(posX[n[i]] - startPosX[n[i]])) > 10)
									curX[n[i]] += int(((posX[n[i]] - startPosX[n[i]]) - 10) / cursorSpeedDivisor);
								if (abs(int(posY[n[i]] - startPosY[n[i]])) > 10)
									curY[n[i]] += int(((posY[n[i]] - startPosY[n[i]]) - 10) / cursorSpeedDivisor);
								curX[n[i]] = min(curX[n[i]], cursorLimitRight);
								curX[n[i]] = max(curX[n[i]], cursorLimitLeft);
								curY[n[i]] = min(curY[n[i]], cursorLimitBottom);
								curY[n[i]] = max(curY[n[i]], cursorLimitTop);

								//printf("User %d | Cursor X: %4d | Cursor Y: %4d\n", user.getId(), curX[n[i]], curY[n[i]]);
							
								mouseMotion(contact_[n[i]],curX[n[i]],curY[n[i]],isDragged[n[i]]);
							}
							else
								mouseMotion(contact_[n[i]],curX[n[i]],curY[n[i]],isDragged[n[i]]);
						}
						else
							mouseMotion(contact_[n[i]],curX[n[i]],curY[n[i]],isDragged[n[i]]);
					}
					#pragma endregion
					else
						mouseMotion(contact_[n[i]],curX[n[i]],curY[n[i]],isDragged[n[i]]);
				}
				else if(user.isLost())
				{
					mUserTracker.stopSkeletonTracking(user.getId());
					//if(uIDDisplay == 2)
						//printf("\t\t\t\t");
					//printf("----- DROPPED USER %d -----\n",n[i] + 1);
					
					#pragma region retain user profile algo - REMOVE
					if(profileAlgo)
					{
						if(isUserSlot1Occupied)
						{
							if(isUserSlot2Occupied)
							{
								if(i == 0)
								{
									n[0] = n[1];			// slot 2 is dropped / slot 1 will fill for user in slot 2
									if(n[0] == 0)			// slot 1 was occupied by user 1 
									{
										n[1] = 1;				// slot 2 will be occupied user 2
									}
									else				  	// slot 1 was occupied by user 2 
									{
										n[1] = 0;				// slot 2 will be occupied user 1
									}
								}
								isUserSlot2Occupied = false;
							}
							else
								isUserSlot1Occupied = false;
						}
					}
					#pragma endregion
					
					if(n[i] == 0)
					{
						curX[n[i]] = desktopResolution.right * 1/4;
					}
					else
					{
						curX[n[i]] = desktopResolution.right * 3/4;
					}
					curY[n[i]] = desktopResolution.bottom/2;
					startPosX[n[i]] = -1;
					startPosY[n[i]] = -1;

					mouseMotion(contact_[n[i]],curX[n[i]],curY[n[i]],isDragged[n[i]]);
				}
				else
					mouseMotion(contact_[n[i]],curX[n[i]],curY[n[i]],isDragged[n[i]]);
			}
			InjectTouchInput(2, contact_);
			#pragma endregion

			mUserFrame.release();
		}
		else
		{
			cerr << "Can't get user frame" << endl;
		}
		//-----------------------------------CURSOR GRAPHICS -----------------------------------//
		#pragma region Cursor Graphics
		HDC dcWnd1	= GetDC(hDesktop); 
		HDC dcWnd2	= GetDC(hDesktop);
		HDC dcMem1	= CreateCompatibleDC(dcWnd1);
		HDC dcMem2	= CreateCompatibleDC(dcWnd2);
		HBITMAP bmp1 = CreateCompatibleBitmap(dcWnd1,desktopResolution.right,desktopResolution.bottom); 
		HBITMAP bmp2 = CreateCompatibleBitmap(dcWnd2,desktopResolution.right,desktopResolution.bottom); 
		HGDIOBJ bmpOld1 = SelectObject(dcMem1,bmp1);
		HGDIOBJ bmpOld2 = SelectObject(dcMem2,bmp2);

		if(curGDIShapes)
		{	
				if(curBitBlt)
				{
					BitBlt(dcMem1,curX[0] - 10,curY[0] - 10,20,20,dcWnd1,curX[0] - 10,curY[0] - 10,SRCCOPY);
					BitBlt(dcMem2,curX[1] - 10,curY[1] - 10,20,20,dcWnd2,curX[1] - 10,curY[1] - 10,SRCCOPY);
				}

				RECT rc1 = {curX[0] - 10,curY[0] - 10,curX[0] + 10,curY[0] + 10};
				DrawText(dcWnd1, TEXT("1"), -1, &rc1,DT_SINGLELINE | DT_CENTER | DT_VCENTER);
				RECT rc2 = {curX[1] - 10,curY[1] - 10,curX[1] + 10,curY[1] + 10};
				DrawText(dcWnd2, TEXT("2"), -1, &rc2,DT_SINGLELINE | DT_CENTER | DT_VCENTER);

				if(!isDragged[0])
					FillRect(dcWnd1, &rc1, hGreenBrush);
				else if(isDragged[0])
					FillRect(dcWnd1, &rc1, hBlueBrush);
				if(!isDragged[1])
					FillRect(dcWnd2, &rc2, hCyanBrush);
				else if(isDragged[1])
					FillRect(dcWnd2, &rc2, hMagentaBrush);

				if(curBitBlt)
				{
					BitBlt(dcWnd1,curX[0] - 10,curY[0] - 10,20,20,dcMem1,curX[0] - 10,curY[0] - 10,SRCCOPY);
					BitBlt(dcWnd2,curX[1] - 10,curY[1] - 10,20,20,dcMem2,curX[1] - 10,curY[1] - 10,SRCCOPY);
				}
		}
		else if(curGDIText)
		{
				if(curBitBlt)
				{
					BitBlt(dcMem1,curX[0] - 39,curY[0] - 10,10,10,dcWnd1,curX[0] - 39,curY[0] - 10,SRCCOPY);
					BitBlt(dcMem2,curX[1] - 39,curY[1] - 10,10,10,dcWnd2,curX[1] - 39,curY[1] - 10,SRCCOPY);
				}

				TextOut(dcWnd1,curX[0] - 24,curY[0] - 10,TEXT("USER 1"),6);
				TextOut(dcWnd2,curX[1] - 24,curY[1] - 10,TEXT("USER 2"),6);
				
				if(curBitBlt)
				{
					BitBlt(dcWnd1,curX[0] - 39,curY[0] - 10,10,10,dcMem1,curX[0] - 39,curY[0] - 10,SRCCOPY);
					BitBlt(dcWnd2,curX[1] - 39,curY[1] - 10,10,10,dcMem2,curX[1] - 39,curY[1] - 10,SRCCOPY);
				}
		}
		SelectObject(dcMem1, bmpOld1);
		SelectObject(dcMem2, bmpOld2);
		DeleteObject(bmp1);
		DeleteObject(bmp2);
		DeleteDC(dcMem1);
		DeleteDC(dcMem2);
		DeleteDC(dcWnd1);
		DeleteDC(dcWnd2);
		#pragma endregion
		//-----------------------------------KEYBOARD KEYS -----------------------------------//
		#pragma region Keyboard Keys
		if (GetAsyncKeyState(0x31)) //Key '1'
		{
			startPosX[0] = -1;
			startPosY[0] = -1;
			curX[0] = desktopResolution.right * 1/4;
			curY[0] = desktopResolution.bottom/2;
		}
		if (GetAsyncKeyState(0x32)) //Key '2'
		{
			startPosX[1] = -1;
			startPosY[1] = -1;
			curX[1] = desktopResolution.right * 3/4;
			curY[1] = desktopResolution.bottom/2;
		}
		if (GetAsyncKeyState(VK_ESCAPE)) // Key 'ESC'
		{
			int msgboxID = MessageBox
			(
				NULL,
				(LPCWSTR)L"Exit the program?",
				(LPCWSTR)L"FPS Interactive Whiteboard",
				MB_ICONQUESTION | MB_YESNO| MB_DEFBUTTON1 | MB_SYSTEMMODAL 
			);

			if(msgboxID == IDYES)
			{
				if(paintKeyPressed)
				{
					system("taskkill /im mspaint.exe");
					paintKeyPressed = false;
				}
				if(helpKeyPressed)
				{
					system("taskkill /f /im FPSIWBGraphics(DevVer).exe");
					helpKeyPressed = false;
				}
				std::cout << "..Exiting the program" << std::endl;
				break;
			}
		}
		#pragma endregion
		//-----------------------------------UTILITY KEYS -----------------------------------//
		#pragma region Additional Utility Keys
		if (GetAsyncKeyState(0x4C)) // Key 'L'
		{
			if(!lockKeyPressed)
			{
				startPosX[0] = -1;
				startPosY[0] = -1;
				curX[0] = desktopResolution.right * 1/4;
				curY[0] = desktopResolution.bottom/2;
				startPosX[1] = -1;
				startPosY[1] = -1;
				curX[1] = desktopResolution.right * 3/4;
				curY[1] = desktopResolution.bottom/2;

				lockKeyPressed = true;
				updateLock = true;
			}
		}
		if (GetAsyncKeyState(0x26)) // Key 'ARROW UP'
		{
			if(lockKeyPressed)
			{
				MoveToEx(dcCover,screenBorderSize, borderTopMax, NULL);
				LineTo(dcCover, desktopResolution.right - screenBorderSize, borderTopMax);
				LineTo(dcCover, desktopResolution.right - screenBorderSize, borderBottomMax);
				LineTo(dcCover,screenBorderSize, borderBottomMax);
				LineTo(dcCover,screenBorderSize, borderTopMax);

				gapSizeUp += 100;
				if(gapSizeUp > desktopResolution.bottom/2)
				{
					gapSizeUp = 100;
				}
				borderTopMax = desktopResolution.bottom/2 - gapSizeUp;
				if(borderTopMax < borderTopLimit)
					borderTopMax = borderTopLimit;

				updateLock = true;
			}
		}
		if (GetAsyncKeyState(0x28)) // Key 'ARROW DOWN'
		{
			if(lockKeyPressed)
			{
				MoveToEx(dcCover,screenBorderSize, borderTopMax, NULL);
				LineTo(dcCover, desktopResolution.right - screenBorderSize, borderTopMax);
				LineTo(dcCover, desktopResolution.right - screenBorderSize, borderBottomMax);
				LineTo(dcCover,screenBorderSize, borderBottomMax);
				LineTo(dcCover,screenBorderSize, borderTopMax);

				gapSizeDown += 100;
				if(gapSizeDown > desktopResolution.bottom/2)
				{
					gapSizeDown = 100;
				}
				borderBottomMax = desktopResolution.bottom/2 + gapSizeDown;
				if(borderBottomMax > borderBottomLimit)
					borderBottomMax = borderBottomLimit;

				updateLock = true;
			}
		}
		if (GetAsyncKeyState(0x50)) // Key 'P'
		{
			if(!paintKeyPressed)
			{
				system("start mspaint.exe");
				paintKeyPressed = true;
			}
		}
		if (GetAsyncKeyState(0x48)) // Key 'H'
		{
			if(!testMode)
				if(!helpKeyPressed)
				{
					//system("start C:\\Users\\Angelo\\Desktop\\FPSIWBGraphics(DevVer)\\Debug\\FPSIWBGraphics(DevVer).exe");
					system("start C:\\FPSIWBGraphics(DevVer)\\Debug\\FPSIWBGraphics(DevVer).exe");

					//xPopText1 = 160;
					//xPopText2 = desktopResolution.right - 280;
					//yPopText = 5;

					yPopText = instructionPosition - instructionBoxSize - instructionGap - instructionLabel;
					xPopText1 = borderGap + 80;
					xPopText2 = desktopResolution.right - borderGap - 300 + 99;

					helpKeyPressed = true;
				}
		}
		if (GetAsyncKeyState(0x20)) // Key 'SPACEBAR'
		{
			if(lockKeyPressed)
			{
				cursorLimitLeft = desktopResolution.left + 10;
				cursorLimitRight = desktopResolution.right - 10;
				cursorLimitTop = desktopResolution.top + 10;
				cursorLimitBottom = desktopResolution.bottom - 10;
					
				startPosX[0] = -1;
				startPosY[0] = -1;
				curX[0] = desktopResolution.right * 1/4;
				curY[0] = desktopResolution.bottom/2;
				startPosX[1] = -1;
				startPosY[1] = -1;
				curX[1] = desktopResolution.right * 3/4;
				curY[1] = desktopResolution.bottom/2;;

				gapSizeUp = 100;
				gapSizeDown = 100;
				borderTopMax = desktopResolution.bottom/2 - gapSizeUp;
				borderBottomMax = desktopResolution.bottom/2 + gapSizeDown;

				lockKeyPressed = false;
				updateLock = false;
			}
			if(paintKeyPressed)
			{
				system("taskkill /im mspaint.exe");
				paintKeyPressed = false;
			}
			if(helpKeyPressed)
			{
				xPopText1 = 160;
				xPopText2 = desktopResolution.right - 280;
				yPopText = 5;

				//yPopText = instructionPosition - instructionBoxSize - instructionGap - instructionLabel;
				//xPopText1 = borderGap + 80;
				//xPopText2 = desktopResolution.right - borderGap - 300 + 99;

				system("taskkill /f /im FPSIWBGraphics(DevVer).exe");
				helpKeyPressed = false;
			}
		}
		#pragma endregion
	//-----------------------------------TEST-----------------------------------//
	if(testMode)
		if(time_counterT > (double)(1 * CLOCKS_PER_SEC))
		{
			time_counterT -= (double)(1 * CLOCKS_PER_SEC);
			if(loopCounter > 3)
			{
				printf("%2.0f loops/sec | %3.2f ms/loop\n",loopCounter,(double)1000/loopCounter);
				iterationCounter++;
				loopAverage += loopCounter;
				speedAverage += (double)1000/loopCounter;
						
				if (GetAsyncKeyState(0x20))
				{
					printf("\n %2.0f average loops/sec and %3.2f average ms/loop\n",(double)loopAverage/(iterationCounter),(double)speedAverage/(iterationCounter));
					cout << "\n			Press [ESC] to End!"<< endl;
					while(1)
					{
						if (GetAsyncKeyState(VK_ESCAPE))
							break;
					}
					break;
				}	
			}	
			loopCounter = 0;
		}
	}
	//-----------------------------------LOOP END-----------------------------------//
	end:

	DeleteObject(hRedBrush);
	DeleteObject(hGreenBrush);
	DeleteObject(hBlueBrush);
	DeleteObject(hCyanBrush);
	DeleteObject(hYellowBrush);
	DeleteObject(hMagentaBrush);
	DeleteObject(hGrayPen);

	DeleteDC(dcText);
	DeleteDC(dcPopText);
	DeleteDC(dcRed);
	DeleteDC(dcGreen);
	DeleteDC(dcBlue);
	DeleteDC(dcCyan);
	DeleteDC(dcYellow);
	DeleteDC(dcMagenta);
	DeleteDC(dcCover);
	DeleteDC(dcBorder);

	DeleteObject(hDesktop);

	vsColorStream.stop();
	cout << "Color Stream Stopped" << endl;
	Sleep(200);
	vsDepthStream.stop();
	cout << "Depth Stream Stopped" << endl;
	Sleep(200);
	mUserTracker.destroy(); 
	cout << "User Tracker Stopped" << endl;
	Sleep(200);
	vsColorStream.destroy();
	cout << "Color Stream Destroyed" << endl;
	Sleep(200);
	vsDepthStream.destroy();
	cout << "Depth Stream Destroyed" << endl;
	Sleep(200);
	devDevice.close();
	cout << "Device Closed" << endl;
	Sleep(200);
	NiTE::shutdown();
	cout << "NiTE Shutdown" << endl;
	Sleep(200);
	OpenNI::shutdown();
	cout << "OpenNI Shutdown" << endl;

	return 0;

}