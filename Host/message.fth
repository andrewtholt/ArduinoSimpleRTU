
only forth also oop definitions


object subclass c-message
    c-byte obj: .address
    c-2byte obj: .cmd
    c-byte obj: .item
    c-byte obj: .v_lo
    c-byte obj: .v_hi
    c-byte obj: .LAST

    : init { 2:this }
        0 this --> .address --> set
    ;

	: display { 2:this }
		this --> print-address
		this --> print-cmd
		this --> print-item
		this --> print-v_lo
		this --> print-v_hi
	;

	: get-address ( 2:this )
		--> .address --> get
	;

	: set-address ( n 2:this )
		--> .address --> set
	;

	: print-address ( 2:this )
		." Address" 9 emit [char] : emit
		--> get-v_lo . cr
	;

	: get-last ( 2:this -- addr )
		--> .LAST drop
	;

    : get-size { 2:this -- n }
        this --> get-last
        this drop -
    ;

    : get-data { 2:this -- addr n }
        this drop
        this --> get-size
    ;

	: get-v_lo ( 2:this )
		--> .v_lo --> get
	;

	: set-v_lo ( n 2:this )
		--> .v_lo --> set
	;

	: print-v_lo ( 2:this )
		." v_lo" 9 emit [char] : emit
		--> get-v_lo . cr
	;

	: get-v_hi ( 2:this )
		--> .v_hi --> get
	;

	: set-v_hi ( n 2:this )
		--> .v_hi --> set
	;
	: print-v_hi ( 2:this )
		." v_hi" 9 emit [char] : emit
		--> get-v_hi . cr
	;

	: get-cmd ( 2:this )
		--> .cmd drop
        2
	;

	: set-cmd { ptr n 2:this }
        ptr c@ this --> .cmd drop c!
        ptr 1+ c@ this --> .cmd drop 1+ c!
	;
	: print-cmd ( 2:this )
		." cmd" 9 emit [char] : emit
		--> get-cmd type cr
	;

	: get-item ( 2:this )
		--> .item --> get
	;

	: set-item ( n 2:this )
		--> .item --> set
	;
	: print-item ( 2:this )
		." item" 9 emit [char] : emit
		--> get-item . cr
	;

	: init { 2:this }
		this drop 0 erase
	;
end-class

