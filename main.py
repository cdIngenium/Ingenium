import cv2
import numpy as np
import time
import serial 

cap = cv2.VideoCapture(0)

colores = {
    "Block_Rojo": ([0, 120, 70], [10, 255, 255]),
    "Block_Verde": ([36, 100, 100], [86, 255, 255]),
}

tiempo_inicio = time.time()
ser = serial.Serial('COM5', 115200, timeout=1)
time.sleep(1)

color_detectado = None  

def enviar_comando(comando): 
    ser.write(comando.encode())

while True:
    ret, frame = cap.read()
    if not ret:
        break

    hsv = cv2.cvtColor(frame, cv2.COLOR_BGR2HSV)
    max_area = 0
    nuevo_color_detectado = None
    contorno_mas_grande = None

    for nombre_color, (lower, upper) in colores.items():
        lower = np.array(lower)
        upper = np.array(upper)

        mask = cv2.inRange(hsv, lower, upper)
        mask = cv2.erode(mask, None, iterations=2)
        mask = cv2.dilate(mask, None, iterations=2)

        contornos, _ = cv2.findContours(mask, cv2.RETR_TREE, cv2.CHAIN_APPROX_SIMPLE)

        for contorno in contornos:
            area = cv2.contourArea(contorno)
            if area > 1000 and area > max_area:
                max_area = area
                nuevo_color_detectado = nombre_color
                contorno_mas_grande = contorno

    if nuevo_color_detectado and nuevo_color_detectado != color_detectado:
        color_detectado = nuevo_color_detectado
        enviar_comando(color_detectado)  
        print(f"{color_detectado} detectado")  

    if contorno_mas_grande is not None:
        x, y, w, h = cv2.boundingRect(contorno_mas_grande)
        cv2.rectangle(frame, (x, y), (x + w, y + h), (0, 255, 0), 2)
        cv2.putText(frame, color_detectado, (x, y - 10), cv2.FONT_HERSHEY_SIMPLEX, 0.9, (0, 255, 0), 2)

    if time.time() - tiempo_inicio >= 3:
        tiempo_inicio = time.time()

    cv2.imshow("Reconocimiento Color", frame)

    if cv2.waitKey(1) & 0xFF == ord("q"):
        break

cap.release()
ser.close()
cv2.destroyAllWindows()
