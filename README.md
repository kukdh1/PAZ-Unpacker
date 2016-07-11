# PAZ Unpacker
Blackdesert paz file unpacker

## Description
- Blackdesert paz file unpacker
- Tested on KR client
- The meta file format is changed on KR client since 2016.05. (Decrypt key changed)
- This program reads all paz file to construct filesystem

## MEMO
- If extracted file begins with 0x6E, 9 bytes means:
```
0x6E: Header
DWORD: Original file size
DWORD: This file size
```
- Example
```
Original file (real file data size: 0109814A)
00000000: 50 4B 03 04 14 00 06 00 08 00 00 00 21 00 B1 5E
....
0109813A: 00 00 5E 01 5E 01 21 5D 00 00 13 24 09 01 00 00
```
```
Damaged (9 bytes are inserted, last 9bytes disappeared, real file data size: 01098153)
00000000: 6E 53 81 09 01 4A 81 09 01 50 4B 03 04 14 00 06
....
0109813A: 62 69 6E 50 4B 05 06 00 00 00 00 5E 01 5E 01 21
```
