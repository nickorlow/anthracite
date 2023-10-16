cd ..
docker build . -t anthracite:latest
cd benchmark
docker build . -t benchmark-anthracite -f anthracite.Dockerfile
docker compose up -d
