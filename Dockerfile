# Lightweight build image for AXIOM Engine
FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    python3 \
    python3-pip \
    libeigen3-dev \
    libopenmpi-dev \
    && apt-get clean && rm -rf /var/lib/apt/lists/*

WORKDIR /workspace

COPY . /workspace
RUN mkdir build && cd build && cmake -DCMAKE_BUILD_TYPE=Release .. && cmake --build .

CMD ["/bin/bash"]
