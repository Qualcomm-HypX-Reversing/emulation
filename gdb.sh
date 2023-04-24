gdb-multiarch --se=bin/hyp.mbn \
                            -ex 'target remote :1234' \
                            -ex 'b *0x80093200'
