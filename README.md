# Ozkled Neopixel LED Kontrolü - ESP32 Bluetooth ile

**Proje Özeti:** Bu proje, ESP32 mikrodenetleyici kullanarak Ws2812 Neopixel LED'lerinin parlaklığını ve renklerini Bluetooth üzerinden kontrol etmenizi sağlar. Özel bir protokol kullanılarak haberleşme sağlanır.

**Teknoloji:**

*   ESP32 Mikrodenetleyici
*   Ws2812 Neopixel LED'ler
*   Bluetooth (ESP32'nin dahili modülü)
*   Müşterek Protokol (Açıklama altında)

**Protokol:** R255,G255,B255,I-1,L100

**Kurulum Talimatları:**

1.  ESP32'yi Arduino IDE veya benzeri bir geliştirme ortamıyla programlayın.
2.  Ws2812 Neopixel LED'lerini ESP32'ye bağlayın.
3.  Proje kodunu GitHub deposundan indirin ve gerekli değişiklikleri yapın.
4.  Projenin çalıştırılması için gerekli kütüphaneleri kurun (örneğin, NeoPixel Arduino kütüphanesi).

**Ek Bilgiler:**

*   [Proje Kaynağı](https://app.hamzaozkan.com.tr/ozlight)
*   [NeoPixel Kütüphanesi](https://github.com/adafruit/Adafruit_NeoPixel)
*   [ESP32 Bluetooth Dokümantasyonu](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/bluetooth_wifi/index.html)

**Branch Durumu:**

*   `main`: Çalışır Halde
*   Testler Onaylandı

**Çalışıyor!** (Proje başarıyla derlenmiş ve çalışmaktadır.)

**Ek Notlar:**

*   Bu proje, Neopixel LED'lerinizi Bluetooth aracılığıyla kontrol etmenize olanak tanır.
*   Protokol detayları ve kod örnekleri GitHub deposunda bulunmaktadır.
