cd ../..
docker build . -t anthracite:latest
cd benchmarks/load_test
docker compose build
docker compose up -d
