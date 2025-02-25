# 0.3.0 
- SSL support via OpenSSL
- Added "Thread Manager" class to allow for multiple (or custom) threading models (process per thread, event loop)
- Default (and only included) threading model is now event loop
- Rewrote request parser for readability and speed
- Rewrote socket system for readability and speed
- Added improved logging with different log levels
- Separated anthracite into libanthracite and anthracite-bin to allow for other projects to implement anthracite (example in ./src/api_main.cpp)
- Cleaned up code and seperated most code into headers & source
- Revamped build system to use CMake properly
- Moved CI/CD over to Forgejo
- Added simple config file system (will be completely replaced by v1.0)
- General system stability improvements were made to enhance the user's experience

## HTTP Request Parser Rewrite

The following benchmark (source in ./tests/speed_tests.cpp) shows the speed 
improvements made between 0.2.0 and 0.3.0, as well as comparison to boost's 
parsing library. 

It should probably be noted that Boost's parser can do a lot more than mine 
and is likely slower for good reason. Also, these were single runs but 
subsequent runs showed similar results.

| Parser Tested      | RPS          |
|--------------------|--------------|
| Anthracite 0.2.0   | 688,042      |
| Anthracite 0.3.0   | 27,027,000   |
| Boost Beast        | 1,023,230    |

# 0.2.0 Fifth Pre-Release
- Added true HTTP/1.1 support with persistent connections
- Added HTTP/1.0 vs HTTP/1.1 test to benchmarking suite
- Slight improvements to benchmarking
- Added version number information to binaries at build-time
- Added error page generation build step
- GitHub CI pipeline will now tag the version when publishing the container to the registry
- General system stability improvements were made to enhance the user's experience

## HTTP/1.1 Speed improvements
The following benchmark shows the speed improvements created by implementing
persistent connections with HTTP/1.1. This test measures the time it takes to 
request a 222 byte file 10,000 times by one user 

```
=====[ Anthracite Benchmarking Tool ]=====
Requests     : 10000
Test         : HTTP 1.0 vs HTTP 1.1


HTTP/1.0 Total Time: 3.1160 seconds
HTTP/1.1 Total Time: 0.3621 seconds
```

## Benchmark Results
Each benchmark makes 1000 requests requesting a 50MB file using 
100 users to the webserver running in a Docker container.

This is a change from previous benchmarks which used a large html
file.

```
=====[ Anthracite Benchmarking Tool ]=====
Requests     : 1000
Users/Threads: 100
Test         : Load Test


====[ anthracite ]=====
Average Response Time: 19.5831 seconds
p995 Response Time   : 38.9563 seconds
p99  Response Time   : 37.1518 seconds
p90  Response Time   : 27.5117 seconds
p75  Response Time   : 21.4345 seconds
p50  Response Time   : 17.7999 seconds
Total Response Time  : 19583.1491 seconds
====[ nginx ]=====
Average Response Time: 19.5464 seconds
p995 Response Time   : 49.9527 seconds
p99  Response Time   : 47.5037 seconds
p90  Response Time   : 29.7642 seconds
p75  Response Time   : 21.4559 seconds
p50  Response Time   : 17.1338 seconds
Total Response Time  : 19546.4399 seconds
====[ apache ]=====
Average Response Time: 20.8133 seconds
p995 Response Time   : 42.5797 seconds
p99  Response Time   : 39.8580 seconds
p90  Response Time   : 30.1892 seconds
p75  Response Time   : 22.3492 seconds
p50  Response Time   : 19.0437 seconds
Total Response Time  : 20813.3035 seconds
==========
Total Test Time      : 612.3112 seconds
```

# 0.1.2 Fourth Pre-Release

- Fixed bug with mapping / to index.html
- Addex Origin-Server header

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
