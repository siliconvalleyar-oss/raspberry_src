import cv2
import os


img = cv2.imread("spritesheet.png")
h, w, _ = img.shape

print("width:", w)
print("height:", h)

gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)

# binarización suave (solo para detectar)
_, thresh = cv2.threshold(gray, 10, 255, cv2.THRESH_BINARY_INV)

# encontrar contornos externos
contours, _ = cv2.findContours(thresh, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)

boxes = []

for cnt in contours:
    x, y, w, h = cv2.boundingRect(cnt)

    # filtrar ruido
    if w < 15 or h < 15:
        continue

    boxes.append((x, y, w, h))

# 🔥 ordenar por filas y columnas
boxes = sorted(boxes, key=lambda b: (b[1] // 40, b[0]))

# guardar sprites
for i, (x, y, w, h) in enumerate(boxes):
    sprite = img[y:y+h, x:x+w]
    cv2.imwrite(f"sprites/sprite_{i:02d}.png", sprite)

print("Sprites guardados:", len(boxes))


h, w, _ = img.shape

cols = 6
rows = 5

sprite_w = w // cols   # 150
sprite_h = h // rows   # 148

os.makedirs("sprites", exist_ok=True)

i = 0
for r in range(rows):
    for c in range(cols):
        x = c * sprite_w
        y = r * sprite_h

        sprite = img[y:y+sprite_h, x:x+sprite_w]

        cv2.imwrite(f"sprites/sprite_{i:02d}.png", sprite)
        i += 1

print("Sprites generados:", i)
