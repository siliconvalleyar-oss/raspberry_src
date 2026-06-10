---
name: www
description: Waveforms Live web application (Ionic + Cordova)
---

# Waveforms Live Web App Skill

This skill helps you work with the Waveforms Live web application — an Ionic Framework
app with Apache Cordova for mobile deployment.

## Project Structure

```
www/
├── assets/
│   ├── devices/         # Device configurations
│   ├── fonts/           # Web fonts
│   ├── icon/            # App icons
│   ├── img/             # Images and SVGs
│   └── js/              # JavaScript libraries
├── beta/                # Beta Cordova build
├── build/               # Ionic build output
│   ├── main.css
│   ├── main.js
│   └── polyfills.js
├── plugins/             # Cordova plugins
├── index.html           # Entry point
├── manifest.json        # PWA manifest
├── cordova.js           # Cordova bridge
└── service-worker.js    # Service worker
```

## Development

```bash
# Serve locally
python3 -m http.server 8000

# Or with Ionic CLI
npm install -g ionic
ionic serve
```

## Build

```bash
cordova platform add android
cordova build android
```

## Key Libraries

- Ionic Framework
- jQuery 1.8.3
- jQuery Flot (charts, tooltips, zoom/pan, cursors, axis labels)
- Math functions library
- Waveform dictionary

## Dependencies (Cordova)

- cordova-plugin-console 1.0.5
- cordova-plugin-device 1.1.4
