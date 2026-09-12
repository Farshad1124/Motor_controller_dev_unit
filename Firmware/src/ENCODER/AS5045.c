#include "definitions.h"
#include "AS5045.h"
#include "./RTT/SEGGER_RTT.h"

volatile _AS5045 packet = {0};
volatile uint8_t packet_count = 0;
bool read_busy = false;
bool frame_ready = false;

void SPI_callback(uintptr_t context);

#define GCLK_SERCOM0_FREQ_HZ 60000000UL
#define AS5045_SCK_FREQ_HZ 1000000UL

void AS5045_ENC_Initialize()
{
    // disable SPI
    SERCOM0_REGS->SPIM.SERCOM_CTRLA &= ~(SERCOM_SPIM_CTRLA_ENABLE_Msk);
    while((SERCOM0_REGS->SPIM.SERCOM_SYNCBUSY) != 0U)
    {
        // do nothing
    }

    SERCOM0_REGS->SPIM.SERCOM_CTRLA = SERCOM0_REGS->SPIM.SERCOM_CTRLA & !(SERCOM_SPIM_CTRLA_SWRST_Msk);
    while((SERCOM0_REGS->SPIM.SERCOM_SYNCBUSY) != 0U)
    {
        // do nothing
    }

    SERCOM0_REGS->SPIM.SERCOM_CTRLA = SERCOM_SPIM_CTRLA_MODE_SPI_MASTER 
                                    | SERCOM_SPIM_CTRLA_DOPO_PAD0 
                                    | SERCOM_SPIM_CTRLA_DIPO_PAD3 
                                    | SERCOM_SPIM_CTRLA_CPOL_IDLE_HIGH 
                                    | SERCOM_SPIM_CTRLA_CPHA_LEADING_EDGE 
                                    | SERCOM_SPIM_CTRLA_DORD_MSB;

    SERCOM0_REGS->SPIM.SERCOM_CTRLB = SERCOM_SPIM_CTRLB_CHSIZE_9_BIT | SERCOM_SPIM_CTRLB_RXEN_Msk ;                                

    while((SERCOM0_REGS->SPIM.SERCOM_SYNCBUSY) != 0U)
    {
        // Do nothing
    }

    uint32_t baud = (GCLK_SERCOM0_FREQ_HZ / (2UL * AS5045_SCK_FREQ_HZ)) - 1UL;
    SERCOM0_REGS->SPIM.SERCOM_BAUD = (uint8_t)SERCOM_SPIM_BAUD_BAUD(baud);

    /* Only RXC drives this whole state machine - no DRE interrupt needed,
     * since each subsequent TX byte is written manually inside the RXC
     * handler rather than fed automatically. */
    SERCOM0_REGS->SPIM.SERCOM_INTENSET= SERCOM_SPIM_INTENCLR_RXC_Msk;

    //NVIC_ClearPendingIRQ(SERCOM0_2_IRQn);  
    //NVIC_EnableIRQ(SERCOM0_2_IRQn);


    SERCOM0_REGS->SPIM.SERCOM_CTRLA = SERCOM0_REGS->SPIM.SERCOM_CTRLA | SERCOM_SPIM_CTRLA_ENABLE_Msk;

    while((SERCOM0_REGS->SPIM.SERCOM_SYNCBUSY) != 0U)
    {
        // Do nothing
    }

}

bool AS5045_ENC_Request()
{
    if (read_busy) 
    {
        return false;   // previous read still in flight - caller can retry later */
    }


    read_busy   = true;
    packet_count  = 0;
    frame_ready = false;

    SPI_CS_Clear();
    SERCOM0_REGS->SPIM.SERCOM_DATA = 0x00;

    return true;
}

void __attribute__((used)) SERCOM0_SPI_InterruptHandler (void)
{
    char str[30] = {0};
    if((SERCOM0_REGS->SPIM.SERCOM_INTFLAG & SERCOM_SPIM_INTFLAG_RXC_Msk) == SERCOM_SPIM_INTFLAG_RXC_Msk)
    {
        packet.raw = (packet.raw << 9) | SERCOM0_REGS->SPIM.SERCOM_DATA;
        packet_count++;

        if (packet_count < 2)
        {
            SERCOM0_REGS->SPIM.SERCOM_DATA = 0x00;
        }
        else 
        {
            SPI_CS_Set();
            read_busy = false;
            frame_ready = true;
            //SEGGER_RTT_printf(0, "Raw: %0x. \n", (packet.raw & 0xFFF0)>>4);
            //SEGGER_RTT_printf(0, "Raw: %0x. \n", packet.bits.position);
            float angle = packet.bits.position*0.087891;
            sprintf(str,"%.4f\n",angle);
            SEGGER_RTT_WriteString(0, str);
        }
    }

}