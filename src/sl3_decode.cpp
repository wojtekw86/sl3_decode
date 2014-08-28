//============================================================================
// Name        : sl3_decode.cpp
// Author      : Wojciech Wróblewski
// Version     :
// Copyright   : Copyright 2014
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/sha.h>
#include <ctime>

unsigned char _master[11];
unsigned char _random[4];
unsigned char imei[7] = {0x35, 0x16, 0x58, 0x05, 0x17, 0x63, 0x82}; // IMEI: 351658051763829
unsigned char code[24];
unsigned long perm_count = 0;
unsigned long perm_count_1000000 = 0;
unsigned char SHA1_CODE[] = {0x73, 0x34, 0xCE, 0x79, 0xE0, 0x30, 0x17, 0x02, 0xDF, 0x2C, 0x95, 0x72, 0x0A, 0x78, 0x51, 0x69, 0x8B, 0xF6, 0x83, 0x9A};

unsigned char temp[SHA_DIGEST_LENGTH];
char buf[SHA_DIGEST_LENGTH*2];
char buf2[48];

clock_t part_start_time;

void bf(int);

int main(int argn, char *argv[]) {

//    if ( argn != 2 ) {
//        printf("Usage: %s string\n", argv[0]);
//        return -1;
//    }

	memset(buf, 0, SHA_DIGEST_LENGTH*2);
	memset(temp, 0, SHA_DIGEST_LENGTH);

	memset(_master, 0, 11);
	memset(_random, 0, 4);

	memcpy(code, (const void*)_master, 11);
	memcpy(code+11, (const void*)_random, 4);
	memcpy(code+11+4+1, (const void*)imei, 7);

	part_start_time = clock();
	bf(0);

	return 0;
}

void bf(int digit) {
	for(int v=0; v<10; v++) {
		perm_count++;
		code[digit] = v;
		if((digit+1) < 14){
			bf(digit+1);
		} else {
			SHA1((unsigned char *)code, 24, temp);

			if(perm_count % 1000000 == 0) {
				perm_count = 0;
				perm_count_1000000++;
				if(perm_count_1000000 % 10 == 0) {
					unsigned long time_ms = (clock()-part_start_time)*1000/CLOCKS_PER_SEC;
					unsigned long long time_secs = ((time_ms*(1000000000000000-(perm_count_1000000*1000000))/1000000)/1000);
					unsigned long long time_mins = time_secs / 60;
					time_secs = time_secs % 60;
					unsigned long long time_hours = time_mins / 60;
					time_mins = time_mins % 60;
					unsigned long long time_days = time_hours / 24;
					time_hours = time_hours % 24;

					printf("DONE: %luM [ %lf%% ] time:%lums est:%llud %lluh %llum %llus\n", perm_count_1000000, (double)(perm_count_1000000 / 10000000.0), time_ms, time_days, time_hours, time_mins, time_secs);
				}
				part_start_time = clock();
			}

			if(memcmp((const void*)temp, (const void*)SHA1_CODE, SHA_DIGEST_LENGTH) == 0){
				for (int i=0; i < 24; i++) {
					sprintf((char*)&(buf2[i*2]), "%02x", code[i]);
				}
				for (int i=0; i < SHA_DIGEST_LENGTH; i++) {
					sprintf((char*)&(buf[i*2]), "%02x", temp[i]);
				}

				printf("MATCH CODE: %s  SHA1: %s\n", buf2, buf);
			}
		}
	}
}
