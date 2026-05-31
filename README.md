# Adlar Aurora III ESPHome Modbus

Deze map bevat een ESPHome-config voor een Seeed Studio XIAO ESP32C6 die via een RS485-naar-TTL module op de Modbus-poort van de JÅN-module van een Adlar Aurora III R290 warmtepomp meeleest en beperkte gebruikersbediening naar Home Assistant brengt.

## Aannames

- De Aurora III gebruikt dezelfde Modbus RTU-registerlaag als de SolarEast/Sunrain R290 BLN/TC-serie.
- Modbus: slave-id `1`, 9600 baud, 8 databits, geen parity, 1 stopbit.
- De JÅN-module heeft een Modbus/RS485-aansluiting met `A`, `B` en `GND`.
- ESPHome is Modbus-master/client. Sluit dit alleen direct op dezelfde bus aan als de JÅN-poort als Modbus-slave/server bedoeld is, of test eerst alleen uitlezen. Als de JÅN-module zelf ook actief als master op dezelfde RS485-bus praat, kan er geen echte bus-arbitrage plaatsvinden. Daarom pollt deze config rustig: standaard elke 30 seconden met 250 ms command throttle.
- Temperaturen zijn in deze config als hele graden verwerkt, omdat de gevonden Adlar/Home Assistant-config dat zo gebruikt. Als jouw waarden exact 10x te hoog binnenkomen, voeg dan bij de temperatuursensoren een `filters: - multiply: 0.1` toe en gebruik bij setpoint-numbers `multiply: 10`.
- De stooklijn-selects staan bewust uitgeschakeld. Sommige registermaps noemen alleen curve 1-8, terwijl het gevonden Adlar-voorbeeld ook H/L-curves gebruikt.

## Bedrading

Gebruik een 3.3V RS485-transceiver. De XIAO ESP32C6 GPIO's zijn niet 5V tolerant; sluit dus geen 5V TTL-uitgang rechtstreeks op de XIAO RX aan.

TTL-zijde tussen XIAO en RS485-module:

| XIAO ESP32C6 | GPIO | RS485 TTL module |
| --- | ---: | --- |
| D6 / TX | GPIO16 | DI / TX-in van module |
| D7 / RX | GPIO17 | RO / RX-out van module |
| D2 | GPIO2 | DE en /RE samen, alleen bij handmatige direction modules |
| 3V3 |  | VCC, bij 3.3V-module |
| GND |  | GND |

RS485-zijde tussen RS485-module en JÅN-module:

| RS485 TTL module | JÅN Modbus-poort |
| --- | --- |
| A | A |
| B | B |
| GND | GND |

Als je RS485-module automatische richting heeft en geen `DE`/`RE` pinnen gebruikt, verwijder dan deze regel uit de YAML:

```yaml
flow_control_pin: D2
```

Als je geen data krijgt, wissel A en B als eerste test. Doe bedrading spanningsloos.

## Gebruik

1. Gebruik `adlar_aurora3_xiao_esp32c6.yaml` als ESPHome-config.
2. Vul je ESPHome `secrets.yaml` aan met de waarden uit `secrets.example.yaml`. Commit `secrets.yaml` niet; deze staat in `.gitignore`.
3. Flash de XIAO ESP32C6.
4. Controleer eerst alleen uitlezen: `Retour water temperatuur T6`, `Aanvoer water temperatuur T7`, `Water flow`, `Running status 1 raw`.
5. Pas daarna voorzichtig bediening toe. De stooklijn-selects staan bewust `disabled_by_default: true`.

## Veilige grenzen

Het verwarmings-setpoint is begrensd op 20-45 graden. Voor radiatoren kun je dat verhogen, maar doe dat bewust. Voor vloerverwarming is een te hoge watertemperatuur onwenselijk.

P- en L-parameters zijn bewust niet opgenomen. Die zijn service-/installatieparameters en horen niet als gewone Home Assistant-bediening beschikbaar te zijn.

## Bronnen

- Adlår Aurora III Pro handleiding: https://www.support.adlar.com/wp-content/uploads/2025/08/250606-Aurora-III-Pro-R290-Gebruikershandleiding.pdf
- Adlar/Home Assistant Modbus voorbeeld: https://github.com/rhjbruins/adlar_homeassistant
- SolarEast/R290 Home Assistant integratie: https://github.com/CNC-Buddy/R290_heatpump
- Seeed XIAO ESP32C6 pinmap: https://wiki.seeedstudio.com/xiao_esp32c6_getting_started/
