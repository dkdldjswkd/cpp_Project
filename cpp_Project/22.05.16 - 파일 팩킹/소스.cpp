#include <iostream>
#include <string>
using namespace std;

#define MAX_FILE_NO 10

#pragma pack(push, 1)
class PACK_HEADER // 8
{
public:
	PACK_HEADER(int n) :iFileNum(n) {}
public:
	unsigned int iType = 0x99886655; // 0x99886655 이 들어감.
	int iFileNum;
};

class PACK_FILEINFO // 5
{
public:
	int iFileSize;
	char szFileName[128];
};
#pragma pack(pop)

int main() {
	FILE*	read_fp[MAX_FILE_NO];
	string	read_file_name[MAX_FILE_NO];
	char*	read_file_buf[MAX_FILE_NO];
	int		read_file_size[MAX_FILE_NO];
	int		read_files_no = 0;

	FILE*	unpacked_fp[MAX_FILE_NO];

	unsigned int total_data_size = 0;

	char* packing_buf;
	unsigned int packing_buf_size = 0;

	for (int i = 0;; i++) {
		cout << "Packing or UnPacking 할 파일을 입력하세요 (다 입력했다면 Q) >> ";
		cin >> read_file_name[i];

		if (read_file_name[i] == "Q" || read_file_name[i] == "q") {
			if (i == 0) {
				cout << "프로그램을 종료 합니다." << endl;
				return 1;
			}
			else {
				break;
			}
		}

		read_files_no++;

		// 입력받은 FILE OPEN
		fopen_s(&read_fp[i], read_file_name[i].c_str(), "rb+");
		if (read_fp[i] == NULL) {
			cout << "해당파일이 존재하지 않습니다." << endl;
			return 0;
		}

		// FILE SIZE 구하기
		fseek(read_fp[i], 0, SEEK_END);
		read_file_size[i] = ftell(read_fp[i]);
		fseek(read_fp[i], 0, SEEK_SET);

		// TOTAL DATA SIZE 구하기
		total_data_size += read_file_size[i];

		// FILE SIZE BUF 동적할당
		read_file_buf[i] = (char*)malloc(read_file_size[i]);
		if (read_file_buf[i] == NULL) {
			cout << "read_file_buf[i] == NULL" << endl;
			return 0;
		}

		// 파일 데이터 가져오기
		fread(read_file_buf[i], read_file_size[i], 1, read_fp[i]);

		// UNPACKING FILE 처리
		if (*(int*)read_file_buf[i] == 0x99886655) {
			char* packing_buf = read_file_buf[i];

			// PACK_HEADER 뜯기
			PACK_HEADER *pHeader = (PACK_HEADER*)packing_buf;
			const int file_no = pHeader->iFileNum;

			// PACK_FILEINFO 뜯기
			PACK_FILEINFO* pInfo = (PACK_FILEINFO*)malloc(sizeof(PACK_FILEINFO) * file_no);
			for (int i = 0; i < file_no; i++) {
				pInfo[i].iFileSize = ( * (PACK_FILEINFO*)(packing_buf + sizeof(PACK_HEADER) + sizeof(PACK_FILEINFO) * i)).iFileSize;
				strcpy_s(pInfo[i].szFileName, (*(PACK_FILEINFO*)(packing_buf + sizeof(PACK_HEADER) + sizeof(PACK_FILEINFO) * i)).szFileName);
				cout << "pInfo[i].szFileName : " << pInfo[i].szFileName << endl;
				cout << "pInfo[i].iFileSize : " << pInfo[i].iFileSize << endl;
			}

			// 각 FILE Data start 지점 구하기
			int data_start_point = sizeof(PACK_HEADER) + sizeof(PACK_FILEINFO) * file_no;

			for (int i = 0; i < file_no; i++) {
				FILE* write_fp;
				fopen_s(&write_fp, pInfo[i].szFileName, "wb+");
				fwrite(packing_buf + data_start_point, pInfo[i].iFileSize, 1, write_fp);
				fclose(write_fp);

				data_start_point += pInfo[i].iFileSize;
			}

			cout << "UNPACKINGING 완료" << endl;
			return 1;
		}

		// 디버그 코드
		cout << "file_name : " << read_file_name[i] << endl;
		cout << "read_file_size : " << read_file_size[i] << endl << endl;
	}

	//PACKING FILE SIZE 구하기 + PACKING BUF 할당
	packing_buf_size += total_data_size;
	packing_buf_size += sizeof(PACK_HEADER) + sizeof(PACK_FILEINFO) * read_files_no;
	packing_buf = (char*)malloc(packing_buf_size);

	// PACKING FILE HEADER 객체들 선언 및 초기화
	PACK_HEADER header(read_files_no);
	PACK_FILEINFO* file_info = (PACK_FILEINFO*)malloc(sizeof(PACK_FILEINFO) * read_files_no);

	for (int i = 0; i < read_files_no; i++) {
		strcpy_s(file_info[i].szFileName, 128, read_file_name[i].c_str());
		file_info[i].iFileSize = read_file_size[i];

		cout << "file_name : " << file_info[i].szFileName << endl;
		cout << "read_file_size : " << file_info[i].iFileSize << endl << endl;
	}

	// PACKING BUF에 데이터 긁어오기
	unsigned int offset = 0;
	memcpy(packing_buf, &header, sizeof(PACK_HEADER));
	offset += sizeof(PACK_HEADER);
	memcpy(packing_buf + offset, file_info, sizeof(PACK_FILEINFO) * read_files_no);
	offset += sizeof(PACK_FILEINFO) * read_files_no;

	for (int i = 0; i < read_files_no; i++) {
		memcpy(packing_buf + offset, read_file_buf[i], file_info[i].iFileSize);
		offset += file_info[i].iFileSize;
	}

	// UNPACKING FILE 만들기
	FILE* packing_fp;
	fopen_s(&packing_fp, "PACKING", "wb+");
	fwrite(packing_buf, packing_buf_size, 1, packing_fp);
	return 1;
}