# WebRTC Audio Demo

This project—used as a test for the Goiaba platform— shows how to:

- Capture audio from an **INMP441** PDM MEMS microphone via I²S
- Encode and stream it over WebRTC (Opus)
- Play back incoming audio through a **MAX98357A** Class-D amplifier

All of this runs on an **ESP32-S3 DevKitC-1**. Audio streaming starts automatically when a remote client connects via the signaling URL (e.g. `https://goiaba-io.github.io/goiaba/webrtc-audio-demo/?id=<your-room>`).

![Wiring Diagram](imgs/diagram.png)

## Hardware Used

| Item    | Details                                                           |
| ------- | ----------------------------------------------------------------- |
| MCU     | ESP32‑S3 DevKitC‑1 (module **ESP32‑S3‑WROOM‑1‑N16**, 16 MB Flash) |
| Mic     | INMP441 I²S MEMS microphone breakout                              |
| Amp     | MAX98357A I²S 3 W audio amplifier breakout                        |
| Control | Momentary push‑button (GPIO14 → GND)                              |
| LED     | On‑board RGB LED (GPIO38)                                         |

## Wiring

| Function     | ESP32‑S3 GPIO | INMP441 | MAX98357A              | Notes                               |
| ------------ | ------------- | ------- | ---------------------- | ----------------------------------- |
| 3.3 V        | 3V3           | VIN     | VIN                    | Common supply                       |
| GND          | GND           | GND     | GND                    | Common ground                       |
| **BCLK Rx**  | **15**        | SCK     | –                      | Mic bit‑clock                       |
| **WS Rx**    | **16**        | WS/LR   | –                      | Mic word‑select                     |
| **DATA IN**  | **17**        | SD      | –                      | Mic → MCU                           |
| **BCLK Tx**  | **18**        | –       | BCLK                   | Amp bit‑clock                       |
| **WS Tx**    | **19**        | –       | LRC                    | Amp word‑select                     |
| **DATA OUT** | **20**        | –       | DIN                    | MCU → Amp                           |
| L/R Select   | —             | L/R→GND | –                      | Mono (Left)                         |
| Enable (SD)  | —             | –       | SD/EN→3V3              | Amp always on                       |
| Gain         | —             | –       | GAIN→3V3 / GND / float | 12 dB / 6 dB / 9 dB                 |
| Button       | **14**        | –       | –                      | Between GPIO14 & GND (INPUT_PULLUP) |
| LED          | **38**        | –       | –                      | Built‑in LED                        |

### Key Libraries / Headers

- **ESP‑IDF HAL** (`driver/i2s.h`) – I²S peripheral access.
- `esp_heap_caps.h` – heap‑capable SRAM allocation.

## Runtime Parameters (default)

- `SAMPLE_RATE` = 16000 Hz

> Using a module **with PSRAM** (e.g. ESP32‑S3‑WROOM‑1‑N8R8) you can raise both values (10 s @ 16 kHz ≈ 320 kB).

## Configuration

In your project root, run `make menuconfig` and navigate to **Component config → ESP PSRAM → Support for external, SPI-RAM** (enable it!). There you’ll find the "PSRAM Type" setting—change it from the default Quad-SPI to **Octo-SPI**. Once enabled, save and exit; the build system will automatically configure the Octo-SPI driver so you can take full advantage of your 8 MB PSRAM for high-bandwidth audio buffering.

Run `idf.py menuconfig` and set:

- **External PSRAM**
  **Component config → ESP32S3-specific**

  - `CONFIG_SPIRAM=y`
  - `CONFIG_SPIRAM_USE_MALLOC=y`

- **Partition table**
  **Partition Table**

  - Select **Custom partition table CSV**
  - `Partition Table Filename` → `partitions.csv`

- **Wi-Fi credentials**
  **Component config → Wi-Fi credentials**

  - `CONFIG_WIFI_CONNECT_SSID` — your network’s SSID
  - `CONFIG_WIFI_CONNECT_PASSWORD` — your network’s password

- **Signaling server URL / token**
  **Component config → Peer-manager configuration**

  - `CONFIG_SIGNALING_URL` — full URL of your signaling endpoint (e.g. `mqtts://libpeer.com/public/striped-lazy-eagle`)
  - `CONFIG_SIGNALING_TOKEN` — optional auth token

- **DTLS-SRTP support**
  **Component config → mbedTLS**
  - Enable **Negotiation of DTLS-SRTP (RFC 5764)** (`CONFIG_MBEDTLS_SSL_DTLS_SRTP=y`)

You can pre-populate these in `sdkconfig.defaults`:

````ini
CONFIG_SPIRAM="y"
CONFIG_SPIRAM_USE_MALLOC="y"
CONFIG_PARTITION_TABLE_CUSTOM=y
CONFIG_PARTITION_TABLE_CUSTOM_FILENAME="partitions.csv"

CONFIG_WIFI_CONNECT_SSID="your_ssid"
CONFIG_WIFI_CONNECT_PASSWORD="your_password"
CONFIG_SIGNALING_URL="mqtts://libpeer.com/public/striped-lazy-eagle"
CONFIG_SIGNALING_TOKEN=""
CONFIG_MBEDTLS_SSL_DTLS_SRTP="y"

## Quick Start

```bash
# 1. (Once) Set up environment
make setup

# 2. Build firmware
make build

# 3. Flash to device
make flash

# 4. Monitor UART (115200 baud)
make monitor
````

Press **Ctrl+]** to exit the serial monitor.

To establish a peer-to-peer connection, open the demo page at

```
https://goiaba-io.github.io/goiaba/webrtc-audio-demo
```

and let it generate (or manually enter) a unique device ID. Once you click “Connect,” the full URL—including your chosen ID will appear (for example, `mqtts://libpeer.com/public/<your-room>`). Past this SIGNALING_URL in Goiaba `webrtc-audio-demo` example.

## License

MIT © 2025
