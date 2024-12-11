#include "LPC17xx.h"
#include "joystick.h"
#include "GLCD.h"
#define __FI 1 /* Font index 16x24                  */

uint8_t wave_drive_lookup_table[4][4] = {
    {1, 0, 0, 0}, // Wave drive anti-clockwise sequence
    {0, 1, 0, 0},
    {0, 0, 1, 0},
    {0, 0, 0, 1}};
uint8_t half_step_lookup_table[8][4] = {
    {1, 0, 0, 0}, // Half-step anti-clockwise sequence
    {1, 1, 0, 0},
    {0, 1, 0, 0},
    {0, 1, 1, 0},
    {0, 0, 1, 0},
    {0, 0, 1, 1},
    {0, 0, 0, 1},
    {0, 1, 0, 1}};
uint8_t full_step_lookup_table[4][4] = {
    {1, 1, 0, 0}, // Full-step anti-clockwise sequence
    {0, 1, 1, 0},
    {0, 0, 1, 1},
    {1, 0, 0, 1}};

uint8_t (*lookup_table)[4] = full_step_lookup_table; // Default to full-step
uint8_t lookup_table_size = 4;                       // Default size for full-step

void set_outputs(int index)
{
  // drive the four main outputs that control the stepper;
  // use the values from the look up table;

  if (lookup_table[index][0] == 0)
  {
    LPC_GPIO0->FIOCLR |= 1 << 0;
  }
  else
  {
    LPC_GPIO0->FIOSET |= 1 << 0;
  }
  if (lookup_table[index][1] == 0)
  {
    LPC_GPIO0->FIOCLR |= 1 << 1;
  }
  else
  {
    LPC_GPIO0->FIOSET |= 1 << 1;
  }
  if (lookup_table[index][2] == 0)
  {
    LPC_GPIO0->FIOCLR |= 1 << 2;
  }
  else
  {
    LPC_GPIO0->FIOSET |= 1 << 2;
  }
  if (lookup_table[index][3] == 0)
  {
    LPC_GPIO0->FIOCLR |= 1 << 3;
  }
  else
  {
    LPC_GPIO0->FIOSET |= 1 << 3;
  }
}

void GLCD_for_stepper(uint32_t index)
{
  GLCD_SetBackColor(White);
  GLCD_SetTextColor(Purple);
  for (int i = 0; i < 4; i++)
  {
    GLCD_DisplayChar(5, 7 + i, __FI, 0x80 + lookup_table[index][i]);
  }
}

volatile uint32_t temp;
void delay_dirty(uint32_t del)
{
  uint32_t i;
  for (i = 0; i < del; i++)
  {
    temp = i;
  }
}

void change_mode(uint32_t joy_keys)
{
  static uint8_t mode = 0; // 0: full-step, 1: half-step, 2: wave-drive

  if (joy_keys & JOYSTICK_UP)
  {
    mode = (mode + 1) % 3;
  }
  else if (joy_keys & JOYSTICK_DOWN)
  {
    mode = (mode + 2) % 3; // Equivalent to (mode - 1) % 3 but handles underflow
  }

  switch (mode)
  {
  case 0:
    lookup_table = full_step_lookup_table;
    lookup_table_size = 4;
    GLCD_DisplayString(3, 0, __FI, "Mode: Full-Step   ");
    break;
  case 1:
    lookup_table = half_step_lookup_table;
    lookup_table_size = 8;
    GLCD_DisplayString(3, 0, __FI, "Mode: Half-Step   ");
    break;
  case 2:
    lookup_table = wave_drive_lookup_table;
    lookup_table_size = 4;
    GLCD_DisplayString(3, 0, __FI, "Mode: Wave-Drive  ");
    break;
  }
}

int main(void)
{
  uint32_t joy_keys;
  uint32_t MY_DELAY = (1 << 14);
  uint32_t i;
  uint32_t k;
  // (1) Power up GPIO
  LPC_SC->PCONP |= (1 << 15);

  LPC_GPIO4->FIODIR |= (1UL << 28);  // Pin P2.28 is output (GLCD BAcklight)
  LPC_GPIO4->FIOPIN &= ~(1UL << 28); // Turn backlight off

  // (2) use P0.0..3 as the four signals to drive the stepper motor;
  LPC_GPIO0->FIODIR |= (1 << 0) | (1 << 1) | (1 << 2) | (1 << 3);
  LPC_GPIO0->FIOCLR |= 1 << 0;
  LPC_GPIO0->FIOCLR |= 1 << 1;
  LPC_GPIO0->FIOCLR |= 1 << 2;
  LPC_GPIO0->FIOCLR |= 1 << 3;
  GLCD_Init();       /* Initialize the GLCD           */
  GLCD_Clear(White); /* Clear the GLCD                */
  GLCD_SetBackColor(Blue);
  GLCD_SetTextColor(White);
  GLCD_DisplayString(0, 0, __FI, "  Lab 5 Assignment  ");
  GLCD_DisplayString(1, 0, __FI, "Full stepper + GLCD ");
  GLCD_DisplayString(2, 0, __FI, "  Embedded lab IU   ");
  GLCD_SetBackColor(Cyan);
  GLCD_SetTextColor(Black);
  GLCD_DisplayString(4, 7, __FI, "A");
  GLCD_SetBackColor(Green);
  GLCD_SetTextColor(Black);
  GLCD_DisplayString(4, 8, __FI, "B");
  GLCD_SetBackColor(Yellow);
  GLCD_SetTextColor(Black);
  GLCD_DisplayString(4, 9, __FI, "C");
  GLCD_SetBackColor(Red);
  GLCD_SetTextColor(Black);
  GLCD_DisplayString(4, 10, __FI, "D");
  // (3) infinite loop;
  while (1)
  {
    // (a) read joystick keys;
    joy_keys = JOYSTICK_GetKeys();

    // (b) change mode if needed;
    change_mode(joy_keys);

    // (c) drive the stepper motor;
    if (joy_keys & JOYSTICK_LEFT)
    {
      for (i = lookup_table_size; i > 0; i--)
      {
        set_outputs(i - 1);
        GLCD_for_stepper(i - 1);
        delay_dirty(MY_DELAY);
      }
    }
    else if (joy_keys & JOYSTICK_RIGHT)
    {
      for (i = 0; i < lookup_table_size; i++)
      {
        set_outputs(i);
        GLCD_for_stepper(i);
        delay_dirty(MY_DELAY);
      }
    }
  }

  return 0;
}