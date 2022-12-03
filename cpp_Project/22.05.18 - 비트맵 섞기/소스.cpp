#include <iostream>
#include <Windows.h>
#include <wingdi.h>
using namespace std;

int main() {
	tagBITMAPFILEHEADER sample1_file_header; //14
	tagBITMAPFILEHEADER sample2_file_header;

	tagBITMAPINFOHEADER	sample1_info_header; //40
	tagBITMAPINFOHEADER	sample2_info_header;

	FILE* sample1_fp;
	FILE* sample2_fp;

	char* sample1_data_buf;
	char* sample2_data_buf;
	char* merge_data_buf;

	fopen_s(&sample1_fp, "sample.bmp", "rb+");
	if (sample1_fp == NULL) { printf("sample1_fp == NULL \n"); return 0; }
	fopen_s(&sample2_fp, "sample2.bmp", "rb+");
	if (sample2_fp == NULL) { printf("sample2_fp == NULL \n"); return 0; }

	// tagBITMAPFILEHEADER / tagBITMAPINFOHEADER ¶â¾î¿À±â
	fread(&sample1_file_header, sizeof(tagBITMAPFILEHEADER), 1, sample1_fp);
	fread(&sample2_file_header, sizeof(tagBITMAPFILEHEADER), 1, sample2_fp);
	fread(&sample1_info_header, sizeof(tagBITMAPINFOHEADER), 1, sample1_fp);
	fread(&sample2_info_header, sizeof(tagBITMAPINFOHEADER), 1, sample2_fp);

	sample1_data_buf = (char*)malloc(sample1_file_header.bfSize - sample1_file_header.bfOffBits);
	if (sample1_data_buf == NULL) { printf("sample1_data_buf == NULL \n"); return 0; }
	sample2_data_buf = (char*)malloc(sample2_file_header.bfSize - sample2_file_header.bfOffBits);
	if (sample2_data_buf == NULL) { printf("sample2_data_buf == NULL \n"); return 0; }
	merge_data_buf = (char*)malloc(sample1_info_header.biSizeImage);
	if (merge_data_buf == NULL) { printf("merge_data_buf == NULL \n"); return 0; }

	fread(sample1_data_buf, sample1_file_header.bfSize - sample1_file_header.bfOffBits, 1, sample1_fp);
	fread(sample2_data_buf, sample2_file_header.bfSize - sample2_file_header.bfOffBits, 1, sample2_fp);

	for (int i = 0; i < sample1_info_header.biSizeImage / sizeof(DWORD); i++) {
		((DWORD*)sample1_data_buf)[i] = (((DWORD*)sample1_data_buf)[i] >> 1) & 0x7F7F7F7F;
		((DWORD*)sample2_data_buf)[i] = (((DWORD*)sample2_data_buf)[i] >> 1) & 0x7F7F7F7F;
		((DWORD*)merge_data_buf)[i] = ((DWORD*)sample1_data_buf)[i] + ((DWORD*)sample2_data_buf)[i];
	}

	char* merge_buf = (char*)malloc(sample1_file_header.bfSize);
	if (merge_buf == NULL) { printf("merge_buf == NULL \n"); return 0; }

	memcpy_s(merge_buf, sample1_file_header.bfSize, &sample1_file_header, sizeof(sample1_file_header));

	memcpy_s(merge_buf + sizeof(sample1_file_header),
		sample1_file_header.bfSize - sizeof(sample1_file_header),
		&sample1_info_header,
		sizeof(sample1_info_header));

	memcpy_s(merge_buf + sample1_file_header.bfOffBits,
		sample1_file_header.bfSize - sample1_file_header.bfOffBits,
		merge_data_buf,
		sample1_info_header.biSizeImage);

	FILE* write_fp;
	fopen_s(&write_fp, "test.bmp", "wb+");
	if (write_fp == NULL) { printf("write_fp == NULL \n"); return 0; }
	fwrite(merge_buf, sample1_file_header.bfSize, 1, write_fp);
}