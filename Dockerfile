FROM alpine as build-env

RUN apk add --no-cache build-base python3 cmake openssl-dev
COPY ./src ./src
COPY ./lib ./lib
COPY ./tests ./tests
COPY ./default_www ./default_www
COPY ./build_supp ./build_supp
COPY ./CMakeLists.txt .


RUN mkdir build
WORKDIR build
RUN cmake -DCMAKE_BUILD_TYPE=Release ..
RUN make anthracite-bin 

FROM alpine 
RUN apk add --no-cache libgcc libstdc++
COPY --from=build-env /build/anthracite-bin /anthracite-bin
COPY --from=build-env /build/error_pages /error_pages 
COPY --from=build-env /build_supp/default_config.cfg /anthracite.cfg
COPY /default_www/docker /www 
CMD ["/anthracite-bin"] 
