#pragma once

// define Func
#define CRASH() do{ *((volatile int*)0) = 0; }while(false)

// Stamp (LockFree)
constexpr ULONG_PTR stampMask = 0xFFFF800000000000; // x64 ȯ�濡�� ������� �ʴ� �ּҰ���
constexpr ULONG_PTR useBitMask = 0x00007FFFFFFFFFFF; // x64 ȯ�濡�� ����ϴ� �ּҰ���
constexpr BYTE		unusedBit = 17;
constexpr DWORD64	stampCount = 0x800000000000;

// ����� �����ѹ�
constexpr ULONG_PTR memGuard = 0xFDFDFDFDFDFDFDFD;