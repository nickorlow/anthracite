docker-compose stop && ./rebuild-container.sh && docker compose up -d && clear && python3 benchmark.py && docker-compose stop
