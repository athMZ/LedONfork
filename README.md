# LedON (ESP8266) — sterowanie taśmami LED przez Wi‑Fi

Projekt pozwala sterować taśmami adresowalnymi z poziomu telefonu, bez instalowania dedykowanej aplikacji. Moduł **Master (ESP8266)** tworzy sieć Wi‑Fi i hostuje stronę WWW, a moduły **Slave (ESP8266)** odbierają komendy po **UDP broadcast** i odtwarzają efekty na taśmach.

## Działanie

1. Telefon łączy się z Wi‑Fi wystawianym przez Mastera (`LED_MASTER`).
2. Użytkownik wchodzi na panel sterowania: `http://led.local`.
3. Master wysyła komendy jako pakiety UDP broadcast na port `4210`.
4. Każdy Slave, który jest w tej sieci, odbiera pakiet i zmienia tryb/parametry efektu.

## Firmware

- **Master (Web + UDP TX):** [wifi-server/wifi-server.ino](wifi-server/wifi-server.ino)
- **Slave (UDP RX + efekty):** [secondary/secondary.ino](secondary/secondary.ino)
  - efekty i helpery są w plikach nagłówkowych w folderze [secondary/](secondary/)

## Konfiguracja sieci

- SSID: `LED_MASTER`
- Hasło: `12345678`
- mDNS: `http://led.local` (host: `led`)
- UDP: broadcast `192.168.4.255`, port `4210`

Jeśli `led.local` nie działa na danym telefonie, użyć IP Mastera: `http://192.168.4.1`.
