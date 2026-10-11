FROM ubuntu:24.04

RUN apt-get update && apt-get install -y \
    build-essential cmake git curl zip unzip tar pkg-config \
    libssl-dev libsqlite3-dev uuid-dev zlib1g-dev \
    libjsoncpp-dev libpq-dev libbrotli-dev libyaml-cpp-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY . .

RUN git clone https://github.com/microsoft/vcpkg.git /opt/vcpkg \
    && /opt/vcpkg/bootstrap-vcpkg.sh -disableMetrics

ENV VCPKG_ROOT=/opt/vcpkg
ENV PATH="${VCPKG_ROOT}:${PATH}"

RUN cmake -S . -B build \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_TOOLCHAIN_FILE=/opt/vcpkg/scripts/buildsystems/vcpkg.cmake \
    -DBUILD_TESTING=OFF

RUN cmake --build build --config Release -j2

EXPOSE 8080

CMD ["./build/blessimart"]
