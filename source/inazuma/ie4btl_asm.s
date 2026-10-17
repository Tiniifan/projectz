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

@ Replaces 0x00409CA4 in the kickoff step of the match, right after the game
@ has stored the team that kicks off and before the one forced by a script is
@ applied. r4 holds the match and r5 its live state buffer; r1 must still be 0
@ afterwards, so the scratch registers are saved around the call. The replaced
@ instruction loads a literal relative to pc, so the same value is loaded from
@ the pool of this file instead.
.global ie4BtlOnKickoffAsm
.type ie4BtlOnKickoffAsm, %function
ie4BtlOnKickoffAsm:
    push    {r0-r3, r12, lr}
    mov     r0, r4
    mov     r1, r5
    bl      ie4BtlOnKickoff
    pop     {r0-r3, r12, lr}
    ldr     r0, =0x0000DA78
    ldr     pc, =0x00409CA8
.ltorg

@ Replaces 0x003D8D08, the first instruction of the function that turns the
@ force win list into the two goal barrier flags, every frame. The replaced
@ instruction saves the registers of the function itself, so it is run once
@ the scratch ones are restored.
.global ie4BtlOnGoalGuardAsm
.type ie4BtlOnGoalGuardAsm, %function
ie4BtlOnGoalGuardAsm:
    push    {r0-r3, r12, lr}
    bl      ie4BtlOnGoalGuard
    pop     {r0-r3, r12, lr}
    push    {r0-r12, lr}
    ldr     pc, =0x003D8D0C
.ltorg

@ Replaces 0x003CA35C in the field setup, where a challenge picks the walls
@ that close its goal lines. r9 holds the collision world and the field
@ geometry sits at sp + 0x4C. ie4BtlChallengeWalls returns 0 for the game's
@ walls closing both goal lines, 1 for its walls leaving both goal mouths open
@ and 2 when it added the walls itself. The scratch registers are saved around
@ the call; pop leaves the flags alone.
.global ie4BtlChallengeWallsAsm
.type ie4BtlChallengeWallsAsm, %function
ie4BtlChallengeWallsAsm:
    push    {r0-r3, r12, lr}
    mov     r0, r9
    ldr     r1, [sp, #0x64]
    bl      ie4BtlChallengeWalls
    cmp     r0, #1
    pop     {r0-r3, r12, lr}
    ldreq   pc, =0x003CA36C
    ldrhi   pc, =0x003CA5AC
    ldr     pc, =0x003CA524
.ltorg

@ Replaces 0x003ECFAC in the check that turns a command of a player into a
@ shot, where a challenge tests whether it has goals at all. sl holds the
@ field geometry and r8 the team of the player; r0 must come out non-zero to
@ forbid the shot. Everything the check may still need is saved around the
@ call, the VFP scratch registers included.
.global ie4BtlShotCheckAsm
.type ie4BtlShotCheckAsm, %function
ie4BtlShotCheckAsm:
    push    {r1-r4, r12, lr}
    vpush   {d0-d7}
    mov     r0, sl
    and     r1, r8, #0xFF
    bl      ie4BtlShotForbidden
    vpop    {d0-d7}
    pop     {r1-r4, r12, lr}
    ldr     pc, =0x003ECFC4
.ltorg
