#pragma once

// define Func
#define CRASH() do{ *((volatile int*)0) = 0; }while(false)

// Stamp (LockFree)
constexpr ULONG_PTR stampMask  = 0xFFFF800000000000; // x64 ȯ�濡�� ������� �ʴ� �ּҰ���
constexpr ULONG_PTR useBitMask = 0x00007FFFFFFFFFFF; // x64 ȯ�濡�� ����ϴ� �ּҰ���
constexpr DWORD64	stampCount = 0x0000800000000000; // ������� �ʴ� �ּҰ����� �ּҴ���
constexpr BYTE		unusedBit = 17;

// ����� �����ѹ�
constexpr ULONG_PTR memGuard = 0xFDFDFDFDFDFDFDFD;