FROM alpine as build-env

RUN apk add --no-cache build-base
COPY . .
RUN make build-release 

CMD ["/anthracite"] 
