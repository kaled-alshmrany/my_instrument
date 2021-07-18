
#ifndef INPUTTYPES_H
#define INPUTTYPES_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
	_char = 1,
	_uchar = 2,
	_short = 3,
	_ushort = 4,
	_int = 5,
	_uint = 6,
	_long = 7,
	_ulong = 8, 
	_longlong = 9,
	_ulonglong = 10,
	_float = 11,
	_double = 12 ,
	_bool = 13,
	_string = 14	
} fuSeBMC_InputType_t;


#ifdef __cplusplus
}
#endif
#endif /* INPUTTYPES_H */

