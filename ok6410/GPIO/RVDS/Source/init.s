;/*************************************************************************************
; 
;	Project Name : OK6410 GPIO
;
;	Copyright 2015 by opentechshare.
;	All rights reserved.
;
;*************************************************************************************/

	IMPORT	Main
    
	AREA |C$$code|, CODE, READONLY
	global	start

start
    bl Main
    
    END
