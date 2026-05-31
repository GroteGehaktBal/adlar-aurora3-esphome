# Adlar Aurora III ESPHome Modbus

![ESPHome](https://img.shields.io/badge/ESPHome-2026.5%2B-2f7d95)
![Home Assistant](https://img.shields.io/badge/Home%20Assistant-ready-41bdf5)
![Hardware](https://img.shields.io/badge/hardware-Seeed%20XIAO%20ESP32--C6-6f42c1)
![Status](https://img.shields.io/badge/status-compile--tested%2C%20hardware%20verify%20needed-d29922)
![License](https://img.shields.io/badge/license-MIT-2da44e)

ESPHome-configuratie voor het uitlezen en beperkt besturen van een Adlår/Adlar Aurora III R290 warmtepomp via de JÅN-module en Modbus RTU over RS485.

Deze repository is bedoeld als een nette, herbruikbare basis voor Home Assistant-gebruikers die een Adlar Aurora III of verwante SolarEast/Sunrain R290-warmtepomp lokaal willen integreren zonder cloudafhankelijkheid.

> Status: de ESPHome-config is gevalideerd en succesvol gecompileerd voor de Seeed Studio XIAO ESP32C6. De registerset is gebaseerd op openbare Adlar/SolarEast/R290-communityconfiguraties. Hardwaregedrag moet per installatie voorzichtig worden getest.

## Wat Dit Project Doet

- Leest statusbits, foutbits, temperaturen, waterflow, compressorfrequentie, stroom en spanning uit.
- Biedt beperkte bediening via Home Assistant: verwarmingssetpoint, koelingssetpoint, bedrijfsmodus, aan/uit en vermogensmodus.
- Houdt risicovollere instellingen zoals stooklijnselectie standaard uitgeschakeld.
- Gebruikt de native ESPHome API, OTA-updates en een eenvoudige lokale webinterface.
- Neemt service-/installatieparameters bewust niet op als normale Home Assistant-bediening.

## Hardware

Getest doelplatform voor deze configuratie:

- Seeed Studio XIAO ESP32C6
- 3.3V RS485-naar-TTL transceiver
- Adlar/JÅN Modbus-aansluiting met `A`, `B` en `GND`

Gebruik geen 5V TTL-signaal rechtstreeks op de XIAO ESP32C6. De GPIO's zijn niet 5V tolerant.

## Snelle Start

1. Clone deze repository of importeer de YAML in ESPHome.
2. Kopieer `secrets.example.yaml` naar je eigen ESPHome `secrets.yaml`.
3. Vul je WiFi-, API- en OTA-secrets in.
4. Controleer de bedrading in [docs/wiring.md](docs/wiring.md).
5. Compileer en flash `adlar_aurora3_xiao_esp32c6.yaml`.
6. Test eerst alleen uitlezen met onder andere `Retour water temperatuur T6`, `Aanvoer water temperatuur T7`, `Water flow` en `Running status 1 raw`.
7. Schakel pas daarna voorzichtig bediening in Home Assistant in.

```bash
esphome config adlar_aurora3_xiao_esp32c6.yaml
esphome compile adlar_aurora3_xiao_esp32c6.yaml
```

## Bedrading Kort

| XIAO ESP32C6 | GPIO | RS485 TTL module |
| --- | ---: | --- |
| D6 / TX | GPIO16 | DI / TX-in |
| D7 / RX | GPIO17 | RO / RX-out |
| D2 | GPIO2 | DE en /RE samen, alleen bij handmatige direction modules |
| 3V3 |  | VCC, bij 3.3V-module |
| GND |  | GND |

| RS485 TTL module | JÅN Modbus-poort |
| --- | --- |
| A | A |
| B | B |
| GND | GND |

Als je RS485-module automatische richting heeft, verwijder dan `flow_control_pin: D2` uit de YAML.

Meer details en een schema staan in [docs/wiring.md](docs/wiring.md).

## Belangrijke Veiligheid

ESPHome werkt in deze configuratie als Modbus master/client. Sluit dit alleen direct op dezelfde bus aan als de JÅN-poort als Modbus slave/server bedoeld is, of test eerst uitsluitend uitlezen. Als de JÅN-module zelf ook actief als master op dezelfde RS485-bus praat, is er geen echte busarbitrage en kunnen telegrammen botsen.

De configuratie pollt daarom rustig: standaard elke 30 seconden met 250 ms command throttle. Verlaag deze intervallen niet voordat je de bus stabiel hebt getest.

Zie [docs/troubleshooting.md](docs/troubleshooting.md) voor symptomen zoals CRC-fouten, geen respons, 10x verkeerde temperaturen of writes die niet blijven staan.

## Repository-Inhoud

| Pad | Doel |
| --- | --- |
| `adlar_aurora3_xiao_esp32c6.yaml` | Hoofdconfiguratie voor ESPHome |
| `secrets.example.yaml` | Voorbeeld van benodigde secrets |
| `docs/wiring.md` | Bedrading en RS485-aandachtspunten |
| `docs/register-map.md` | Registeroverzicht, schaling en schrijfbare adressen |
| `docs/troubleshooting.md` | Problemen oplossen en eerste testprocedure |
| `CHANGELOG.md` | Wijzigingsgeschiedenis |
| `CONTRIBUTING.md` | Richtlijnen voor issues, pull requests en hardware-testfeedback |

## Aannames En Grenzen

- De Aurora III gebruikt dezelfde Modbus RTU-registerlaag als de SolarEast/Sunrain R290 BLN/TC-serie.
- Modbus: slave-id `1`, 9600 baud, 8 databits, geen parity, 1 stopbit.
- Temperaturen zijn in deze config als hele graden verwerkt, omdat de gevonden Adlar/Home Assistant-config dat zo gebruikt. Als jouw waarden exact 10x te hoog binnenkomen, pas dan de temperatuurschaling aan.
- De stooklijnselecties staan bewust `disabled_by_default: true`. Sommige registermaps noemen alleen curve 1-8, terwijl het gevonden Adlar-voorbeeld ook H/L-curves gebruikt.
- Het verwarmingssetpoint is begrensd op 20-45 graden. Voor radiatoren kun je dat verhogen, maar doe dat bewust. Voor vloerverwarming is een te hoge watertemperatuur onwenselijk.

## Bronnen

- [Adlår Aurora III Pro handleiding](https://www.support.adlar.com/wp-content/uploads/2025/08/250606-Aurora-III-Pro-R290-Gebruikershandleiding.pdf)
- [Adlar/Home Assistant Modbus voorbeeld](https://github.com/rhjbruins/adlar_homeassistant)
- [SolarEast/R290 Home Assistant integratie](https://github.com/CNC-Buddy/R290_heatpump)
- [Seeed XIAO ESP32C6 pinmap](https://wiki.seeedstudio.com/xiao_esp32c6_getting_started/)
- [ESPHome Modbus](https://esphome.io/components/modbus/)
- [ESPHome Modbus Controller](https://esphome.io/components/modbus_controller/)

## Meedoen

Hardware-testresultaten zijn heel welkom, vooral met exacte warmtepompvariant, JÅN-moduleversie, RS485-module, bekabeling en welke registers kloppen. Gebruik daarvoor de issue templates.

## Licentie

MIT. Zie [LICENSE](LICENSE).
