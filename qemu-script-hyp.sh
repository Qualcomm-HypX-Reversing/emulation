#!/bin/bash

#./qemu-system-aarch64 -M virt -cpu cortex-a57 -nographic -kernel el3/el3_firmware.elf -machine "secure=on" -machine virtualization=on -machine gic-version=3 -machine "iommu=smmuv3" -monitor telnet:127.0.0.1:1235,server,nowait -s -S -smp 1 -d int -m 4G -device "qcom-smem" -device loader,file="bin/devcfg.mbn" -device loader,file="bin/hyp.mbn" -serial stdio

#-bios el3/el3_firmware.elf

./qemu-system-aarch64 -M virt -cpu cortex-a57 -nographic -kernel bin/hyp.mbn -machine virtualization=on -machine gic-version=3  -machine "iommu=smmuv3" -monitor telnet:127.0.0.1:1235,server,nowait -s -S -smp 1 -m 4G -d int -device "qcom-smem"  -device loader,file="bin/devcfg.mbn" 



