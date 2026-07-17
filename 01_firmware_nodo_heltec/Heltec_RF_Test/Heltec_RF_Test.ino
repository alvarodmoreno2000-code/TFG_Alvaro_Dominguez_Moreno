#include <Arduino.h>
#include <RadioLib.h>
#include <SPI.h>

// ============================================================
// PRUEBA RF DEL HELTEC WIFI LORA 32 V4
// Transmisión LoRa directa en 868,1 MHz
// ============================================================


// ============================================================
// 1. PINES DEL TRANSCEPTOR SX1262
// ============================================================

// Pines del bus SPI y señales de control del SX1262.
// Son los mismos pines que ya se habían validado en las pruebas
// LoRa anteriores.

#define LORA_NSS   8
#define LORA_SCK   9
#define LORA_MOSI  10
#define LORA_MISO  11
#define LORA_RST   12
#define LORA_BUSY  13
#define LORA_DIO1  14


// ============================================================
// 2. CONTROL DE ALIMENTACIÓN Y FRONTAL RF DEL HELTEC V4
// ============================================================

// Control de alimentación externa de la placa.
// En el Heltec V4 esta señal es activa a nivel bajo.
#define VEXT_CTRL   36

// Habilitación del amplificador/frontal RF.
#define FEM_ENABLE  2

// Alimentación del frontal RF del Heltec V4.
#define VFEM_CTRL   7

// Control del modo de ganancia del frontal RF.
#define FEM_MODE    46


// ============================================================
// 3. CREACIÓN DEL OBJETO RADIO SX1262
// ============================================================

// Se crea el objeto que representa el transceptor SX1262.
//
// Orden de parámetros:
// NSS, DIO1, RESET y BUSY.

SX1262 radio = new Module(
  LORA_NSS,
  LORA_DIO1,
  LORA_RST,
  LORA_BUSY
);


// ============================================================
// 4. VARIABLES DE PRUEBA
// ============================================================

// Contador utilizado para identificar cada trama transmitida.
uint32_t contador = 0;


// ============================================================
// 5. CONFIGURACIÓN INICIAL
// ============================================================

void setup() {

  // Inicialización del monitor serie.
  Serial.begin(115200);
  delay(2000);

  Serial.println();
  Serial.println("========================================");
  Serial.println(" PRUEBA RF HELTEC V4 - LORA DIRECTA");
  Serial.println("========================================");


  // ----------------------------------------------------------
  // 5.1. Habilitación de alimentación y frontal RF
  // ----------------------------------------------------------

  pinMode(VEXT_CTRL, OUTPUT);
  digitalWrite(VEXT_CTRL, LOW);

  pinMode(FEM_ENABLE, OUTPUT);
  digitalWrite(FEM_ENABLE, HIGH);

  pinMode(VFEM_CTRL, OUTPUT);
  digitalWrite(VFEM_CTRL, HIGH);

  pinMode(FEM_MODE, OUTPUT);
  digitalWrite(FEM_MODE, HIGH);

  Serial.println("[OK] Alimentación y frontal RF habilitados");


  // ----------------------------------------------------------
  // 5.2. Inicialización explícita del bus SPI
  // ----------------------------------------------------------

  // Orden:
  // SCK, MISO, MOSI y NSS.

  SPI.begin(
    LORA_SCK,
    LORA_MISO,
    LORA_MOSI,
    LORA_NSS
  );

  Serial.println("[OK] Bus SPI inicializado");


  // ----------------------------------------------------------
  // 5.3. Inicialización del SX1262 en modo LoRa directo
  // ----------------------------------------------------------

  // Parámetros usados en la prueba:
  //
  // Frecuencia:       868,1 MHz
  // Ancho de banda:   125 kHz
  // Spreading Factor: SF7
  // Coding Rate:      4/5
  // Sync word:        0x34
  // Potencia:         14 dBm
  // Preámbulo:        8 símbolos
  // TCXO:             1,8 V
  //
  // El sync word 0x34 se utiliza porque el gateway está ahora
  // configurado en modo LoRaWAN público.

  int16_t state = radio.begin(
    868.1,
    125.0,
    7,
    5,
    0x34,
    14,
    8,
    1.8
  );

  if (state != RADIOLIB_ERR_NONE) {
    Serial.print("[ERROR] No se pudo inicializar el SX1262: ");
    Serial.println(state);

    while (true) {
      delay(1000);
    }
  }

  Serial.println("[OK] Transceptor SX1262 inicializado");


  // ----------------------------------------------------------
  // 5.4. Configuración del conmutador RF
  // ----------------------------------------------------------

  // En SX1262, DIO2 puede controlar automáticamente el cambio
  // entre transmisión y recepción en el conmutador RF.

  state = radio.setDio2AsRfSwitch(true);

  if (state != RADIOLIB_ERR_NONE) {
    Serial.print("[ERROR] No se pudo configurar DIO2: ");
    Serial.println(state);

    while (true) {
      delay(1000);
    }
  }

  Serial.println("[OK] Conmutador RF configurado mediante DIO2");
  Serial.println("[READY] Comienza la prueba de transmisión");
  Serial.println();
}


// ============================================================
// 6. BUCLE PRINCIPAL DE TRANSMISIÓN
// ============================================================

void loop() {

  // ----------------------------------------------------------
  // 6.1. Construcción del mensaje de prueba
  // ----------------------------------------------------------

  char mensaje[40];

  snprintf(
    mensaje,
    sizeof(mensaje),
    "HELTEC_V4_TEST,%lu",
    (unsigned long)contador
  );

  Serial.print("[TX] ");
  Serial.println(mensaje);


  // ----------------------------------------------------------
  // 6.2. Transmisión LoRa directa
  // ----------------------------------------------------------

  int16_t state = radio.transmit(mensaje);

  if (state == RADIOLIB_ERR_NONE) {
    Serial.println("[OK] Transmisión completada");
  } else {
    Serial.print("[ERROR] Fallo de transmisión: ");
    Serial.println(state);
  }

  contador++;


  // ----------------------------------------------------------
  // 6.3. Espera antes de la siguiente transmisión
  // ----------------------------------------------------------

  delay(5000);
}