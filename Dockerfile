FROM alpine as build-env

RUN apk add --no-cache build-base
COPY ./src/ .
RUN make build-release 

FROM alpine 
RUN apk add --no-cache build-base
COPY --from=build-env /anthracite /anthracite 
COPY --from=build-env /www /www 
COPY --from=build-env /error_pages /error_pages 
CMD ["/anthracite"] 
