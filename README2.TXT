FWIW, these were the steps of the RE-Linuxization of dcaenc:

1) internal renaming from dcaenc to ffdcaenc

2) add _T(__GNUC_PATCHLEVEL__) to compiler_info.h

3) drop the Windows-specific Unicode support

4) replace all the occurrences of __int8 and __int32 with int8_t and int32_t , plus

5) replace "unsigned __int8" and "unsigned __int32" with "uint8_t" and "uint32_t"

6) remove the MSVC stuff from (ff)dcaenc.h

7) replace %d and %02d with %u and %02u in the file main.c, in order to eliminate the warning messages
