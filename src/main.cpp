#define Log(x) std::cout << x << std::endl

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <iostream>
#include <cstring>
#include <stdint.h>

// Draw the data into the frame buffer
int Draw(uint32_t* framebuffer, XWindowAttributes winAttrs) {
	int height = winAttrs.height;
	int width = winAttrs.width;
	// Just a sample
	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			uint8_t r = x * 255 / width;
			uint8_t g = y * 255 / height;
			uint8_t b = 128;
			
			framebuffer[y * width + x] = (r << 16) | (g << 8) | b;
		}
	}

	return 0;
}

// Make a window
int CreateWindow(Display** mainDisplay, Window* mainWindow) {
	*mainDisplay = XOpenDisplay(0); 
	if (!*mainDisplay) {
		Log("Could not open display");
		return 1;
	}

	// Get the desktop manager for that display as windows need a hierarchy
	Window rootWindow = XDefaultRootWindow(*mainDisplay);

	// Window Paramaters
	int windowX = 0;
	int windowY = 0;
	int windowWidth = 800;
	int windowHeight = 600;
	int borderWidth = 0;
	int windowDepth = CopyFromParent; // Color Depth
	int windowClass = InputOutput; // Allow input (User interaction) and output (UI Display)
	Visual* windowVisual = CopyFromParent;
	int attributeValueMask = CWBackPixel;
	
	XSetWindowAttributes windowAttributes = {};
	windowAttributes.background_pixel = 0x00000000; // Hex code of background color

	// Make the new window
	*mainWindow = XCreateWindow( 
		*mainDisplay, rootWindow, windowX, windowY, 
		windowWidth, windowHeight, borderWidth,
		windowDepth, windowClass, windowVisual, 
		attributeValueMask, &windowAttributes
	);

	Log("Finished CreateWindow");

	return 0;
}

int main() {
	Log("Running");

	Display* mainDisplay;
	Window mainWindow; 
	int result = CreateWindow(&mainDisplay, &mainWindow);
	if (result != 0) {
		Log("Failed CreateWindow");
		return 1;
	}

	// Setup what user inputs will be handled for the window
	Log("DefaultScreen");
	int screen = DefaultScreen(mainDisplay);
	Log("XSelectInput");
	XSelectInput(mainDisplay, mainWindow, ExposureMask | KeyPressMask);


	// Show the window to users
	Log("XMapWindow");
	XMapWindow(mainDisplay, mainWindow); 


	Log("Default GC");
	GC gc = DefaultGC(mainDisplay, screen);


	// Setup the frame buffer
	XWindowAttributes windowAttrs;
	Log("XGetWindowAttributes");
	XGetWindowAttributes(mainDisplay, mainWindow, &windowAttrs);
	
	// Array of uint32_t, each uint32_t is 4 bytes (ARGB)
	uint32_t* framebuffer = new uint32_t[windowAttrs.height * windowAttrs.width];

	Log("XCreateImage");
	XImage* image = XCreateImage(
		mainDisplay, 
		DefaultVisual(mainDisplay, screen),
		DefaultDepth(mainDisplay, screen),
		ZPixmap, // format
		0,
		reinterpret_cast<char*>(framebuffer),
		windowAttrs.width,
		windowAttrs.height,
		32, // Color Depth (32 bits each pixel)
		0
	);


	Log("Starting Loop");

	bool running = true;

	while(running) {
		XEvent generalEvent = {};
		XNextEvent(mainDisplay, &generalEvent); // wait for the next event

		switch(generalEvent.type) {
			case Expose:
				result = Draw(framebuffer, windowAttrs);
				if (result != 0) {
					return 1;
				}

				image->width = windowAttrs.width;
				image->height = windowAttrs.height;

				XPutImage(
					mainDisplay, mainWindow, gc, 
					image, 0, 0, 0, 0, 
					windowAttrs.width, windowAttrs.height
				);

				break;

			case KeyPress: {
				XKeyPressedEvent *event = (XKeyPressedEvent *)&generalEvent;
				if(event->keycode == XKeysymToKeycode(mainDisplay, XK_Escape)) {
					running = false;
				}
			} break;
				
		}

		

		XFlush(mainDisplay);
	}
	Log("Finished Loop");

	image->data = nullptr; // Don't free frame buffer
	XDestroyImage(image);
	delete[] framebuffer;

	XCloseDisplay(mainDisplay);

	Log("Done");

	return 0;
}
