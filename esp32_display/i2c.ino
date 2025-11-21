// i2c.ino - I2C GIF receiver for ESP32 display
// Receives GIF data over I2C and stores to SPIFFS (immediate display) or SD card (later)
// I2C pins: SDA=GPIO13, SCL=GPIO14

#include <Wire.h>

// I2C configuration
#define I2C_SDA_PIN 13
#define I2C_SCL_PIN 14
#define I2C_SLAVE_ADDR 0x42  // ESP32 I2C slave address

// Protocol constants
#define I2C_CMD_START_TRANSFER 0x10  // Match keyboard HID_REPORT_ID_START_GIF
#define I2C_CMD_DATA_CHUNK 0x11      // Match keyboard HID_REPORT_ID_GIF_DATA
#define I2C_CMD_END_TRANSFER 0x12    // Match keyboard HID_REPORT_ID_END_GIF
#define I2C_CMD_ABORT 0x04

// Transfer destination flags
#define DEST_SPIFFS_IMMEDIATE 0x01  // Save to SPIFFS and display now (DEST_SCREEN)
#define DEST_SD_LATER 0x02          // Save to SD card for later (DEST_SD_CARD)

// Transfer state
enum TransferState {
  IDLE,
  RECEIVING_METADATA,
  RECEIVING_DATA,
  FINALIZING
};

struct I2CTransferContext {
  TransferState state;
  uint8_t destination;        // DEST_SPIFFS_IMMEDIATE or DEST_SD_LATER
  String filename;
  uint32_t totalSize;
  uint32_t receivedBytes;
  uint32_t expectedCRC;
  File outputFile;
  bool active;
} i2cTransfer;

// Buffer for incoming I2C data
#define I2C_BUFFER_SIZE 256
uint8_t i2cRxBuffer[I2C_BUFFER_SIZE];
volatile uint16_t i2cRxIndex = 0;
volatile bool i2cDataReady = false;

// Forward declarations
void i2cReceiveHandler(int numBytes);
void i2cRequestHandler();
bool processI2CCommand(uint8_t *buffer, uint16_t length);
void startI2CTransfer(uint8_t destination, const char* fname, uint32_t size, uint32_t crc);
bool writeI2CDataChunk(uint8_t *data, uint16_t length);
bool finalizeI2CTransfer();
void abortI2CTransfer();
uint32_t computeFileCRC32(const char* filepath, bool useSD);

// Initialize I2C slave
void i2cInitSlave() {
  // Reset transfer state
  i2cTransfer.state = IDLE;
  i2cTransfer.active = false;
  i2cRxIndex = 0;
  i2cDataReady = false;

  // Initialize I2C on custom pins as slave
  Wire.begin(I2C_SLAVE_ADDR, I2C_SDA_PIN, I2C_SCL_PIN);
  Wire.onReceive(i2cReceiveHandler);
  Wire.onRequest(i2cRequestHandler);

  Serial.printf("I2C initialized as slave 0x%02X on SDA=%d, SCL=%d\n", 
                I2C_SLAVE_ADDR, I2C_SDA_PIN, I2C_SCL_PIN);
  Serial.println("I2C GIF receiver ready");
}

// I2C receive interrupt handler
void i2cReceiveHandler(int numBytes) {
  Serial.printf("[I2C] Received %d bytes\n", numBytes);
  i2cRxIndex = 0;
  while (Wire.available() && i2cRxIndex < I2C_BUFFER_SIZE) {
    i2cRxBuffer[i2cRxIndex++] = Wire.read();
  }
  if (i2cRxIndex > 0) {
    i2cDataReady = true;
    Serial.printf("[I2C] Buffer ready with %d bytes, first byte: 0x%02X\n", i2cRxIndex, i2cRxBuffer[0]);
  }
}

// I2C request handler (master requesting data from us)
void i2cRequestHandler() {
  // Send status byte: 0x00 = ready, 0x01 = busy, 0xFF = error
  uint8_t status = 0x00;
  if (i2cTransfer.active) {
    status = 0x01;
  }
  Wire.write(status);
  Serial.printf("[I2C] Status requested, sent: 0x%02X\n", status);
}

// Process incoming I2C data (call from main loop)
void i2cProcess() {
  if (i2cDataReady) {
    i2cDataReady = false;
    processI2CCommand(i2cRxBuffer, i2cRxIndex);
  }
}

// Process I2C command
bool processI2CCommand(uint8_t *buffer, uint16_t length) {
  if (length < 1) return false;

  uint8_t cmd = buffer[0];
  Serial.printf("[I2C] Processing command 0x%02X, length=%d\n", cmd, length);

  switch (cmd) {
    case I2C_CMD_START_TRANSFER: {
      // Format: CMD(1) | DEST(1) | FILENAME_LEN(1) | FILENAME(...) | SIZE(4) | CRC(4)
      if (length < 7) {
        Serial.println("I2C: START_TRANSFER too short");
        return false;
      }
      uint8_t dest = buffer[1];
      uint8_t fnameLen = buffer[2];
      if (length < 7 + fnameLen) {
        Serial.println("I2C: START_TRANSFER invalid length");
        return false;
      }
      
      char fname[64];
      memcpy(fname, &buffer[3], fnameLen);
      fname[fnameLen] = '\0';
      
      uint32_t size = (uint32_t)buffer[3 + fnameLen] |
                      ((uint32_t)buffer[4 + fnameLen] << 8) |
                      ((uint32_t)buffer[5 + fnameLen] << 16) |
                      ((uint32_t)buffer[6 + fnameLen] << 24);
      
      uint32_t crc = (uint32_t)buffer[7 + fnameLen] |
                     ((uint32_t)buffer[8 + fnameLen] << 8) |
                     ((uint32_t)buffer[9 + fnameLen] << 16) |
                     ((uint32_t)buffer[10 + fnameLen] << 24);
      
      startI2CTransfer(dest, fname, size, crc);
      return true;
    }

    case I2C_CMD_DATA_CHUNK: {
      // Format: CMD(1) | DATA(...)
      Serial.printf("[I2C] DATA_CHUNK received, length=%d\n", length);
      if (i2cTransfer.state != RECEIVING_DATA) {
        Serial.printf("I2C: DATA_CHUNK received but not in RECEIVING_DATA state (state=%d)\n", i2cTransfer.state);
        return false;
      }
      return writeI2CDataChunk(&buffer[1], length - 1);
    }

    case I2C_CMD_END_TRANSFER: {
      Serial.println("[I2C] END_TRANSFER received");
      return finalizeI2CTransfer();
    }

    case I2C_CMD_ABORT: {
      abortI2CTransfer();
      return true;
    }

    default:
      Serial.printf("I2C: Unknown command 0x%02X\n", cmd);
      return false;
  }
}

// Start a new I2C transfer
void startI2CTransfer(uint8_t destination, const char* fname, uint32_t size, uint32_t crc) {
  // Abort any existing transfer
  if (i2cTransfer.active) {
    abortI2CTransfer();
  }

  i2cTransfer.state = RECEIVING_DATA;
  i2cTransfer.destination = destination;
  i2cTransfer.filename = String(fname);
  i2cTransfer.totalSize = size;
  i2cTransfer.receivedBytes = 0;
  i2cTransfer.expectedCRC = crc;
  i2cTransfer.active = true;

  // Open file for writing
  String filepath;
  if (destination == DEST_SPIFFS_IMMEDIATE) {
    filepath = "/current.gif";  // Overwrite current SPIFFS GIF
    if (spiMutex) xSemaphoreTake(spiMutex, portMAX_DELAY);
    i2cTransfer.outputFile = SPIFFS.open(filepath.c_str(), FILE_WRITE);
    if (spiMutex) xSemaphoreGive(spiMutex);
    Serial.printf("I2C: Starting SPIFFS transfer -> %s (%u bytes)\n", filepath.c_str(), size);
  } else if (destination == DEST_SD_LATER) {
    filepath = "/" + i2cTransfer.filename;
    if (spiMutex) xSemaphoreTake(spiMutex, portMAX_DELAY);
    i2cTransfer.outputFile = SD.open(filepath.c_str(), FILE_WRITE);
    if (spiMutex) xSemaphoreGive(spiMutex);
    Serial.printf("I2C: Starting SD transfer -> %s (%u bytes)\n", filepath.c_str(), size);
  } else {
    Serial.printf("I2C: Invalid destination 0x%02X\n", destination);
    abortI2CTransfer();
    return;
  }

  if (!i2cTransfer.outputFile) {
    Serial.println("I2C: Failed to open output file");
    abortI2CTransfer();
    return;
  }

  Serial.println("I2C: Transfer started");
}

// Write a data chunk
bool writeI2CDataChunk(uint8_t *data, uint16_t length) {
  if (!i2cTransfer.outputFile) {
    Serial.println("I2C: No output file open");
    return false;
  }

  if (spiMutex) xSemaphoreTake(spiMutex, portMAX_DELAY);
  size_t written = i2cTransfer.outputFile.write(data, length);
  if (spiMutex) xSemaphoreGive(spiMutex);

  if (written != length) {
    Serial.printf("I2C: Write failed (wrote %d of %d bytes)\n", written, length);
    abortI2CTransfer();
    return false;
  }

  i2cTransfer.receivedBytes += written;
  
  // Progress indication every 10KB
  if (i2cTransfer.receivedBytes % 10240 == 0 || i2cTransfer.receivedBytes >= i2cTransfer.totalSize) {
    Serial.printf("I2C: Progress %u/%u bytes (%.1f%%)\n", 
                  i2cTransfer.receivedBytes, i2cTransfer.totalSize,
                  (float)i2cTransfer.receivedBytes * 100.0 / i2cTransfer.totalSize);
  }

  return true;
}

// Finalize transfer and verify CRC
bool finalizeI2CTransfer() {
  if (!i2cTransfer.active) {
    Serial.println("I2C: No active transfer to finalize");
    return false;
  }

  // Close file
  if (i2cTransfer.outputFile) {
    if (spiMutex) xSemaphoreTake(spiMutex, portMAX_DELAY);
    i2cTransfer.outputFile.close();
    if (spiMutex) xSemaphoreGive(spiMutex);
  }

  // Check size
  if (i2cTransfer.receivedBytes != i2cTransfer.totalSize) {
    Serial.printf("I2C: Size mismatch! Expected %u, got %u\n", 
                  i2cTransfer.totalSize, i2cTransfer.receivedBytes);
    abortI2CTransfer();
    return false;
  }

  // Verify CRC
  String filepath;
  bool useSD = (i2cTransfer.destination == DEST_SD_LATER);
  if (useSD) {
    filepath = "/" + i2cTransfer.filename;
  } else {
    filepath = "/current.gif";
  }

  uint32_t computedCRC = computeFileCRC32(filepath.c_str(), useSD);
  if (computedCRC != i2cTransfer.expectedCRC) {
    Serial.printf("I2C: CRC mismatch! Expected 0x%08X, got 0x%08X\n", 
                  i2cTransfer.expectedCRC, computedCRC);
    abortI2CTransfer();
    return false;
  }

  Serial.printf("I2C: Transfer complete! %s (%u bytes, CRC OK)\n", 
                filepath.c_str(), i2cTransfer.receivedBytes);

  // If immediate display, trigger GIF reload
  if (i2cTransfer.destination == DEST_SPIFFS_IMMEDIATE) {
    Serial.println("I2C: SPIFFS GIF ready for immediate display");
    // The main loop will detect the new /current.gif and play it
  }

  // Reset state
  i2cTransfer.state = IDLE;
  i2cTransfer.active = false;
  return true;
}

// Abort current transfer
void abortI2CTransfer() {
  if (i2cTransfer.outputFile) {
    if (spiMutex) xSemaphoreTake(spiMutex, portMAX_DELAY);
    i2cTransfer.outputFile.close();
    if (spiMutex) xSemaphoreGive(spiMutex);
  }
  
  i2cTransfer.state = IDLE;
  i2cTransfer.active = false;
  Serial.println("I2C: Transfer aborted");
}

// Compute CRC32 of a file
uint32_t computeFileCRC32(const char* filepath, bool useSD) {
  File f;
  if (spiMutex) xSemaphoreTake(spiMutex, portMAX_DELAY);
  if (useSD) {
    f = SD.open(filepath, FILE_READ);
  } else {
    f = SPIFFS.open(filepath, FILE_READ);
  }
  if (spiMutex) xSemaphoreGive(spiMutex);

  if (!f) {
    Serial.printf("I2C: Failed to open %s for CRC\n", filepath);
    return 0;
  }

  uint32_t crc = 0xFFFFFFFF;
  uint8_t buffer[512];
  
  while (f.available()) {
    if (spiMutex) xSemaphoreTake(spiMutex, portMAX_DELAY);
    size_t bytesRead = f.read(buffer, sizeof(buffer));
    if (spiMutex) xSemaphoreGive(spiMutex);
    
    for (size_t i = 0; i < bytesRead; i++) {
      crc = crc ^ buffer[i];
      for (int j = 0; j < 8; j++) {
        if (crc & 1) {
          crc = (crc >> 1) ^ 0xEDB88320;
        } else {
          crc = crc >> 1;
        }
      }
    }
  }

  if (spiMutex) xSemaphoreTake(spiMutex, portMAX_DELAY);
  f.close();
  if (spiMutex) xSemaphoreGive(spiMutex);

  return crc ^ 0xFFFFFFFF;
}
