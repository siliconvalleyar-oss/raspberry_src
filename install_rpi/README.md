# Install RPi — Instalación de dependencias básicas para Raspberry Pi

Script único que instala paquetes esenciales para la puesta a punto inicial
de una Raspberry Pi: herramientas de desarrollo, red, bases de datos y utilidades.

## Uso

```bash
chmod +x install_rpi.sh
./install_rpi.sh
```

## Paquetes instalados

- **Desarrollo**: git, curl, wget, python3, pip, virtualenv
- **Sistema**: htop, tree, unzip, zip, nano
- **Red**: net-tools, avahi-daemon, nmap, openssh-server
- **Base de datos**: mariadb-server, mysql-client
