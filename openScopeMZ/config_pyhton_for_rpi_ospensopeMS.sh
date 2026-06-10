#!/bin/bash

# 1. Instalar python3-venv (si no está)
sudo apt install python3-venv python3-full -y

# 2. Crear un entorno virtual para tu proyecto
cd ~
python3 -m venv openscope_env

# 3. Activar el entorno virtual
source openscope_env/bin/activate

# 4. Ahora sí instalar los paquetes (dentro del entorno)
pip install pyserial requests numpy

# 5. Verificar que están instalados
pip list

# Para salir del entorno virtual cuando termines
deactivate
