import serial
import keyboard
import time

# --- PENGATURAN ---
# GANTI 'COMx' DENGAN COM PORT OUTGOING ANDA (Contoh: 'COM5')
bluetooth_port = 'COM3' 
baud_rate = 115200

try:
    ser = serial.Serial(bluetooth_port, baud_rate, timeout=1)
    print(f"Terhubung ke {bluetooth_port}. Gunakan WSAD untuk mengemudi!")
    print("Tekan ESC untuk keluar.")
except Exception as e:
    print(f"Gagal terhubung: {e}")
    print("Pastikan Bluetooth terhubung dan COM Port benar.")
    exit()

# Status tombol untuk mencegah pengiriman data berulang-ulang
keys_status = {'w': False, 's': False, 'a': False, 'd': False}

def send_command(char):
    try:
        ser.write(char.encode())
        # print(f"Sent: {char}") # Uncomment untuk debugging
    except:
        print("Error mengirim data")

while True:
    try:
        # --- MAJU (W) ---
        if keyboard.is_pressed('w'):
            if not keys_status['w']: # Jika sebelumnya belum ditekan
                send_command('W')
                keys_status['w'] = True
        else:
            if keys_status['w']: # Jika sebelumnya ditekan, sekarang dilepas
                send_command('w')
                keys_status['w'] = False

        # --- MUNDUR (S) ---
        if keyboard.is_pressed('s'):
            if not keys_status['s']:
                send_command('S')
                keys_status['s'] = True
        else:
            if keys_status['s']:
                send_command('s')
                keys_status['s'] = False

        # --- KIRI (A) ---
        if keyboard.is_pressed('a'):
            if not keys_status['a']:
                send_command('A')
                keys_status['a'] = True
        else:
            if keys_status['a']:
                send_command('a')
                keys_status['a'] = False

        # --- KANAN (D) ---
        if keyboard.is_pressed('d'):
            if not keys_status['d']:
                send_command('D')
                keys_status['d'] = True
        else:
            if keys_status['d']:
                send_command('d')
                keys_status['d'] = False

        # Keluar Program
        if keyboard.is_pressed('esc'):
            print("Keluar...")
            break
            
        time.sleep(0.01) # Delay kecil agar CPU tidak panas

    except KeyboardInterrupt:
        break

ser.close()