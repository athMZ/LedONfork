## Sterowanie paskami LED z użyciem telefonu

- Podpinamy paski LED do prądu z użyciem wtyczek będących w zestawie.

- Korzystając z kabla USB podpinamy płytkę do zasilania (port USB / power bank / ładowarka).

- Po podłączeniu łączymy telefon do sieci WiFi generowanej przez płytkę.
  - Nazwa sieci: `LED_MASTER`
  - Hasło: `12345678`

- Wyłączenie danych komórkowych w telefonie może pomóc w stabilniejszym połączeniu z siecią WiFi płytki.

![Kod QR umożliwiający połączenie z siecią WiFi](LED_MASTER-qrcode.png)

- W przeglądarce telefonu otwieramy adres: `led.local`

![Kod QR z linkiem do strony](qr-led-local.jpg)

- Strona do sterowania pozwala na:
  - odtwarzanie różnych efektów świetlnych na paskach LED
  - zmianę aktualnego koloru poprzez zmianę wartości dla Hue, Saturation i Brightness
  - zmianę ilości wykorzystanych diod LED na pasku

![Strona do sterowania paskami LED](ledMasterGUI.png)
