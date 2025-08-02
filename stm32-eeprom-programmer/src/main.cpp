#include "stm32f1xx_hal.h"

int main(void)
{
  // Enable GPIO C clock directly
  RCC->APB2ENR |= RCC_APB2ENR_IOPCEN;

  // Configure PC13 as output (push-pull, 2MHz)
  GPIOC->CRH &= ~(0xF << 20); // Clear bits 20-23
  GPIOC->CRH |= (0x2 << 20);  // Set to output push-pull, 2MHz

  // Initialize LED to OFF state
  GPIOC->BSRR = GPIO_BSRR_BR13;

  // Simple delay function
  volatile uint32_t delay_count;

  // Simple LED test - just toggle continuously
  while (1)
  {
    // LED ON
    GPIOC->BSRR = GPIO_BSRR_BS13;

    // Simple delay loop
    for (delay_count = 0; delay_count < 500000; delay_count++)
    {
    }

    // LED OFF
    GPIOC->BSRR = GPIO_BSRR_BR13;

    // Simple delay loop
    for (delay_count = 0; delay_count < 500000; delay_count++)
    {
    }
  }
}