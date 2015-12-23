
s" iniparser" environment? 0= abort" No ini file parser."

-1 value buffer
0 value initRun

255 constant /buffer

-1 value ini

: init 
    initRun  0= if
        s" ./rtu.ini" ini-load to ini
        /buffer allocate abort" allocate" to buffer

        buffer /buffer erase
        -1 to initRun
    then
;

: mk-sql ( ss: section )
    sdup s" :pin" spush s+ 
    buffer /buffer spop

;

: test 
    init
    cr
    ." Number of [sections]:"
    ini #sections . cr

    ini #sections 0 do
        i . ." :"
        ini i ini-get-section-name spush stype
        cr
    loop
;


