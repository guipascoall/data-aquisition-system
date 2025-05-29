import argparse
import socket

def main(server_ip, server_port, sensor_id, num_records):
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.connect((server_ip, server_port))
        request = f"GET|{sensor_id}|{num_records}\r\n"
        s.sendall(request.encode())
        
        print(f"Sent request: {request.strip()}")
        
        response = s.recv(1024).decode().strip()
        print(f"Received response: {response}")

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Client emulator for data acquisition system.')
    parser.add_argument('--ip', type=str, default='localhost',
                        help='The IP address of the server.')
    parser.add_argument('--port', type=int, default=9000,
                        help='The port number of the server.')
    parser.add_argument('sensor_id', type=str, help='The ID of the sensor.')
    parser.add_argument('num_records', type=int, help='The number of records to request.')
    
    args = parser.parse_args()
    main(args.ip, args.port, args.sensor_id, args.num_records)
