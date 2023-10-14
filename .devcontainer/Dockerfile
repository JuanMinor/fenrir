FROM gcc:latest

USER root

COPY ./scripts/setup.sh /tmp/setup.sh

RUN chmod +x /tmp/setup.sh \
    && ./tmp/setup.sh
