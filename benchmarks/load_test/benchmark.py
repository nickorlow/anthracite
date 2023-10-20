import requests
import time
import math
from concurrent.futures import ThreadPoolExecutor
from http.client import HTTPConnection

HTTPConnection._http_vsn_str = 'HTTP/1.0'
urls = {  'anthracite': 'http://localhost:8081/50MB.zip','nginx': 'http://localhost:8082/50MB.zip', 'apache': 'http://localhost:8083/50MB.zip' }
num_requests = 1000
num_users = 100 # number of threads
response_times = {} 

def percentile(N, percent, key=lambda x:x):
    if not N:
        return None
    N.sort()
    k = (len(N)-1) * percent
    f = math.floor(k)
    c = math.ceil(k)
    if f == c:
        return key(N[int(k)])
    d0 = key(N[int(f)]) * (c-k)
    d1 = key(N[int(c)]) * (k-f)
    return d0+d1

# Define a function to make an HTTP request and measure the response time
def make_request(request_number, server_name):
    start_time = time.time()
    response = requests.get(urls[server_name])
    end_time = time.time()

    if response.status_code == 200:
        response_time = end_time - start_time
        response_times[server_name].append(response_time)
    else:
        print(f'Request {request_number}: Request failed with status code {response.status_code}')

print('=====[ Anthracite Benchmarking Tool ]=====')
print(f'Requests     : {num_requests}')
print(f'Users/Threads: {num_users}')
print(f'Test         : Load Test\n\n')
start_all_time = time.time()

futures = []

for server_name in urls:
    response_times[server_name] = []
    with ThreadPoolExecutor(max_workers=num_users) as executor:
        futures += [executor.submit(make_request, i + 1, server_name) for i in range(num_requests)]

# Wait for all requests to complete
for future in futures:
    future.result()

end_all_time = time.time()

for server_name in urls:
    print(f'====[ {server_name} ]=====')
    average_response_time = sum(response_times[server_name]) / len(response_times[server_name])
    total_response_time = sum(response_times[server_name])
    print(f'Average Response Time: {average_response_time:.4f} seconds')
    print(f'p995 Response Time   : {percentile(response_times[server_name], .995):.4f} seconds')
    print(f'p99  Response Time   : {percentile(response_times[server_name], .99):.4f} seconds')
    print(f'p90  Response Time   : {percentile(response_times[server_name], .90):.4f} seconds')
    print(f'p75  Response Time   : {percentile(response_times[server_name], .75):.4f} seconds')
    print(f'p50  Response Time   : {percentile(response_times[server_name], .50):.4f} seconds')
    print(f'Total Response Time  : {total_response_time:.4f} seconds')

total_test_time = end_all_time - start_all_time
print('==========')
print(f'Total Test Time      : {total_test_time:.4f} seconds')
