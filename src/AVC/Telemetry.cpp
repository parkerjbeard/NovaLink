#include "Telemetry.hpp"
#include <stdexcept> // For exceptions

namespace RocketLink {
namespace AVC {

Telemetry::Telemetry(uint8_t senderID,
                     uint8_t receiverID,
                     TelemetryDescriptor descriptorType,
                     uint16_t v1,
                     uint16_t v2,
                     int16_t px,
                     int16_t py,
                     int16_t pz,
                     int16_t vx,
                     int16_t vy,
                     int16_t vz,
                     int16_t ax,
                     int16_t ay,
                     int16_t az,
                     uint8_t memLog[3],
                     uint8_t status)
    : descriptor(descriptorType),
      voltage1(v1),
      voltage2(v2),
      posX(px),
      posY(py),
      posZ(pz),
      velX(vx),
      velY(vy),
      velZ(vz),
      accX(ax),
      accY(ay),
      accZ(az),
      statusFlags(status) {
    header.senderID = senderID;
    header.receiverID = receiverID;
    std::memcpy(memoryLog, memLog, 3);
}

std::vector<uint8_t> Telemetry::encode() const {
    std::vector<uint8_t> encoded;
    // Header: Sender and Receiver IDs packed into one byte
    encoded.push_back(header.pack());

    // Payload Descriptor
    encoded.push_back(static_cast<uint8_t>(descriptor));

    // Voltage Measurements (4 bytes)
    encoded.push_back(static_cast<uint8_t>((voltage1 >> 8) & 0xFF));
    encoded.push_back(static_cast<uint8_t>(voltage1 & 0xFF));
    encoded.push_back(static_cast<uint8_t>((voltage2 >> 8) & 0xFF));
    encoded.push_back(static_cast<uint8_t>(voltage2 & 0xFF));

    // Position (6 bytes)
    encoded.push_back(static_cast<uint8_t>((posX >> 8) & 0xFF));
    encoded.push_back(static_cast<uint8_t>(posX & 0xFF));
    encoded.push_back(static_cast<uint8_t>((posY >> 8) & 0xFF));
    encoded.push_back(static_cast<uint8_t>(posY & 0xFF));
    encoded.push_back(static_cast<uint8_t>((posZ >> 8) & 0xFF));
    encoded.push_back(static_cast<uint8_t>(posZ & 0xFF));

    // Velocity (6 bytes)
    encoded.push_back(static_cast<uint8_t>((velX >> 8) & 0xFF));
    encoded.push_back(static_cast<uint8_t>(velX & 0xFF));
    encoded.push_back(static_cast<uint8_t>((velY >> 8) & 0xFF));
    encoded.push_back(static_cast<uint8_t>(velY & 0xFF));
    encoded.push_back(static_cast<uint8_t>((velZ >> 8) & 0xFF));
    encoded.push_back(static_cast<uint8_t>(velZ & 0xFF));

    // Acceleration (6 bytes)
    encoded.push_back(static_cast<uint8_t>((accX >> 8) & 0xFF));
    encoded.push_back(static_cast<uint8_t>(accX & 0xFF));
    encoded.push_back(static_cast<uint8_t>((accY >> 8) & 0xFF));
    encoded.push_back(static_cast<uint8_t>(accY & 0xFF));
    encoded.push_back(static_cast<uint8_t>((accZ >> 8) & 0xFF));
    encoded.push_back(static_cast<uint8_t>(accZ & 0xFF));

    // Memory Usage (3 bytes)
    encoded.insert(encoded.end(), memoryLog, memoryLog + 3);

    // Status Flags (1 byte)
    encoded.push_back(statusFlags);

    return encoded;
}

Telemetry Telemetry::decode(const std::vector<uint8_t>& data) {
    size_t expectedSize = 1 + 1 + 4 + 6 + 6 + 6 + 3 + 1; // 1 header, 1 descriptor, 4 voltage, 6 pos, 6 vel, 6 acc, 3 memory, 1 status
    if (data.size() < expectedSize) {
        throw std::invalid_argument("Data too short to decode Telemetry.");
    }

    TelemetryHeader hdr;
    hdr.unpack(data[0]);

    TelemetryDescriptor descriptorType = static_cast<TelemetryDescriptor>(data[1]);

    // Extract Voltage Measurements
    uint16_t v1 = (static_cast<uint16_t>(data[2]) << 8) | data[3];
    uint16_t v2 = (static_cast<uint16_t>(data[4]) << 8) | data[5];

    // Extract Position
    int16_t px = static_cast<int16_t>((static_cast<uint16_t>(data[6]) << 8) | data[7]);
    int16_t py = static_cast<int16_t>((static_cast<uint16_t>(data[8]) << 8) | data[9]);
    int16_t pz = static_cast<int16_t>((static_cast<uint16_t>(data[10]) << 8) | data[11]);

    // Extract Velocity
    int16_t vx = static_cast<int16_t>((static_cast<uint16_t>(data[12]) << 8) | data[13]);
    int16_t vy = static_cast<int16_t>((static_cast<uint16_t>(data[14]) << 8) | data[15]);
    int16_t vz = static_cast<int16_t>((static_cast<uint16_t>(data[16]) << 8) | data[17]);

    // Extract Acceleration
    int16_t ax = static_cast<int16_t>((static_cast<uint16_t>(data[18]) << 8) | data[19]);
    int16_t ay = static_cast<int16_t>((static_cast<uint16_t>(data[20]) << 8) | data[21]);
    int16_t az = static_cast<int16_t>((static_cast<uint16_t>(data[22]) << 8) | data[23]);

    // Extract Memory Usage
    uint8_t memLog[3];
    memLog[0] = data[24];
    memLog[1] = data[25];
    memLog[2] = data[26];

    // Extract Status Flags
    uint8_t status = data[27];

    return Telemetry(hdr.senderID, hdr.receiverID, descriptorType,
                    v1, v2, px, py, pz, vx, vy, vz, ax, ay, az,
                    memLog, status);
}

// Getters Implementation
uint16_t Telemetry::getVoltage1() const {
    return voltage1;
}

uint16_t Telemetry::getVoltage2() const {
    return voltage2;
}

int16_t Telemetry::getPosX() const {
    return posX;
}

int16_t Telemetry::getPosY() const {
    return posY;
}

int16_t Telemetry::getPosZ() const {
    return posZ;
}

int16_t Telemetry::getVelX() const {
    return velX;
}

int16_t Telemetry::getVelY() const {
    return velY;
}

int16_t Telemetry::getVelZ() const {
    return velZ;
}

int16_t Telemetry::getAccX() const {
    return accX;
}

int16_t Telemetry::getAccY() const {
    return accY;
}

int16_t Telemetry::getAccZ() const {
    return accZ;
}

void Telemetry::getMemoryLog(uint8_t (&memLogOut)[3]) const {
    std::memcpy(memLogOut, memoryLog, 3);
}

uint8_t Telemetry::getStatusFlags() const {
    return statusFlags;
}

TelemetryDescriptor Telemetry::getDescriptor() const {
    return descriptor;
}

// Setters Implementation
void Telemetry::setDescriptor(TelemetryDescriptor descriptorType) {
    descriptor = descriptorType;
}

void Telemetry::setVoltage1(uint16_t v1) {
    voltage1 = v1;
}

void Telemetry::setVoltage2(uint16_t v2) {
    voltage2 = v2;
}

void Telemetry::setPosX(int16_t px) {
    posX = px;
}

void Telemetry::setPosY(int16_t py) {
    posY = py;
}

void Telemetry::setPosZ(int16_t pz) {
    posZ = pz;
}

void Telemetry::setVelX(int16_t vx) {
    velX = vx;
}

void Telemetry::setVelY(int16_t vy) {
    velY = vy;
}

void Telemetry::setVelZ(int16_t vz) {
    velZ = vz;
}

void Telemetry::setAccX(int16_t axVal) {
    accX = axVal;
}

void Telemetry::setAccY(int16_t ayVal) {
    accY = ayVal;
}

void Telemetry::setAccZ(int16_t azVal) {
    accZ = azVal;
}

void Telemetry::setMemoryLog(const uint8_t memLogIn[3]) {
    std::memcpy(memoryLog, memLogIn, 3);
}

void Telemetry::setStatusFlags(uint8_t status) {
    statusFlags = status;
}

} // namespace AVC
} // namespace RocketLink