# build with: docker build -t pin-docker .

FROM ubuntu:16.04

RUN apt-get update \
     && apt-get -y upgrade \
     && apt-get -y install build-essential vim-tiny wget git gcc\
     && apt-get clean

RUN wget http://software.intel.com/sites/landingpage/pintool/downloads/pin-3.2-81205-gcc-linux.tar.gz -P /usr/local/ \
&& tar -xvf /usr/local/pin-3.2-81205-gcc-linux.tar.gz \
&& rm -f /usr/local/pin-3.2-81205-gcc-linux.tar.gz \
&& mv pin-3.2-81205-gcc-linux/ /usr/local/

ENV PIN_ROOT /usr/local/pin-3.2-81205-gcc-linux

ENV PATH="${PIN_ROOT}:${PATH}"

COPY . /usr/local/pin_object

RUN mkdir -p /usr/local/trace

RUN cd /usr/local/pin_object && mkdir -p obj-intel64/ && make obj-intel64/pin_tracer.so

#TODO: get benckmarks, for now just test with linux built in tools
#RUN mkdir -p /usr/local/bench_marks && cd /usr/local/benchmarks

#RUN git clone https://github.com/Multi2Sim/m2s-bench-parsec-3.0-src
#
#RUN cd m2s-bench-parsec-3.0-src && make
# this is not working correctly
#ENTRYPOINT ["pin", "-ifeellucky", "-t", "/usr/local/pin_object/obj-intel64/pin_tracer.so"]
#CMD ["-o", "/usr/local/trace/ls.trace", "--", "ls"]

ENTRYPOINT ["bash", "/usr/local/pin_object/get_trace.sh"]
CMD []
