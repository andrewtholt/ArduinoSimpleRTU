\ Uses Posic, willneed modified sender/listener

s" POSIX_IPC" environment? 0= abort" POSIX_IPC Not available" drop

load lib.fth
load message.fth

0 value init-run
-1 value mqd
-1 value shm
-1 value ptr

c-message --> new cmd

: .curmsgs  { qid }
    qid MQ_CURMSGS mq-getattr abort" mq-getattr failed."
    
    ." Queue size is " . cr
;

: set-config \ config_byte --
    s" SC" cmd --> set-cmd
    cmd --> set-v_lo
    0 cmd --> set-v_hi
    0 cmd --> set-item
    cmd --> display
    mqd cmd --> get-data 0 mq-send abort" mq-send Failed." 
;

: set-delay \ <delay in 10ms incremenst>
    s" SD" cmd --> set-cmd
    cmd --> set-v_lo
    0 cmd --> set-v_hi
    0 cmd --> set-item
    cmd --> display
    mqd cmd --> get-data 0 mq-send abort" mq-send Failed." 
;

: init
    init-run 0= if
        -1 to init-run

        s" /SIMPLE_RTU" O_WRONLY mq-open abort" mq-open"  to mqd

        s" /RTUState" O_RDWR 0x1b0 shm-open abort" shm-open" to shm

        shm 0 40 mmap abort" mmap failed." to ptr
        0 set-config
    then
;

\ bit shm-ptr -- true|false
: shm-get-pin
    swap 2* + c@ 0<>
;

\ channel shm-ptr -- value
: shm-get-analog
    28 + swap 2* + w@
;

: set-mode \ bit direction
    s" SM" cmd --> set-cmd
    cmd --> set-v_lo
    0 cmd --> set-v_hi
    cmd --> set-item

    cmd --> display

    mqd cmd --> get-data 0 mq-send abort" mq-send Failed." 
;

: set-pin \ bit value
    s" WD" cmd --> set-cmd
    cmd --> set-v_lo
    0 cmd --> set-v_hi
    cmd --> set-item

    cmd --> display
    mqd cmd --> get-data 

    .s 
    0 mq-send abort" mq-send Failed." 
;

: get-analog \ channel
    s" RA" cmd --> set-cmd
    cmd --> set-item
    0 cmd --> set-v_lo
    0 cmd --> set-v_hi

    cmd --> display
    mqd cmd --> get-data 0 mq-send abort" mq-send Failed." 
;

: get-pin \ pin
    s" RD" cmd --> set-cmd
    cmd --> set-item
    0 cmd --> set-v_lo
    0 cmd --> set-v_hi

    cmd --> display
    mqd cmd --> get-data 0 mq-send abort" mq-send Failed." 
;

: main
    init

    13 1 set-mode

    13 1 set-pin

    500 ms
    13 0 set-pin

    1 get-analog

;
seal

: fred
    init
    1 set-config
    13 1 set-mode
    13 0 set-pin
    1 get-analog
    
    begin
        1 ptr shm-get-analog dup 

        dup . cr

        200 > if
            13 0 set-pin
        then
        
        150 < if
            13 1 set-pin
        then
        500 ms
    again
;



