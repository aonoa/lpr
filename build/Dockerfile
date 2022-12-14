FROM ubuntu:22.04

#ENV DEBIAN_FRONTEND=noninteractive

COPY ./lpr /usr/local/lpr/
COPY ./models /usr/local/lpr/models
ADD ./libs/libs.tar.gz /usr/lib/

WORKDIR /usr/local/lpr

CMD /usr/local/lpr/lpr

