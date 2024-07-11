    .file    "/home/mcq/zp/ci/1.sy"
    .option    pic
    .text
    .align    1
    .globl    main
    .type    main,@function
main:
    sd    ra, -8(sp)
    sd    fp, -16(sp)
    mv    fp, sp
    addi    sp, sp, -16
    j    main_ret
main_ret:
    li    s1, 1080209441
    fmv.s.x    fa0, s1
    addi    sp, sp, 16
    ld    fp, -16(sp)
    ld    ra, -8(sp)
    ret
    .size    main, .-main
