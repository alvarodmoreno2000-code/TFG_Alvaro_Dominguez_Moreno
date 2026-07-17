#include <Arduino.h>
#include <SPI.h>
#include <RadioLib.h>

// ============================================================
// HELTEC WIFI LORA 32 V4 + SX1262
// TRANSMISOR LORA P2P PARA CAPTURA RSSI/SNR EN RASPBERRY
// ============================================================

// Pines SX1262 Heltec WiFi LoRa 32 V4
#define LORA_NSS   8
#define LORA_SCK   9
#define LORA_MOSI  10
#define LORA_MISO  11
#define LORA_RST   12
#define LORA_BUSY  13
#define LORA_DIO1  14

// Alimentación / frontal RF Heltec V4
#define VEXT_CTRL  36
#define RF_CTRL    2
#define VFEM_CTRL  7
#define FEM_MODE   46

// Configuración LoRa P2P
const float FREQ_MHZ = 868.1;
const float BW_KHZ   = 125.0;
const uint8_t SF     = 7;
const uint8_t CR     = 5;      // 4/5
const int8_t PWR_DBM = 14;

// Identificador del nodo
const char* NODE_ID = "Heltec01";

// Objeto radio
SX1262 radio = new Module(LORA_NSS, LORA_DIO1, LORA_RST, LORA_BUSY);

uint32_t seq = 0;

// Tabla RF switch Heltec V4
static const uint32_t rfswitch_pins[Module::RFSWITCH_MAX_PINS] = {
  RF_CTRL,
  FEM_MODE,
  RADIOLIB_NC,
  RADIOLIB_NC,
  RADIOLIB_NC
};

static const Module::RfSwitchMode_t rfswitch_table[] = {
  { Module::MODE_IDLE, { LOW,  LOW  } },
  { Module::MODE_RX,   { HIGH, LOW  } },
  { Module::MODE_TX,   { HIGH, HIGH } },
  END_OF_MODE_TABLE
};

void detenerPorError(const char* mensaje, int16_t codigo) {
  Serial.print("[ERROR] ");
  Serial.print(mensaje);
  Serial.print(" | Codigo RadioLib: ");
  Serial.println(codigo);

  while (true) {
    delay(1000);
  }
}

void setup() {
  Serial.begin(115200);
  delay(2000);

  Serial.println();
  Serial.println("==============================================");
  Serial.println(" HELTEC WIFI LORA 32 V4 - TX LORA P2P TFG");
  Serial.println("==============================================");

  // Alimentación externa de la placa
  pinMode(VEXT_CTRL, OUTPUT);
  digitalWrite(VEXT_CTRL, LOW);

  // Habilitación del frontal RF
  pinMode(VFEM_CTRL, OUTPUT);
  digitalWrite(VFEM_CTRL, HIGH);

  // Control automático RF switch por RadioLib
  radio.setRfSwitchTable(rfswitch_pins, rfswitch_table);

  Serial.println("[OK] Alimentacion y tabla RF configuradas");

  // Inicialización SPI
  SPI.begin(LORA_SCK, LORA_MISO, LORA_MOSI, LORA_NSS);
  Serial.println("[OK] Bus SPI inicializado");

  // Inicialización SX1262
  int16_t state = radio.begin(
    FREQ_MHZ,
    BW_KHZ,
    SF,
    CR,
    0x12,       // Sync word privado LoRa P2P
    PWR_DBM,
    8,
    1.8         // TCXO Heltec V4
  );

  if (state != RADIOLIB_ERR_NONE) {
    detenerPorError("No se pudo inicializar el SX1262", state);
  }

  Serial.println("[OK] Transceptor SX1262 inicializado");

  // DIO2 como RF switch
  state = radio.setDio2AsRfSwitch(true);

  if (state != RADIOLIB_ERR_NONE) {
    detenerPorError("No se pudo configurar DIO2 como RF switch", state);
  }

  Serial.println("[OK] DIO2 configurado como RF switch");

  Serial.println("[INFO] Configuracion LoRa P2P:");
  Serial.print("  Nodo: "); Serial.println(NODE_ID);
  Serial.print("  Frecuencia: "); Serial.print(FREQ_MHZ); Serial.println(" MHz");
  Serial.print("  BW: "); Serial.print(BW_KHZ); Serial.println(" kHz");
  Serial.print("  SF: "); Serial.println(SF);
  Serial.print("  CR: 4/"); Serial.println(CR);
  Serial.print("  Potencia: "); Serial.print(PWR_DBM); Serial.println(" dBm");
  Serial.println("----------------------------------------------");
}

void loop() {
  seq++;

  String payload = "LORA1,";
  payload += NODE_ID;
  payload += ",";
  payload += String(seq);
  payload += ",";
  payload += String(millis());
  payload += ",";
  payload += String(FREQ_MHZ, 1);
  payload += ",";
  payload += String(BW_KHZ, 1);
  payload += ",";
  payload += String(SF);
  payload += ",";
  payload += String(CR);
  payload += ",";
  payload += String(PWR_DBM);

  Serial.print("[TX] ");
  Serial.println(payload);

  int16_t state = radio.transmit(payload);

  if (state == RADIOLIB_ERR_NONE) {
    Serial.println("[TX] done");
  } else {
    Serial.print("[TX] failed | Codigo RadioLib: ");
    Serial.println(state);
  }

  delay(3000);
}