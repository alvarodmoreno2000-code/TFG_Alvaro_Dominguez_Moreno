#include <Arduino.h>
#include <RadioLib.h>
#include <SPI.h>

// ============================================================
// HELTEC WIFI LORA 32 V4 + SX1262
// NODO LORAWAN OTAA PARA THE THINGS NETWORK
// ============================================================


// ============================================================
// 1. ASIGNACIÓN DE PINES DEL TRANSCEPTOR SX1262
// ============================================================

// Pines del bus SPI y señales de control del SX1262.
// Estos valores se mantienen respecto al firmware LoRa anterior
// porque ya fueron validados para la placa Heltec utilizada.

#define LORA_NSS   8
#define LORA_SCK   9
#define LORA_MOSI  10
#define LORA_MISO  11
#define LORA_RST   12
#define LORA_BUSY  13
#define LORA_DIO1  14


// ============================================================
// 2. CONTROL DE ALIMENTACIÓN Y ETAPA DE RADIOFRECUENCIA
// ============================================================

// GPIO que habilita la alimentación externa de la placa.
// En esta placa la señal es activa a nivel bajo.
#define VEXT_CTRL  36

// GPIO utilizado para habilitar la etapa de RF/antena.
#define RF_CTRL    2

// Habilita la alimentación de la etapa frontal RF del Heltec V4
#define VFEM_CTRL 7

// Control del modo de ganancia del frontal RF
#define FEM_MODE 46

// ============================================================
// 3. CREACIÓN DEL TRANSCEPTOR SX1262
// ============================================================

// Se crea el objeto que representa físicamente al SX1262.
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
// 4. CREACIÓN DEL NODO LORAWAN
// ============================================================

// Se crea el nodo LoRaWAN sobre el transceptor SX1262.
//
// EU868 indica que se utilizará el plan regional europeo
// comprendido entre 863 y 870 MHz.
//
// RadioLib gestionará automáticamente los canales,
// frecuencias, velocidades y ventanas de recepción LoRaWAN.

LoRaWANNode node(&radio, &EU868);

// ============================================================
// 4.1. TABLA DE CONMUTACIÓN DEL FRONTAL RF DEL HELTEC V4
// ============================================================
//
// El Heltec V4 utiliza pines externos para habilitar el frontal RF.
// RadioLib puede controlar estos pines automáticamente según el
// estado de la radio: reposo, recepción o transmisión.
//
// RF_CTRL habilita el frontal RF.
// FEM_MODE selecciona el modo de transmisión.
// En recepción, FEM_MODE se mantiene a LOW.
// En transmisión, FEM_MODE pasa a HIGH.

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

// ============================================================
// 5. CREDENCIALES OTAA GENERADAS EN TTN
// ============================================================

// JoinEUI configurado durante el registro del dispositivo.
// En nuestro caso se registró con todos sus valores a cero.

const uint64_t JOIN_EUI = 0x0000000000000000;


// IMPORTANTE:
// Coger el DEV_EUI de TTN 

const uint64_t DEV_EUI = 0x70B3D57ED00781AE;


// IMPORTANTE:
// Sustituir estos 16 bytes por el AppKey generado por TTN.
//
// En TTN puede copiarse seleccionando el formato:
// "C-style array" o "Array of bytes".
//
// El ejemplo siguiente contiene únicamente ceros y no permitirá
// que el dispositivo se una a la red hasta que sea reemplazado.

const uint8_t APP_KEY[16] = {
  0xE6, 0x38, 0xE0, 0xFB, 
  0x59, 0xF8, 0x45, 0x6D, 
  0xD4, 0x00, 0x57, 0x3E, 
  0x18, 0x90, 0xBE, 0x33
};


// No se utiliza NwkKey porque el dispositivo fue registrado
// mediante LoRaWAN 1.0.4. En la llamada beginOTAA se indicará
// mediante el valor NULL.


// ============================================================
// 6. IDENTIFICACIÓN Y DATOS DEL NODO
// ============================================================

// Identificador incluido en el payload para distinguir la ubicación del nodo
// durante las pruebas y la generación posterior del dataset.

const char* BEACON_ID = "H03";

// Contador incremental de las tramas transmitidas.
uint32_t seq = 0;

// Puerto de aplicación LoRaWAN utilizado para los uplinks.
// Los puertos 1-223 pueden utilizarse para datos de aplicación.
const uint8_t LORAWAN_PORT = 1;

// Se utilizarán uplinks no confirmados para no generar un
// downlink de confirmación por cada transmisión.
const bool CONFIRMED_UPLINK = false;

// Intervalo entre transmisiones: 30 segundos.
// Sustituye al periodo anterior de dos segundos.
const uint32_t UPLINK_INTERVAL_MS = 30000UL;


// ============================================================
// 7. FUNCIÓN DE DETENCIÓN EN CASO DE ERROR CRÍTICO
// ============================================================

// Si se produce un error que impide continuar, el programa
// permanece detenido y muestra el código correspondiente
// en el monitor serie.

void detenerPorError(const char* mensaje, int16_t codigo) {
  Serial.print("[ERROR] ");
  Serial.print(mensaje);
  Serial.print(" | Código RadioLib: ");
  Serial.println(codigo);

  while (true) {
    delay(1000);
  }
}


// ============================================================
// 8. CONFIGURACIÓN INICIAL
// ============================================================

void setup() {

  // Inicialización del puerto serie para visualizar el proceso
  // de arranque, unión OTAA y envío de uplinks.
  Serial.begin(115200);
  delay(2000);

  Serial.println();
  Serial.println("========================================");
  Serial.println(" HELTEC ESP32 V4 - NODO LORAWAN OTAA");
  Serial.println("========================================");


  // ----------------------------------------------------------
  // 8.1. Alimentación de la etapa de radiofrecuencia
  // ----------------------------------------------------------

// Alimentación externa de la placa
pinMode(VEXT_CTRL, OUTPUT);
digitalWrite(VEXT_CTRL, LOW);

// Alimentación del frontal RF del Heltec V4.
// Este pin permanece habilitado.
pinMode(VFEM_CTRL, OUTPUT);
digitalWrite(VFEM_CTRL, HIGH);

// Se entrega a RadioLib el control automático de RF_CTRL y FEM_MODE.
// Así el firmware puede cambiar correctamente entre TX y RX.
radio.setRfSwitchTable(rfswitch_pins, rfswitch_table);

Serial.println("[OK] Alimentación y tabla RF configuradas");



  // ----------------------------------------------------------
  // 8.2. Inicialización explícita del bus SPI
  // ----------------------------------------------------------

  // Orden de los parámetros:
  // SCK, MISO, MOSI y NSS.

  SPI.begin(
    LORA_SCK,
    LORA_MISO,
    LORA_MOSI,
    LORA_NSS
  );

  Serial.println("[OK] Bus SPI inicializado");


  // ----------------------------------------------------------
  // 8.3. Inicialización del transceptor SX1262
  // ----------------------------------------------------------

  // No se fija una frecuencia manual como en el firmware LoRa.
  // LoRaWANNode aplicará las frecuencias y canales establecidos
  // en el plan regional EU868.

  // Inicialización del SX1262 con TCXO de 1,8 V,
// valor utilizado por el Heltec WiFi LoRa 32 V4.
// Los parámetros físicos iniciales serán reemplazados
// posteriormente por la configuración LoRaWAN EU868.

int16_t state = radio.begin(
  868.1,  // Frecuencia inicial en MHz
  125.0,  // Ancho de banda inicial en kHz
  7,      // Spreading Factor inicial
  5,      // Coding Rate inicial: 4/5
  0x34,   // Sync word público LoRaWAN
  14,     // Potencia inicial en dBm
  8,      // Longitud del preámbulo
  1.8     // Tensión del TCXO del Heltec V4
);

  if (state != RADIOLIB_ERR_NONE) {
    detenerPorError("No se pudo inicializar el SX1262", state);
  }

  Serial.println("[OK] Transceptor SX1262 inicializado");


  // El SX1262 utiliza DIO2 para controlar automáticamente
  // el conmutador de radiofrecuencia de la antena.

  state = radio.setDio2AsRfSwitch(true);

  if (state != RADIOLIB_ERR_NONE) {
    detenerPorError(
      "No se pudo configurar DIO2 como conmutador RF",
      state
    );
  }

  Serial.println("[OK] Conmutador RF configurado mediante DIO2");


  // ----------------------------------------------------------
  // 8.4. Configuración de las credenciales OTAA
  // ----------------------------------------------------------

  // Para LoRaWAN 1.0.4 se pasa NULL en la posición de NwkKey.
  // La AppKey se utiliza para autenticar el procedimiento OTAA.

  state = node.beginOTAA(
    JOIN_EUI,
    DEV_EUI,
    NULL,
    APP_KEY
  );

  if (state != RADIOLIB_ERR_NONE) {
    detenerPorError(
      "No se pudieron configurar las credenciales OTAA",
      state
    );
  }

  Serial.println("[OK] Credenciales OTAA configuradas");


  // ----------------------------------------------------------
  // 8.5. Unión del nodo a The Things Network
  // ----------------------------------------------------------

  // El nodo envía una Join Request y espera la respuesta
  // Join Accept remitida por TTN a través del gateway.

  Serial.println("[OTAA] Enviando solicitud de unión a TTN...");

  state = node.activateOTAA();

  if (state != RADIOLIB_LORAWAN_NEW_SESSION) {
    detenerPorError(
      "No se recibió una respuesta Join Accept",
      state
    );
  }

  Serial.println("[OTAA] Nodo unido correctamente a TTN");


  // ----------------------------------------------------------
  // 8.6. Configuración de funcionamiento LoRaWAN
  // ----------------------------------------------------------

  // Se habilita ADR para permitir que TTN adapte la velocidad
  // de transmisión y la potencia según la calidad del enlace.

  node.setADR(true);

  Serial.println("[OK] ADR habilitado");

  // El nodo funcionará como dispositivo de clase A.
  // Tras cada uplink abrirá las ventanas RX1 y RX2.

  state = node.setClass(RADIOLIB_LORAWAN_CLASS_A);

  if (state != RADIOLIB_ERR_NONE) {
    detenerPorError(
      "No se pudo configurar la clase A",
      state
    );
  }

  Serial.println("[OK] Nodo configurado como clase A");


  // Se muestra la dirección dinámica asignada por TTN
  // después de completar correctamente el procedimiento OTAA.

  Serial.print("[OTAA] DevAddr asignado: 0x");
  Serial.println((unsigned long)node.getDevAddr(), HEX);

  Serial.println("[READY] Nodo preparado para enviar uplinks");
  Serial.println();
}


// ============================================================
// 9. BUCLE PRINCIPAL DE TRANSMISIÓN
// ============================================================

void loop() {

// 9.1. Construcción del payload
// ----------------------------------------------------------

// Se crea un payload CSV sencillo con:
//
// formato, identificador lógico de baliza, número de secuencia
// y tiempo desde el arranque.
//
// Ejemplo:
// LORAWAN1,H01,15,123456
//
// Los parámetros físicos como frecuencia, SF y ancho de banda
// no se incluyen como valores fijos, puesto que en LoRaWAN
// pueden ser gestionados por la red y por ADR.

char payload[64];
unsigned long t_ms = millis();

snprintf(
  payload,
  sizeof(payload),
  "LORAWAN1,%s,%lu,%lu",
  BEACON_ID,
  (unsigned long)seq,
  t_ms
);

Serial.print("[UPLINK] Payload: ");
Serial.println(payload);


  // ----------------------------------------------------------
  // 9.2. Envío del uplink LoRaWAN
  // ----------------------------------------------------------

  // sendReceive transmite el mensaje y abre automáticamente
  // las ventanas RX1 y RX2 propias de un dispositivo clase A.

  int16_t state = node.sendReceive(
    payload,
    LORAWAN_PORT,
    CONFIRMED_UPLINK
  );


  // ----------------------------------------------------------
  // 9.3. Comprobación del resultado
  // ----------------------------------------------------------

  if (state < RADIOLIB_ERR_NONE) {

    // Un valor negativo representa un error de RadioLib.
    Serial.print("[ERROR] Fallo al enviar el uplink | Código: ");
    Serial.println(state);

  } else {

    // Estado 0: uplink enviado sin downlink.
    // Estado 1: downlink recibido en RX1.
    // Estado 2: downlink recibido en RX2.

    Serial.print("[OK] Uplink enviado | FCntUp: ");
    Serial.println((unsigned long)node.getFCntUp());

    if (state == 0) {
      Serial.println("[DOWNLINK] No se recibió ningún downlink");
    } else if (state == 1) {
      Serial.println("[DOWNLINK] Recibido en la ventana RX1");
    } else if (state == 2) {
      Serial.println("[DOWNLINK] Recibido en la ventana RX2");
    }
  }


  // El contador aumenta incluso si una transmisión falla,
  // permitiendo detectar posteriormente posibles pérdidas.
  seq++;


  // ----------------------------------------------------------
  // 9.4. Espera antes del siguiente uplink
  // ----------------------------------------------------------

  Serial.println("[WAIT] Próximo uplink en 30 segundos");
  Serial.println();

  delay(UPLINK_INTERVAL_MS);
}