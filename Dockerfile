# syntax=docker/dockerfile:1

FROM ubuntu:22.04
LABEL maintainer=aquinn1@ucsc.edu

# home directory, toolchain directory, and pintos root
ENV HOME /home/cse134
ENV SWD=${HOME}/toolchain
ENV PINTOS_ROOT=${HOME}/pintos-tmp

WORKDIR ${HOME}

# install prerequisite packages
RUN apt update && apt -y install \
    build-essential \
    automake \
    expat \
    git \
    libexpat1-dev \
    libgmp-dev \
    libmpc-dev \
    libncurses5-dev \
    pkg-config \
    texinfo \
    wget \
    libx11-dev \
    libxrandr-dev \
    tmux \
    vim \
    python3 \
    cgdb \
    locales \
    libreadline-dev \
    python3-dev \
    file

# set locales
RUN locale-gen en_US.UTF-8 && update-locale LC_ALL=en_US.UTF-8 LANG=en_US.UTF-8
ENV LANG en_US.UTF-8
ENV LANGUAGE en_US:en
ENV LC_ALL en_US.UTF-8

# copy over the source
COPY . ${PINTOS_ROOT}

# build bochs
RUN cd ${PINTOS_ROOT} && src/misc/bochs-2.6.2-build.sh ${SWD}

# install qemu
 RUN apt update && apt install -y \
    bridge-utils \
    libvirt-clients \
    libvirt-daemon \
    libvirt-daemon-system \
    qemu \
    qemu-kvm \
    qemu-system-i386 \
    virtinst

# build toolchain
 RUN cd ${PINTOS_ROOT} && src/misc/toolchain-build.sh ${SWD}

# build pintos utility tools
 RUN cd ${PINTOS_ROOT}/src/utils && \
    make && \
    mkdir -p ${SWD}/bin && \
    cp backtrace pintos Pintos.pm pintos-gdb pintos-set-cmdline pintos-mkdisk setitimer-helper squish-pty squish-unix ${SWD}/bin && \
    mkdir -p ${SWD}/misc && \
    cp ../misc/gdb-macros ${SWD}/misc

# add toolchain into path
 RUN echo PATH=${HOME}/toolchain/`uname -m`/bin:${HOME}/toolchain/bin:$PATH >> ~/.bashrc

# remove the pintos root
 RUN rm -rf ${PINTOS_ROOT}

# docker will end executing immediately without this
CMD ["bash"]
