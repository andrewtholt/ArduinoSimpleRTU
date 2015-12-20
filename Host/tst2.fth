load lib.fth
load struct.fth
\ load mydump.fth

\ struct message {
\    uint8_t address;  // defaults to 0
\    uint8_t cmd[2];
\    uint8_t item;
\    uint8_t v_lo;
\    uint8_t v_hi;
\ };

42 constant MSG_KEY
struct 
1 chars field address
2 chars field cmd
1 chars field item
1 chars field v_lo
1 chars field v_hi
endstruct /cmd

-1 value command
-1 value qid
-1 value ptr
-1 value shm
-1 value shmSem
-1 value startSem

0 value initRun

: .semvalue ( semid -- )
    sem-getvalue abort" sem-getvalue failed."
    ." sem-getvalue : " . cr
;

: init
    initRun 0= if
        -1 to initRun
        /cmd allocate abort" record alloc" to command
        MSG_KEY openqueue abort" openqueue" to qid

        s" /RTUState" O_RDWR 0x1b0 shm-open abort" shm-open" to shm
        shm 0 40 mmap abort" mmap failed." to ptr

        s" SHM_SEM" O_RDWR sem-open abort" SHM_SEM" to shmSem
        s" START_SEM" O_RDWR sem-open abort" START_SEM" to startSem
    then
;

: set-mode  \ bit dir --
    command v_lo c!   \ direction
    command item c!   \ bit
    
    s" SM" command cmd swap move
    0 command address c!
    0 command v_hi c!
    command /cmd  qid msg-send abort" msg-send"
;

: set-config \ config_byte --
    command v_lo c!   \ direction
    0 command item c!   \ bit
    
    s" SC" command cmd swap move
    0 command address c!
    0 command v_hi c!
    command /cmd  qid msg-send abort" msg-send"
;

: set-delay
    command v_lo c!   \ direction
    0 command item c!   \ bit
    
    s" SD" command cmd swap move
    0 command address c!
    0 command v_hi c!
    command /cmd  qid msg-send abort" msg-send"
;

\ 255 allocate abort" allocate" to ptr

: set-pin \ bit value
    command v_lo c!
    command item c!

    s" WD" command cmd swap move
    0 command address c!
    0 command v_hi c!

    command /cmd  qid msg-send abort" msg-send"
;

: get-analog \ channel
    command item c!
    0 command v_lo c!
    0 command v_hi c!

    s" RA" command cmd swap move
    0 command address c!

    command /cmd  qid msg-send abort" msg-send"
;

: get-pin \ pin --
    command item c!
    0 command v_lo c!
    0 command v_hi c!

    s" RD" command cmd swap move
    0 command address c!

    command /cmd  qid msg-send abort" msg-send"
;

: main
    init
    command 16 dump
;

