# UART Communication Application for ARTY Z7-20

This project implements a comprehensive UART communication application for the ARTY Z7-20 FPGA board using Xilinx Vitis.

## Features

### Main Application (`main.c`)
- **UART Initialization**: Configures PS UART0 at 115200 baud rate
- **Interactive Menu System**: User-friendly command interface
- **Multiple Test Modes**:
  1. **Echo Test**: Simple character echo for basic connectivity testing
  2. **Send Test Message**: Transmits predefined test messages
  3. **Receive Data Test**: Captures and displays incoming data
  4. **Continuous Echo Mode**: Real-time bidirectional communication
  5. **USB Bridge Statistics**: Displays bridge performance metrics

### USB-to-UART Bridge (`usb_to_uart.c`)
- **Circular Buffering**: 2KB buffers for TX and RX with overflow protection
- **Flow Control**: XON/XOFF software flow control
- **Statistics Tracking**: Monitors data transmission, reception, and errors
- **Bidirectional Communication**: Simultaneous TX/RX operations
- **Buffer Management**: Efficient circular buffer implementation

## Hardware Configuration

### Target Platform
- **Board**: Digilent ARTY Z7-20 
- **SoC**: Xilinx Zynq-7000 (7z020)
- **Processor**: Dual-core ARM Cortex-A9 @ 667 MHz
- **UART**: PS UART0 at base address 0xe0000000

### Memory Configuration
- **DDR3**: 512MB system memory
- **OCM**: 192KB on-chip memory for boot
- **AXI BRAM**: Additional 128KB + 8KB block RAM

### Clock Configuration
- **CPU Clock**: 666.67 MHz
- **UART Clock**: 100 MHz
- **Fabric Clock**: 100 MHz

## Software Architecture

```
main.c
├── UART Initialization
├── Menu System
├── Test Functions
└── Bridge Integration

usb_to_uart.c
├── Circular Buffers
├── Flow Control
├── Statistics
└── Bridge API

usb_to_uart.h
└── Function Declarations
```

## Usage Instructions

### Building the Application
1. Open Xilinx Vitis 2025.1
2. Import the workspace from the project directory
3. Build the application using the provided CMake configuration
4. Program the FPGA with the bitstream and run the application

### Running the Application
1. Connect UART cable to ARTY Z7-20 USB-UART port
2. Open terminal emulator (115200, 8N1)
3. Power on/reset the board
4. Follow the interactive menu prompts

### Menu Options
```
=== UART Test Menu ===
1. Echo Test (Type and see echo)
2. Send Test Message  
3. Receive Data Test
4. Continuous Echo Mode
5. USB Bridge Statistics
Select option (1-5):
```

## Application Features Detail

### 1. Echo Test
- Single character echo functionality
- Press 'q' to quit
- Handles special characters (CR/LF)

### 2. Send Test Messages
- Sends predefined test patterns
- Useful for connectivity verification
- Includes alphanumeric and special characters

### 3. Receive Data Test
- Captures incoming data stream
- Press ESC to stop
- Shows total bytes received

### 4. Continuous Echo Mode
- Real-time bidirectional communication
- Type 'EXIT' or Ctrl+C to quit
- Demonstrates bridge functionality

### 5. Bridge Statistics
- Displays transmission/reception counters
- Shows error counts and buffer status
- Useful for performance monitoring

## Technical Specifications

### UART Configuration
```c
Base Address: 0xe0000000
Baud Rate: 115200 bps
Data Bits: 8
Parity: None
Stop Bits: 1
Flow Control: Software (XON/XOFF)
```

### Buffer Specifications
```c
Buffer Size: 2048 bytes each (TX/RX)
Flow Control Threshold: 75% full (1536 bytes)
Resume Threshold: 37.5% full (768 bytes)
```

### Performance Characteristics
- **Throughput**: Up to 11.5 KB/s (115200 baud)
- **Latency**: < 1ms typical
- **Buffer Depth**: 2KB prevents data loss
- **Error Handling**: Overflow detection and reporting

## Development Environment

### Prerequisites
- Xilinx Vitis 2025.1 or later
- ARTY Z7-20 board with USB cable
- Terminal emulator (PuTTY, Tera Term, etc.)

### File Structure
```
app_component/src/
├── main.c              # Main application
├── usb_to_uart.c       # Bridge implementation  
├── usb_to_uart.h       # Bridge header
├── CMakeLists.txt      # Build configuration
└── README.txt          # Original template info
```

## Troubleshooting

### Common Issues
1. **No Output**: Check UART cable connection and baud rate
2. **Garbled Text**: Verify terminal settings (115200, 8N1)
3. **Application Won't Start**: Ensure bitstream is loaded correctly
4. **Build Errors**: Check include paths and library dependencies

### Debug Tips
- Use Vitis debugger for step-through debugging
- Check UART register values for hardware issues
- Monitor bridge statistics for data flow verification
- Use oscilloscope on UART pins for signal integrity

## Extension Ideas

1. **Multiple UART Ports**: Add AXI UART Lite cores for additional ports
2. **USB Integration**: Add USB device controller for true USB-UART bridge
3. **Protocol Support**: Add RS-232, RS-485 protocol handling
4. **GUI Interface**: Create PC-side application for enhanced control
5. **Data Logging**: Add SD card logging capabilities
6. **Wireless Bridge**: Add WiFi or Bluetooth connectivity

## License

This project is provided as educational example code. Modify and use as needed for your applications.

## Support

For questions or issues:
1. Check Xilinx documentation for Zynq-7000 and Vitis
2. Review ARTY Z7-20 reference manual
3. Consult Xilinx community forums
4. Check hardware connections and power supply