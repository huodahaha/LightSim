#!/bin/bash

#Todo: handle the argument of benchmark

for i in "$@"
do
case $i in
    -o=*|--output*)
    output_file="${i#*=}"
    ;;
    -t=*|--trace_count=*)
    trace_count="${i#*=}"
    ;;
    -b=*|--benchmark=*)
    benchmark="${i#*=}"
    ;;

esac
done

pin -ifeellucky -t /usr/local/pin_object/obj-intel64/pin_tracer.so \
    -t ${trace_count} -o /usr/local/trace/${output_file}.trace -- ${benchmark} \
    && gzip /usr/local/trace/${output_file}.trace
