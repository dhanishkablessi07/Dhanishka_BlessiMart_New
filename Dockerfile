FROM ubuntu:24.04

RUN apt-get update && apt-get install -y --no-install-recommends \
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
ENV VCPKG_MAX_CONCURRENCY=1

RUN cmake -S . -B build \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_TOOLCHAIN_FILE=/opt/vcpkg/scripts/buildsystems/vcpkg.cmake \
    -DBUILD_TESTING=OFF \
    -DVCPKG_BUILD_TYPE=release

RUN cmake --build build --config Release -j1

EXPOSE 8080

CMD ["./build/blessimart"]
