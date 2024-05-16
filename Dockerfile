FROM ubuntu:22.04

RUN apt-get update
RUN apt-get install -y --no-install-recommends curl sudo
RUN curl https://raw.githubusercontent.com/spiritEcosse/aws-sailfish-sdk/master/install.sh | bash -s -- --func=add_user=ubuntu
USER ubuntu
WORKDIR /home/ubuntu
RUN curl https://raw.githubusercontent.com/spiritEcosse/aws-sailfish-sdk/master/install.sh | bash -s -- --func=foxy_sever_libs
