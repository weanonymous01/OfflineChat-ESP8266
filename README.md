# 📡 OfflineChat — ESP8266 Offline Mesh Chat

A fully offline, real-time chat system built on a single ESP8266 board. No internet. No router. No server. Just plug in and chat.

---

## What It Does

One ESP8266 board creates a Wi-Fi hotspot. Any device (phone, laptop, tablet) connects to that hotspot and opens a browser. Everyone on the same hotspot can send and receive messages in real time — completely offline.

---

## Hardware Required

| Component | Qty | Notes |
|-----------|-----|-------|
| ESP8266 NodeMCU (ESP-12E) | 1 | Any ESP8266 board works |
| USB cable (Micro-USB) | 1 | For flashing + power |
| USB power bank or charger | 1 | To run it standalone |

That's it. No extra components.

---

## Software / Tools

| Tool | Purpose |
|------|---------|
| Arduino IDE 2.x | Flashing the firmware |
| ESP8266 Arduino Core 3.x | Board support package |
| Chrome / Edge browser | Accessing the chat UI |

No external Arduino libraries needed. Only built-in `ESP8266WiFi` and `ESP8266WebServer`.

---

## Architecture

```
┌─────────────────────────────────────────────────┐
│              ESP8266 Board (Board 1)             │
│                                                  │
│  ┌─────────────┐     ┌──────────────────────┐   │
│  │  Wi-Fi AP   │     │   Web Server :80     │   │
│  │  "Offline   │     │                      │   │
│  │   Chat"     │     │  GET  /      → HTML  │   │
│  │  2.4GHz     │     │  GET  /msg   → JSON  │   │
│  └─────────────┘     │  POST /send  → store │   │
│                      └──────────────────────┘   │
│                                                  │
│  In-memory message store (ring buffer, 8 msgs)  │
└─────────────────────────────────────────────────┘
          │
          │ Wi-Fi (192.168.4.x)
          │
   ┌──────┴──────────────────────────┐
   │                                 │
   │  Any device with a browser      │
   │  Phone / Laptop / Tablet        │
   │  Opens http://192.168.4.1       │
   │  JS polls /msg every 2 seconds  │
   └─────────────────────────────────┘
```

### How Messages Flow

1. User types message in browser → JS sends `POST /send` to `192.168.4.1`
2. ESP8266 stores message in RAM (ring buffer, max 8 messages)
3. All connected browsers poll `GET /msg` every 2 seconds
4. New messages render instantly on all devices

### Key Design Decisions

- **No database** — messages stored in ESP8266 RAM. Reboot clears history. Simple and reliable.
- **No WebSockets** — plain HTTP polling every 2s. Works on every browser including old Android.
- **HTML in PROGMEM** — page served in 8 small chunks from flash memory, not RAM. Prevents stack overflow on the constrained 80KB RAM.
- **No external libraries** — only built-in ESP8266 libraries. Nothing to install except board support.
- **Ring buffer** — oldest message is overwritten when buffer is full. No memory leaks.

---

## File Structure

```
OfflineChat-ESP8266/
├── Board1_v3/
│   └── Board1_v3.ino       ← Flash this to your ESP8266
└── README.md
```

---

## Setup — Step by Step

### 1. Install Arduino IDE
Download from https://www.arduino.cc/en/software and install.

### 2. Add ESP8266 Board Support
- Open Arduino IDE → **File → Preferences**
- Paste this in "Additional boards manager URLs":
  ```
  https://arduino.esp8266.com/stable/package_esp8266com_index.json
  ```
- Go to **Tools → Board → Boards Manager**
- Search `esp8266` → Install **esp8266 by ESP8266 Community** (v3.x)

### 3. Erase Old Firmware (Important)
If the board was used before:
- Connect board via USB
- **Tools → Erase Flash → All Flash Contents**

### 4. Upload Firmware
- Open `Board1_v3/Board1_v3.ino`
- **Tools → Board → NodeMCU 1.0 (ESP-12E Module)**
- **Tools → Port** → select your COM port
- Click **Upload**
- If you see `Connecting.....____` — hold the **FLASH button + Press Reset Button then release: FLASH button** on the board until writing starts

### 5. Verify It's Running
- Open **Tools → Serial Monitor** → baud rate: **115200**
- Press RST button on board
- You should see:
  ```
  === OfflineChat v3 ===
  AP OK
  IP: 192.168.4.1
  Ready! http://192.168.4.1
  ```

### 6. Connect and Chat
- On any phone or laptop → Wi-Fi settings → connect to **OfflineChat**
- Password: **`chat1234`**
- Open browser → go to **`192.168.4.1`**
- Select your name (Board1 / Board2 / Guest)
- Type a message → Send

Everyone connected to the same hotspot sees messages in real time.

---

## API Endpoints

| Method | Endpoint | Description |
|--------|----------|-------------|
| GET | `/` | Serves the chat HTML page |
| GET | `/msg` | Returns all messages as JSON |
| POST | `/send` | Sends a new message. Params: `s` (sender), `t` (text) |

### Example `/msg` response
```json
{
  "n": 3,
  "m": [
    { "s": "Board1", "t": "Hello!", "ts": "00:01:23" },
    { "s": "Guest",  "t": "Hi there", "ts": "00:01:45" }
  ]
}
```

---

## Specs

| Feature | Value |
|---------|-------|
| Wi-Fi standard | 802.11 b/g/n (2.4GHz) |
| Hotspot range | ~50–100m open area |
| Max simultaneous users | 5–8 devices |
| Messages stored | 8 (ring buffer) |
| Poll interval | 2 seconds |
| Power draw | ~80mA @ 3.3V |
| Works without internet | ✅ Yes |
| Works without a router | ✅ Yes |

---

## Use Cases

- **Disaster communication** — internet and cell networks are down, this still works
- **Camping / hiking** — coordinate with your group off-grid
- **Classroom** — private local discussion, no internet needed
- **Events** — local chat without exposing a network
- **Security research / demo** — shows how local mesh communication works

---

## Troubleshooting

| Problem | Fix |
|---------|-----|
| Upload fails with `Invalid SLIP escape` | Hold FLASH button on board when `Connecting....` appears |
| Hotspot not visible on phone | Old firmware conflict — erase flash first, then re-upload |
| Board keeps restarting (crash loop) | Old code using ArduinoJson — use v3 sketch (this repo) |
| Page doesn't load at 192.168.4.1 | Make sure you're connected to OfflineChat, not your home Wi-Fi |
| Power bank turns off the board | Power bank auto-shuts off at low current — use a phone charger instead |

---

