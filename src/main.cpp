#include <unistd.h>
#include <X11/Xlib.h>
#include <iostream>

int main() {
	std::cout << "Running" << std::endl;

	// Just make a basic window

	// Get the default display ($DISPLAY)
	Display* MainDisplay = XOpenDisplay(0); 
	// Get the desktop manager for that display as windows need a hierarchy
	Window RootWindow = XDefaultRootWindow(MainDisplay); 

	// Make the new window
	Window MainWindow = XCreateSimpleWindow( 
		MainDisplay, // Display
		RootWindow, // Parent
		0, 0, // X and Y
		800, 600, // Width and Height
		0, // Border Width
		0x00000000, // Border Color in Hex (the last six bits determin the color)
		0x00000000 // Background Color in Hex
	);

	// Show the window to users
	XMapWindow(MainDisplay, MainWindow); 

	// All of those commands were added to a buffer 
	// to be immediatly ran after one another,
	// flush that buffer
	XFlush(MainDisplay);

	// As soon as the program ends the window closes, so infinite loop to keep it open
	for(;;) { sleep(1); }
}
