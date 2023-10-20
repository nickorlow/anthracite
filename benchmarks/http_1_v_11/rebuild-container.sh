cd ../..
docker build . -t anthracite:latest
cd benchmarks/http_1_v_11
docker compose build
