# Waveforms Live — Web App

Aplicación web para visualización de formas de onda. Construida con Ionic Framework + Apache Cordova para soporte móvil nativo.

## Descripción

Waveforms Live es una herramienta de visualización de señales y formas de onda en tiempo real. Utiliza jQuery Flot para renderizado de gráficos y soporta múltiples canales, zoom, pan y cursores.

## Tecnologías

- **Ionic Framework** — UI components
- **Apache Cordova** — Empaquetado nativo (iOS/Android)
- **jQuery Flot** — Gráficos interactivos
- **Service Worker** — Soporte offline parcial

## Estructura

```
www/
├── assets/
│   ├── devices/     # Configuraciones de dispositivos
│   ├── fonts/       # Fuentes
│   ├── icon/        # Iconos de la app
│   ├── img/         # Imágenes y SVG
│   └── js/          # Librerías JS (jQuery, Flot, etc.)
├── beta/            # Versión beta (Cordova)
├── build/           # Build output (Ionic)
├── plugins/         # Plugins Cordova
├── index.html       # Punto de entrada
├── manifest.json    # Web manifest (PWA)
├── cordova.js       # Cordova bridge
└── service-worker.js
```

## Desarrollo

```bash
# Servir localmente
python3 -m http.server 8000

# O con Ionic
npm install -g ionic
ionic serve
```

## Build Cordova

```bash
cordova platform add android
cordova build android
```

## Dependencias Cordova

- `cordova-plugin-console` 1.0.5
- `cordova-plugin-device` 1.1.4
