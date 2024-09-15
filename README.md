# NovaLink

![License](https://img.shields.io/badge/license-MIT-blue.svg)
![Build Status](https://img.shields.io/badge/build-passing-brightgreen.svg)
![Version](https://img.shields.io/badge/version-1.0.0-blue.svg)

## Overview

**NovaLink** is a high-performance, reliable two-way communication software package designed to establish a seamless link between a model rocket's flight computer and a ground control station during flight. Engineered to handle real-time data transmission in challenging environments prone to noise and interference, NovaLink enhances rocket telemetry by ensuring robust and efficient communication.

## Key Features

- **Real-Time Communication:** Facilitates timely transmission of commands from the ground to the rocket and telemetry data from the rocket back to the ground.
- **Reliability:** Ensures data integrity and minimizes packet loss with robust error detection and correction mechanisms.
- **Performance Optimization:** Achieves low latency and efficient CPU usage to meet the stringent timing requirements of active control systems.
- **Flexibility and Modularity:** Features an abstraction layer supporting various physical communication interfaces and easy integration into diverse systems.

## Core Components

### All Vehicle Communications (AVC) Protocol

- **Command Structure:**
  - **Unique Command Identifiers:** Each command is assigned a unique number (e.g., 101 for a fin test).
  - **Header Bytes:**
    - **First Byte:** Contains sender and receiver IDs, enabling multiple devices to communicate on the same network without confusion.
    - **Second Byte:** Acts as a payload descriptor, indicating the type of command or telemetry data.
  - **Payload:** Contains command-specific parameters, optimized for size and precision using appropriate data types (e.g., 16-bit integers for voltages in millivolts).
  - **Command Acknowledgment:** Implements an acknowledgment system where the receiver confirms receipt of commands. Unacknowledged commands are stored with timestamps and retransmitted until acknowledged or timed out.

- **Telemetry Structure:**
  - **Multiple Payload Types:** Supports different telemetry payloads (e.g., Telemetry A, B) identified by unique descriptors.
  - **Data Fields:**
    - **Voltage Measurements:** Uses unsigned 16-bit integers to represent voltages with millivolt precision.
    - **Position, Velocity, Acceleration:** Uses signed 16-bit integers with scaling factors to balance range and precision.
    - **Memory Usage:** Represents percentages using unsigned 8-bit integers.
    - **Status Flags:** Packs multiple boolean values into a single byte using bit manipulation.

### Systems Communications Asynchronous Protocol Lightweight (SCALPEL)

- **Packet Structure:**
  - **Start Byte:** A fixed value (170 decimal) that signals the beginning of a packet.
  - **Payload Length Byte:** Includes the length of the payload and a checksum for error detection.
  - **COBS Byte:** Utilizes Consistent Overhead Byte Stuffing (COBS) to prevent the start byte value from appearing in the payload.
  - **Payload:** Contains the AVC-encoded data.
  - **Checksum Byte:** Calculated over the payload to detect errors.

- **Error Handling:**
  - **Checksum Verification:** Ensures that the payload has not been corrupted during transmission.
  - **COBS Encoding:** Prevents misinterpretation of the start byte within the payload, avoiding synchronization issues.
  - **Payload Length and COBS Byte Checksums:** Additional checksums verify the integrity of critical packet components.

### Physical Layer Abstraction

- **Supported Devices:**
  - **XBee Pro 900 HP Radio:** A 900 MHz radio module commonly used for telemetry.
  - **RFD900 Radio Module:** An alternative that may offer different performance characteristics.

- **Implementation:**
  - **RadioInterface Class:** Defines a generic interface with methods for sending and receiving data, configuring settings, and monitoring status.
  - **Specific Drivers:** (e.g., `XBeePro900HP`, `RFD900`) inherit from `RadioInterface` and implement hardware-specific functionality.

### Command and Telemetry Management

- **Command Queue:**
  - Implements a priority-based queue to manage outgoing commands.
  - Prioritizes critical commands over less urgent ones.
  - Handles timeouts and retransmissions based on acknowledgments received.

- **Telemetry Buffer:**
  - Uses a circular buffer to store incoming telemetry data efficiently.
  - Ensures that the most recent data is readily accessible while older data is overwritten as needed.

- **Thread-Safe Operations:**
  - Employs synchronization mechanisms (e.g., mutexes, lock-free structures) to allow safe access to shared resources in a multithreaded environment.

### Performance Optimization

- **Low Latency and High Throughput:**
  - Aims for processing latency under 10 milliseconds and the ability to handle at least 100 packets per second.
  
- **Efficient Memory Management:**
  - Uses memory pools to reduce the overhead of frequent dynamic memory allocations.
  
- **Lock-Free Algorithms:**
  - Implements lock-free data structures where possible to minimize synchronization overhead and avoid bottlenecks.
  
- **Critical Path Optimization:**
  - Profiles and optimizes performance-critical sections of code to reduce CPU usage (targeting less than 5% on typical embedded processors).

### Diagnostics and Monitoring

- **Link Quality Metrics:**
  - Monitors signal strength, noise levels, and other parameters to assess the health of the communication link.
  
- **Packet Loss Tracking:**
  - Keeps statistics on sent, received, and lost packets to identify issues.
  
- **Latency Measurement:**
  - Measures round-trip times and processing delays to ensure that the system meets real-time requirements.
  
- **Logging:**
  - Provides detailed logs for debugging and analysis using a centralized logging utility.

### API Development

- **User-Friendly Interface:**
  - Offers a clear and intuitive API for initializing the communication system, sending commands, and receiving telemetry data.
  
- **Event Callbacks:**
  - Allows users to register callback functions that are invoked upon specific events (e.g., new telemetry received, command acknowledgment).
  
- **Comprehensive Documentation:**
  - Includes detailed documentation and examples to facilitate integration with other systems.
  
- **Cross-Platform Compatibility:**
  - Designed to work on both Linux (for the ground station) and real-time operating systems used by the flight computer.

## Technical Challenges & Solutions

- **Reliable Communication in Harsh Environments:**
  - **Challenge:** Radio communications during rocket flight are subject to interference, signal attenuation, and other issues.
  - **Solution:** Implement robust error detection and correction mechanisms within the SCALPEL protocol, use checksums, and design the system to handle packet loss gracefully.

- **Efficient Use of Limited Resources:**
  - **Challenge:** Embedded systems on the rocket have limited processing power and memory.
  - **Solution:** Optimize code for performance, use efficient data structures, and minimize CPU and memory usage through techniques like memory pooling and lock-free algorithms.

- **Synchronization and Timing:**
  - **Challenge:** Real-time systems require precise timing and synchronization to function correctly.
  - **Solution:** Use high-resolution timers, prioritize critical tasks, and design the system to meet stringent timing constraints.

- **Scalability and Flexibility:**
  - **Challenge:** The system should be adaptable to different hardware configurations and future upgrades.
  - **Solution:** Abstract hardware-specific details using interfaces, allowing for easy integration of new radio modules or communication mediums without significant code changes.

## Practical Application Example

**During a Rocket Launch:**

- **Pre-Launch:**
  - The ground control station sends a series of commands to the rocket to configure systems, perform pre-flight checks, or initiate calibration routines.
  - NovaLink ensures these commands are reliably transmitted, acknowledged, and any necessary retransmissions are handled automatically.

- **In Flight:**
  - The rocket's flight computer continuously sends telemetry data back to the ground station.
  - This data includes critical parameters such as altitude, speed, orientation, system voltages, and status flags indicating the health of various subsystems.
  - The ground station uses this data to monitor the rocket's performance in real time and can send commands to adjust flight parameters if necessary.

- **Post-Flight:**
  - After the rocket lands, the system may continue to transmit data or receive commands for post-flight analysis and data retrieval.

## Overall Architecture and Workflow

1. **Initialization:**
   - The NovaLink main class initializes all components, sets up the physical layer interface, and prepares the system for communication.

2. **Command Transmission:**
   - Commands are created and passed to the CommandManager, which queues them based on priority.
   - The AVCProtocol encodes the commands, which are then packetized by the SCALPEL layer.
   - The packet is transmitted via the physical layer (radio module), where the hardware driver handles the low-level transmission details.

3. **Telemetry Reception:**
   - Incoming data is received by the radio module and passed up to the SCALPEL layer.
   - The SCALPEL layer unpacks the packet, verifies checksums, and passes the payload to the AVCProtocol.
   - The AVCProtocol decodes the telemetry data, which is then stored in the TelemetryBuffer.
   - Event callbacks notify the ground station software of new telemetry data for processing or display.

4. **Error Handling and Retransmission:**
   - If errors are detected (e.g., checksum failures), the system handles them according to predefined policies (e.g., request retransmission, log the error).
   - The CommandManager handles retransmission of unacknowledged commands based on timeouts.

## Key Technologies and Standards

- **Programming Language:** C++17 or later, leveraging modern language features for performance and safety.
- **Build System:** CMake, for cross-platform build configuration and management.
- **Multithreading:** Utilizes C++11 threading libraries for concurrent operations.
- **Synchronization Primitives:** Employs mutexes, condition variables, and atomic operations to ensure thread safety.

## Benefits

- **Enhanced Reliability:** Robust protocols and error handling mechanisms reduce the risk of communication failures.
- **Real-Time Performance:** Optimized for low latency and high throughput, meeting the demands of active control systems.
- **Flexibility:** Hardware abstraction allows for easy adaptation to different communication interfaces and future technologies.
- **Ease of Integration:** A well-designed API and comprehensive documentation simplify the integration process with other software systems.
- **Scalability:** Modular design enables the system to be extended or modified with minimal impact on existing components.

## Getting Started

### Prerequisites

- **C++17 Compiler:** Ensure you have a compatible C++17 compiler installed.
- **CMake:** Version 3.10 or later.
- **Supported Radio Modules:** XBee Pro 900 HP or RFD900.

### Installation

1. **Clone the Repository:**
   ```bash
   git clone https://github.com/yourusername/NovaLink.git
   cd NovaLink
   ```

2. **Build the Project:**
   ```bash
   mkdir build
   cd build
   cmake ..
   make
   ```

3. **Configure Radio Module:**
   - Follow the instructions in the `docs/` directory to configure your specific radio module.

### Usage

1. **Initialize NovaLink:**
   ```cpp
   #include "NovaLink.h"

   int main() {
       NovaLink novaLink;
       novaLink.initialize();
       // Additional setup...
   }
   ```

2. **Send Commands:**
   ```cpp
   Command cmd = createFinTestCommand();
   novaLink.sendCommand(cmd);
   ```

3. **Receive Telemetry:**
   ```cpp
   novaLink.registerTelemetryCallback([](Telemetry data) {
       // Process telemetry data
   });
   ```

For detailed examples and API documentation, refer to the [Documentation](docs/) folder.

## Contributing

Contributions are welcome! Please follow these steps:

1. **Fork the Repository**
2. **Create a Feature Branch:**
   ```bash
   git checkout -b feature/YourFeature
   ```
3. **Commit Your Changes:**
   ```bash
   git commit -m "Add your feature"
   ```
4. **Push to the Branch:**
   ```bash
   git push origin feature/YourFeature
   ```
5. **Open a Pull Request**

Please see our [Contributing Guidelines](CONTRIBUTING.md) for more details.

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.

## Acknowledgments

- Special thanks to Lafayette Systems for the inspiration behind this project.

## Contact

For questions or support, please open an issue on [GitHub Issues](https://github.com/parkerjbeard/NovaLink/issues) or contact [parkerjohnsonbeard@gmail.com](mailto:parkerjohnsonbeard@gmail.com).

---