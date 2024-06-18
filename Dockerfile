FROM ubuntu:22.04

RUN apt-get update && \
    apt-get install -y --no-install-recommends curl sudo ca-certificates && \
    rm -rf /var/lib/apt/lists/*

ARG LLVM_TAG

RUN curl https://raw.githubusercontent.com/spiritEcosse/aws-sailfish-sdk/master/install.sh | bash -s -- --func='add_user=ubuntu'
USER ubuntu
WORKDIR /home/ubuntu
RUN export LLVM_TAG=$LLVM_TAG && curl https://raw.githubusercontent.com/spiritEcosse/aws-sailfish-sdk/master/install.sh | bash -s -- --func=foxy_sever_libs
