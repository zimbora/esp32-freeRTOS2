# Wi-Fi Connection Behavior

This document reflects the current startup logic in `mRTOS_task`.

## Base Flow (No LTE)

When `ENABLE_LTE` is not defined, Wi-Fi is initialized and supervised by `mRTOS_task`.

Current sequence:

1. Start Wi-Fi with `settings.wifi.ssid` and `settings.wifi.pwd` through `mRTOS.init(...)`.
2. Configure MQTT client 1 and, if enabled, MQTT client 2.
3. Wait up to 15 seconds for the initial Wi-Fi session to come up.
4. If still disconnected, call `mRTOS.wifiReconnect(WIFI_SSID, WIFI_PASSWORD)` and wait another 15 seconds.
5. If `ENABLE_AP` is defined and Wi-Fi is still down, start the local AP portal once for 5 minutes.
6. After the AP window ends, stop SoftAP explicitly with `ap.stop()`.
7. If Wi-Fi is still disconnected, request one more reconnect using `settings.wifi.ssid` and `settings.wifi.pwd`.
8. Enter the final infinite loop that only runs `mRTOS.loop()` with wifi connecting retries.

Notes:

- The first Wi-Fi attempt happens implicitly in `mRTOS.init(...)`; there is no explicit reconnect call before the first 15-second wait.
- The fallback order is: stored settings at boot, compile-time default credentials, AP provisioning, then stored settings again.
- `mRTOS_task` does not keep cycling through credentials. The timed fallback sequence runs only during startup, before the steady-state `mRTOS.loop()` phase.

References:

- task startup and MQTT setup: [../esp32-freeRTOS2.ino](../esp32-freeRTOS2.ino#L183)
- first 15-second wait on stored settings: [../esp32-freeRTOS2.ino](../esp32-freeRTOS2.ino#L236)
- fallback to compile-time credentials: [../esp32-freeRTOS2.ino](../esp32-freeRTOS2.ino#L243)
- final reconnect before steady-state loop: [../esp32-freeRTOS2.ino](../esp32-freeRTOS2.ino#L271)

## Timers And Gating Logic

The current Wi-Fi startup path uses two separate timer windows:

1. `wifiTimeout = now() + 15` for the first connection wait after `mRTOS.init(...)`.
2. `wifiTimeout = now() + 15` again after reconnecting with `WIFI_SSID` and `WIFI_PASSWORD`.

If `ENABLE_AP` is active, the AP portal has its own timeout:

1. `timeout = now() + 5 * 60`.
2. The AP loop runs until that 5-minute window expires or the device reboots because new credentials were saved.

The `boot` flag gates the AP block so it only runs once inside `mRTOS_task`.

References:

- timed stored-credential wait: [../esp32-freeRTOS2.ino](../esp32-freeRTOS2.ino#L237)
- timed default-credential wait: [../esp32-freeRTOS2.ino](../esp32-freeRTOS2.ino#L245)
- AP one-shot gate and 5-minute timeout: [../esp32-freeRTOS2.ino](../esp32-freeRTOS2.ino#L252)

## If ENABLE_AP Is Defined

`ENABLE_AP` adds a one-shot local provisioning phase, but only after both Wi-Fi client attempts fail.

Behavior:

1. AP starts only if the device is still disconnected after the two 15-second Wi-Fi waits.
2. AP SSID is `ESP32AP` plus the last 6 hexadecimal characters of the MAC address.
3. Local setup page is served at `192.168.4.1`.
4. The page currently applies only Wi-Fi SSID and Wi-Fi password.
5. Submitted credentials are written to the settings file through `CALLBACKS_WIFI_AP::onWiFiSet(...)`.
6. A successful save triggers `call.fw_reboot()`, so the normal path after a valid submission is reboot, not AP timeout.
7. If nothing is submitted, the AP loop times out after 5 minutes, then `ap.stop()` shuts SoftAP down.
8. After the AP block exits without a reboot, the code requests one more reconnect using `settings.wifi.ssid` and `settings.wifi.pwd`.

References:

- AP startup block: [../esp32-freeRTOS2.ino](../esp32-freeRTOS2.ino#L251)
- AP stop in task flow: [../esp32-freeRTOS2.ino](../esp32-freeRTOS2.ino#L265)
- AP SSID generation and SoftAP startup: [../src/wifi/wifiAP.cpp](../src/wifi/wifiAP.cpp#L31)
- AP form submit path: [../src/wifi/wifiAP.cpp](../src/wifi/wifiAP.cpp#L277)
- settings save and reboot callback: [../core.cpp](../core.cpp#L9)

## If ENABLE_BLE Is Defined

`ENABLE_BLE` adds BLE provisioning and control in `setup()`.

Behavior:

1. BLE service starts with the device UID.
2. BLE writes can update stored settings, including Wi-Fi credentials.
3. BLE does not replace the `mRTOS_task` Wi-Fi startup logic; it only changes what credentials may be present on the next connect or reboot path.

References:

- BLE startup in `setup()`: [../esp32-freeRTOS2.ino](../esp32-freeRTOS2.ino#L381)
- BLE callback and settings writes: [../core.cpp](../core.cpp#L22)

## Build Combinations

If neither `ENABLE_AP` nor `ENABLE_BLE` is defined:

1. The device still runs the two-stage Wi-Fi startup logic in `mRTOS_task`.
2. No local provisioning channel is opened.
3. Runtime falls through to the steady-state `mRTOS.loop()` loop.

If both `ENABLE_AP` and `ENABLE_BLE` are defined:

1. BLE starts in `setup()`.
2. AP is still deferred until both Wi-Fi client attempts fail.
3. AP remains one-shot because of the `boot` guard.
4. After AP timeout, the code retries stored Wi-Fi credentials once more.

Reference:

- non-LTE Wi-Fi task flow: [../esp32-freeRTOS2.ino](../esp32-freeRTOS2.ino#L183)

## LTE Build Note

If `ENABLE_LTE` is defined, `mRTOS_task` is not created for network connection handling. LTE builds use `network_lte_task` instead.

Reference:

- LTE task path: [../esp32-freeRTOS2.ino](../esp32-freeRTOS2.ino#L93)
