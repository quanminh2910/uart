/**
 * @file main_simple.c
 * @brief Simplified UART test for debugging - use this if main.c has issues
 */

#include <stdio.h>
#include "xuartps.h"
#include "xparameters.h" 
#include "xstatus.h"
#include "sleep.h"

XUartPs UartInst;

int main()
{
    int Status;
    XUartPs_Config *Config;
    
    // Simple initialization - no complex menu system
    Config = XUartPs_LookupConfig(XPAR_XUARTPS_0_BASEADDR);
    if (NULL == Config) {
        return XST_FAILURE;
    }
    
    Status = XUartPs_CfgInitialize(&UartInst, Config, Config->BaseAddress);
    if (Status != XST_SUCCESS) {
        return XST_FAILURE;
    }
    
    // Set baud rate
    XUartPs_SetBaudRate(&UartInst, 115200);
    
    // Simple test message
    const char* msg = "Hello ARTY Z7-20!\r\n";
    XUartPs_Send(&UartInst, (u8*)msg, 19);
    
    // Simple echo loop
    while (1) {
        u8 RecvByte;
        u32 RecvCount = XUartPs_Recv(&UartInst, &RecvByte, 1);
        if (RecvCount > 0) {
            XUartPs_Send(&UartInst, &RecvByte, 1);
        }
        sleep(1);
    }
    
    return XST_SUCCESS;
}