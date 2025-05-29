import argparse
import random
import string
import socket
import time
from datetime import datetime

def random_string(length):
    letters = string.ascii_lowercase
    return ''.join(random.choice(letters) for i in range(length))

def main(server_ip, server_port, sensor_id, frequency):
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.connect((server_ip, server_port))
        while True:
            value = random.uniform(-100, 100)
            timestamp = datetime.now().isoformat()  # Generate current timestamp
            message = f"LOG|{sensor_id}|{timestamp}|{value}\r\n"
            s.sendall(message.encode())
            time.sleep(frequency / 1000.0)  # Convert frequency from ms to s

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Sensor simulator for data acquisition system.')
    parser.add_argument('--ip', type=str, default='localhost',
                        help='The IP address of the server.')
    parser.add_argument('--port', type=int, default=9000,
                        help='The port number of the server.')
    parser.add_argument('--sensor_id', type=str, default=random_string(10),
                        help='The ID of the sensor.')
    parser.add_argument('--frequency', type=int, default=1000,
                        help='The frequency in milliseconds of sensor readings.')

    args = parser.parse_args()
    main(args.ip, args.port, args.sensor_id, args.frequency)
