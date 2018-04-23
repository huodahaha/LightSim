#!/bin/bash

#Todo: handle the argument of benchmark
POSITIONAL=()
while [[ $# -gt 0 ]]
do
key="$1"
case $key in
    -o|--output)
    output_file=$2
    shift
    shift
    ;;
    -t|--trace_count)
    trace_count=$2
    shift
    shift
    ;;
    *)
    break
    ;;
esac
done

echo output_file=${output_file}
echo trace_count=${trace_count}
echo benchmark=$@

pin -ifeellucky -t /usr/local/pin_object/obj-intel64/pin_tracer.so \
    -t ${trace_count} -o /usr/local/trace/${output_file}.trace -- $@ \
     && gzip /usr/local/trace/${output_file}.trace

