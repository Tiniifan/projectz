@ Trampolines for the two places where the game inlined its flag writes instead
@ of calling an accessor. Each one is branched to in place of the first
@ instruction of the block it replaces, marshals the registers the block was
@ using into a call, then jumps to the instruction the block fell through to.

.text
.align 4

@ Replaces 0x0039B900 to 0x0039BA1C, the flag block of the recruit routine.
@ r5 holds the CharaFlags object, r7 the chara_base record of the recruited
@ character and r9 the object owning the "players collected" tally. r0 to r4,
@ r5, r8, r9 and fp are all dead once the block is over, and lr was saved by the
@ prologue, so the call is free to clobber them.
.global ie4CharaMarkRecruitedAsm
.type ie4CharaMarkRecruitedAsm, %function
ie4CharaMarkRecruitedAsm:
    mov     r0, r5
    mov     r1, r7
    mov     r2, r9
    bl      ie4CharaMarkRecruited
    ldr     pc, =0x0039BA1C
.ltorg

@ Replaces 0x0028EF5C to 0x0028EFA8, the tail of the script command toggling the
@ "new" marker. r0 already holds the CharaFlags object, r4 the model number and
@ r6 the value to store; the epilogue only needs r0, which it sets itself.
.global ie4CharaFlagMarkNewAsm
.type ie4CharaFlagMarkNewAsm, %function
ie4CharaFlagMarkNewAsm:
    mov     r2, r6
    mov     r1, r4
    bl      ie4CharaFlagMarkNew
    ldr     pc, =0x0028EFAC
.ltorg
