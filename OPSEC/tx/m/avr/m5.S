;
; XERIS/APEX System I
; Autonomous Poly-Executive
;
; Release 1
;
; Copyright (C) 2017, Richard Wai
; All rights reserved.
;

;
; XERIS CORE
;
; OPSEC%
; Operational Secretariat
;
; TX$
; Transaction Manager
;
; Core functions
;

;
; AVR 16-bit PC Implementation (No EIND)
; Compatible with all XERIS/APEX supported AVR
; models.
;


#include <sys/m.h>

.globl opsec_tx_event
.globl opsec_tx_rollback

.text

;
; extern void opsec_tx_event
; ( volatile dispatch_transact *, volatile xta *, xdcf * )
;
; Attempt to commit a new event as a transaction to the account
; OPSEC% has set-up the dispatch transaction account with the
; appropriate function pointer of dispatch desk of the transacting
; secretariat.
; Our job is to save the call-saved registers and record the stack
; pointer, before setting an active transaction.
;
; If we need to do a roll-back, we restore the call-saved registers
;
; r25:24  dispatch transact * active dispatch transaction file
; r23:22  xta * registered transaction account in question
; r21:20  xdcf * the active case file pointer
;

;
; Important Operational Note:
;
; tx_commit is called from the Central Dispatch Desk, not xs$
; so we do not have any re-entrancy protection. But, tx_rollback
; can only be called from xs$. In fact, the only way that the
; transaction will be interrupted is via XS$. XS$ always checks
; to see if we are in a transaction, and can do this safely as
; no preemption can happen inside of XS$. This means we do not need
; to check if we were interrupted from opsec_tx_event, after
; returning from the transaction executive. This is because XS$
; will ALWAYS call tx_rollback if we actually get interrupted
;

opsec_tx_event:

    ; Call-saved registers:
    ; R2-R17
    ; R28,29
    ; Include r0 for good measure

    ;
    push r0
    push r2
    push r3
    push r4
    push r5
    push r6
    push r7
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15
    push r16
    push r17
    
    push r28
    push r29
    push r30
    push r31

    ; first load Y with the dispatch transaction file
    ; and then load Z with the account registration

    movw r28, r24
    movw r30, r22

    ; XS$ uses the value of the transact file "mark" element
    ; to determine if the dispatch is in a transaction
    ; so we need to store our stack mark also in the
    ; following critical section

    ; save Y + Z pointers
    push r28
    push r29
    push r30
    push r31
    ; get stack pointer
    in r22, r_SPL
    in r23, r_SPH

    ; now we need to increase the active semaphore
    ; and then set our commit target based on the
    ; commits we find. After that, we will be in
    ; the transaction, and can proceed with interrupts
    ; enabled

    ; go critical
    in r0, r_SREG
    cli

    ; increment active semaphore
    ldd r24, Z+4  ; xta.act
    ldd r25, Z+5

    adiw r24, 0x01

    std Z+4, r24  ; xta.act
    std Z+5, r25

    ; Also keep a copy of our semaphore, so that
    ; we know what "position" we are, and can tell
    ; if we are preempting someone who preempted us
    ; (this is checked in XS$)
    std Y+6, r24  ; dispatch_transact.amark;
    std Y+7, r25

    ; grab block commits
    ldd r24, Z+6  ; xta.com
    ldd r25, z+7

    ; save the current stack pointer to
    ; the transaction file - which also signals
    ; XS$ that we are now in a transaction
    ; (this is why we do this while still critical)

    std Y+2, r22  ; dispatch_transact.mark
    out r_SREG, r0
    std Y+3, r23

    ; Our commit target is the state of the transaction block
    ; before we do our commit (now), instead of what we do
    ; to the block. This means we dont need to waste time
    ; incrementing the target. That's all
    std Y+4, r24  ; dispatch_transact.ctarget
    std Y+5, r25

    ; now this is the moment, we've set the semaphore,
    ; and we have our commit target. So we are in.
    ; We will then call into the transaction executor
    ; indicated. They can be sure that the current account
    ; is current, and they are free to write to the
    ; event slip. If the account transaction is interrupted
    ; at any point before we actually do the commit later
    ; XS$ will call tx_rollback, and we won't make it
    ; past the executive call


groundhogday:

    ; load the (*exec) pointer to Z
    ldd r30, Y+0  ; uint16_t (*dispatch_transact.exec)(xdcf *)
    ldd r31, Y+1

    ; set the active case file as the ingoing argument
    movw r24, r20
    
    ; aannnd execute
    icall
    
    ; ok so we've made it this far, but its not for sure
    ; yet, but lets continue and try to do a commit

    ; refresh Y and Z pointers
    pop r31
    pop r30
    pop r29
    pop r28

    ; go critical
    in r0, r_SREG
    cli

    ; we're safe now, no possiblilty of being
    ; rolled back anymore, we are... committed
    push r1
    clr r1

    ; grab the semaphore
    ldd r24, Z+4  ; xta.act
    ldd r25, Z+5

    ; check if it's already zero
    ; (it damned-well shoudln't be)
    ; if it is, we won't let things get too screwy
    ; and will just act "normally". The account might
    ; be messed up, but that is definatly not our fault
    cpse r1, r24
    rjmp 1f
    cpse r1, r25
1:
    sbiw r24, 0x01

    ; write-out
    std Z+4, r24  ; xta.act
    std Z+5, r25

    ; check again if zero, because if it is
    ; then we're closing the block, and we need
    ; to clear the block commits, otherwise
    ; we will increment the block commits

    cpse r1, r24
    rjmp 1f
    cpse r1, r25
    rjmp 1f

    ; it is equal!
    clr r24
    clr r25
    rjmp 2f

1:
    movw r24, r22
    adiw r24, 0x01
2:
    ; write-out
    std Z+6, r24  ; xta.com
    std Z+7, r25

    ; OK!
    ; now we do the actual commit, which involves swapping
    ; the values of the current and event pointers in the
    ; account, thereby confirming the event as a transaction

    ldd r22, Z+0  ; xta.current
    ldd r23, Z+1
    ldd r24, Z+2  ; xta.event
    ldd r25, Z+3

    std Z+0, r24
    std Z+1, r25
    std Z+2, r22
    std Z+3, r23

    ; lastly,
    ; we need to clear the transact file mark element
    ; so that XS$ knows we're out of the transaction
    std Y+2, r1  ; dispatch_transact.mark
    out r_SREG, r0
    std Y+3, r1    
    
    ; congrats ! back to normal
    pop r1

    ; restore the call-saved registers
    pop r31
    pop r30
    pop r29
    pop r28
    pop r17
    pop r16
    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop r7
    pop r6
    pop r5
    pop r4
    pop r3
    pop r2
    pop r0
    
    ret


;
; extern void opsec_tx_rollback
; ( volatile dispatch_transact *, volatile xta *, xdcf * )
;
; The commit failed, so we need to do it again
;
; r25:24  dispatch transact * active dispatch transaction file
; r23:22  xta * registered transaction account in question
; r21:20  xdcf * pointer to active case file
;

;
; Note that, although xs$ calls us (and only xs$), it does so
; AFTER releaseing it's lock. So we are not thread-safe anymore
;

opsec_tx_rollback:

    ; set-up Y pointer to transaction file,
    ; Z with account registration
    movw r28, r24
    movw r30, r22

    ; load SP to start of commit
    ldd r24, Y+2  ; dispatch_transact.mark
    ldd r25, Y+3

    ; load "our" semaphore value
    ldd r22, Y+6  ; dispatch_trasact.amark
    ldd r23, Y+7
    

    ; go critical
    in r0, r_SREG
    cli

    ; reset semaphore
    std Z+4, r22
    std Z+5, r23    

    ; load new commit target
    ldd r22, Z+6  ; xta.com
    ldd r23, Z+7

    ; reset SP
    out r_SPL, r24
    out r_SREG, r0
    out r_SPH, r25

    ; save new commit target
    std Y+4, r22  ; dispatch_transact.ctarget
    std Y+5, r23

    ; memory erased, back in the front entrace
    ; good luck this time
    rjmp groundhogday

    

    





