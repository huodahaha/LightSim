#!/bin/bash

#usage ./run_docker -o=ls_trace -t=100000 =b=ls

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

#Todo check docker image existence and build docker

mkdir -p mount
docker run --privileged -v ${DIR}/mount:/usr/local/trace docker_pin $*

