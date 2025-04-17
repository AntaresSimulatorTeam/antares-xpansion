FROM ubuntu:20.04

# Set environment variables to avoid tzdata interactive dialog
ENV DEBIAN_FRONTEND=noninteractive

SHELL ["/bin/bash", "-c"]

ENV CLANG_VERSION=19
#Max supported version of CMake by CLion
ENV CMAKE_VERSION=3.28.6
ENV NINJA_VERSION=v1.12.1
#Max supported version of gdb by CLion
ENV GDB_VERSION=14.1

RUN addgroup --gid 1000 docker && \
    adduser --uid 1000 --ingroup docker --home /home/docker --shell /bin/sh --disabled-password --gecos "" docker

# Update the system and install necessary packages
RUN apt update
RUN apt update --fix-missing
#install clang and lld-10 to speed up the build over default ld
#curl and ca-certificates to be able to use vcpkg
#texinfo required for GDB 14 (not anymore for 16.x I think)
RUN apt install -y --no-install-recommends --no-install-suggests \
    build-essential \
    wget \
    git \
    python3 \
    pkg-config \
    zip \
    curl \
    libmpfr-dev libgmp3-dev libmpc-dev \
    libssl-dev \
    unzip \
    ca-certificates \
    texinfo \
    lsb-release wget software-properties-common gnupg \
    && rm -rf /var/lib/apt/lists/*

#Cmake
RUN wget --no-check-certificate https://github.com/Kitware/CMake/releases/download/v${CMAKE_VERSION}/cmake-${CMAKE_VERSION}-linux-x86_64.tar.gz \
    && tar xvf cmake-${CMAKE_VERSION}-linux-x86_64.tar.gz -C /usr/local --strip-components=1\
    && rm cmake-${CMAKE_VERSION}-linux-x86_64.tar.gz

#Ninja
RUN wget --no-check-certificate https://github.com/ninja-build/ninja/releases/download/${NINJA_VERSION}/ninja-linux.zip && \
    unzip ninja-linux.zip -d /usr/local/bin/ && \
    rm ninja-linux.zip

# Clang
RUN wget https://apt.llvm.org/llvm.sh \
    && chmod +x llvm.sh \
    && ./llvm.sh ${CLANG_VERSION} \
    && apt-get install clang-tools-19 clang-format-19 clangd-19 clang-tidy-19 -y

#gdb
RUN update-alternatives --install /usr/local/bin/cc cc /usr/bin/clang-19 100 \
    && update-alternatives --install /usr/local/bin/clang clang /usr/bin/clang-19 100 \
    && update-alternatives --install /usr/local/bin/cpp ccp /usr/bin/clang++-19 100 \
    && update-alternatives --install /usr/local/bin/c++ c++ /usr/bin/clang++-19 100 \
    && update-alternatives --install /usr/local/bin/clang++ clang++ /usr/bin/clang++-19 100 \
    && update-alternatives --install /usr/local/bin/ld ld /usr/bin/ld.lld-19 100 \
    && update-alternatives --install /usr/local/bin/clang-format clang-format /usr/bin/clang-format-19 100 \
    && update-alternatives --install /usr/local/bin/clang-tidy clang-tidy /usr/bin/clang-tidy-19 100 \
    && rm /etc/ld.so.cache \
    && ldconfig -C /etc/ld.so.cache \
    && update-alternatives --install /usr/bin/cmake cmake /usr/local/bin/cmake 100 \
    && update-alternatives --install /usr/bin/ninja ninja /usr/local/bin/ninja 100

RUN CXX=/usr/bin/clang && CC=/usr/bin/clang && \
    wget "http://ftp.gnu.org/gnu/gdb/gdb-${GDB_VERSION}.tar.gz" && \
    tar -xvf gdb-${GDB_VERSION}.tar.gz && \
    pushd gdb-${GDB_VERSION} && \
    mkdir build && \
    pushd build && \
    ../configure --prefix=/usr/local && \
    make -j$(nproc) && \
    make install-strip && \
    popd && \
    popd && \
    rm gdb-${GDB_VERSION}.tar.gz && \
    rm -rf gdb-${GDB_VERSION} && \
    update-alternatives --install /usr/bin/gdb gdb /usr/local/bin/gdb 100 \
    && apt remove texinfo -y

#https://github.com/boxboat/fixuid
RUN USER=docker && \
    GROUP=docker && \
    curl -SsL https://github.com/boxboat/fixuid/releases/download/v0.6.0/fixuid-0.6.0-linux-amd64.tar.gz | tar -C /usr/local/bin -xzf - && \
    chown root:root /usr/local/bin/fixuid && \
    chmod 4755 /usr/local/bin/fixuid && \
    mkdir -p /etc/fixuid && \
    printf "user: $USER\ngroup: $GROUP\n" > /etc/fixuid/config.yml

# Set the working directory
WORKDIR /root

USER root

RUN apt-get update \
    && apt-get install -y lsb-release python3-distutils libstdc++-10-dev \
    && wget https://bootstrap.pypa.io/get-pip.py \
    && python3 get-pip.py


RUN mkdir /work
WORKDIR /work
ADD requirements.txt requirements-doc.txt requirements-tests.txt /work/

RUN python3 -m pip install --upgrade pip \
    && python3 -m pip install -r /work/requirements-tests.txt -r /work/requirements-doc.txt

#https://github.com/boxboat/fixuid
#Prevent messing permissions especially on mounted volumes
RUN USER=docker && \
    GROUP=docker && \
    curl -SsL https://github.com/boxboat/fixuid/releases/download/v0.6.0/fixuid-0.6.0-linux-amd64.tar.gz | tar -C /usr/local/bin -xzf - && \
    chown root:root /usr/local/bin/fixuid && \
    chmod 4755 /usr/local/bin/fixuid && \
    mkdir -p /etc/fixuid && \
    printf "user: $USER\ngroup: $GROUP\n" > /etc/fixuid/config.yml

ENTRYPOINT ["fixuid"]