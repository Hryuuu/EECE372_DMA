/*
Name: Hanseo Ryu
Date: 2026-4-2
Description: DMA memory-to-memory transfer example on MSPM0C1104, Displaying values using four LEDs.
*/
#include <stdint.h>

#define PIN_22_MASK     (1u<<22)
#define PIN_16_MASK     (1u<<16)
#define PIN_26_MASK     (1u<<26)
#define PIN_27_MASK     (1u<<27)
#define PIN_24_MASK     (1u<<24)
#define PIN_28_MASK     (1u<<28)

#define LED_0 PIN_26_MASK
#define LED_1 PIN_27_MASK
#define LED_2 PIN_24_MASK
#define LED_3 PIN_28_MASK

//Base address of peripherals (Datasheet Table 8-5)
#define GPIOA   0x400A0000u
#define IOMUX   0x40428000u
#define DMA     0x4042A000u

//GPIOA registers (TRM Table 10-2)
#define GPIOA_PWREN         *(volatile uint32_t *)(GPIOA+0x800u)
#define GPIOA_DOESET31_0    *(volatile uint32_t *)(GPIOA+0x12D0u)
#define GPIOA_DOUTSET31_0   *(volatile uint32_t *)(GPIOA+0x1290u)
#define GPIOA_DOUTCLR31_0   *(volatile uint32_t *)(GPIOA+0x12A0u)
#define GPIOA_IMASK         *(volatile uint32_t *)(GPIOA+0x1028u)
#define GPIOA_MIS           *(volatile uint32_t *)(GPIOA+0x1038u)
#define GPIOA_ICLR          *(volatile uint32_t *)(GPIOA+0x1048u)
#define GPIOA_POLARITY31_16 *(volatile uint32_t *)(GPIOA+0x13a0u)


// IOMUX Base + 0x04(PINCM offset) + (PINCM_index - 1)*4, PIN22-> PINCM23 (Datasheet Table 6-1) 
#define IOMUX_PINCM23       *(volatile uint32_t *)(IOMUX+0x5Cu) 
#define IOMUX_PINCM17       *(volatile uint32_t *)(IOMUX+0x44u)
#define IOMUX_PINCM27       *(volatile uint32_t *)(IOMUX+0x6Cu)
#define IOMUX_PINCM28       *(volatile uint32_t *)(IOMUX+0x70u)
#define IOMUX_PINCM25       *(volatile uint32_t *)(IOMUX+0x64u)
#define IOMUX_PINCM29       *(volatile uint32_t *)(IOMUX+0x74u)

//ARM Cortex-M0+ Register (TRM Table 3-2)
#define NVIC_ISER           *(volatile uint32_t *)(0xE000E100u)

//DMA channel 0 and interrupt registers (TRM Table 5-8)
#define DMA_IMASK           *(volatile uint32_t *)(DMA+0x1028u)
#define DMA_MIS             *(volatile uint32_t *)(DMA+0x1038u)
#define DMA_ICLR            *(volatile uint32_t *)(DMA+0x1048u)
#define DMA_CH0_DMATCTL     *(volatile uint32_t *)(DMA+0x1110u)
#define DMA_CH0_DMACTL      *(volatile uint32_t *)(DMA+0x1200u)
#define DMA_CH0_DMASA       *(volatile uint32_t *)(DMA+0x1204u)
#define DMA_CH0_DMADA       *(volatile uint32_t *)(DMA+0x1208u)
#define DMA_CH0_DMASZ       *(volatile uint32_t *)(DMA+0x120Cu)

void LED_Controller(uint32_t n);

static void MY_init(void) {
    GPIOA_PWREN = 0x26000001u;          //Enable GPIO Power (TRM Table 10-8)
    IOMUX_PINCM23 = 0x00000081u;        //Configure PA22 as GPIO (TRM Table 9-5)
    IOMUX_PINCM27 = 0x00000081u;
    IOMUX_PINCM28 = 0x00000081u;
    IOMUX_PINCM25 = 0x00000081u;
    IOMUX_PINCM29 = 0x00000081u;
    GPIOA_DOESET31_0 = PIN_22_MASK;     //Enable PA22 as output
    GPIOA_DOUTSET31_0 = PIN_22_MASK;    //Set PA22 output high
    GPIOA_DOESET31_0  = PIN_26_MASK | PIN_27_MASK | PIN_24_MASK | PIN_28_MASK;
    GPIOA_DOUTCLR31_0 = PIN_26_MASK | PIN_27_MASK | PIN_24_MASK | PIN_28_MASK;
    
    //Init User Button
    IOMUX_PINCM17 = 0x00060081u;        //Configure PA16 - INENA, Pullup, PC, PF (TRM Table 9-5)
    GPIOA_POLARITY31_16 &= ~0x00000003u;//Clear polarity field for DIO16
    GPIOA_POLARITY31_16 |=  0x00000002u;//Set falling edge trigger for DIO16
    GPIOA_ICLR = PIN_16_MASK;           //Clear any pending flag
    GPIOA_IMASK |= PIN_16_MASK;         //Enable interrupt
    NVIC_ISER |= (1u<<1);               //Enable NVIC IRQ1

    //Init DMA
    DMA_CH0_DMATCTL = 0x00000000u;      // Set DMA CH0 trigger source to software

}

static void MY_delayCycles(volatile uint32_t c) {
   __asm volatile (
        ".syntax unified \n"  //Avoid compiler error
        "1: subs %0, #1 \n"
        "bne 1b \n"
        : "+r" (c) // R/W
    );
}

#define DMA_TRANSFER_SIZE_WORDS (2)

const uint32_t SrcData[DMA_TRANSFER_SIZE_WORDS] = {0x1973A314u, 0x19051905u};
uint32_t DstData[DMA_TRANSFER_SIZE_WORDS];
volatile uint32_t displayNumber = 0;

int main(void) {

    MY_init();

    DstData[0] = 0x00000000u;
    DstData[1] = 0x00000000u;

    while (1) {
        switch (displayNumber) {                
            case 7: LED_Controller((uint32_t)(DstData[0]%16));
            break;                
            case 6: LED_Controller((uint32_t)((DstData[0]/0x10)%16));
            break;                
            case 5: LED_Controller((uint32_t)((DstData[0]/0x100)%16));
            break;                
            case 4: LED_Controller((uint32_t)((DstData[0]/0x1000)%16));
            break;                
            case 3: LED_Controller((uint32_t)((DstData[0]/0x10000)%16));
            break;                
            case 2: LED_Controller((uint32_t)((DstData[0]/0x100000)%16));
            break;                
            case 1: LED_Controller((uint32_t)((DstData[0]/0x1000000)%16));
            break;                
            case 0: LED_Controller((uint32_t)((DstData[0]/0x10000000)%16));
            break;  
            default:;
        }
        displayNumber++;
        if(displayNumber>7) displayNumber=0;
        MY_delayCycles(8000000);
    }
}


void GPIOA_IRQHandler(void) {
    if(GPIOA_MIS & PIN_16_MASK) {
        GPIOA_ICLR = PIN_16_MASK;  //Clear PA16 interrupt flag
        DMA_CH0_DMASA = (uint32_t) &SrcData[0];
        DMA_CH0_DMADA = (uint32_t) &DstData[0];
        DMA_CH0_DMASZ = sizeof(SrcData)/sizeof(uint32_t);
        DMA_CH0_DMACTL = 0x10332202u;       //DMA address mode setting (TRM Table 5-30)
        DMA_ICLR = 0x00000001u;             //Clear any pending DMA CH0 interrupt flag
        DMA_IMASK = 0x00000001u;            //Enable DMA CH0 transfer-done interrupt
        NVIC_ISER |= (1u<<31);              //Enable NVIC IRQ31
        DMA_CH0_DMACTL |= 0x00000001u;      //Start DMA
    }
}

void DMA_IRQHandler(void) {
    if(DMA_MIS & 0x00000001u) {
        DMA_ICLR = 0x00000001u;
        GPIOA_DOUTCLR31_0 = PIN_22_MASK;  //LED on
    }
}

void LED_Controller(uint32_t n) {
    switch (n) {
        case 0:
            GPIOA_DOUTCLR31_0 = LED_0 | LED_1 | LED_2 | LED_3;
            break;
        case 1:
            GPIOA_DOUTCLR31_0 = LED_1 | LED_2 | LED_3;
            GPIOA_DOUTSET31_0  = LED_0;
            break;
        case 2:
            GPIOA_DOUTCLR31_0 = LED_0 | LED_2 | LED_3;
            GPIOA_DOUTSET31_0  = LED_1;
            break;
        case 3:
            GPIOA_DOUTCLR31_0 = LED_2 | LED_3;
            GPIOA_DOUTSET31_0  = LED_0 | LED_1 ;
            break;
        case 4:
            GPIOA_DOUTCLR31_0 = LED_0 | LED_1 |LED_3;
            GPIOA_DOUTSET31_0  = LED_2;
            break;
        case 5:
            GPIOA_DOUTCLR31_0 = LED_1 | LED_3;
            GPIOA_DOUTSET31_0  = LED_0 | LED_2;
            break;
        case 6:
            GPIOA_DOUTCLR31_0 = LED_0 | LED_3;
            GPIOA_DOUTSET31_0  = LED_1 | LED_2;
            break;
        case 7:
            GPIOA_DOUTCLR31_0 = LED_3;
            GPIOA_DOUTSET31_0  = LED_0 | LED_1 | LED_2;
            break;
        case 8:
            GPIOA_DOUTCLR31_0 = LED_0 | LED_1 | LED_2;
            GPIOA_DOUTSET31_0  = LED_3;
            break;
        case 9:
            GPIOA_DOUTCLR31_0 = LED_1 | LED_2;
            GPIOA_DOUTSET31_0  = LED_0 | LED_3;
            break;
        case 10:
            GPIOA_DOUTCLR31_0 = LED_0 | LED_2;
            GPIOA_DOUTSET31_0  = LED_1 | LED_3;        
            break;
        case 11:
            GPIOA_DOUTCLR31_0 = LED_2;
            GPIOA_DOUTSET31_0  = LED_0 | LED_1 | LED_3;        
            break;
        case 12:
            GPIOA_DOUTCLR31_0 = LED_0 | LED_1;
            GPIOA_DOUTSET31_0  = LED_2 | LED_3;
            break;
        case 13:
            GPIOA_DOUTCLR31_0 = LED_1;
            GPIOA_DOUTSET31_0  = LED_0 | LED_2 | LED_3;
            break;
        case 14:
            GPIOA_DOUTCLR31_0 = LED_0;
            GPIOA_DOUTSET31_0  = LED_1 | LED_2 | LED_3;
            break;
        case 15:            
            GPIOA_DOUTSET31_0  = LED_0 | LED_1 | LED_2 | LED_3;
            break;
        default:;
    }
}