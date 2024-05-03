FROM ubuntu:22.04

RUN apt-get update
RUN apt-get install -y curl
RUN curl https://raw.githubusercontent.com/spiritEcosse/aws-sailfish-sdk/master/install.sh | bash -s -- --func=foxy_sever_libs
