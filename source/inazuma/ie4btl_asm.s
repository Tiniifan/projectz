@ Trampoline for the match loader, which has no call of its own at the point
@ every match start path goes through.

.text
.align 4

@ Replaces 0x00417B84, the first instruction of the block all start paths of
@ the match loader meet at, right before InitBtl and BtlSetting are called.
@ Every register may still be live there, so the scratch ones are saved around
@ the call; the flags are not, the block sets them itself before testing them.
@ The replaced instruction does not depend on pc, so it is run as is here.
.global ie4BtlOnMatchSetupAsm
.type ie4BtlOnMatchSetupAsm, %function
ie4BtlOnMatchSetupAsm:
    push    {r0-r3, r12, lr}
    bl      ie4BtlOnMatchSetup
    pop     {r0-r3, r12, lr}
    ldr     r0, [r4]
    ldr     pc, =0x00417B88
.ltorg
