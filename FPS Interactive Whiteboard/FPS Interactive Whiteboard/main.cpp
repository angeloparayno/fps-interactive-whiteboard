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
int cRed[7] = { 1,160,179,170,255,125,255 };
int cGreen[7] = {2,30,75,80,255,64,255};
int cBlue[7] = { 3,115,130,188,255,170,255 };
int cCyan[7] = { 4,75,115,100,255,147,241 };
int cYellow[7] = {5,0,15,125,250,131,255 };
int cMagenta[7] = { 6,145,160,125,255,125,255};

//int cRed[7] = { 1,160,179,170,255,170,255 };
//int cMagenta[7] = { 6,138,171,232,255,188,255};

// userID variable for display
int uIDDisplay;
// desktop resolution global var.
int desktopMaxWidth = 0;
// max number of objects to be detected in frame
const int MAX_NUM_OBJECTS = 50;
// max number of users
const int MAX_USERS = 2;
const int MAX_TRACKED = 10;
// default capture width and height
const int FRAME_WIDTH = 640;
const int FRAME_HEIGHT = 480;
// minimum and maximum object area
const int MIN_OBJECT_AREA = 10 * 10;
const int MAX_OBJECT_AREA = FRAME_HEIGHT*FRAME_WIDTH / 1.5;
// clock time interval 
const double NUM_SECONDS_START_TRACKING_INTERVAL = .5;
const double NUM_SECONDS_COLOR_TRACKING_INTERVAL = 0.125;
const double NUM_SECONDS_RECALIBRATE_AGLORITHM1 = 1;
const double NUM_SECONDS_USER_DISPLAY_DELAY = 1;
const double NUM_SECONDS_CLICK_DELAY = 0.25;
// get desktop window resolution
const HWND hDesktop = GetDesktopWindow();
// maximum mumber of lines the output console should have
static const WORD MAX_CONSOLE_LINES = 500;
// touch injection pointer contact
POINTER_TOUCH_INFO contact_[2];
// user hand prefeference (default is right hand for both users)
bool rHand1 = true,rHand2 = true;
// initialize how to start the program
bool continueToPaint = false;
// profile algorithm
bool profileAlgorithm = true;
// Pop-up graphics text
int xPopText1 = 0;
int xPopText2 = 0;
int yPopText = 0;
HDC dcPopText1 = GetDC(NULL);
HDC dcPopText2 = GetDC(NULL);
HDC dcPopText3 = GetDC(NULL);
HDC dcPopText4 = GetDC(NULL);

// color tracking
bool colorTracking = true;
bool colorTrackingIntervalRGB = true;
bool colorTrackingIntervalCYM = true;
// click delay
bool clickDelay = false; 
// start track delay algorithm
bool startTrackDelayAlgorithm = true;
// recalibration algorithm
bool recalibrationAlgorithm1 = true;
int rAlgoRange = 0;
int maxTimeOut = 3;
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
			TextOut(dcPopText3,(int)curX + 29,curY + 35,TEXT("USER 1"),6);
		// Max Left
		else if(curX < 24 + cursorLimitLeft)
			TextOut(dcPopText3,(int)curX + 29,curY - 10,TEXT("USER 1"),6); 
		// Max Top
		else if(curY < 45 + cursorLimitTop)
			TextOut(dcPopText3,(int)curX - 24,curY + 35,TEXT("USER 1"),6); 
		// Normal
		else
			TextOut(dcPopText3,(int)curX - 24,curY - 45,TEXT("USER 1"),6); 
	}
	else if(user == 1)
	{
		// Corner
		if(curX < 24 + cursorLimitLeft && curY < 45 + + cursorLimitTop)	
			TextOut(dcPopText4,(int)curX + 29,curY + 35,TEXT("USER 2"),6);
		// Max Left
		else if(curX < 24 + cursorLimitLeft)
			TextOut(dcPopText4,(int)curX + 29,curY - 10,TEXT("USER 2"),6); 
		// Max Top
		else if(curY < 45 + + cursorLimitTop)
			TextOut(dcPopText4,(int)curX - 24,curY + 35,TEXT("USER 2"),6); 
		// Normal
		else
			TextOut(dcPopText4,(int)curX - 24,curY - 45,TEXT("USER 2"),6);	
	}		
}
#pragma endregion

#pragma region Display User State Function
void updateUserState(const nite::UserData& user)
{
	if (user.isNew())
	{	
		if(uIDDisplay == 1)
		{
			if(!continueToPaint)
			{
				TextOut(dcPopText1,110,5,TEXT("USER 1"),6);
			}
			printf("User %d:\t New\n",uIDDisplay);
			TextOut(dcPopText1,xPopText1,yPopText,TEXT(" - NEW                  "),24);
		}
		else if(uIDDisplay == 2)
		{
			if(!continueToPaint)
			{
				TextOut(dcPopText2,desktopMaxWidth - 330,5,TEXT("USER 2"),6);
			}
			printf("\t\t\t\tUser %d:\t New\n",uIDDisplay);
			TextOut(dcPopText2,xPopText2,yPopText,TEXT(" - NEW                  "),24);
		}
	}
	else if (user.isVisible() && !g_visibleUsers[user.getId()])
	{
		if(uIDDisplay == 1)
		{ 
			if(!continueToPaint)
			{
				TextOut(dcPopText1,110,5,TEXT("USER 1"),6);
			}
			printf("User %d:\t Visible\n",uIDDisplay);
			TextOut(dcPopText1,xPopText1,yPopText,TEXT(" - VISIBLE              "),24);
		}
		else if(uIDDisplay == 2)
		{
			if(!continueToPaint)
			{
				TextOut(dcPopText2,desktopMaxWidth - 330,5,TEXT("USER 2"),6);
			}
			printf("\t\t\t\tUser %d:\t Visible\n",uIDDisplay);
			TextOut(dcPopText2,xPopText2,yPopText,TEXT(" - VISIBLE		       "),24);
		}
	}
	else if (!user.isVisible() && g_visibleUsers[user.getId()])
	{
		if(uIDDisplay == 1)
		{
			if(!continueToPaint)
			{
				TextOut(dcPopText1,110,5,TEXT("USER 1"),6);
			}
			printf("User %d:\t Out of Scene\n",uIDDisplay);
			TextOut(dcPopText1,xPopText1,yPopText,TEXT(" - OUT OF SCENE	        "),24);
		}
		else if(uIDDisplay == 2)
		{
			if(!continueToPaint)
			{
				TextOut(dcPopText2,desktopMaxWidth - 330,5,TEXT("USER 2"),6);
			}
			printf("\t\t\t\tUser %d:\t Out of Scene\n",uIDDisplay);
			TextOut(dcPopText2,xPopText2,yPopText,TEXT(" - OUT OF SCENE	        "),24);
		}
	}
	else if (user.isLost())
	{
		if(uIDDisplay == 1)
		{
			if(!continueToPaint)
			{
				TextOut(dcPopText1,110,5,TEXT("USER 1"),6);
			}
			printf("User %d:\t Lost\n",uIDDisplay);
			TextOut(dcPopText1,xPopText1,yPopText,TEXT(" - LOST                 "),24);
		}
		else if(uIDDisplay == 2)
		{
			if(!continueToPaint)
			{
				TextOut(dcPopText2,desktopMaxWidth - 330,5,TEXT("USER 2"),6);
			}
			printf("\t\t\t\tUser %d:\t Lost\n",uIDDisplay);
			TextOut(dcPopText2,xPopText2,yPopText,TEXT(" - LOST                 "),24);
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
				if(!continueToPaint)
				{
					TextOut(dcPopText1,110,5,TEXT("USER 1"),6);
				}
				("User %d:\t Stopped Tracking\n",uIDDisplay);
				TextOut(dcPopText1,xPopText1,yPopText,TEXT(" - STOPPED TRACKING     "),24);
			}
			else if(uIDDisplay == 2)
			{
				if(!continueToPaint)
				{
					TextOut(dcPopText2,desktopMaxWidth - 330,5,TEXT("USER 2"),6);
				}
				printf("\t\t\t\tUser %d:\t Stopped Tracking\n",uIDDisplay);
				TextOut(dcPopText2,xPopText2,yPopText,TEXT(" - STOPPED TRACKING     "),24);
			}
			break;
		case nite::SKELETON_CALIBRATING:
			if(uIDDisplay == 1)
			{
				if(!continueToPaint)
				{
					TextOut(dcPopText1,110,5,TEXT("USER 1"),6);
				}
				("User %d:\t Calibrating...\n",uIDDisplay);
				TextOut(dcPopText1,xPopText1,yPopText,TEXT(" - CALIBRATING...       "),24);
			}
			else if(uIDDisplay == 2)
			{
				if(!continueToPaint)
				{
					TextOut(dcPopText2,desktopMaxWidth - 330,5,TEXT("USER 2"),6);
				}
				printf("\t\t\t\tUser %d:\t Calibrating...\n",uIDDisplay);
				TextOut(dcPopText2,xPopText2,yPopText,TEXT(" - CALIBRATING...       "),24);
			}
			break;
		case nite::SKELETON_TRACKED:
			if(uIDDisplay == 1)
			{
				if(!continueToPaint)
				{
					TextOut(dcPopText1,110,5,TEXT("USER 1"),6);
				}
				("User %d:\t Tracking!\n",uIDDisplay);
				TextOut(dcPopText1,xPopText1,yPopText,TEXT(" - TRACKING!            "),24);
			}
			else if(uIDDisplay == 2)
			{
				if(!continueToPaint)
				{
					TextOut(dcPopText2,desktopMaxWidth - 330,5,TEXT("USER 2"),6);
				}
				printf("\t\t\t\tUser %d:\t Tracking!\n",uIDDisplay);
				TextOut(dcPopText2,xPopText2,yPopText,TEXT(" - TRACKING!            "),24);
			}
			break;
		case nite::SKELETON_CALIBRATION_ERROR_NOT_IN_POSE:
		case nite::SKELETON_CALIBRATION_ERROR_HANDS:
		case nite::SKELETON_CALIBRATION_ERROR_LEGS:
		case nite::SKELETON_CALIBRATION_ERROR_HEAD:
		case nite::SKELETON_CALIBRATION_ERROR_TORSO:
			if(uIDDisplay == 1)
			{
				if(!continueToPaint)
				{
					TextOut(dcPopText1,110,5,TEXT("USER 1"),6);
				}
				("User %d:\t Calibration Failed...\n",uIDDisplay);
				TextOut(dcPopText1,xPopText1,yPopText,TEXT(" - Calibration Failed..."),24);
			}
			else if(uIDDisplay == 2)
			{
				if(!continueToPaint)
				{
					TextOut(dcPopText2,desktopMaxWidth - 330,5,TEXT("USER 2"),6);
				}
				printf("\t\t\t\tUser %d:\t Calibration Failed...\n",uIDDisplay);
				TextOut(dcPopText2,xPopText2,yPopText,TEXT(" - Calibration Failed..."),24);
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
void promptMenu()
{
	char cHandPreference1 = 'x';
	char cHandPreference2 = 'x';

	cout << "-----------------------------------------------------------------------------" << endl;
	cout << "                    WELCOME TO FPS INTERACTIVE WHITEBOARD                    " << endl;
	cout << "-----------------------------------------------------------------------------" << endl;
	cout << endl;
	cout << "STEP 1 OF 4: SELECT FIRST USER HAND MOVEMENT PREREFENCE" << endl;
	cout << "-> Input either [R] for Right and [L] for Left, then press the [Enter] key: ";
	do
	{
		cin >> cHandPreference1;
		switch(cHandPreference1)
		{
			case 'r':
			case 'R':
				rHand1 = true;
				cout																			<< endl;
				cout << "RIGHT HAND IS CHOSEN FOR HAND MOVEMENT PREFERENCE FOR THE FIRST USER"	<< endl;
				cout																			<< endl;
				break;
			case 'l':
			case 'L':
				rHand1 = false;
				cout																			<< endl;
				cout << "LEFT HAND IS CHOSEN FOR HAND MOVEMENT PREFERENCE FOR THE FIRST USER"	<< endl;
				cout																			<< endl;
				break;
			default:
				cout << "-> Invalid Input, input either [R] for Right and [L] for Left: ";
				cHandPreference1 = 'x';
				break;
		}
	}
	while(cHandPreference1 == 'x');
	cout																<< endl;
	cout << "STEP 2 OF 4: SELECT SECOND USER HAND MOVEMENT PREREFENCE"	<< endl;
	cout << "-> Input either [R] for Right and [L] for Left, then press the [Enter] key: ";
	do
	{
		cin >> cHandPreference2;
		switch(cHandPreference2)
		{
		case 'r':
		case 'R':
			rHand2 = true;
			cout																			<< endl;
			cout << "RIGHT HAND IS CHOSEN FOR HAND MOVEMENT PREFERENCE FOR THE SECOND USER" << endl;
			cout																			<< endl;
			break;
		case 'l':
		case 'L':
			rHand2 = false;
			cout																			<< endl;
			cout << "LEFT HAND IS CHOSEN FOR HAND MOVEMENT PREFERENCE FOR THE SECOND USER"	<< endl;
			cout																			<< endl;
			break;
		default:
			cHandPreference2 = 'x';
			cout << "-> Invalid Input, input either [R] for Right and [L] for Left: ";
			break;
		}
	}
	while(cHandPreference2 == 'x');
	Sleep(500);
	cout << endl;
	cout << "STEP 3 OF 4: READ REMINDERS"																														<< endl;
	cout << "-> Make sure that you are at least a meter away from the Kinect Sensor."		<< endl;
	cout << "-> For accurate cursor movement, raise your hand on the shoulder level."		<< endl;
	cout << "-> Initially the left cursor is for the first user, and the right cursor is"	<< endl;
	cout << "   for the second user"														<< endl;
	cout << endl;
	cout << "LED Pen Buttons"																<< endl;
	cout << "-> Use the LED pens to change the cursor behavior"								<< endl;
	cout << "   First  User (RGB LED Pen)       Second User (CYM LED Pen)"					<< endl;
	cout << "   RED   - Click                   Yellow  - Click"							<< endl;
	cout <<	"   Blue  - Drag                    Magenta - Drag"								<< endl;
	cout <<	"   Green - Move (Default)          Cyan    - Move (Default)"					<< endl;
	cout << "-> Press the [MOVE] button on the LED pen to provide label on which cursor is" << endl;
	cout << "   for the first or second user cursor"										<< endl;
	cout << endl;
	cout << "Keyboard Help Keys"															<< endl;
	cout << "-> In case cursor movement is moving incorrectly, tracked  user/s must raise"	<< endl;
	cout <<	"   their hand on the shoulder level and press:"								<< endl;
	cout << "   [1] to reset first user cursor position"									<< endl;
	cout <<	"   [2] to reset second user cursor position"									<< endl;
	cout << "-> Press [ESC] to exit"														<< endl; 
	cout <<	endl;
	cout << "         Press [SPACE]  to continue"										    << endl;
	cout << "         Press   [P]    to continue on MS Paint w/ assitive graphics & border" << endl;
	while(1)
	{
		if (GetAsyncKeyState(0x20))
		{
			continueToPaint = false;
			break;
		}
		if (GetAsyncKeyState(0x50))
		{
			continueToPaint = true;
			break;
		}
	}
	cout																			<< endl;
	cout << "STEP 4 OF 4: YOU ARE NOW READY TO USE THE SYSTEM!"						<< endl;
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
	int cursorSpeedDivisor = 4;
	// Confidence Level
	float nConfidence = .5;
	// Minimum Pixel
	int minPixelTreshold = 10; 
	// Resolution Ratio
	double rRatio = 0.17;

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
	int tempCurX[2] = {0,0};
	int tempCurY[2] = {0,0};
	// Cursor Limitations
	int cursorLimitLeft = desktopResolution.left;
	int cursorLimitRight = desktopResolution.right - 5;
	int cursorLimitTop = desktopResolution.top;
	int cursorLimitBottom = desktopResolution.bottom - 5;

	#pragma region Timer Initialization
		// Display User Text Delay
		double time_counter0 = 0;
		clock_t this_time0 = clock();
		clock_t last_time0 = this_time0;
		double time_counter1 = 0;
		clock_t this_time1 = clock();
		clock_t last_time1 = this_time1;
		bool displayUserNTextLock1 = false;
		bool displayUserNTextLock2 = false;

		// Click Time Delay
		double time_counter2 = 0;
		clock_t this_time2 = clock();
		clock_t last_time2 = this_time2;
		double time_counter3 = 0;
		clock_t this_time3 = clock();
		clock_t last_time3 = this_time3;
		bool clickDelayUser1 = false;
		bool clickDelayUser2 = false;

		// Color Tracking Interval
		double time_counter4 = 0;
		clock_t this_time4 = clock();
		clock_t last_time4 = this_time4;
		bool colorTrackLockRGB = false;
		bool colorTrackLockCYM = false;
		bool skip = false;
	
		// Start Tracking Delay
		double time_counterDelay[2] = {0.0,0.0};
		clock_t this_time5 = clock();
		clock_t this_time6 = clock();
		clock_t last_time[2] = {this_time5,this_time6};
		bool noDelayTimer[2] = {false,false};
		bool startTimer[2] = {false,false};
		bool startCapturePosition[2] = {false,false};

		// Recalibration Algorithm1
		double time_counter7 = 0;
		clock_t this_time7 = clock();
		clock_t last_time7 = this_time7;
		double time_counter8 = 0;
		clock_t this_time8 = clock();
		clock_t last_time8 = this_time8;
		bool startAlgoTimer1 = false;
		bool startAlgoTimer2 = false;
		bool timeOut1 = false;
		bool timeOut2 = false;
		int timeOutCounter1 = 0;
		int timeOutCounter2 = 0;
	#pragma endregion

	// TEST TIMER
	double time_counterT = 0;
	clock_t this_timeT = clock();
	clock_t last_timeT = this_timeT;
	int iterationCounter = 0;
	double loopCounter = 0;
	double loopAverage = 0;
	double speedAverage = 0;

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
	// BORDER
	int borderTopMax = (int) desktopResolution.bottom * rRatio;
	int borderBottomMax = desktopResolution.bottom - indicatorTotalSize - 10;
	int borderGap = indicatorGap + 3;

#pragma region Auto Adjust Resolution
	if(desktopResolution.right == 1920)
		rRatio = 0.14;
	else if(desktopResolution.right == 1366)
		rRatio = 0.17;
	else
		rRatio = 0.17;
	
	borderTopMax = (int) desktopResolution.bottom * rRatio;
	instructionPosition = (int) desktopResolution.bottom * rRatio;
#pragma endregion

#pragma region Pop Text Coordinates
	yPopText = instructionPosition - instructionBoxSize - instructionGap - instructionLabel;
	xPopText1 = borderGap + 80;
	xPopText2 = desktopResolution.right - borderGap - 300 + 99;
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
	HBRUSH hYellowBrush = CreateSolidBrush(RGB(224,202,0));
	HBRUSH hMagentaBrush = CreateSolidBrush(RGB(224,0,224));

	HPEN hGrayPen = CreatePen(PS_SOLID, instructionBorderSize, RGB(64, 64, 64));

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
#pragma endregion
	//-----------------------------------START-----------------------------------//
	RedirectIOToConsole();

	vsColorStream.start();
	vsDepthStream.start();

	promptMenu();

	if(continueToPaint)
	{
		cursorLimitLeft = borderGap + 30;
		cursorLimitRight = desktopResolution.right - borderGap - 30;
		cursorLimitTop =  borderTopMax + 30;
		cursorLimitBottom = borderBottomMax - 30;
		system("start mspaint.exe");
		system("start C:\\FPSIWBGraphics3\\Debug\\FPSIWBGraphics3.exe");
		//system("start C:\\Users\\Angelo\\Desktop\\FPSIWBGraphicsVer3\\Debug\\FPSIWBGraphicsVer3.exe");
	}
	else if(!continueToPaint)
	{
		cursorLimitLeft = 10;
		cursorLimitRight = desktopResolution.right - 10;
		cursorLimitTop =  10;
		cursorLimitBottom = desktopResolution.bottom - 10;

		desktopMaxWidth = desktopResolution.right;

		xPopText1 = 160;
		xPopText2 = desktopResolution.right - 280;
		yPopText = 5;
	}
	
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

		#pragma region TIMERS
			#pragma region Display User N Text Clock Timer
			if(displayUserNTextLock1)
			{
				this_time0 = clock();
				time_counter0 += (double)(this_time0 - last_time0);
				last_time0 = this_time0;

				if(time_counter0 > (double)(NUM_SECONDS_USER_DISPLAY_DELAY * CLOCKS_PER_SEC))
				{
					time_counter0 = 0;
					displayUserNTextLock1 = false;
				}
			}
			if(displayUserNTextLock2)
			{
				this_time1 = clock();
				time_counter1 += (double)(this_time1 - last_time1);
				last_time1 = this_time1;

				if(time_counter1 > (double)(NUM_SECONDS_USER_DISPLAY_DELAY * CLOCKS_PER_SEC))
				{
					time_counter1 = 0;
					displayUserNTextLock2 = false;
				}
			}
			#pragma endregion
			#pragma region Click Delay Timer
			if(clickDelayUser1)
			{
				this_time2 = clock();
				time_counter2 += (double)(this_time2 - last_time2);
				last_time2 = this_time2;

				if(time_counter2 > (double)(NUM_SECONDS_CLICK_DELAY * CLOCKS_PER_SEC))
				{
					time_counter2 = 0;
					clickDelayUser1 = false;
				}
			}
			if(clickDelayUser2)
			{
				this_time3 = clock();
				time_counter3 += (double)(this_time3 - last_time3);
				last_time3 = this_time3;

				if(time_counter3 > (double)(NUM_SECONDS_CLICK_DELAY * CLOCKS_PER_SEC))
				{
					time_counter3 = 0;
					clickDelayUser2 = false;
				}
			}
			#pragma endregion
			#pragma region Color Tracking Interval
			this_time4 = clock();
			time_counter4 += (double)(this_time4 - last_time4);
			last_time4 = this_time4;
	
			if(time_counter4 > (double)(NUM_SECONDS_COLOR_TRACKING_INTERVAL * CLOCKS_PER_SEC))
			{
				time_counter4 = 0;
				if(!skip)
				{
					colorTrackLockRGB = false;
					skip = true;
				}
				else if(skip)
				{
					colorTrackLockCYM = false;
					skip = false;
				}
			}
			#pragma endregion
			#pragma region Start Track Delay Algorithm
			if(startTrackDelayAlgorithm)
			{	
				if(startTimer[0])
				{
					this_time5 = clock();
				
					time_counterDelay[0] += (double)(this_time5 - last_time[0]);
					last_time[0] = this_time5;

					if(time_counterDelay[0] > (double)(NUM_SECONDS_START_TRACKING_INTERVAL * CLOCKS_PER_SEC))
					{
						time_counterDelay[0] = 0;
						startCapturePosition[0] = true;
					}
				}
				if(startTimer[1])
				{
					this_time6 = clock();
					time_counterDelay[1] += (double)(this_time6 - last_time[1]);
					last_time[1] = this_time6;

					if(time_counterDelay[1] > (double)(NUM_SECONDS_START_TRACKING_INTERVAL * CLOCKS_PER_SEC))
					{
						time_counterDelay[1] = 0;
						startCapturePosition[1] = true;
					}
				}
			}
			#pragma endregion
			#pragma region Recalibration Algorithm 1
			if(recalibrationAlgorithm1)
			{	
				if(startAlgoTimer1)
				{
					this_time7 = clock();
					time_counter7 += (double)(this_time7 - last_time7);
					last_time7 = this_time7;

					if(time_counter7 > (double)(NUM_SECONDS_RECALIBRATE_AGLORITHM1 * CLOCKS_PER_SEC))
					{
						time_counter7 = 0;
						timeOut1 = true;
						startAlgoTimer1 = false;
					}
				}
				if(startAlgoTimer2)
				{
					this_time8 = clock();
					time_counter8 += (double)(this_time8 - last_time8);
					last_time8 = this_time8;

					if(time_counter8 > (double)(NUM_SECONDS_RECALIBRATE_AGLORITHM1 * CLOCKS_PER_SEC))
					{
						time_counter8 = 0;
						timeOut2 = true; 
						startAlgoTimer2 = false;
					}
				}
			}
			#pragma endregion
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
				if(colorTracking)
				{
					if(!colorTrackLockRGB)
					{
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
							if(!clickDelayUser1)
							{
								mousePressed(contact_[0], curX[0], curY[0]);
								InjectTouchInput(2, contact_);
								Sleep(20);
								mouseReleased(contact_[0], curX[0], curY[0]);
								InjectTouchInput(2, contact_);
								Sleep(20);

								if(clickDelay)
								{
									clickDelayUser1 = true;
									last_time2 = clock();
								}
							}
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
							if(!displayUserNTextLock1)
							{
								displayUserNText(curX[0],curY[0],0,cursorLimitLeft,cursorLimitTop);
								displayUserNTextLock1 = true;
								last_time0 = clock();
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
						if(colorTrackingIntervalRGB)
							colorTrackLockRGB = true;
					}
					
					if(!colorTrackLockCYM)
					{
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
							if(!clickDelayUser2)
							{
								mousePressed(contact_[1], curX[1], curY[1]);
								InjectTouchInput(2, contact_);
								Sleep(20);
								mouseReleased(contact_[1], curX[1], curY[1]);
								InjectTouchInput(2, contact_);
								Sleep(20);

								if(clickDelay)
								{
									clickDelayUser2 = true;
									last_time3 = clock();
								}
							}

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
							if(!displayUserNTextLock2)
							{
								displayUserNText(curX[1],curY[1],1,cursorLimitLeft,cursorLimitTop);
								displayUserNTextLock2 = true;
								last_time1 = clock();
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
						if(colorTrackingIntervalCYM)
							colorTrackLockCYM = true;
					}
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
				updateUserState(user);

				if (user.isNew())
				{
					mUserTracker.startSkeletonTracking(user.getId());
					//if(uIDDisplay == 2)
						//printf("\t\t\t\t");
					//printf("----- REGISTER USER %d -----\n",n[i] + 1);

					mouseMotion(contact_[n[i]],curX[n[i]],curY[n[i]],isDragged[n[i]]);
					
					#pragma region retain user profile algo - ADD
					if(profileAlgorithm)
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
								#pragma region Start Track Delay Algorithm
								if(!startTrackDelayAlgorithm)
								{	
									mUserTracker.convertJointCoordinatesToDepth(
											nHandR.getPosition().x,
											nHandR.getPosition().y,
											nHandR.getPosition().z,
											&startPosX[n[i]], &startPosY[n[i]]);

									mouseMotion(contact_[n[i]],curX[n[i]],curY[n[i]],isDragged[n[i]]);
								}
								else if(noDelayTimer[n[i]])
								{
									mUserTracker.convertJointCoordinatesToDepth(
											nHandR.getPosition().x,
											nHandR.getPosition().y,
											nHandR.getPosition().z,
											&startPosX[n[i]], &startPosY[n[i]]);

									//printf("Starting Cursor X: %4f | Cursor Y: %4f\n", startPosX[n[i]], startPosY[n[i]]);

									noDelayTimer[n[i]] = false;
									startTimer[n[i]] = false;
									startCapturePosition[n[i]] = false;
									time_counterDelay[n[i]] = 0;

									mouseMotion(contact_[n[i]],curX[n[i]],curY[n[i]],isDragged[n[i]]);
								}
								else if(!startTimer[n[i]])
								{
									startTimer[n[i]] = true;
									last_time[n[i]] = clock();
									mouseMotion(contact_[n[i]],curX[n[i]],curY[n[i]],isDragged[n[i]]);
								}
								else if(startTimer[n[i]])
								{
									if(startCapturePosition[n[i]])
									{
										startTimer[n[i]] = false;
										startCapturePosition[n[i]] = false;

										mUserTracker.convertJointCoordinatesToDepth(
											nHandR.getPosition().x,
											nHandR.getPosition().y,
											nHandR.getPosition().z,
											&startPosX[n[i]], &startPosY[n[i]]);

										//printf("Starting Cursor X: %4f | Cursor Y: %4f\n", startPosX[n[i]], startPosY[n[i]]);	
									}
										mouseMotion(contact_[n[i]],curX[n[i]],curY[n[i]],isDragged[n[i]]);
								}
								#pragma endregion
							}
							else if (startPosX[n[i]] >= 0 && startPosY[n[i]] >= 0)
							{
								float posX[MAX_USERS], posY[MAX_USERS];
							
								mUserTracker.convertJointCoordinatesToDepth(
									nHandR.getPosition().x,
									nHandR.getPosition().y,
									nHandR.getPosition().z,
									&posX[n[i]], &posY[n[i]]);

								if (abs(int(posX[n[i]] - startPosX[n[i]])) > minPixelTreshold )
									curX[n[i]] += int(((posX[n[i]] - startPosX[n[i]]) - 0) / cursorSpeedDivisor);
								if (abs(int(posY[n[i]] - startPosY[n[i]])) > minPixelTreshold )
									curY[n[i]] += int(((posY[n[i]] - startPosY[n[i]]) - 0) / cursorSpeedDivisor);
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
								#pragma region Start Track Delay Algorithm
								if(!startTrackDelayAlgorithm)
								{	
									mUserTracker.convertJointCoordinatesToDepth(
											nHandR.getPosition().x,
											nHandR.getPosition().y,
											nHandR.getPosition().z,
											&startPosX[n[i]], &startPosY[n[i]]);

									mouseMotion(contact_[n[i]],curX[n[i]],curY[n[i]],isDragged[n[i]]);
								}
								else if(noDelayTimer[n[i]])
								{
									mUserTracker.convertJointCoordinatesToDepth(
											nHandR.getPosition().x,
											nHandR.getPosition().y,
											nHandR.getPosition().z,
											&startPosX[n[i]], &startPosY[n[i]]);

									//printf("Starting Cursor X: %4f | Cursor Y: %4f\n", startPosX[n[i]], startPosY[n[i]]);

									noDelayTimer[n[i]] = false;
									startTimer[n[i]] = false;
									startCapturePosition[n[i]] = false;
									time_counterDelay[n[i]] = 0;

									mouseMotion(contact_[n[i]],curX[n[i]],curY[n[i]],isDragged[n[i]]);
								}
								else if(!startTimer[n[i]])
								{
									startTimer[n[i]] = true;
									last_time[n[i]] = clock();
									mouseMotion(contact_[n[i]],curX[n[i]],curY[n[i]],isDragged[n[i]]);
								}
								else if(startTimer[n[i]])
								{
									if(startCapturePosition[n[i]])
									{
										startTimer[n[i]] = false;
										startCapturePosition[n[i]] = false;

										mUserTracker.convertJointCoordinatesToDepth(
											nHandR.getPosition().x,
											nHandR.getPosition().y,
											nHandR.getPosition().z,
											&startPosX[n[i]], &startPosY[n[i]]);

										//printf("Starting Cursor X: %4f | Cursor Y: %4f\n", startPosX[n[i]], startPosY[n[i]]);	
									}
										mouseMotion(contact_[n[i]],curX[n[i]],curY[n[i]],isDragged[n[i]]);
								}
								#pragma endregion
							}
							else if (startPosX[n[i]] >= 0 && startPosY[n[i]] >= 0)
							{
								float posX[MAX_USERS], posY[MAX_USERS];
							
								mUserTracker.convertJointCoordinatesToDepth(
									nHandL.getPosition().x,
									nHandL.getPosition().y,
									nHandL.getPosition().z,
									&posX[n[i]], &posY[n[i]]);

								if (abs(int(posX[n[i]] - startPosX[n[i]])) > minPixelTreshold )
									curX[n[i]] += int(((posX[n[i]] - startPosX[n[i]]) - 0) / cursorSpeedDivisor);
								if (abs(int(posY[n[i]] - startPosY[n[i]])) > minPixelTreshold )
									curY[n[i]] += int(((posY[n[i]] - startPosY[n[i]]) - 0) / cursorSpeedDivisor);
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
								#pragma region Start Track Delay Algorithm
								if(!startTrackDelayAlgorithm)
								{	
									mUserTracker.convertJointCoordinatesToDepth(
											nHandR.getPosition().x,
											nHandR.getPosition().y,
											nHandR.getPosition().z,
											&startPosX[n[i]], &startPosY[n[i]]);

									mouseMotion(contact_[n[i]],curX[n[i]],curY[n[i]],isDragged[n[i]]);
								}
								else if(noDelayTimer[n[i]])
								{
									mUserTracker.convertJointCoordinatesToDepth(
											nHandR.getPosition().x,
											nHandR.getPosition().y,
											nHandR.getPosition().z,
											&startPosX[n[i]], &startPosY[n[i]]);

									//printf("Starting Cursor X: %4f | Cursor Y: %4f\n", startPosX[n[i]], startPosY[n[i]]);

									noDelayTimer[n[i]] = false;
									startTimer[n[i]] = false;
									startCapturePosition[n[i]] = false;
									time_counterDelay[n[i]] = 0;

									mouseMotion(contact_[n[i]],curX[n[i]],curY[n[i]],isDragged[n[i]]);
								}
								else if(!startTimer[n[i]])
								{
									startTimer[n[i]] = true;
									last_time[n[i]] = clock();
									mouseMotion(contact_[n[i]],curX[n[i]],curY[n[i]],isDragged[n[i]]);
								}
								else if(startTimer[n[i]])
								{
									if(startCapturePosition[n[i]])
									{
										startTimer[n[i]] = false;
										startCapturePosition[n[i]] = false;

										mUserTracker.convertJointCoordinatesToDepth(
											nHandR.getPosition().x,
											nHandR.getPosition().y,
											nHandR.getPosition().z,
											&startPosX[n[i]], &startPosY[n[i]]);

										//printf("Starting Cursor X: %4f | Cursor Y: %4f\n", startPosX[n[i]], startPosY[n[i]]);	
									}
										mouseMotion(contact_[n[i]],curX[n[i]],curY[n[i]],isDragged[n[i]]);
								}
								#pragma endregion
							}
							else if (startPosX[n[i]] >= 0 && startPosY[n[i]] >= 0)
							{
								float posX[MAX_USERS], posY[MAX_USERS];
							
								mUserTracker.convertJointCoordinatesToDepth(
									nHandR.getPosition().x,
									nHandR.getPosition().y,
									nHandR.getPosition().z,
									&posX[n[i]], &posY[n[i]]);

								if (abs(int(posX[n[i]] - startPosX[n[i]])) > minPixelTreshold )
									curX[n[i]] += int(((posX[n[i]] - startPosX[n[i]]) - 0) / cursorSpeedDivisor);
								if (abs(int(posY[n[i]] - startPosY[n[i]])) > minPixelTreshold )
									curY[n[i]] += int(((posY[n[i]] - startPosY[n[i]]) - 0) / cursorSpeedDivisor);
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
								#pragma region Start Track Delay Algorithm
								if(!startTrackDelayAlgorithm)
								{	
									mUserTracker.convertJointCoordinatesToDepth(
											nHandR.getPosition().x,
											nHandR.getPosition().y,
											nHandR.getPosition().z,
											&startPosX[n[i]], &startPosY[n[i]]);

									mouseMotion(contact_[n[i]],curX[n[i]],curY[n[i]],isDragged[n[i]]);
								}
								else if(noDelayTimer[n[i]])
								{
									mUserTracker.convertJointCoordinatesToDepth(
											nHandR.getPosition().x,
											nHandR.getPosition().y,
											nHandR.getPosition().z,
											&startPosX[n[i]], &startPosY[n[i]]);

									//printf("Starting Cursor X: %4f | Cursor Y: %4f\n", startPosX[n[i]], startPosY[n[i]]);

									noDelayTimer[n[i]] = false;
									startTimer[n[i]] = false;
									startCapturePosition[n[i]] = false;
									time_counterDelay[n[i]] = 0;

									mouseMotion(contact_[n[i]],curX[n[i]],curY[n[i]],isDragged[n[i]]);
								}
								else if(!startTimer[n[i]])
								{
									startTimer[n[i]] = true;
									last_time[n[i]] = clock();
									mouseMotion(contact_[n[i]],curX[n[i]],curY[n[i]],isDragged[n[i]]);
								}
								else if(startTimer[n[i]])
								{
									if(startCapturePosition[n[i]])
									{
										startTimer[n[i]] = false;
										startCapturePosition[n[i]] = false;

										mUserTracker.convertJointCoordinatesToDepth(
											nHandR.getPosition().x,
											nHandR.getPosition().y,
											nHandR.getPosition().z,
											&startPosX[n[i]], &startPosY[n[i]]);

										//printf("Starting Cursor X: %4f | Cursor Y: %4f\n", startPosX[n[i]], startPosY[n[i]]);	
									}
										mouseMotion(contact_[n[i]],curX[n[i]],curY[n[i]],isDragged[n[i]]);
								}
								#pragma endregion
							}
							else if (startPosX[n[i]] >= 0 && startPosY[n[i]] >= 0)
							{
								float posX[MAX_USERS], posY[MAX_USERS];
							
								mUserTracker.convertJointCoordinatesToDepth(
									nHandL.getPosition().x,
									nHandL.getPosition().y,
									nHandL.getPosition().z,
									&posX[n[i]], &posY[n[i]]);

								if (abs(int(posX[n[i]] - startPosX[n[i]])) > minPixelTreshold )
									curX[n[i]] += int(((posX[n[i]] - startPosX[n[i]]) - 0) / cursorSpeedDivisor);
								if (abs(int(posY[n[i]] - startPosY[n[i]])) > minPixelTreshold )
									curY[n[i]] += int(((posY[n[i]] - startPosY[n[i]]) - 0) / cursorSpeedDivisor);
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
					if(profileAlgorithm)
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
		#pragma region Recalibration Algorithm 1
		if(recalibrationAlgorithm1)
		{	
			#pragma region Recalibration Algorithm 1 - User 1
			if((tempCurX[0] == curX[0]) && (tempCurY[0] == curY[0]))
			{
				if(timeOut1)
				{
					timeOutCounter1++;
					
					if(timeOutCounter1 >= maxTimeOut)
					{
						noDelayTimer[0] = true;
						startPosX[0] = -1;
						startPosY[0] = -1;
						curX[0] = desktopResolution.right * 1/4;
						curY[0] = desktopResolution.bottom/2;

						timeOutCounter1 = 0;
					}
					else
						startAlgoTimer1 = true;
						last_time7 = clock();

					timeOut1 = false;
				}
			}
			else if(((cursorLimitLeft <= curX[0]) && (curX[0] <= cursorLimitLeft + rAlgoRange)) ||
					((cursorLimitTop <= curY[0]) && (curY[0] <= cursorLimitTop + rAlgoRange)) ||
					((cursorLimitRight - rAlgoRange <= curX[0]) && (curX[0] <= cursorLimitRight)) ||
					((cursorLimitBottom - rAlgoRange <= curY[0]) && (curY[0] <= cursorLimitBottom)))
			{
				tempCurX[0] = curX[0];
				tempCurY[0] = curY[0];

				startAlgoTimer1 = true;
				last_time7 = clock();

				time_counter7 = 0;
				timeOutCounter1 = 0;
				timeOut1 = false;
			}
			else if((tempCurX[0] != -1 && tempCurY[0] != -1) && (tempCurX[0] != curX[0] && tempCurY[0] != curY[0]))
			{
				tempCurX[0] = -1;
				tempCurY[0] = -1; 

				time_counter7 = 0;
				timeOutCounter1 = 0;
				timeOut1 = false;
			}
			#pragma endregion
			#pragma region Recalibration Algorithm 1 - User 2
			if((tempCurX[1] == curX[1]) && (tempCurY[1] == curY[1]))
			{
				if(timeOut2)
				{
					timeOutCounter2++;
					
					if(timeOutCounter2 >= maxTimeOut)
					{
						noDelayTimer[1] = true;
						startPosX[1] = -1;
						startPosY[1] = -1;
						curX[1] = desktopResolution.right * 3/4;
						curY[1] = desktopResolution.bottom/2;

						timeOutCounter2 = 0;
					}
					else
						startAlgoTimer2 = true;
						last_time8 = clock();

					timeOut2 = false;
				}
			}
			else if(((cursorLimitLeft <= curX[1]) && (curX[1] <= cursorLimitLeft + rAlgoRange)) ||
					((cursorLimitTop <= curY[1]) && (curY[1] <= cursorLimitTop + rAlgoRange)) ||
					((cursorLimitRight - rAlgoRange <= curX[1]) && (curX[1] <= cursorLimitRight)) ||
					((cursorLimitBottom - rAlgoRange <= curY[1]) && (curY[1] <= cursorLimitBottom)))
			{
				
				tempCurX[1] = curX[1];
				tempCurY[1] = curY[1];

				startAlgoTimer2 = true;
				last_time8 = clock();

				time_counter8 = 0;
				timeOutCounter2 = 0;
				timeOut2 = false;
			}
			else if((tempCurX[1] != -1 && tempCurY[1] != -1) && (tempCurX[1] != curX[1] && tempCurY[1] != curY[1]))
			{
				tempCurX[1] = -1;
				tempCurY[1] = -1; 

				time_counter8 = 0;
				timeOutCounter2 = 0;
				timeOut2 = false;
			}
			#pragma endregion
		}
		#pragma endregion
		//-----------------------------------KEYBOARD KEYS -----------------------------------//
		if (GetAsyncKeyState(0x31)) //Key '1'
		{
			noDelayTimer[0] = true;
			startPosX[0] = -1;
			startPosY[0] = -1;
			curX[0] = desktopResolution.right * 1/4;
			curY[0] = desktopResolution.bottom/2;
		}
		if (GetAsyncKeyState(0x32)) //Key '2'
		{
			noDelayTimer[1] = true;
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
				if(continueToPaint)
				{
					system("taskkill /im mspaint.exe");
					system("taskkill /f /im FPSIWBGraphics3.exe");
				}
				std::cout << "..Exiting the program" << std::endl;
				break;
			}
        }
		#pragma region Speedtest Function
		if(testMode)
		{
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
		#pragma endregion 
	}
	//-----------------------------------LOOP END-----------------------------------//
	DeleteObject(hRedBrush);
	DeleteObject(hGreenBrush);
	DeleteObject(hBlueBrush);
	DeleteObject(hCyanBrush);
	DeleteObject(hYellowBrush);
	DeleteObject(hMagentaBrush);
	DeleteObject(hGrayPen);

	DeleteDC(dcText);
	DeleteDC(dcPopText1);
	DeleteDC(dcPopText2);
	DeleteDC(dcPopText3);
	DeleteDC(dcPopText4);
	DeleteDC(dcRed);
	DeleteDC(dcGreen);
	DeleteDC(dcBlue);
	DeleteDC(dcCyan);
	DeleteDC(dcYellow);
	DeleteDC(dcMagenta);
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
