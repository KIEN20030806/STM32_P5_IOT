#ifndef P5_H
#define P5_H
#include "stm32f4xx_hal.h"
#include "stdio.h"
#include "P5.h"

// Pin definitions
#define R1_PIN 2
#define G1_PIN 3
#define B1_PIN 4
#define R2_PIN 5
#define G2_PIN 6
#define B2_PIN 7

#define A_PIN 1
#define B_PIN 14
#define C_PIN 10
#define D_PIN 13

#define LAT_PIN 12
#define OE_PIN 1
#define CLK_PIN 0

#define A_SSET 1<<A_PIN
#define B_SSET 1<<B_PIN
#define C_SSET 1<<C_PIN
#define D_SSET 1<<D_PIN
#define A_RSET 1<<(A_PIN+16)
#define B_RSET 1<<(B_PIN+16)
#define C_RSET 1<<(C_PIN+16)
#define D_RSET 1<<(D_PIN+16)

#define OE_P       GPIOA
#define xuat_P     GPIOB
#define clk_P      GPIOB
#define Control_P  GPIOB
#define data_PORT  GPIOA

// Color definitions
#define BLACK     0x00
#define WHITE     0x07
#define RED       0x01
#define GREEN     0x02
#define BLUE      0x04
#define MAGENTA   0x05
#define YELLOW    0x03
#define CYAN      0x06

#define R_MASK      0x01
#define G_MASK      0x02
#define B_MASK      0x04

// Graphic dimensions
#define Graphic_x 64
#define Graphic_y 32


void ngatquetled();
void Matrix_chonvitri(int x,int y);
void Matrix_setpx(int32_t x,int32_t y,unsigned char color);
void Matrix_guikitu(unsigned char txt,unsigned char color);
void Matrix_guichuoi(const char *s,unsigned char color);
void Matrix_showIcon(int32_t x, int32_t y, int32_t k, unsigned char color);
void Matrix_clearArea(int32_t x, int32_t y, int32_t width, int32_t height) ;
void Matrix_guichuoi_MAKE_COLOR(unsigned char *s,unsigned char *color);
#endif // P5_H
