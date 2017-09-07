#include <stdio.h>
#include <stdlib.h>
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include "lcdsim.h"

LCDSim* LCDSim_Create(SDL_Surface *screen, int x, int y) {

    LCDSim* self = malloc(sizeof(LCDSim));
    if (self != NULL) {
        HD44780_Init(&self->mcu);
        GraphicUnit_Init(&self->gu);
        self->gu.screen = screen;
        self->gu.on_screen.x = x;
        self->gu.on_screen.y = y;
        self->lastTime = SDL_GetTicks();
    }
    return self;

}

void LCDSim_Draw(LCDSim *self) {

    Uint32 nowTime = SDL_GetTicks();
    SDL_BlitSurface(self->gu.image, NULL, self->gu.temp_screen, &self->gu.position);
    if (nowTime - self->lastTime > 500) {
        self->mcu.LCD_CursorState = !self->mcu.LCD_CursorState;
        self->lastTime = nowTime;
    }
    Pixel_Refresh(self->mcu, self->gu.pixel);
    Pixel_Draw(&self->gu);
    SDL_BlitSurface(self->gu.temp_screen, NULL, self->gu.screen, &self->gu.on_screen);

}

void LCDSim_Instruction(LCDSim *self, Uint16 instruction) {

    Uint8 i, n, m;
    if (instruction & 0x0200) {
        if (self->mcu.RAM_current == CGR) {
            n = self->mcu.CGRAM_counter / 8;
            m = self->mcu.CGRAM_counter % 8;
            self->mcu.CGROM[n][m] = instruction & 0xFF;
            if (self->mcu.CGRAM_counter < 64)
                self->mcu.CGRAM_counter++;
        } else {
            self->mcu.DDRAM[self->mcu.DDRAM_counter] = instruction & 0xFF;
            if (self->mcu.LCD_EntryMode & 0x02) {
                if (self->mcu.DDRAM_counter < 104) {
                    if (self->mcu.DDRAM_counter == 0x27)
                        self->mcu.DDRAM_counter = 0x40;
                    else
                        self->mcu.DDRAM_counter++;
                }
                if (self->mcu.LCD_EntryMode & 0x01) {
                    if (self->mcu.DDRAM_display < 24)
                        self->mcu.DDRAM_display++;
                }
            } else {
                if (self->mcu.DDRAM_counter > 0) {
                    if (self->mcu.DDRAM_counter == 0x40)
                        self->mcu.DDRAM_counter = 0x27;
                    else
                        self->mcu.DDRAM_counter--;
                }
                if (self->mcu.LCD_EntryMode & 0x01) {
                    if (self->mcu.DDRAM_display > 0)
                        self->mcu.DDRAM_display--;
                }
            }
        }
    } else {
        for (i = 0; i < 8; i++)
            if (instruction & (0x80 >> i))
                break;
        switch (i) {
            // SET DDRAM ADDRESS
            case 0:
                self->mcu.DDRAM_counter = instruction & 0x7F;
                self->mcu.RAM_current = DDR;
                break;
            // SET CGRAM ADDRESS
            case 1:
                self->mcu.CGRAM_counter = instruction & 0x3F;
                self->mcu.RAM_current = CGR;
                break;
            // CURSOR/DISPLAY SHIFT
            case 3:
                if (instruction & 0x08) {
                    if (instruction & 0x04) {
                        if (self->mcu.DDRAM_display < 24)
                            self->mcu.DDRAM_display++;
                    } else {
                        if (self->mcu.DDRAM_display > 0)
                            self->mcu.DDRAM_display--;
                    }
                } else {
                    if (instruction & 0x04) {
                        if (self->mcu.DDRAM_counter < 104) {
                            if (self->mcu.DDRAM_counter == 0x27)
                                self->mcu.DDRAM_counter = 0x40;
                            else
                                self->mcu.DDRAM_counter++;
                        }
                    } else {
                        if (self->mcu.DDRAM_counter > 0) {
                            if (self->mcu.DDRAM_counter == 0x40)
                                self->mcu.DDRAM_counter = 0x27;
                            else
                                self->mcu.DDRAM_counter--;
                        }
                    }
                }
                break;
            // DISPLAY ON/OFF CONTROL
            case 4:
                self->mcu.LCD_CursorBlink = instruction & 0x01;
                self->mcu.LCD_CursorEnable = (instruction & 0x02) >> 1;
                self->mcu.LCD_DisplayEnable = (instruction & 0x04) >> 2;
                self->mcu.LCD_CursorState = 0;
                break;
            // ENTRY MODE SET
            case 5:
                self->mcu.LCD_EntryMode = instruction & 0x03;
                break;
            // HOME
            case 6:
                self->mcu.DDRAM_counter = 0;
                self->mcu.DDRAM_display = 0;
                break;
            // CLEAR
            case 7:
                for (i = 0; i < 80; i++)
                    self->mcu.DDRAM[i] = 0x20;
                self->mcu.DDRAM_counter = 0;
                break;
        }
    }

}

LCDSim* LCDSim_Destroy(LCDSim *self) {

    SDL_FreeSurface(self->gu.color[0]);
    SDL_FreeSurface(self->gu.color[1]);
    SDL_FreeSurface(self->gu.image);
    SDL_FreeSurface(self->gu.temp_screen);
    free(self);
    return NULL;

}

void HD44780_Init(HD44780 *self) {

    // Init counter
    self->CGRAM_counter = 0;
    self->DDRAM_counter = 0;
    self->DDRAM_display = 0;
    self->LCD_EntryMode = 0x02;
    self->LCD_CursorEnable = 0;
    self->LCD_CursorBlink = FIXED;
    self->LCD_CursorState = 0;
    self->LCD_DisplayEnable = 1;
    self->RAM_current = DDR;

    // Init CGROM
    Uint16 i;
    Uint8 j, cgrom_array[1152];
    FILE *cgrom = fopen("cgrom.bin", "rb");
    fread(cgrom_array, sizeof(cgrom_array), 1, cgrom);
    for (i = 0; i < 1152; i += 9) {
        if (cgrom_array[i] != 0) {
            for (j = 0; j < 8; j++)
                self->CGROM[cgrom_array[i]][j] = cgrom_array[i+j+1];
        } else break;
    }
    fclose(cgrom);

    // Init DDRAM
    for (i = 0; i < 104; i++)
        self->DDRAM[i] = 0x20;

}

void GraphicUnit_Init(GraphicUnit *self) {

    Uint8 i;
    self->position.x = 0;
    self->position.y = 0;

    for (i = 0; i < 2; i++)
        self->color[i] = SDL_CreateRGBSurface(SDL_HWSURFACE,PIXEL_DIM,PIXEL_DIM, 32, 0, 0, 0, 0);

    SDL_FillRect(self->color[0],NULL,SDL_MapRGB(self->color[0]->format, 0, 0, 0));
    SDL_FillRect(self->color[1],NULL,SDL_MapRGB(self->color[1]->format, 125, 159, 50));
    self->temp_screen = SDL_CreateRGBSurface(SDL_HWSURFACE, 331, 149, 32, 0, 0, 0, 0);
    self->image = IMG_Load("lcd_layout.png");
    Pixel_Init(self->pixel);

}

void Pixel_Init(Pixel pixel[][CASE_WIDTH][CASE_HEIGHT]) {

    Uint8 z, x , y;
    for (z = 0; z < 32; z++) {
        for(x = 0 ; x < CASE_WIDTH; x++) {
            for(y = 0 ; y < CASE_HEIGHT; y++) {
                pixel[z][x][y].position.x = OFFSET_X + (z % 16) * 16 + x*PIXEL_DIM;
                pixel[z][x][y].position.y = OFFSET_Y + (z >= 16) * 25 + y*PIXEL_DIM;
                pixel[z][x][y].color = GREEN;
            }
        }
    }

}

void Pixel_Refresh(HD44780 mcu, Pixel pixel[][CASE_WIDTH][CASE_HEIGHT]) {

    Uint8 z, x ,y, n;
    for (z = 0; z < 32; z++) {
        for (x = 0; x < CASE_WIDTH; x++) {
            for (y = 0; y < CASE_HEIGHT; y++) {
                if (!mcu.LCD_DisplayEnable)
                    pixel[z][x][y].color = GREEN;
                else {
                    n = mcu.DDRAM_display + z%16 + (z >= 16) * 0x40;
                    if ((mcu.CGROM[mcu.DDRAM[n]][y] >> (CASE_WIDTH - 1 - x)) & 0x01)
                        pixel[z][x][y].color = BLACK;
                    else
                        pixel[z][x][y].color = GREEN;
                }
            }
        }
    }
    if ((mcu.LCD_CursorState || mcu.LCD_CursorBlink == FIXED) && mcu.LCD_DisplayEnable && mcu.LCD_CursorEnable) {
        if ((mcu.DDRAM_display <= mcu.DDRAM_counter) && ((mcu.DDRAM_display + 0x0F) >= mcu.DDRAM_counter)) {
            for (x = 0; x < CASE_WIDTH; x++)
                for (y = 0; y < CASE_HEIGHT; y++)
                    pixel[mcu.DDRAM_counter - mcu.DDRAM_display][x][y].color = BLACK;
        }
        if ((0x40 + mcu.DDRAM_display <= mcu.DDRAM_counter) && ((mcu.DDRAM_display + 0x4F) >= mcu.DDRAM_counter)) {
            for (x = 0; x < CASE_WIDTH; x++)
                for (y = 0; y < CASE_HEIGHT; y++)
                    pixel[16 + mcu.DDRAM_counter - mcu.DDRAM_display - 0x40][x][y].color = BLACK;
        }
    }

}

void Pixel_Draw(GraphicUnit *self) {

    Uint8 z, x, y;
    for (z = 0; z < 32; z++) {
        for (x = 0; x < CASE_WIDTH; x++) {
            for (y = 0; y < CASE_HEIGHT; y++) {
                SDL_BlitSurface(self->color[self->pixel[z][x][y].color], NULL, self->temp_screen, &self->pixel[z][x][y].position);
            }
        }
    }

}

void LCD_PutChar(LCDSim *self, char car) {
	LCDSim_Instruction(self, 0x200 | car);
}

void LCD_PutS(LCDSim *self, char *s) {
	while (*s)
		LCD_PutChar(self, *s++);
}

void LCD_Clear(LCDSim *self) {
	LCDSim_Instruction(self, CLEAR_DISPLAY);
}

void LCD_Home(LCDSim *self) {
	LCDSim_Instruction(self, LCD_HOME);
}

void LCD_State(LCDSim *self, Uint8 display_enable, Uint8 cursor_enable, Uint8 blink) {
    LCDSim_Instruction(self, 0x08 | (display_enable << 2) | (cursor_enable << 1) | blink);
}

void LCD_Sh_Cursor_R(LCDSim *self) {
	LCDSim_Instruction(self, SHIFT_CURSOR_RIGHT);
}

void LCD_Sh_Cursor_L(LCDSim *self) {
   LCDSim_Instruction(self, SHIFT_CURSOR_LEFT);
}

void LCD_Sh_Display_R(LCDSim *self) {
   LCDSim_Instruction(self, SHIFT_DISPLAY_RIGHT);
}

void LCD_Sh_Display_L(LCDSim *self) {
    LCDSim_Instruction(self, SHIFT_DISPLAY_LEFT);
}

void LCD_ClearLine(LCDSim *self, Uint8 line) {

    Uint8 i;
    if ((line == 0) || (line == 1)) {
        LCD_SetCursor(self, line, 0);
        for (i = 0; i < 40; i++)
        LCD_PutChar(self, ' ');
        LCD_SetCursor(self, line, 0);
    }

}

void LCD_SetCursor(LCDSim *self, Uint8 line, Uint8 column) {

	Uint8 pos;
	if ((column > 15) || (line > 1))
        return;
	pos = (line * 0x40) + column;
	LCDSim_Instruction(self, SET_DDRAM_AD | pos);

}

void LCD_CustomChar(LCDSim *self, Uint8 char_number, Uint8* custom) {

	Uint8 i;
	if(char_number < 0 && char_number > 7)
		return;
	LCDSim_Instruction(self, SET_CGRAM_AD | char_number * 0x08);
	for(i = 0; i < 8; i++)
		LCD_PutChar(self, custom[i]);
	LCDSim_Instruction(self, SET_DDRAM_AD);

}
