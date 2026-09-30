# esp32-sensor-pusher

ESP32 firmware that pushes random readings to the hnu sensor API
(`POST /sensors/:id/readings`). Wokwi-ready: the ESP32's WiFi is routed through
the Wokwi IoT Gateway, so the simulation makes real HTTP requests over the
internet.

## API contract

```
POST {API_BASE_URL}/sensors/{id}/readings
Content-Type: application/json

{"sensor-id":1,"value":22.53,"timestamp":"2026-09-30T10:00:00Z"}
```

On boot the firmware calls `GET {API_BASE_URL}/sensors`, picks one of the
returned sensor ids at random (falls back to sensor 1 if that call fails), then
posts a new random value every 5 s and blinks the LED on each POST.

## 1. Expose the API

`localhost` does **not** work from Wokwi — the simulated ESP32 runs on Wokwi's
servers, so `http://localhost:3000` resolves to Wokwi, not your machine. Publish
the API with a tunnel:

```bash
cd ../api && clojure -M:run          # serves on :3000
npx localtunnel --port 3000          # -> https://....loca.lt (plain HTTP also fine)
# or: cloudflared tunnel --url http://localhost:3000
```

## 2. Set the base URL

Edit `platformio.ini`:

```ini
build_flags =
    -D API_BASE_URL="https://abc123.loca.lt"
```

`API_BASE_URL`, `WIFI_SSID`, `WIFI_PASSWORD` and `WIFI_CHANNEL` are all
`#ifndef`-guarded in `src/main.cpp`, so you can also set them there (useful for
real hardware, where you'd use your own SSID/password).

## 3. Build

```bash
pio run    # -> .pio/build/esp32dev/firmware.bin + firmware.elf
```

## 4. Run on Wokwi

**Web editor (docs.wokwi.com)**

1. New project -> ESP32, paste `src/main.cpp` into the code editor.
2. `Diagram` -> `Save as JSON`, then replace the file with this folder's
   `diagram.json` (delete the default parts first).
3. `Tools` -> `Firmware Upload` -> pick `.pio/build/esp32dev/firmware.bin`.
4. Start the simulation, open the serial monitor at **115200 baud**.

**VS Code** — open this folder, build, then run the `Wokwi: Launch Simulation`
command (`wokwi.toml` already points at the PlatformIO output paths).

## WiFi note

Wokwi's default virtual access point is the open network `Wokwi-GUEST`
(no password), which is what the firmware connects to out of the box. Add a
`wokwi-wifi-ap` part to the diagram if you need a custom SSID/credentials or a
local-only (no internet) network.

## Expected serial output

```
[boot] esp32 sensor pusher
[wifi] connecting to Wokwi-GUEST
[wifi] connected, ip 10.0.0.4
[time] synced
[GET /sensors] found 2 sensor(s)
[boot] posting as sensor 2 every 5000 ms
[POST] https://abc123.loca.lt/sensors/2/readings
         {"sensor-id":2,"value":20.85,"timestamp":"2026-09-30T10:12:03Z"}
         -> 201 {:sensor-id 2, :value 20.85, :timestamp "2026-09-30T10:12:03Z"}
```
