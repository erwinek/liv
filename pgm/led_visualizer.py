import serial
import pygame
import sys

# Konfiguracja
WINDOW_WIDTH = 1000
WINDOW_HEIGHT = 400
LED_SIZE = 20
LED_SPACING = 5
LINE_SPACING = 40

# Konfiguracja układu LED
LED_LAYOUT = [
    4,   # front left
    12,  # czacha
    4,   # front right
    20   # bramka 
]

# Inicjalizacja PyGame
pygame.init()
screen = pygame.display.set_mode((WINDOW_WIDTH, WINDOW_HEIGHT))
pygame.display.set_caption("LED2 Array Visualizer")

# Konfiguracja portu szeregowego
ser = serial.Serial('COM3', 1000000)  # Dostosuj numer portu

def draw_leds(led_data):
    screen.fill((0, 0, 0))
    led_index = 0
    
    # Rysuj każdą linię LEDów
    for line_num, num_leds in enumerate(LED_LAYOUT):
        y = LINE_SPACING + line_num * (LED_SIZE * 2 + LINE_SPACING)
        # Wycentruj linię
        x_start = (WINDOW_WIDTH - (num_leds * (LED_SIZE * 2 + LED_SPACING))) // 2
        
        # Rysuj LEDy w linii
        for i in range(num_leds):
            if led_index < len(led_data):
                color = led_data[led_index]
                x = x_start + i * (LED_SIZE * 2 + LED_SPACING)
                pygame.draw.circle(screen, color, (x + LED_SIZE, y), LED_SIZE)
                # Dodaj etykietę z numerem LED
                font = pygame.font.Font(None, 20)
                text = font.render(str(led_index), True, (128, 128, 128))
                screen.blit(text, (x + LED_SIZE - 6, y + LED_SIZE + 5))
            led_index += 1
    
    pygame.display.flip()

def parse_led_data(data):
    try:
        if not data.startswith('LED2:'): return None
        data = data[5:].strip()  # Usuń "LED2:" i whitespace
        leds = []
        for led in data.split(';'):
            if not led: continue
            r, g, b = map(int, led.split(','))
            leds.append((r, g, b))
        return leds
    except:
        return None

# Główna pętla
running = True
while running:
    for event in pygame.event.get():
        if event.type == pygame.QUIT:
            running = False
    
    if ser.in_waiting:
        data = ser.readline().decode('ascii').strip()
        led_data = parse_led_data(data)
        if led_data:
            draw_leds(led_data)

pygame.quit()
ser.close()
sys.exit()