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
	unsigned int iType = 0x99886655; // 0x99886655 �� ��.
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
		cout << "Packing or UnPacking �� ������ �Է��ϼ��� (�� �Է��ߴٸ� Q) >> ";
		cin >> read_file_name[i];

		if (read_file_name[i] == "Q" || read_file_name[i] == "q") {
			if (i == 0) {
				cout << "���α׷��� ���� �մϴ�." << endl;
				return 1;
			}
			else {
				break;
			}
		}

		read_files_no++;

		// �Է¹��� FILE OPEN
		fopen_s(&read_fp[i], read_file_name[i].c_str(), "rb+");
		if (read_fp[i] == NULL) {
			cout << "�ش������� �������� �ʽ��ϴ�." << endl;
			return 0;
		}

		// FILE SIZE ���ϱ�
		fseek(read_fp[i], 0, SEEK_END);
		read_file_size[i] = ftell(read_fp[i]);
		fseek(read_fp[i], 0, SEEK_SET);

		// TOTAL DATA SIZE ���ϱ�
		total_data_size += read_file_size[i];

		// FILE SIZE BUF �����Ҵ�
		read_file_buf[i] = (char*)malloc(read_file_size[i]);
		if (read_file_buf[i] == NULL) {
			cout << "read_file_buf[i] == NULL" << endl;
			return 0;
		}

		// ���� ������ ��������
		fread(read_file_buf[i], read_file_size[i], 1, read_fp[i]);

		// UNPACKING FILE ó��
		if (*(int*)read_file_buf[i] == 0x99886655) {
			char* packing_buf = read_file_buf[i];

			// PACK_HEADER ���
			PACK_HEADER *pHeader = (PACK_HEADER*)packing_buf;
			const int file_no = pHeader->iFileNum;

			// PACK_FILEINFO ���
			PACK_FILEINFO* pInfo = (PACK_FILEINFO*)malloc(sizeof(PACK_FILEINFO) * file_no);
			for (int i = 0; i < file_no; i++) {
				pInfo[i].iFileSize = ( * (PACK_FILEINFO*)(packing_buf + sizeof(PACK_HEADER) + sizeof(PACK_FILEINFO) * i)).iFileSize;
				strcpy_s(pInfo[i].szFileName, (*(PACK_FILEINFO*)(packing_buf + sizeof(PACK_HEADER) + sizeof(PACK_FILEINFO) * i)).szFileName);
				cout << "pInfo[i].szFileName : " << pInfo[i].szFileName << endl;
				cout << "pInfo[i].iFileSize : " << pInfo[i].iFileSize << endl;
			}

			// �� FILE Data start ���� ���ϱ�
			int data_start_point = sizeof(PACK_HEADER) + sizeof(PACK_FILEINFO) * file_no;

			for (int i = 0; i < file_no; i++) {
				FILE* write_fp;
				fopen_s(&write_fp, pInfo[i].szFileName, "wb+");
				fwrite(packing_buf + data_start_point, pInfo[i].iFileSize, 1, write_fp);
				fclose(write_fp);

				data_start_point += pInfo[i].iFileSize;
			}

			cout << "UNPACKINGING �Ϸ�" << endl;
			return 1;
		}

		// ����� �ڵ�
		cout << "file_name : " << read_file_name[i] << endl;
		cout << "read_file_size : " << read_file_size[i] << endl << endl;
	}

	//PACKING FILE SIZE ���ϱ� + PACKING BUF �Ҵ�
	packing_buf_size += total_data_size;
	packing_buf_size += sizeof(PACK_HEADER) + sizeof(PACK_FILEINFO) * read_files_no;
	packing_buf = (char*)malloc(packing_buf_size);

	// PACKING FILE HEADER ��ü�� ���� �� �ʱ�ȭ
	PACK_HEADER header(read_files_no);
	PACK_FILEINFO* file_info = (PACK_FILEINFO*)malloc(sizeof(PACK_FILEINFO) * read_files_no);

	for (int i = 0; i < read_files_no; i++) {
		strcpy_s(file_info[i].szFileName, 128, read_file_name[i].c_str());
		file_info[i].iFileSize = read_file_size[i];

		cout << "file_name : " << file_info[i].szFileName << endl;
		cout << "read_file_size : " << file_info[i].iFileSize << endl << endl;
	}

	// PACKING BUF�� ������ �ܾ����
	unsigned int offset = 0;
	memcpy(packing_buf, &header, sizeof(PACK_HEADER));
	offset += sizeof(PACK_HEADER);
	memcpy(packing_buf + offset, file_info, sizeof(PACK_FILEINFO) * read_files_no);
	offset += sizeof(PACK_FILEINFO) * read_files_no;

	for (int i = 0; i < read_files_no; i++) {
		memcpy(packing_buf + offset, read_file_buf[i], file_info[i].iFileSize);
		offset += file_info[i].iFileSize;
	}

	// UNPACKING FILE �����
	FILE* packing_fp;
	fopen_s(&packing_fp, "PACKING", "wb+");
	fwrite(packing_buf, packing_buf_size, 1, packing_fp);
	return 1;
}