#!/usr/bin/env python3
import json
import csv
import base64
import argparse
from pathlib import Path


TARGET_GATEWAY_ID = "loragateway-tfgalvaro"


FIELDNAMES = [
    "timestamp_ttn",
    "application_id",
    "device_id",
    "dev_eui",
    "dev_addr",
    "f_cnt",
    "f_port",
    "frm_payload_b64",
    "payload_decoded",
    "format",
    "beacon_id",
    "seq",
    "tx_ms",
    "gateway_id",
    "gateway_eui",
    "gateway_used_for_analysis",
    "rssi",
    "channel_rssi",
    "snr",
    "frequency_hz",
    "spreading_factor",
    "bandwidth_hz",
    "coding_rate",
    "consumed_airtime",
    "cluster_id",
]


def decode_payload(frm_payload_b64):
    if not frm_payload_b64:
        return ""

    try:
        return base64.b64decode(frm_payload_b64).decode("utf-8", errors="replace")
    except Exception:
        return ""


def parse_payload(payload):
    result = {
        "format": "",
        "beacon_id": "",
        "seq": "",
        "tx_ms": "",
    }

    parts = payload.strip().split(",")

    if len(parts) == 4 and parts[0] == "LORAWAN1":
        result["format"] = parts[0]
        result["beacon_id"] = parts[1]
        result["seq"] = parts[2]
        result["tx_ms"] = parts[3]

    return result


def main():
    parser = argparse.ArgumentParser(
        description="Convierte JSON exportado desde TTN en CSV limpio para dataset LoRaWAN."
    )
    parser.add_argument("--input", required=True, help="Archivo JSON exportado desde TTN")
    parser.add_argument("--output", default="ttn_lorawan_dataset.csv", help="CSV de salida")
    parser.add_argument(
        "--gateway",
        default=TARGET_GATEWAY_ID,
        help="Gateway propio que se usara para el analisis principal",
    )

    args = parser.parse_args()

    input_path = Path(args.input)
    output_path = Path(args.output)

    if not input_path.exists():
        raise SystemExit(f"[ERROR] No existe el fichero de entrada: {input_path}")

    with input_path.open("r", encoding="utf-8") as f:
        events = json.load(f)

    rows = []
    total_uplinks = 0
    total_metadata = 0
    used_gateway_rows = 0
    external_gateway_rows = 0

    for event in events:
        data = event.get("data", {})
        uplink = data.get("uplink_message")

        if not uplink:
            continue

        total_uplinks += 1

        end_device_ids = data.get("end_device_ids", {})
        application_ids = end_device_ids.get("application_ids", {})

        application_id = application_ids.get("application_id", "")
        device_id = end_device_ids.get("device_id", "")
        dev_eui = end_device_ids.get("dev_eui", "")
        dev_addr = end_device_ids.get("dev_addr", "")

        timestamp_ttn = uplink.get("received_at", event.get("time", ""))
        f_cnt = uplink.get("f_cnt", "")
        f_port = uplink.get("f_port", "")
        frm_payload_b64 = uplink.get("frm_payload", "")
        payload_decoded = decode_payload(frm_payload_b64)
        parsed_payload = parse_payload(payload_decoded)

        settings = uplink.get("settings", {})
        frequency_hz = settings.get("frequency", "")

        lora_settings = (
            settings
            .get("data_rate", {})
            .get("lora", {})
        )

        spreading_factor = lora_settings.get("spreading_factor", "")
        bandwidth_hz = lora_settings.get("bandwidth", "")
        coding_rate = lora_settings.get("coding_rate", "")

        consumed_airtime = uplink.get("consumed_airtime", "")

        network_ids = uplink.get("network_ids", {})
        cluster_id = network_ids.get("cluster_id", "")

        rx_metadata = uplink.get("rx_metadata", [])

        for metadata in rx_metadata:
            total_metadata += 1

            gateway_ids = metadata.get("gateway_ids", {})
            gateway_id = gateway_ids.get("gateway_id", "")
            gateway_eui = gateway_ids.get("eui", "")

            gateway_used = gateway_id == args.gateway

            if gateway_used:
                used_gateway_rows += 1
            else:
                external_gateway_rows += 1

            row = {
                "timestamp_ttn": timestamp_ttn,
                "application_id": application_id,
                "device_id": device_id,
                "dev_eui": dev_eui,
                "dev_addr": dev_addr,
                "f_cnt": f_cnt,
                "f_port": f_port,
                "frm_payload_b64": frm_payload_b64,
                "payload_decoded": payload_decoded,
                "format": parsed_payload["format"],
                "beacon_id": parsed_payload["beacon_id"],
                "seq": parsed_payload["seq"],
                "tx_ms": parsed_payload["tx_ms"],
                "gateway_id": gateway_id,
                "gateway_eui": gateway_eui,
                "gateway_used_for_analysis": gateway_used,
                "rssi": metadata.get("rssi", ""),
                "channel_rssi": metadata.get("channel_rssi", ""),
                "snr": metadata.get("snr", ""),
                "frequency_hz": frequency_hz,
                "spreading_factor": spreading_factor,
                "bandwidth_hz": bandwidth_hz,
                "coding_rate": coding_rate,
                "consumed_airtime": consumed_airtime,
                "cluster_id": cluster_id,
            }

            rows.append(row)

    with output_path.open("w", newline="", encoding="utf-8") as f:
        writer = csv.DictWriter(f, fieldnames=FIELDNAMES)
        writer.writeheader()
        writer.writerows(rows)

    print("[OK] Conversion finalizada")
    print(f"[INFO] JSON entrada: {input_path}")
    print(f"[INFO] CSV salida: {output_path}")
    print(f"[INFO] Uplinks procesados: {total_uplinks}")
    print(f"[INFO] Filas rx_metadata generadas: {total_metadata}")
    print(f"[INFO] Filas del gateway propio ({args.gateway}): {used_gateway_rows}")
    print(f"[INFO] Filas de gateways externos: {external_gateway_rows}")


if __name__ == "__main__":
    main()
