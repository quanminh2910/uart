/**
 * @file main.c
 * @brief Simple UART Communication Application for ARTY Z7-20
 * 
 * This application demonstrates basic UART communication on the Zynq-7000
 * Processing System UART. It provides:
 * - UART initialization and configuration
 * - Echo functionality (receives and sends back data)
 * - Menu-driven interface for testing
 * - Basic error handling
 */

#include <stdio.h>
#include <string.h>
#include "xuartps.h"
#include "xparameters.h"
#include "xstatus.h"
#include "xil_printf.h"
#include "xil_types.h"
#include "sleep.h"
#include "usb_to_uart.h"

/************************** Constant Definitions *****************************/
#define UART_DEVICE_ID      XPAR_XUARTPS_0_BASEADDR
#define UART_IRPT_INTR      XPAR_XUARTPS_0_INTERRUPTS

#define TEST_BUFFER_SIZE    1024
#define MAX_MESSAGE_SIZE    256

/************************** Variable Definitions *****************************/
XUartPs UartPs;              /* Instance of the UART Device */
static char RecvBuffer[TEST_BUFFER_SIZE];   /* Buffer for receiving data */
static char SendBuffer[TEST_BUFFER_SIZE];   /* Buffer for sending data */

/************************** Function Prototypes ******************************/
int InitializeUART(void);
void DisplayMenu(void);
void ProcessUserInput(void);
void EchoTest(void);
void SendTestMessage(void);
void ReceiveDataTest(void);
int UartPsEchoExample(void);
void DisplayBridgeStatistics(void);

/*****************************************************************************/
/**
 * Main function to demonstrate UART functionality
 *
 * @param   None
 *
 * @return  XST_SUCCESS if successful, otherwise XST_FAILURE
 *
 * @note    None
 *
 ******************************************************************************/
int main(void)
{
    int Status;
    
    xil_printf("\r\n*** ARTY Z7-20 UART Communication Demo ***\r\n");
    xil_printf("Initializing UART...\r\n");
    
    /* Initialize the UART */
    Status = InitializeUART();
    if (Status != XST_SUCCESS) {
        xil_printf("UART Initialization Failed\r\n");
        return XST_FAILURE;
    }
    
    xil_printf("UART Initialized Successfully!\r\n");
    xil_printf("UART Base Address: 0x%08X\r\n", UART_DEVICE_ID);
    xil_printf("Baud Rate: 115200\r\n");
    
    /* Initialize USB-to-UART bridge */
    Status = UsbUartBridge_Init();
    if (Status != XST_SUCCESS) {
        xil_printf("USB-UART Bridge Initialization Failed\r\n");
        return XST_FAILURE;
    }
    
    /* Display welcome message */
    const char* welcome_msg = "\r\n=== UART Communication Ready ===\r\n";
    XUartPs_Send(&UartPs, (u8*)welcome_msg, strlen(welcome_msg));
    
    /* Main application loop */
    while (1) {
        /* Process USB-UART bridge */
        UsbUartBridge_Process();
        
        DisplayMenu();
        ProcessUserInput();
        sleep(1);  /* Small delay to prevent overwhelming the system */
    }
    
    return XST_SUCCESS;
}

/*****************************************************************************/
/**
 * Initialize UART with proper configuration
 *
 * @param   None
 *
 * @return  XST_SUCCESS if successful, otherwise XST_FAILURE
 *
 ******************************************************************************/
int InitializeUART(void)
{
    int Status;
    XUartPs_Config *Config;
    
    /* Initialize the UART driver so that it's ready to use */
    Config = XUartPs_LookupConfig(UART_DEVICE_ID);
    if (NULL == Config) {
        return XST_FAILURE;
    }
    
    Status = XUartPs_CfgInitialize(&UartPs, Config, Config->BaseAddress);
    if (Status != XST_SUCCESS) {
        return XST_FAILURE;
    }
    
    /* Perform a self-test to check hardware build */
    Status = XUartPs_SelfTest(&UartPs);
    if (Status != XST_SUCCESS) {
        return XST_FAILURE;
    }
    
    /* Set the UART in Normal Mode */
    XUartPs_SetOperMode(&UartPs, XUARTPS_OPER_MODE_NORMAL);
    
    /* Set baud rate to 115200 bps */
    XUartPs_SetBaudRate(&UartPs, 115200);
    
    return XST_SUCCESS;
}

/*****************************************************************************/
/**
 * Display menu options to the user
 *
 * @param   None
 *
 * @return  None
 *
 ******************************************************************************/
void DisplayMenu(void)
{
    const char* menu = "\r\n=== UART Test Menu ===\r\n"
                      "1. Echo Test (Type and see echo)\r\n"
                      "2. Send Test Message\r\n"
                      "3. Receive Data Test\r\n"
                      "4. Continuous Echo Mode\r\n"
                      "5. USB Bridge Statistics\r\n"
                      "Select option (1-5): ";
    
    XUartPs_Send(&UartPs, (u8*)menu, strlen(menu));
}

/*****************************************************************************/
/**
 * Process user input and execute selected function
 *
 * @param   None
 *
 * @return  None
 *
 ******************************************************************************/
void ProcessUserInput(void)
{
    u8 ReceivedByte;
    u32 ReceivedCount = 0;
    
    /* Wait for user input with timeout */
    int timeout_counter = 0;
    while (timeout_counter < 1000000) { /* Adjust timeout as needed */
        ReceivedCount = XUartPs_Recv(&UartPs, &ReceivedByte, 1);
        if (ReceivedCount > 0) {
            break;
        }
        timeout_counter++;
    }
    
    if (ReceivedCount > 0) {
        /* Echo the received character */
        XUartPs_Send(&UartPs, &ReceivedByte, 1);
        
        switch (ReceivedByte) {
            case '1':
                EchoTest();
                break;
            case '2':
                SendTestMessage();
                break;
            case '3':
                ReceiveDataTest();
                break;
            case '4':
                UartPsEchoExample();
                break;
            case '5':
                DisplayBridgeStatistics();
                break;
            default:
                {
                    const char* invalid_msg = "\r\nInvalid option. Please select 1-5.\r\n";
                    XUartPs_Send(&UartPs, (u8*)invalid_msg, strlen(invalid_msg));
                }
                break;
        }
    }
}

/*****************************************************************************/
/**
 * Simple echo test - user types, system echoes back
 *
 * @param   None
 *
 * @return  None
 *
 ******************************************************************************/
void EchoTest(void)
{
    const char* echo_msg = "\r\n=== Echo Test ===\r\n"
                          "Type characters (press 'q' to quit):\r\n";
    
    XUartPs_Send(&UartPs, (u8*)echo_msg, strlen(echo_msg));
    
    u8 ReceivedByte;
    u32 ReceivedCount;
    
    while (1) {
        ReceivedCount = XUartPs_Recv(&UartPs, &ReceivedByte, 1);
        if (ReceivedCount > 0) {
            if (ReceivedByte == 'q' || ReceivedByte == 'Q') {
                const char* quit_msg = "\r\nEcho test completed.\r\n";
                XUartPs_Send(&UartPs, (u8*)quit_msg, strlen(quit_msg));
                break;
            }
            
            /* Echo the character back */
            XUartPs_Send(&UartPs, &ReceivedByte, 1);
            
            /* Handle special characters */
            if (ReceivedByte == '\r') {
                const char newline = '\n';
                XUartPs_Send(&UartPs, (u8*)&newline, 1);
            }
        }
        
        /* Small delay to prevent CPU hogging */
        usleep(1000);
    }
}

/*****************************************************************************/
/**
 * Send a test message through UART
 *
 * @param   None
 *
 * @return  None
 *
 ******************************************************************************/
void SendTestMessage(void)
{
    const char* test_messages[] = {
        "\r\n=== Sending Test Messages ===\r\n",
        "Message 1: Hello from ARTY Z7-20!\r\n",
        "Message 2: UART Communication Test\r\n", 
        "Message 3: Zynq-7000 SoC UART Demo\r\n",
        "Message 4: 0123456789ABCDEF\r\n",
        "=== Test Messages Complete ===\r\n"
    };
    
    int num_messages = sizeof(test_messages) / sizeof(test_messages[0]);
    
    for (int i = 0; i < num_messages; i++) {
        XUartPs_Send(&UartPs, (u8*)test_messages[i], strlen(test_messages[i]));
        sleep(1);  /* Delay between messages */
    }
}

/*****************************************************************************/
/**
 * Receive and display data from UART
 *
 * @param   None
 *
 * @return  None
 *
 ******************************************************************************/
void ReceiveDataTest(void)
{
    const char* receive_msg = "\r\n=== Receive Data Test ===\r\n"
                             "Send data (press ESC to stop):\r\n";
    
    XUartPs_Send(&UartPs, (u8*)receive_msg, strlen(receive_msg));
    
    u8 ReceivedByte;
    u32 ReceivedCount;
    u32 TotalReceived = 0;
    
    memset(RecvBuffer, 0, TEST_BUFFER_SIZE);
    
    while (1) {
        ReceivedCount = XUartPs_Recv(&UartPs, &ReceivedByte, 1);
        if (ReceivedCount > 0) {
            if (ReceivedByte == 27) {  /* ESC key */
                break;
            }
            
            if (TotalReceived < (TEST_BUFFER_SIZE - 1)) {
                RecvBuffer[TotalReceived] = ReceivedByte;
                TotalReceived++;
            }
            
            /* Echo received character */
            XUartPs_Send(&UartPs, &ReceivedByte, 1);
        }
        
        usleep(1000);
    }
    
    sprintf(SendBuffer, "\r\nReceived %d bytes total.\r\n", (int)TotalReceived);
    XUartPs_Send(&UartPs, (u8*)SendBuffer, strlen(SendBuffer));
    
    const char* complete_msg = "Receive test completed.\r\n";
    XUartPs_Send(&UartPs, (u8*)complete_msg, strlen(complete_msg));
}

/*****************************************************************************/
/**
 * Continuous echo mode - demonstrates real-time UART communication
 *
 * @param   None
 *
 * @return  XST_SUCCESS if successful, otherwise XST_FAILURE
 *
 ******************************************************************************/
int UartPsEchoExample(void)
{
    const char* echo_mode_msg = "\r\n=== Continuous Echo Mode ===\r\n"
                               "All typed characters will be echoed back.\r\n"
                               "Press Ctrl+C or send 'EXIT' to stop.\r\n";
    
    XUartPs_Send(&UartPs, (u8*)echo_mode_msg, strlen(echo_mode_msg));
    
    u8 ReceivedByte;
    u32 ReceivedCount;
    char exit_buffer[5] = {0};
    int exit_index = 0;
    
    while (1) {
        ReceivedCount = XUartPs_Recv(&UartPs, &ReceivedByte, 1);
        if (ReceivedCount > 0) {
            /* Echo the character immediately */
            XUartPs_Send(&UartPs, &ReceivedByte, 1);
            
            /* Check for EXIT command */
            if (ReceivedByte == 'E' || ReceivedByte == 'e') {
                exit_buffer[0] = ReceivedByte;
                exit_index = 1;
            } else if (exit_index > 0 && exit_index < 4) {
                exit_buffer[exit_index] = ReceivedByte;
                exit_index++;
                if (exit_index == 4) {
                    if (strncmp(exit_buffer, "EXIT", 4) == 0 || 
                        strncmp(exit_buffer, "exit", 4) == 0) {
                        const char* exit_msg = "\r\nExiting continuous echo mode.\r\n";
                        XUartPs_Send(&UartPs, (u8*)exit_msg, strlen(exit_msg));
                        break;
                    }
                    exit_index = 0;
                    memset(exit_buffer, 0, 5);
                }
            } else {
                exit_index = 0;
                memset(exit_buffer, 0, 5);
            }
            
            /* Handle carriage return */
            if (ReceivedByte == '\r') {
                const char newline = '\n';
                XUartPs_Send(&UartPs, (u8*)&newline, 1);
            }
        }
        
        /* Small delay to prevent CPU hogging */
        usleep(100);
    }
    
    return XST_SUCCESS;
}

/*****************************************************************************/
/**
 * Display USB-to-UART bridge statistics
 *
 * @param   None
 *
 * @return  None
 *
 ******************************************************************************/
void DisplayBridgeStatistics(void)
{
    u32 tx_count, rx_count, error_count;
    u32 data_available, tx_space;
    
    UsbUartBridge_GetStatistics(&tx_count, &rx_count, &error_count);
    data_available = UsbUartBridge_DataAvailable();
    tx_space = UsbUartBridge_TxSpaceAvailable();
    
    sprintf(SendBuffer, 
           "\r\n=== USB-UART Bridge Statistics ===\r\n"
           "Bytes Transmitted: %d\r\n"
           "Bytes Received: %d\r\n"
           "Error Count: %d\r\n"
           "RX Data Available: %d bytes\r\n"
           "TX Space Available: %d bytes\r\n"
           "===================================\r\n",
           (int)tx_count, (int)rx_count, (int)error_count, 
           (int)data_available, (int)tx_space);
           
    XUartPs_Send(&UartPs, (u8*)SendBuffer, strlen(SendBuffer));
}
