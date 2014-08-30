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
#include <time.h>
#include <pthread.h>
#include <unistd.h>

typedef struct part_data {
	unsigned char _master[11];
	unsigned char _random[4];
	unsigned char imei[7] = {0x35, 0x16, 0x58, 0x05, 0x17, 0x63, 0x82}; // IMEI: 351658051763829
	unsigned char SHA1_CODE[20] = {0x73, 0x34, 0xCE, 0x79, 0xE0, 0x30, 0x17, 0x02, 0xDF, 0x2C, 0x95, 0x72, 0x0A, 0x78, 0x51, 0x69, 0x8B, 0xF6, 0x83, 0x9A};
	unsigned char code[24];
	unsigned long perm_count = 0;
	unsigned long perm_count_1000000 = 0;

	unsigned char temp[SHA_DIGEST_LENGTH];
	char buf[SHA_DIGEST_LENGTH*2];
	char buf2[48];
} part_data_t;
part_data_t parts_data[4];

unsigned long perm_count_1000000 = 0;
time_t start_time;
time_t end_time;

void bf(int, int, int, int);
void* bf_part(void* part);
void* master_loop(void* arg);

int main(int argn, char *argv[]) {

	time(&start_time);

	pthread_t t1, t2, t3, t4, master; // declare 4 threads.
    int p1 = 0, p2 = 1, p3 = 2, p4 = 3, master_arg = 0; // args
    pthread_create( &t1, NULL, bf_part, &p1); // create a thread running bf_part
    sleep(1);
    pthread_create( &t2, NULL, bf_part, &p2);
    sleep(1);
    pthread_create( &t3, NULL, bf_part, &p3);
    sleep(1);
    pthread_create( &t4, NULL, bf_part, &p4);
    sleep(1);
    pthread_create( &master, NULL, master_loop, &master_arg);

    pthread_join( t1, NULL);
    pthread_join( t2, NULL);
    pthread_join( t3, NULL);
    pthread_join( t4, NULL);
    pthread_join( master, NULL);

    // Because all created threads are terminated when main() finishes, we have
    // to give the threads some time to finish. Unfortunately for function1, main()
    // will give only 1 second, but function1 needs at least 2 seconds. So function1 will
    // probably be terminated before it can finish. This is a BAD way to manage threads.
    sleep(1);

	return 0;
}

void bf(int digit, int start_val, int vals, int part) {
	for(int v=start_val; v<(start_val+vals); v++) {
		parts_data[part].perm_count++;
		parts_data[part].code[digit] = v;
		if((digit+1) < 14){
			bf(digit+1, 0, 10, part);
		} else {
			SHA1((unsigned char *)parts_data[part].code, 24, parts_data[part].temp);

			//printf("parts_data[%d].perm_count = %lu\n", part, parts_data[part].perm_count);
			if(parts_data[part].perm_count >= 1000000) {
				parts_data[part].perm_count = 0;
				parts_data[part].perm_count_1000000++;
				//printf("%d) perm_count_1000000 = %lu\n", part, parts_data[part].perm_count_1000000);
//				if(parts_data[part].perm_count_1000000 % 10 == 0) {
//					time(&parts_data[part].end_time);
//					unsigned long long elapsed_s = difftime(parts_data[part].end_time, start_time);
//					unsigned long long time_secs = elapsed_s * ((1000000000 / (parts_data[part].perm_count_1000000))-1);
//					unsigned long long time_mins = time_secs / 60;
//					time_secs = time_secs % 60;
//					unsigned long long time_hours = time_mins / 60;
//					time_mins = time_mins % 60;
//					unsigned long long time_days = time_hours / 24;
//					time_hours = time_hours % 24;
//
//					printf("DONE: %luM [ %lf%% ] elapsed:%llus est:%llud %lluh %llum %llus\n", parts_data[part].perm_count_1000000, (double)(parts_data[part].perm_count_1000000 / 10000000.0), elapsed_s, time_days, time_hours, time_mins, time_secs);
//				}
				//part_start_time = clock();
			}

			if(memcmp((const void*)parts_data[part].temp, (const void*)parts_data[part].SHA1_CODE, SHA_DIGEST_LENGTH) == 0){
				for (int i=0; i < 24; i++) {
					sprintf((char*)&(parts_data[part].buf2[i*2]), "%02x", parts_data[part].code[i]);
				}
				for (int i=0; i < SHA_DIGEST_LENGTH; i++) {
					sprintf((char*)&(parts_data[part].buf[i*2]), "%02x", parts_data[part].temp[i]);
				}

				printf("MATCH CODE: %s  SHA1: %s\n", parts_data[part].buf2, parts_data[part].buf);
			}
		}
	}
}

void* bf_part(void* part)
{
	memset(parts_data[0].buf, 0, SHA_DIGEST_LENGTH*2);
	memset(parts_data[0].temp, 0, SHA_DIGEST_LENGTH);

	memset(parts_data[0]._master, 0, 11);
	memset(parts_data[0]._random, 0, 4);

	memcpy(parts_data[0].code, (const void*)parts_data[0]._master, 11);
	memcpy(parts_data[0].code+11, (const void*)parts_data[0]._random, 4);
	memcpy(parts_data[0].code+11+4+1, (const void*)parts_data[0].imei, 7);

	time(&end_time);

	int* p = (int*)part;
	if(*p == 0){
		printf("Part 1: started.\n");
		bf(0, 0, 3, 0);
	}
	else if(*p == 1){
		printf("Part 2: started.\n");
		bf(0, 3, 3, 1);
	}
	else if(*p == 2){
		printf("Part 3: started.\n");
		bf(0, 6, 2, 2);
	}
	else if(*p == 3){
		printf("Part 4: started.\n");
		bf(0, 8, 2, 3);
	}
    return NULL;
}

void* master_loop(void* arg)
{
	while(1){
		sleep(2);
		perm_count_1000000 = 0;
		for(int p=0; p<4; p++){
			perm_count_1000000 += parts_data[p].perm_count_1000000;
		}
		//if(perm_count_1000000 % 10 == 0) {
			time(&end_time);
			unsigned long long elapsed_s = difftime(end_time, start_time);
			unsigned long long time_secs = elapsed_s * ((1000000000 / (perm_count_1000000))-1);
			unsigned long long time_mins = time_secs / 60;
			time_secs = time_secs % 60;
			unsigned long long time_hours = time_mins / 60;
			time_mins = time_mins % 60;
			unsigned long long time_days = time_hours / 24;
			time_hours = time_hours % 24;

			printf("DONE: %luM [ %lf%% ] elapsed:%llus est:%llud %lluh %llum %llus\n", perm_count_1000000, (double)(perm_count_1000000 / 10000000.0), elapsed_s, time_days, time_hours, time_mins, time_secs);
		//}
	}
	return NULL;
}
