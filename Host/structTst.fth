load struct.fth
struct
10 chars field first-name
20 chars field  surname
endstruct /name
\ 
/name allocate abort" Allocation Failed" value name-buffer

\
name-buffer /name erase
\ 
\ name-buffer surname
\

