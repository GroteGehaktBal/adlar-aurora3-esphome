# Publication Notes

This repository is prepared so it can be made public later, but publishing any hardware integration still deserves a careful check.

## Public Repository Checklist

- Keep the project clearly marked as unofficial.
- Do not include Adlar/Adlår, JÅN, SolarEast, Seeed, ESPHome or Home Assistant logos.
- Do not copy large parts of vendor manuals, PDFs, marketing pages or screenshots.
- Use product names only to identify compatible hardware.
- Keep `secrets.yaml`, WiFi credentials, API keys and OTA passwords out of git.
- Remove private photos, serial numbers, QR codes, addresses and installation-specific details.
- Keep write controls conservative and disabled by default where risk is unclear.
- Keep service and installer parameters out of normal Home Assistant controls.
- Prefer links to original sources over copying their content.

## Interoperability

The purpose of this project is interoperability: making independently created ESPHome/Home Assistant configuration work with hardware the user owns. The repository should stay focused on that purpose.

Do not publish information obtained from a private NDA, a dealer-only portal, leaked documents, copied firmware or circumvention of access controls. If register information comes from real hardware testing or public community projects, describe it as such.

## Trademarks

Names such as Adlar, Adlår, Aurora III, JÅN, SolarEast, ESPHome and Home Assistant may be trademarks of their respective owners. Use them only for identification. Avoid any wording that suggests endorsement, partnership or official support.

## Safety

This configuration can write settings to heating equipment. Public documentation should keep the safety-first defaults:

- Start read-only.
- Verify stable RS485 communication.
- Confirm values are plausible before enabling writes.
- Keep setpoint ranges conservative.
- Clearly document uncertainty around unverified registers.

## Not Legal Advice

This file is a practical maintainer checklist, not legal advice. If you plan to publish commercially sensitive information, copied vendor material, firmware-derived information or anything obtained under contract, ask a qualified lawyer or the vendor first.
