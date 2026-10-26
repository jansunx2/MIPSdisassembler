# Reconstructed FizzBuzz MIPS program
.text
.globl main
main:
    addi $t0, $zero, 1
L_00400004:
    slti $t1, $t0, 101
    beq $t1, $zero, L_004000A0
    addi $t1, $zero, 15
    div $t0, $t1
    mfhi $t2
    beq $t2, $zero, L_0040004C
    addi $t1, $zero, 3
    div $t0, $t1
    mfhi $t2
    beq $t2, $zero, L_00400060
    addi $t1, $zero, 5
    div $t0, $t1
    mfhi $t2
    beq $t2, $zero, L_00400074
    add $a0, $t0, $zero
    addi $v0, $zero, 1
    syscall
    j L_00400088
L_0040004C:
    addi $v0, $zero, 4
    lui $a0, 4097
    ori $a0, $a0, 12
    syscall
    j L_00400098
L_00400060:
    addi $v0, $zero, 4
    lui $a0, 4097
    ori $a0, $a0, 0
    syscall
    j L_00400098
L_00400074:
    addi $v0, $zero, 4
    lui $a0, 4097
    ori $a0, $a0, 6
    syscall
    j L_00400098
L_00400088:
    addi $v0, $zero, 4
    lui $a0, 4097
    ori $a0, $a0, 22
    syscall
L_00400098:
    addi $t0, $t0, 1
    j L_00400004
L_004000A0:
    addi $v0, $zero, 10
    syscall
