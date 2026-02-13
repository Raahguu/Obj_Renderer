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

	// The main display the window is on
	Display* mainDisplay;
	// The actual window item we are rendering to
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
	XSelectInput(mainDisplay, mainWindow, ExposureMask | StructureNotifyMask | KeyPressMask);


	// Show the window to users
	Log("XMapWindow");
	XMapWindow(mainDisplay, mainWindow); 


	Log("Default GC");
	// The display's garbage collector
	GC gc = DefaultGC(mainDisplay, screen);


	// Setup the frame buffer

	// the attributes of the window
	XWindowAttributes windowAttrs;
	Log("XGetWindowAttributes");
	XGetWindowAttributes(mainDisplay, mainWindow, &windowAttrs);
	
	// Array of uint32_t, each uint32_t is 4 bytes (ARGB)
	uint32_t* framebuffer = new uint32_t[windowAttrs.height * windowAttrs.width];

	Log("XCreateImage");
	// The image that is being drawn onto screen
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

	// Is the window running or not
	bool running = true;

	while(running) {
		XEvent generalEvent = {};
		XNextEvent(mainDisplay, &generalEvent); // wait for the next event

		switch(generalEvent.type) {
			// Attributes of the window changed
			case ConfigureNotify: {
				XConfigureEvent* cfg = (XConfigureEvent*)&generalEvent;
				
				// If the size has not changed
				if (cfg->width == windowAttrs.width && cfg->height == windowAttrs.height) break;

				// If the size has changed
				// Change winAttrs dimensions
				windowAttrs.width = cfg->width;
				windowAttrs.height = cfg->height;

				// Destroy old
				if (framebuffer) {
					delete[] framebuffer;
				}
				if (image) {
					image->data = nullptr;
					XDestroyImage(image);
				}

				// Allocate new
				framebuffer = new uint32_t[windowAttrs.width * windowAttrs.height];

				// Update XImage
				image = XCreateImage(
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
			}

			// Window needs to be redrawn
			case Expose:
				result = Draw(framebuffer, windowAttrs);
				if (result != 0) {
					return 1;
				}

				XPutImage(
					mainDisplay, mainWindow, gc, 
					image, 0, 0, 0, 0, 
					windowAttrs.width, windowAttrs.height
				);

				break;
			
			// User pressed a key down
			case KeyPress: {
				XKeyPressedEvent* event = (XKeyPressedEvent*)&generalEvent;
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
