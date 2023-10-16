# 0.1.0 Second Pre-Release 

- Allowed multiple clients to be handled at once via multithreadding
- Enabled file cacheing in file_backend
- Added benchmarking utils 

## Known Issues
- High resource utilization
- Content other than HTMl will be treated as `text/html`

## Benchmark Results

Each benchmark makes 1000 requests requesting a 21.9Mb file using 
100 users to the webserver running in a Docker container.

```
=====[ Anthracite Benchmarking Tool ]=====
Requests     : 1000
Users/Threads: 100


====[ anthracite ]=====
Average Response Time: 5.9561 seconds
p995 Response Time   : 65.9016 seconds
p99  Response Time   : 12.3729 seconds
p90  Response Time   : 7.8775 seconds
p75  Response Time   : 5.9906 seconds
p50  Response Time   : 5.0612 seconds
Total Response Time  : 5956.0972 seconds
====[ nginx ]=====
Average Response Time: 6.0074 seconds
p995 Response Time   : 12.5307 seconds
p99  Response Time   : 11.9421 seconds
p90  Response Time   : 8.4831 seconds
p75  Response Time   : 6.6225 seconds
p50  Response Time   : 5.5751 seconds
Total Response Time  : 6007.4155 seconds
====[ apache ]=====
Average Response Time: 5.9028 seconds
p995 Response Time   : 10.7204 seconds
p99  Response Time   : 10.0042 seconds
p90  Response Time   : 6.9419 seconds
p75  Response Time   : 6.3974 seconds
p50  Response Time   : 5.8380 seconds
Total Response Time  : 5902.8490 seconds
==========
Total Test Time      : 188.6373 seconds
```

# 0.0.0 First Pre-Release
- Added file based backend
- Support for text/html