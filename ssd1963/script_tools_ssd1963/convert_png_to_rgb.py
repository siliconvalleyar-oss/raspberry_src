# convertir_png_a_rgb565.py
from PIL import Image

def convert_to_rgb565(filename, output):
    img = Image.open(filename).convert('RGB')
    img = img.resize((480, 272))  # Por si no está ya escalada
    with open(output, 'wb') as f:
        for y in range(img.height):
            for x in range(img.width):
                r, g, b = img.getpixel((x, y))
                rgb565 = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3)
                f.write(rgb565.to_bytes(2, byteorder='big'))

convert_to_rgb565("assets/capibaras.png", "assets/capibaras.rgb565")
