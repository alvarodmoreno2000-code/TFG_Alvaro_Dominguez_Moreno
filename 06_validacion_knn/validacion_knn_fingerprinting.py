#!/usr/bin/env python3

import sys
from pathlib import Path

import pandas as pd
import sklearn
from sklearn.metrics import accuracy_score, confusion_matrix
from sklearn.model_selection import LeaveOneOut, cross_val_predict
from sklearn.neighbors import KNeighborsClassifier


BASE_DIR = Path(__file__).resolve().parent
DATASET = BASE_DIR / "dataset_lorawan_R01_H01_H02_H03_gateway_propio.csv"
SUMMARY = BASE_DIR / "resultados_validacion_knn.txt"

LABELS = ["H01", "H02", "H03"]


def evaluar_modelo(df, nombre, variables):
    X = df[variables].astype(float).to_numpy()
    y = df["posicion"].astype(str).to_numpy()

    # k-NN con k=1, distancia euclídea y pesos uniformes.
    modelo = KNeighborsClassifier(
        n_neighbors=1,
        metric="euclidean",
        weights="uniform",
    )

    # Cada muestra se evalúa utilizando las otras 38 como entrenamiento.
    predicciones = cross_val_predict(
        modelo,
        X,
        y,
        cv=LeaveOneOut(),
    )

    matriz = confusion_matrix(y, predicciones, labels=LABELS)
    exactitud = accuracy_score(y, predicciones)

    matriz_df = pd.DataFrame(
        matriz,
        index=[f"Real_{etiqueta}" for etiqueta in LABELS],
        columns=[f"Predicha_{etiqueta}" for etiqueta in LABELS],
    )

    resultados_df = df[
        ["posicion", "rssi", "snr", "spreading_factor"]
    ].copy()
    resultados_df["posicion_predicha"] = predicciones
    resultados_df["clasificacion_correcta"] = (
        resultados_df["posicion"] == resultados_df["posicion_predicha"]
    )

    matriz_df.to_csv(BASE_DIR / f"matriz_confusion_{nombre}.csv")
    resultados_df.to_csv(
        BASE_DIR / f"predicciones_{nombre}.csv",
        index=False,
    )

    acierto_por_posicion = {}
    for posicion in LABELS:
        mascara = y == posicion
        acierto_por_posicion[posicion] = accuracy_score(
            y[mascara],
            predicciones[mascara],
        )

    return exactitud, matriz_df, acierto_por_posicion


def main():
    if not DATASET.exists():
        raise FileNotFoundError(f"No se encuentra el dataset: {DATASET}")

    df = pd.read_csv(DATASET)

    columnas = {"posicion", "rssi", "snr", "spreading_factor"}
    faltantes = columnas.difference(df.columns)
    if faltantes:
        raise ValueError(f"Faltan columnas: {sorted(faltantes)}")

    modelos = [
        ("rssi_snr", ["rssi", "snr"]),
        ("rssi_snr_sf", ["rssi", "snr", "spreading_factor"]),
    ]

    lineas = [
        "VALIDACIÓN PRELIMINAR MEDIANTE k-NN",
        "=" * 45,
        f"Dataset: {DATASET.name}",
        f"Número de muestras: {len(df)}",
        f"Python: {sys.version.split()[0]}",
        f"pandas: {pd.__version__}",
        f"scikit-learn: {sklearn.__version__}",
        "Clasificador: k-NN",
        "Número de vecinos: k = 1",
        "Métrica: distancia euclídea",
        "Pesos: uniformes",
        "Validación: leave-one-out",
        "Normalización o estandarización: no aplicada",
        "Tratamiento del SF: variable numérica",
        "",
    ]

    for nombre, variables in modelos:
        exactitud, matriz, aciertos = evaluar_modelo(
            df,
            nombre,
            variables,
        )

        lineas.extend([
            f"Modelo: {nombre}",
            f"Variables: {', '.join(variables)}",
            f"Exactitud global: {exactitud * 100:.1f} %",
            "Matriz de confusión:",
            matriz.to_string(),
            "Acierto por posición:",
        ])

        for posicion, valor in aciertos.items():
            lineas.append(f"  {posicion}: {valor * 100:.1f} %")

        lineas.append("")

    texto = "\n".join(lineas)
    SUMMARY.write_text(texto, encoding="utf-8")
    print(texto)
    print(f"Resultados guardados en: {SUMMARY}")


if __name__ == "__main__":
    main()
