[BITS 16]
%define ROOT_SEG 0x60
%define FAT_SEG 0x800
%define IMG_SEG 0x1000
%define first_data_sector 0x7c00+0800h
%define next_seg 0x7c00+0208h

%define MaxRootEntries 0x7c00+11h ;this is part of the fat system 
%define sc_p_clu 0x7c00+0Dh ;byte Sectors per cluster

%define sc_b4_fat 0x7c00+0Eh ;word Sectors (in partition) before FAT
%define fats 0x7c00+10h ;byte Number of FATs
%define dir_ent 0x7c00+11h ;word Number of root directory entries
%define sc_p_fat 0x7c00+16h ;word Sectors per FAT
%define sc_p_trk 0x7c00+18h ;word Sectors per track
%define heads 0x7c00+1Ah ;word Number of heads
%define sc_b4_prt 0x7c00+1Ch ;dword Sectors before partition
%define drive 0x7c00+24h ;byte Drive number set by bios

org 0x7e00
;
;
; Second Segment
;
; This portion of the boot sector is split and added in during the first disk read. I order to deal with
; a kernel larger the 64k the file is read in to memory and the segment register is adjusted instead of the
; memory offset.
 
    xor ax, ax ; to clear the ax register (the accepted way to clear the register)
    mov ds, ax ; to clear ds register (done for speed)
    mov ss, ax ; to clear ss register (done for speed)
    mov dx, ax ;
    mov sp, 0x7e00
    mov bp, sp 
    mov sp, ax
    mov es, ax
    ;mov ax, bp
    ;call print_reg_value
    ;xor ax,ax
 

read_part_2:
; in order to read the fat file system, portions of the inforation is contained ; in the boot sector
    
    mov [drive], dl ;Drive number
    ;not really important
    mov al, [fats] ;Number of FATs
    ; al = [fats] (my case 2)
    mul word [sc_p_fat] ; * Sectors per FAT
    ; al = [fats] * [sc_p_fat] (my case 18)
    add ax, [sc_b4_fat] ; + Sectors before FAT
    ; ax = [fats] * [sc_p_fat] + [sc_b4_fat] (my case 19)
    ; in my case this should be 19
    ;AX = logical Sector of Root directory
    push ax ;preserving ax
 
;int compute_max_root(){
;
; int *maxroot, size_of_ent_bytes, size_of_sec_bytes, output;
;
; maxroot=short int(*)"0x7C11";
; size_of_ent_bytes=32;
; size_of_sec_bytes=512;
;
; output=(*maxroot * size_of_ent_bytes)/size_of_sec_bytes;
; 
; return(output);
;}
 ;; call print_reg_value 
compute_max_root_2:
    mov ax, [MaxRootEntries] 
    ;setting ax to mac root entry
    shl ax, 5 ;seting bx to 32 size of each files entry in the root directory
    shr ax, 9 ;deviding by 512
    mov cx, ax
    pop ax
    call print_reg_value
    mov [first_data_sector], ax
    add [first_data_sector], cx
    mov bx, 0x0800
 
.1: 
    push cx

    call read_disk;
    
    pop cx
    inc ax
    add bx, 0x200
    loop .1 ; looped to read cx sectors into [0000:0800]
    push ax
    mov ax, WORD [MaxRootEntries]
    call print_reg_value
    pop ax
    mov cx, WORD [MaxRootEntries]
    ; cx set for loop to compare filenames
    mov di, 0x0800 ; segment where disk was read to, needed for lodsb
    
    
.loop: 
    push cx ; protecting CX from rep statement 
    mov cx, 0x000B ; length of filename
    mov si, filename2 ; si = filename to compare to
    push di ; preserve di from increment cmpsb
    rep cmpsb
    je goody_2 ; jmp out of loop 
    pop di ; get DI prior to increment
    pop cx ; get previous CX for loop
    add di, 0x0020 ; length of entry
    loop .loop ; do CX times
    
    jmp reboot 

goody_2:

    xor ax, ax
    mov si, di ; set si = di
    add si, 0Fh ; skip data not needed, to get to fat sector
    lodsw ; load 16bit value in AX, ax=fat cluster
    push ax ; save AX from computation
    mov ax, [sc_b4_fat] ; get sector to start reading at
    mov cx, [sc_p_fat] ; set loop to number of sectors in fat
    mov bx, 0x800 ; set memory segment
 .1:
    push cx ; save cx
    call read_disk 
    pop cx ; return cx
    inc ax ; adjust ax, and read again
    add bx, 0x0200 ; adjust bx for sector
    loop .1 ; loop for number of sectors
    pop ax ; get ax
    ;call print_reg_value
    push ax ; save fat cluster
    xor bx, bx ; clear BX for initial read of kernel
    
    add bx, 0x1000
    mov es, bx
    xor bx, bx
    call disk_to_mem_2 
    pop ax
    push es
    mov es,bx
    ; get fat cluster for further information
    pop es
    
next_cluster_in_chain_2:

.1: 
    mov si, 0x800 ; 
    mov dx, ax
    mov cx, ax ; set cx = ax
    shr cx, 1 ; devide by 2
    add cx, ax ; add ax and cx which is file_sys_cluster * 1.5 = fat bytes
    add si, cx
    lodsw ;16 bit word into ax
    ;call print_reg_value
    test dx, 01h
    jz .2
    shr ax,4
    jmp .3
.2:
    and ax,0xFFF
.3: 
    cmp ax, 0xFF8
    jae next_part_2
    push ax
    push dx
    call disk_to_mem_2
    pop dx
    pop ax
    ;jmp next_part_2
    jmp .1

disk_to_mem_2: 
    ;Rewritten to use ES as counter instead of BX
    call cluster_to_sector_2
    push cx
    xor bx,bx
    call read_disk
    mov bx, es
    add bx, 0x020
    mov es, bx
    xor bx,bx
    pop cx
    ret
 
cluster_to_sector_2:
    sub ax, 03h
    add ax, [first_data_sector]
    inc ax
    ret
 
 
next_part_2:
    mov bx, es
    mov dx,0x1000
    mov ds, dx
    mov si, 0x0000
    mov cx, 0x0004
.1: 
    lodsb
    push ds
    xor dx, dx
    mov ds, dx
    push es
    mov es, dx
    ;call print_reg_value
    pop es
    pop ds
    loop .1
 
    mov bx, es
    xor ax, ax
    mov es, ax
    mov ds, ax
    sub bx, 0xa000 ; bx contains the number of 16b segments the kernel is contained in
    mov ecx, 0x0010
    xor eax, eax ;maxing sure theres not garbage in the high register
    mov ax, bx ;ax =number of segments
    mul ecx ;edx:eax = eax*ecx, length in bytes of sectors that comprise kernel
    push eax ;save eax for prosperity(I don't think that ecx*eax is large engouh to spill into edx)
    mov eax, 0xa000 ;number of segments before kernel
    mul ecx ;edx:eax = eax*ecx, physical location of first segment
    mov edx, eax 
    pop ebx ;end of segment
    xor ecx,ecx
    mov ds, cx
    mov es, cx
    
    push eax
    cli
    in al, 0x92
    mov al,0x2
    out 0x92, al
    sti
    push edx
 
int_15_call_setup:
    xor ax,ax
    mov es, ax
    mov eax, 0x11000
    call print_reg_value
    mov [lowword], ax
    shr eax, 16
    call print_reg_value
    mov [highword], al
    
    mov bx, 0x1000
    push ds
    mov ds, bx
    mov eax, dword[0x10044]
    call print_reg_value
    mov ecx, 0x2
    
    call print_reg_value
    mov cx, ax
    xor eax,eax
    pop ds
    
    mov ah, 0x87
    mov si, int_15_gdt
    int 0x15
    
    mov bx, 0x1000
    push ds
    mov ds, bx
    mov eax, [0x10044]
    pop ds
    mov ecx, eax
    mov ebx, 0x11000
    add eax, ebx
    mov [lowword], ax
    shr eax, 16
    mov [highword], al
    mov ah, 0x87
    mov [dlowword], cx
    mov si, int_15_gdt
    int 0x15
    
    mov bx, 0x1000
    push ds
    mov ds, bx
    xor eax, eax
    call print_reg_value
    mov eax, dword[0x10058]
    call print_reg_value
    mov ebx, 0x10000
    
    add eax, ebx
    pop ds
    mov [lowword], ax
    call print_reg_value
    mov bx, 0x1000
    push ds
    mov ds, bx
    mov eax, [0x1005c]
    call print_reg_value
    pop ds
    mov [dlowword], ax
    mov bx, 0x1000
    push ds
    mov ds, bx
    mov eax, [0x10064]
    call print_reg_value
    pop ds
    mov ecx, 0x2
    mul ecx
    call print_reg_value
    mov cx, ax
    
    xor eax, eax
    mov ah, 0x87
    mov si, int_15_gdt
    int 0x15
    
    
    call print_reg_value
    
    xor dx,dx
    mov es, dx
    mov si, dx
    mov es, dx 
    
 
 
probe_hw:

base_of_screen:
    xor ax,ax
    mov ds, ax
    mov es, ax
    mov di, 0x800
    mov ah, 0x0f
    int 0x10
    cmp ax, 0x07
    jne vga
herc:
    mov eax, 0xb0000
    
    jmp store_card 

vga:
    mov eax, 0xb8000

store_card:

    stosd ;0x100 1

main_mem: 

    mov eax, 0x280 ; 640k 
    stosd ;0x104 2
 
determine_mem:

    xor eax,eax
    mov ah,0x88
    int 0x15
    stosd ;0x108 3

size_of_code_segment:
    push ds
    mov bx, 0x1000
    mov ds, bx
    mov eax, [0x0044]
    pop ds
    stosd ;0x10c 4
 
size_of_data_segment:
    push ds
    mov bx, 0x1000
    mov ds, bx
    mov eax, [0x0064]
    pop ds
    stosd ; 0x110 5
 
size_of_bss_seg:
    push ds
    mov bx, 0x1000
    mov ds, bx
    mov eax, [0x0068]
    pop ds
    stosd ; 0x114 6
    
size_of_debug:
    xor eax, eax
    stosd ; 0x118 7
    
size_of_fs_cache:
    xor eax,eax
    stosd ; 0x11c 8
    
screen_mode:
    mov ax, 0xf00
    int 0x10
    and eax, 0xff
    stosd ; 0x120 9
    call print_reg_value

video_type:
    xor ebx,ebx
    mov ax, 0x1a00
    int 0x10
    ;call print_reg_value
    cmp al, 0x1a
    ;jne reboot
    and bx, 0x00ff
    mov eax, ebx
    stosd ; 0x124 10
 
clock:
    xor eax, eax
    int 0x1a;
    mov ax, cx
    shl eax, 16
    mov ax, dx
    stosd ; 0x128 11


    cli
    call enablea20
    mov eax, dword[0x800]
    mov [0x100], eax
    mov eax, dword[0x804]
    mov [0x104], eax
    mov eax, dword[0x808]
    mov [0x108], eax
    mov eax, dword[0x80c]
    mov [0x10c], eax 
    mov eax, dword[0x810]
    mov [0x110], eax
    mov eax, dword[0x814]
    mov [0x114], eax
    mov eax, dword[0x818]
    mov [0x118], eax
    mov eax, dword[0x81c]
    mov [0x11c], eax
    mov eax, dword[0x820]
    mov [0x120], eax
    mov eax, dword[0x824]
    mov [0x124], eax
    mov eax, dword[0x828]
    mov [0x128], eax
    lgdt[gdt_ptr]
    smsw ax
    or ax,1
    lmsw ax
    
    jmp flush
flush:
    db 066h,0eah
    dw 0,0
    dw 8
    

align 8, db 0

int_15_gdt:
    ; Null segment 
    
    dw 0
    dw 0
    dw 0
    dw 0
    
    ; Initialized By BIOS
    
    dw 0
    dw 0
    dw 0
    dw 0
    
    ; Descriptors 3: source
    dw 0xffff ; length 
    lowword: 
    dw 0x0000 ; 16 bit low portion of 24bit real address
    highword: 
    db 0x01 ; 8 bit high portion of 24bit real address
    db 0x92 ; protection
    db 0x00
    db 0x00
    
    ; Descriptor 4: dest
    dw 0xffff ; segment length
    dlowword:
    dw 0x0000 ; 16bit low portion
    db 0x10 ; 8 bit high portion of 24bit real address
    db 0x92 
    db 0x00
    db 0x00
    
    ; Initalized by bios
    
    dw 0
    dw 0
    dw 0
    dw 0
    
    ; initialized by bios
    
    dw 0
    dw 0
    dw 0
    dw 0
    
    align 8, db 0
 
real_gdt: 
    ;; First 8 bytes are all zero
    ;; null segment descriptor
    db 0x00
    db 0x00
    db 0x00
    db 0x00
    db 0x00
    db 0x00
    db 0x00
    db 0x00
    ;; OS text descriptor
    db 0xff
    db 0xff
    db 0x00
    db 0x00
    db 0x10
    db 0x9a
    db 0xcf
    db 0x00
    ;; OS Data Descriptor
    db 0xff
    db 0xff
    db 0x00
    db 0x00
    db 0x10
    db 0x92
    db 0xcf
    db 0x00
    ;; Global data descriptor
    db 0xff
    db 0xff
    db 0x00
    db 0x00
    db 0x00
    db 0x92
    db 0xcf
    db 0x00
 
gdt_ptr:
    dw 0x001f
    dw real_gdt
    dw 0x0000
    
enablea20:
    call empty_8042
    mov al,0xd1 ; command write
    out 0x64,al
    call empty_8042
    mov al,0xdf ; A20 on
    out 0x60,al
    call empty_8042
    ret
    
empty_8042:
    in al,0x64
    test al,2
    jnz empty_8042
    ret
    
%include "utils.s"
filename2 db "KERNEL IMG"
