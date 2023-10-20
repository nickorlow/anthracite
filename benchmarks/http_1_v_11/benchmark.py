import socket
import time

num_requests = 10000

http_1_times = []
http_11_times = []

print('=====[ Anthracite Benchmarking Tool ]=====')
print(f'Requests     : {num_requests}')
print(f'Test         : HTTP 1.0 vs HTTP 1.1\n\n')

with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.connect(("localhost" , 8091))
    for i in range(num_requests):
        start_time = time.time()
        s.sendall(b"GET /test.html HTTP/1.1\r\nAccept: text/html\r\nConnection: keep-alive\r\n\r\n")
        data = s.recv(220)
        end_time = time.time()
        http_11_times.append((end_time - start_time))
    s.close()

for i in range(num_requests):
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        start_time = time.time()
        s.connect(("localhost" , 8091))
        s.sendall(b"GET /test.html HTTP/1.0\r\nAccept: text/html\r\n\r\n")
        data = s.recv(220)
        end_time = time.time()
        http_1_times.append((end_time - start_time))
        s.close()

run_time_1 = sum(http_1_times) 
run_time_11 = sum(http_11_times)
print(f'HTTP/1.0 Total Time: {run_time_1:.4f} seconds')
print(f'HTTP/1.1 Total Time: {run_time_11:.4f} seconds')
