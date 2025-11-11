/**
 * @file usb_to_uart.h
 * @brief Header file for USB-to-UART Bridge functionality
 */

#ifndef USB_TO_UART_H
#define USB_TO_UART_H

#include "xil_types.h"
#include "xstatus.h"

/************************** Function Prototypes ******************************/

/**
 * Initialize the USB-to-UART bridge
 * @return XST_SUCCESS if successful, otherwise XST_FAILURE
 */
int UsbUartBridge_Init(void);

/**
 * Process USB-to-UART bridge operations (call regularly in main loop)
 */
void UsbUartBridge_Process(void);

/**
 * Send data through the USB-to-UART bridge
 * @param data Pointer to data to send
 * @param length Number of bytes to send
 * @return Number of bytes successfully queued for transmission
 */
u32 UsbUartBridge_SendData(const u8* data, u32 length);

/**
 * Receive data from the USB-to-UART bridge
 * @param buffer Buffer to store received data
 * @param max_length Maximum number of bytes to receive
 * @return Number of bytes actually received
 */
u32 UsbUartBridge_ReceiveData(u8* buffer, u32 max_length);

/**
 * Get bridge statistics
 * @param tx_count Pointer to store transmitted byte count
 * @param rx_count Pointer to store received byte count
 * @param error_count Pointer to store error count
 */
void UsbUartBridge_GetStatistics(u32* tx_count, u32* rx_count, u32* error_count);

/**
 * Enable or disable flow control
 * @param enable 1 to enable, 0 to disable
 */
void UsbUartBridge_SetFlowControl(u8 enable);

/**
 * Clear all buffers and reset statistics
 */
void UsbUartBridge_Reset(void);

/**
 * Check if data is available for reading
 * @return Number of bytes available for reading
 */
u32 UsbUartBridge_DataAvailable(void);

/**
 * Check available space in transmit buffer
 * @return Number of bytes that can be queued for transmission
 */
u32 UsbUartBridge_TxSpaceAvailable(void);

#endif /* USB_TO_UART_H */