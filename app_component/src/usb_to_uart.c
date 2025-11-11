/**
 * @file usb_to_uart.c
 * @brief USB-to-UART Bridge Implementation for ARTY Z7-20
 * 
 * This file implements USB-to-UART bridge functionality using the PS UART.
 * It provides data buffering, flow control, and bidirectional communication.
 */

#include <stdio.h>
#include <string.h>
#include "xuartps.h"
#include "xparameters.h"
#include "xstatus.h"
#include "xil_printf.h"
#include "sleep.h"

/************************** Constant Definitions *****************************/
#define USB_UART_BUFFER_SIZE    2048
#define MAX_PACKET_SIZE         64
#define BRIDGE_TIMEOUT_MS       1000

/* Flow control characters */
#define XON_CHAR               0x11    /* DC1 - Resume transmission */
#define XOFF_CHAR              0x13    /* DC3 - Pause transmission */

/************************** Type Definitions *******************************/
typedef struct {
    u8 data[USB_UART_BUFFER_SIZE];
    u32 head;
    u32 tail;
    u32 count;
    u32 overflow_count;
} CircularBuffer_t;

typedef struct {
    CircularBuffer_t tx_buffer;
    CircularBuffer_t rx_buffer;
    u32 bytes_transmitted;
    u32 bytes_received;
    u32 error_count;
    u8 flow_control_enabled;
    u8 transmission_paused;
} UsbUartBridge_t;

/************************** Variable Definitions *****************************/
static UsbUartBridge_t bridge;
extern XUartPs UartPs;  /* Defined in main.c */

/************************** Function Prototypes ******************************/
static int CircularBuffer_Init(CircularBuffer_t* buffer);
static int CircularBuffer_Put(CircularBuffer_t* buffer, u8 data);
static int CircularBuffer_Get(CircularBuffer_t* buffer, u8* data);
static u32 CircularBuffer_GetCount(CircularBuffer_t* buffer);
static u32 CircularBuffer_GetSpace(CircularBuffer_t* buffer);
static void CircularBuffer_Clear(CircularBuffer_t* buffer);

/************************** Bridge API Functions *****************************/

/*****************************************************************************/
/**
 * Initialize the USB-to-UART bridge
 *
 * @param   None
 *
 * @return  XST_SUCCESS if successful, otherwise XST_FAILURE
 *
 ******************************************************************************/
int UsbUartBridge_Init(void)
{
    /* Initialize circular buffers */
    if (CircularBuffer_Init(&bridge.tx_buffer) != XST_SUCCESS) {
        return XST_FAILURE;
    }
    
    if (CircularBuffer_Init(&bridge.rx_buffer) != XST_SUCCESS) {
        return XST_FAILURE;
    }
    
    /* Initialize bridge statistics */
    bridge.bytes_transmitted = 0;
    bridge.bytes_received = 0;
    bridge.error_count = 0;
    bridge.flow_control_enabled = 1;  /* Enable flow control by default */
    bridge.transmission_paused = 0;
    
    xil_printf("USB-to-UART Bridge initialized\r\n");
    return XST_SUCCESS;
}

/*****************************************************************************/
/**
 * Process USB-to-UART bridge operations
 * This function should be called regularly in the main loop
 *
 * @param   None
 *
 * @return  None
 *
 ******************************************************************************/
void UsbUartBridge_Process(void)
{
    u8 received_byte;
    u32 received_count;
    
    /* Process incoming UART data */
    received_count = XUartPs_Recv(&UartPs, &received_byte, 1);
    if (received_count > 0) {
        /* Handle flow control characters */
        if (bridge.flow_control_enabled) {
            if (received_byte == XOFF_CHAR) {
                bridge.transmission_paused = 1;
                return;
            } else if (received_byte == XON_CHAR) {
                bridge.transmission_paused = 0;
                return;
            }
        }
        
        /* Store received data in RX buffer */
        if (CircularBuffer_Put(&bridge.rx_buffer, received_byte) != XST_SUCCESS) {
            bridge.error_count++;
        } else {
            bridge.bytes_received++;
        }
    }
    
    /* Process outgoing UART data */
    if (!bridge.transmission_paused && 
        CircularBuffer_GetCount(&bridge.tx_buffer) > 0) {
        
        u8 tx_byte;
        if (CircularBuffer_Get(&bridge.tx_buffer, &tx_byte) == XST_SUCCESS) {
            /* Send data through UART */
            XUartPs_Send(&UartPs, &tx_byte, 1);
            bridge.bytes_transmitted++;
        }
    }
    
    /* Check for flow control - pause transmission if RX buffer is getting full */
    if (bridge.flow_control_enabled) {
        u32 rx_count = CircularBuffer_GetCount(&bridge.rx_buffer);
        u32 buffer_threshold = (USB_UART_BUFFER_SIZE * 3) / 4;  /* 75% full */
        
        if (rx_count > buffer_threshold && !bridge.transmission_paused) {
            /* Send XOFF to pause transmission */
            u8 xoff_char = XOFF_CHAR;
            XUartPs_Send(&UartPs, &xoff_char, 1);
        } else if (rx_count < (buffer_threshold / 2) && bridge.transmission_paused) {
            /* Send XON to resume transmission */
            bridge.transmission_paused = 0;
            u8 xon_char = XON_CHAR;
            XUartPs_Send(&UartPs, &xon_char, 1);
        }
    }
}

/*****************************************************************************/
/**
 * Send data through the USB-to-UART bridge
 *
 * @param   data - Pointer to data to send
 * @param   length - Number of bytes to send
 *
 * @return  Number of bytes successfully queued for transmission
 *
 ******************************************************************************/
u32 UsbUartBridge_SendData(const u8* data, u32 length)
{
    u32 bytes_queued = 0;
    
    if (data == NULL || length == 0) {
        return 0;
    }
    
    for (u32 i = 0; i < length; i++) {
        if (CircularBuffer_Put(&bridge.tx_buffer, data[i]) == XST_SUCCESS) {
            bytes_queued++;
        } else {
            /* Buffer full, stop queuing */
            break;
        }
    }
    
    return bytes_queued;
}

/*****************************************************************************/
/**
 * Receive data from the USB-to-UART bridge
 *
 * @param   buffer - Buffer to store received data
 * @param   max_length - Maximum number of bytes to receive
 *
 * @return  Number of bytes actually received
 *
 ******************************************************************************/
u32 UsbUartBridge_ReceiveData(u8* buffer, u32 max_length)
{
    u32 bytes_received = 0;
    
    if (buffer == NULL || max_length == 0) {
        return 0;
    }
    
    for (u32 i = 0; i < max_length; i++) {
        if (CircularBuffer_Get(&bridge.rx_buffer, &buffer[i]) == XST_SUCCESS) {
            bytes_received++;
        } else {
            /* No more data available */
            break;
        }
    }
    
    return bytes_received;
}

/*****************************************************************************/
/**
 * Get bridge statistics
 *
 * @param   tx_count - Pointer to store transmitted byte count
 * @param   rx_count - Pointer to store received byte count  
 * @param   error_count - Pointer to store error count
 *
 * @return  None
 *
 ******************************************************************************/
void UsbUartBridge_GetStatistics(u32* tx_count, u32* rx_count, u32* error_count)
{
    if (tx_count != NULL) {
        *tx_count = bridge.bytes_transmitted;
    }
    
    if (rx_count != NULL) {
        *rx_count = bridge.bytes_received;
    }
    
    if (error_count != NULL) {
        *error_count = bridge.error_count;
    }
}

/*****************************************************************************/
/**
 * Enable or disable flow control
 *
 * @param   enable - 1 to enable, 0 to disable
 *
 * @return  None
 *
 ******************************************************************************/
void UsbUartBridge_SetFlowControl(u8 enable)
{
    bridge.flow_control_enabled = enable;
    if (!enable) {
        bridge.transmission_paused = 0;  /* Resume transmission if disabled */
    }
}

/*****************************************************************************/
/**
 * Clear all buffers and reset statistics
 *
 * @param   None
 *
 * @return  None
 *
 ******************************************************************************/
void UsbUartBridge_Reset(void)
{
    CircularBuffer_Clear(&bridge.tx_buffer);
    CircularBuffer_Clear(&bridge.rx_buffer);
    
    bridge.bytes_transmitted = 0;
    bridge.bytes_received = 0;
    bridge.error_count = 0;
    bridge.transmission_paused = 0;
    
    xil_printf("USB-to-UART Bridge reset\r\n");
}

/*****************************************************************************/
/**
 * Check if data is available for reading
 *
 * @param   None
 *
 * @return  Number of bytes available for reading
 *
 ******************************************************************************/
u32 UsbUartBridge_DataAvailable(void)
{
    return CircularBuffer_GetCount(&bridge.rx_buffer);
}

/*****************************************************************************/
/**
 * Check available space in transmit buffer
 *
 * @param   None
 *
 * @return  Number of bytes that can be queued for transmission
 *
 ******************************************************************************/
u32 UsbUartBridge_TxSpaceAvailable(void)
{
    return CircularBuffer_GetSpace(&bridge.tx_buffer);
}

/************************** Circular Buffer Implementation ******************/

/*****************************************************************************/
/**
 * Initialize a circular buffer
 *
 * @param   buffer - Pointer to circular buffer structure
 *
 * @return  XST_SUCCESS if successful
 *
 ******************************************************************************/
static int CircularBuffer_Init(CircularBuffer_t* buffer)
{
    if (buffer == NULL) {
        return XST_FAILURE;
    }
    
    buffer->head = 0;
    buffer->tail = 0;
    buffer->count = 0;
    buffer->overflow_count = 0;
    
    memset(buffer->data, 0, USB_UART_BUFFER_SIZE);
    
    return XST_SUCCESS;
}

/*****************************************************************************/
/**
 * Put data into circular buffer
 *
 * @param   buffer - Pointer to circular buffer structure
 * @param   data - Data byte to store
 *
 * @return  XST_SUCCESS if successful, XST_FAILURE if buffer full
 *
 ******************************************************************************/
static int CircularBuffer_Put(CircularBuffer_t* buffer, u8 data)
{
    if (buffer == NULL) {
        return XST_FAILURE;
    }
    
    if (buffer->count >= USB_UART_BUFFER_SIZE) {
        buffer->overflow_count++;
        return XST_FAILURE;  /* Buffer full */
    }
    
    buffer->data[buffer->head] = data;
    buffer->head = (buffer->head + 1) % USB_UART_BUFFER_SIZE;
    buffer->count++;
    
    return XST_SUCCESS;
}

/*****************************************************************************/
/**
 * Get data from circular buffer
 *
 * @param   buffer - Pointer to circular buffer structure
 * @param   data - Pointer to store retrieved data
 *
 * @return  XST_SUCCESS if successful, XST_FAILURE if buffer empty
 *
 ******************************************************************************/
static int CircularBuffer_Get(CircularBuffer_t* buffer, u8* data)
{
    if (buffer == NULL || data == NULL) {
        return XST_FAILURE;
    }
    
    if (buffer->count == 0) {
        return XST_FAILURE;  /* Buffer empty */
    }
    
    *data = buffer->data[buffer->tail];
    buffer->tail = (buffer->tail + 1) % USB_UART_BUFFER_SIZE;
    buffer->count--;
    
    return XST_SUCCESS;
}

/*****************************************************************************/
/**
 * Get number of bytes in circular buffer
 *
 * @param   buffer - Pointer to circular buffer structure
 *
 * @return  Number of bytes in buffer
 *
 ******************************************************************************/
static u32 CircularBuffer_GetCount(CircularBuffer_t* buffer)
{
    if (buffer == NULL) {
        return 0;
    }
    
    return buffer->count;
}

/*****************************************************************************/
/**
 * Get available space in circular buffer
 *
 * @param   buffer - Pointer to circular buffer structure
 *
 * @return  Number of bytes that can be stored
 *
 ******************************************************************************/
static u32 CircularBuffer_GetSpace(CircularBuffer_t* buffer)
{
    if (buffer == NULL) {
        return 0;
    }
    
    return (USB_UART_BUFFER_SIZE - buffer->count);
}

/*****************************************************************************/
/**
 * Clear circular buffer
 *
 * @param   buffer - Pointer to circular buffer structure
 *
 * @return  None
 *
 ******************************************************************************/
static void CircularBuffer_Clear(CircularBuffer_t* buffer)
{
    if (buffer == NULL) {
        return;
    }
    
    buffer->head = 0;
    buffer->tail = 0;
    buffer->count = 0;
    buffer->overflow_count = 0;
    
    memset(buffer->data, 0, USB_UART_BUFFER_SIZE);
}
