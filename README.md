# TFG - Sistema híbrido de localización basado en LoRa/LoRaWAN

Repositorio asociado al Trabajo Fin de Grado:

**Análisis y desarrollo de un sistema híbrido de localización en interiores y exteriores basado en LoRa**

Autor: **Álvaro Domínguez Moreno**  
Titulación: **Grado en Ingeniería en Tecnologías de la Telecomunicación**  
Mención: **Telemática**  
Universidad: **Universidad de Las Palmas de Gran Canaria**

---

## Descripción

Este repositorio contiene los archivos relacionados con la implementación práctica del banco de pruebas desarrollado en el Trabajo Fin de Grado.

El proyecto se centra en el análisis, diseño, implementación y validación de un sistema experimental basado en tecnología **LoRa/LoRaWAN**, orientado al estudio de técnicas de localización aproximada en entornos interiores y exteriores.

La arquitectura implementada está formada por un nodo LoRaWAN basado en una placa **Heltec ESP32-S3 con transceptor SX1262**, un gateway LoRaWAN propio basado en **Raspberry Pi 5** y un concentrador **SX1302/SX1303**, junto con la integración con **The Things Network** como servidor de red.

---

## Contenido del repositorio

El repositorio incluye los elementos necesarios para consultar y reproducir la parte práctica del proyecto:

```text
01_firmware_nodo_heltec/
Firmware empleado en el nodo Heltec LoRa/LoRaWAN.

02_gateway_lorawan/
Archivos de configuración y documentación de apoyo del gateway LoRaWAN.

03_datos_ttn_json/
Datos originales exportados desde The Things Network durante la campaña experimental.

03_csv_gateway_propio/
Archivos CSV intermedios obtenidos tras filtrar las recepciones asociadas al gateway propio.

04_scripts_procesado/
Scripts utilizados para convertir, filtrar y procesar los datos experimentales.

05_resultados/
Dataset final, gráficas y resultados obtenidos durante el análisis experimental.

06_validacion_knn/
Scripts y resultados de la validación preliminar mediante clasificación k-NN.

07_documentacion_apoyo/
Figuras, esquemas o documentación complementaria del sistema implementado.
