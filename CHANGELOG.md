# 0.1.1 Third Pre-Release

- Add mappings for common MIME types
- Heavily improved resource utilization
- Removed non-cached file backend 
- Updated dockerfile to remove build files after build

## Known Issues

- Benchmark program does not include resource utilization

## Benchmark Results

Each benchmark makes 1000 requests requesting a 21.9Mb file using 
100 users to the webserver running in a Docker container.

The results from this benchmark vary quite a bit. About half the time,
Apache can beat anthracite.

```
=====[ Anthracite Benchmarking Tool ]=====
Requests     : 1000
Users/Threads: 100


====[ anthracite ]=====
Average Response Time: 5.5128 seconds
p995 Response Time   : 8.3105 seconds
p99  Response Time   : 8.1796 seconds
p90  Response Time   : 6.4393 seconds
p75  Response Time   : 5.8587 seconds
p50  Response Time   : 5.4393 seconds
Total Response Time  : 5512.8397 seconds
====[ nginx ]=====
Average Response Time: 6.0201 seconds
p995 Response Time   : 12.4648 seconds
p99  Response Time   : 11.9635 seconds
p90  Response Time   : 8.7204 seconds
p75  Response Time   : 6.7331 seconds
p50  Response Time   : 5.5341 seconds
Total Response Time  : 6020.1369 seconds
====[ apache ]=====
Average Response Time: 5.9795 seconds
p995 Response Time   : 12.8266 seconds
p99  Response Time   : 11.6336 seconds
p90  Response Time   : 7.1465 seconds
p75  Response Time   : 6.4420 seconds
p50  Response Time   : 5.8224 seconds
Total Response Time  : 5979.5446 seconds
==========
Total Test Time      : 179.7469 seconds
```

# 0.1.0 Second Pre-Release 

- Allowed multiple clients to be handled at once via multithreadding
- Enabled file caching in file_backend
- Added benchmarking utilities 

## Known Issues
- High resource utilization
- Content other than HTML will still be treated as `text/html`

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
