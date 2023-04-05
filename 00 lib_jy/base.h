#pragma once

// define Func
#define CRASH() do{ *((volatile int*)0) = 0; }while(false)

// Stamp (LockFree)
constexpr ULONG_PTR stampMask = 0xFFFF800000000000; // x64 환경에서 사용하지 않는 주소공간
constexpr ULONG_PTR useBitMask = 0x00007FFFFFFFFFFF; // x64 환경에서 사용하는 주소공간
constexpr BYTE		unusedBit = 17;
constexpr DWORD64	stampCount = 0x800000000000;

// 디버깅 매직넘버
constexpr ULONG_PTR memGuard = 0xFDFDFDFDFDFDFDFD;