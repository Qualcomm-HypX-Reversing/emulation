#!/bin/bash

./qemu-system-aarch64 -M virt -cpu cortex-a57 -nographic -kernel bin/hyp.mbn -machine virtualization=on -machine gic-version=3 -monitor telnet:127.0.0.1:1235,server,nowait -s -S -smp 1 -m 4G -d int -device loader,file="bin/devcfg.mbn"


