FROM alpine as build-env

RUN apk add --no-cache build-base python3 cmake
COPY ./src/ .
RUN rm -rf CMakeCache.txt CMakeFiles
RUN cmake -DCMAKE_BUILD_TYPE=Release .
RUN make anthracite-bin 

FROM alpine 
RUN apk add --no-cache build-base
COPY --from=build-env /anthracite /anthracite 
COPY --from=build-env /www /www 
COPY --from=build-env /error_pages /error_pages 
CMD ["/anthracite"] 
