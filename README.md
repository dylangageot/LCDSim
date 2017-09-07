# LCDSim

LCDSim is a library written in C that allow you to emulate a HD44780 LCD 16x2 display on your computer. The display function is possible with the use of the SDL (with some extra lib that allow loading of image).

## Code example

'''c
#include "lcdsim.h"

int main (int argc, char** argv) {

    // Declaration of the variables
    SDL_Event event;
    SDL_Surface *screen = NULL;
    LCDSim *lcd = NULL;
    Uint8 hold = 1;

    // Initialization of the SDL
    SDL_Init(SDL_INIT_VIDEO);
    screen = SDL_SetVideoMode(331, 149, 32, SDL_HWSURFACE | SDL_DOUBLEBUF);
    SDL_WM_SetCaption("LCD 16x2", NULL);

    // Initialize the lcd variable
    lcd = LCDSim_Create(screen, 0, 0);

    LCD_State(lcd, 1, 1, 1);    // Display ON, Cursor ON, Blink ON
    LCD_SetCursor(lcd, 0, 3);   // Set the cursor position to 1st line, 4th column
    LCD_PutS(lcd, "Hello,");    // Display the text "Hello,"
    LCD_SetCursor(lcd, 1, 5);   // Set the cursor position to 2nd line, 6th column
    LCD_PutS(lcd, "GitHub!");   // Display the text "GitHub!"

    // Run the program until the close button hasn't been pressed
    while (hold) {
        SDL_PollEvent(&event);
        switch (event.type) {
            case SDL_QUIT:
                hold = 0;
                break;
            default:
                break;
        }
        // Draw the LCD on window
        LCDSim_Draw(lcd);
        // Refresh the window at 50 FPS
        SDL_Flip(screen);
        SDL_Delay(20);
    }

    // Free the memory
    lcd = LCDSim_Destroy(lcd);
    SDL_FreeSurface(screen);
    SDL_Quit();

    return EXIT_SUCCESS;

}
'''
