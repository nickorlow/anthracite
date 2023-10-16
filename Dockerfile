FROM alpine as build-env

RUN apk add --no-cache build-base
COPY ./src/ .
RUN make build-release 

CMD ["/anthracite"] 
