#include <msp430.h>
//using the build-in SPI
//Anthor:LWhatever

void Port_Mapping(void);

unsigned char MoL = 1;
unsigned char RData_M, RData_L;
unsigned int cnt = 0;

void GPIO_Init()
{
    P2SEL |= BIT0+BIT1+BIT2;                  // Assign P2.0 to UCB0CLK and...
    P2DIR |=  BIT0+BIT1+BIT2;                 // P2.1 UCB0SOMI P2.2 UCB0SIMO

    P4DIR = BIT0;
    P4OUT &= ~BIT0;
}

void SPI_Init()
{
    UCA0CTL1 |= UCSWRST;                      // **Put state machine in reset**
    UCA0CTL0 |= UCMST+UCSYNC+UCMSB;    // 3-pin, 8-bit SPI master
                                              // Clock polarity low, MSB
    UCA0CTL1 |= UCSSEL_1;                     // SMCLK
    UCA0BR0 = 0x02;                           // /2
    UCA0BR1 = 0;                              //
    UCA0MCTL = 0;                             // No modulation
    UCA0CTL1 &= ~UCSWRST;                     // **Initialize USCI state machine**

    UCA0IE = UCTXIE;                          //first turn on the transfer buff
    __delay_cycles(100);                      // Wait for slave to initialize
}

int main(void)
{

    WDTCTL = WDTPW+WDTHOLD;                   // Stop watchdog timer
    Port_Mapping();
    GPIO_Init();
    SPI_Init();

    while (!(UCA0IFG&UCTXIFG));               // USCI_A0 TX buffer ready?
    __bis_SR_register(GIE);       // CPU off, enable interrupts
}

#pragma vector=USCI_A0_VECTOR
__interrupt void USCI_A0_ISR(void)
{

    switch(__even_in_range(UCA0IV,4))
    {
    case 0: break;                          // Vector 0 - no interrupt
    case 2:                                 // Vector 2 - RXIFG
        while (!(UCA0IFG&UCTXIFG));           // USCI_A0 TX buffer ready?
        cnt++;
        if(MoL)
        {
          UCA0TXBUF = 0x02;
          RData_M = UCA0RXBUF;
          MoL = 0;
        }
        else
        {
          UCA0TXBUF = 0x8B;
          RData_L = UCA0RXBUF;
          MoL = 1;
        }
        __no_operation();
        __delay_cycles(5000);
        break;
    case 4:                                 // Vector 4 - TXIFG
        UCA0TXBUF = 0x02;
        UCA0TXBUF = 0x8B;
        MoL = 0;
        cnt++;
        UCA0IE |= UCRXIE ;
        UCA0IE &=~ UCTXIE;
        break;
    default: break;
    }
}

void Port_Mapping(void)
{
    // Disable Interrupts before altering Port Mapping registers
    __disable_interrupt();
    // Enable Write-access to modify port mapping registers
    PMAPPWD = 0x02D52;

    #ifdef PORT_MAP_RECFG
    // Allow reconfiguration during runtime
    PMAPCTL = PMAPRECFG;
    #endif

    P2MAP0 = PM_UCA0CLK;
    P2MAP1 = PM_UCA0SOMI;
    P2MAP2 = PM_UCA0SIMO;

    // Disable Write-Access to modify port mapping registers
    PMAPPWD = 0;
    #ifdef PORT_MAP_EINT
    __enable_interrupt();                     // Re-enable all interrupts
    #endif
}
