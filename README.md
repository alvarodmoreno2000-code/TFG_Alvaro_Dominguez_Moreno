# TFG - Sistema híbrido de localización basado en LoRa/LoRaWAN

Repositorio asociado al Trabajo Fin de Grado:

**Análisis y desarrollo de un sistema híbrido de localización en interiores y exteriores basado en LoRa**

Autor: **Álvaro Domínguez Moreno**  
Titulación: **Grado en Ingeniería en Tecnologías de la Telecomunicación**  
Mención: **Telemática**  
Universidad: **Universidad de Las Palmas de Gran Canaria**

-------------------------X----------------------------X-----------------------------X-------------------------------X------------------------------X------------------------------X-------------------------------X---------

## Descripción

Este repositorio contiene los archivos relacionados con la implementación práctica del banco de pruebas desarrollado en el Trabajo Fin de Grado.

El proyecto se centra en el análisis, diseño, implementación y validación de un sistema experimental basado en tecnología **LoRa/LoRaWAN**, orientado al estudio de técnicas de localización aproximada en entornos interiores y exteriores.

La arquitectura implementada está formada por un nodo LoRaWAN basado en una placa **Heltec ESP32-S3 con transceptor SX1262**, un gateway LoRaWAN propio basado en **Raspberry Pi 5** y un concentrador **SX1302/SX1303**, junto con la integración con **The Things Network** como servidor de red.

-------------------------X----------------------------X-----------------------------X-------------------------------X------------------------------X------------------------------X-------------------------------X---------

## Contenido del repositorio

El repositorio incluye los elementos necesarios para consultar y reproducir la parte práctica del proyecto:

```text
01_firmware_nodo_heltec/
Firmware empleado en el nodo Heltec LoRa/LoRaWAN.

02_datos_ttn_json/
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

-------------------------X----------------------------X-----------------------------X-------------------------------X------------------------------X------------------------------X-------------------------------X---------

## Campaña experimental

La campaña experimental se realizó utilizando un gateway fijo, identificado como **R01/G01**, y varias posiciones conocidas del nodo móvil, identificadas como **H01**, **H02** y **H03**.

Durante las pruebas se transmitieron tramas LoRaWAN desde el nodo Heltec y se registraron métricas radioeléctricas asociadas a cada recepción, principalmente:

- RSSI.
- SNR.
- Factor de expansión.
- Identificador del gateway receptor.
- Número de secuencia.
- Posición de medida.

Posteriormente, los datos exportados desde The Things Network fueron convertidos a formato CSV, filtrados para conservar únicamente las recepciones asociadas al gateway propio y procesados para construir el dataset final empleado en el análisis.

-------------------------X----------------------------X-----------------------------X-------------------------------X------------------------------X------------------------------X-------------------------------X---------

## Validación mediante k-NN

Como validación preliminar de las huellas radioeléctricas obtenidas, se aplicó un clasificador **k-NN** con validación leave-one-out.

El objetivo de esta validación fue comprobar si las métricas registradas permitían diferenciar las posiciones conocidas del nodo a partir de las medidas de RSSI, SNR y factor de expansión.

Los resultados generados incluyen:

- Dataset final.
- Matrices de confusión.
- Predicciones obtenidas.
- Porcentaje de acierto por posición.
- Resultados de validación usando RSSI/SNR.
- Resultados de validación usando RSSI/SNR/SF.

-------------------------X----------------------------X-----------------------------X-------------------------------X------------------------------X------------------------------X-------------------------------X---------

## Tecnologías y herramientas utilizadas

- LoRa.
- LoRaWAN.
- The Things Network.
- Heltec ESP32-S3 SX1262.
- Raspberry Pi 5.
- Concentrador LoRaWAN SX1302/SX1303.
- Python.
- Arduino IDE.
- k-NN para clasificación preliminar de huellas radioeléctricas.

-------------------------X----------------------------X-----------------------------X-------------------------------X------------------------------X------------------------------X-------------------------------X---------

## Aviso sobre credenciales

Por motivos de seguridad, no se incluyen claves reales, tokens, contraseñas ni credenciales OTAA utilizadas durante el desarrollo del proyecto.

Los campos correspondientes a credenciales LoRaWAN, como `DEV_EUI`, `JOIN_EUI` o `APP_KEY`, deberán ser sustituidos por los valores propios de cada despliegue en The Things Network.

-------------------------X----------------------------X-----------------------------X-------------------------------X------------------------------X------------------------------X-------------------------------X---------

## Uso académico

Este repositorio se publica como material de apoyo al Trabajo Fin de Grado y tiene como finalidad facilitar la trazabilidad, consulta y reproducibilidad de la implementación realizada.

El contenido puede utilizarse como referencia académica, siempre citando adecuadamente al autor y el trabajo original.
