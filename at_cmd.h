#ifndef __AT_CMD_H__
#define __AT_CMD_H__

#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>


#undef PRN_DUMP

#define wait_ack_def 5000
#define max_at_len 16


typedef struct {
    char *cmd;
    uint32_t wait;
} s_at_cmd;

#pragma pack(push,1)
typedef struct {
    unsigned char hpv; //0..255
    unsigned short fsv;//0..65535
    char syncw[16];    //0xXX........

    unsigned syncl:4;  //0..8
    unsigned power:3;  //0..7 //0—20dbm,  1—17dbm, 2—15dbm, 3—10dbm, 4-?, 5—8dbm, 6—5dbm, 7—2dbm
    unsigned crc:1;    //0..1

    unsigned chan:4;   //0..F — 0..15 channel
    unsigned bandw:4;  //6—62.5KHZ, 7—125KHZ, 8—250KHZ, 9—500KHZ

    unsigned hfss:1;   //0..1
    unsigned plen:7;   //1..127

    unsigned sf:4;     //7—SF=7, 8—SF=8, 9—SF=9, A—SF=10, B—SF=11, C—SF=12
    unsigned crcode:2; //0—CR4/5, 1—CR4/6, 2—CR4/7, 3—CR4/8
    unsigned mode:2;   //0-LoRa, 1-OOK, 2-FSK, 3-GFSK

    unsigned freq:2;   //0-434MHZ Band, 1-470MHZ Band, 2-868MHZ Band, 3-915MHZ Band
    unsigned spr:4;    //0-1200bps 1-2400bps 2-4800bps 3-9600bps 4-14400bps 5-19200bps 6-38400bps 7-56000bps 8-57600bps 9-115200bps
    unsigned spc:2;    //0..2  :  0-none 1-even 2-odd
} s_lora_stat;
#pragma pack(pop)

extern const char *lora_uspeed[];
extern const char *lora_ucheck[];
extern const char *lora_power[];
extern const char *lora_crcode[];
extern const char *lora_bandw[];
extern const char *lora_main_mode[];
extern const char *lora_freq[];
extern const char *lora_hopping[];


extern s_lora_stat lora_stat;
extern s_at_cmd at_cmd[];

//#define TotalCmd  ((sizeof(at_cmd) / sizeof(s_at_cmd)) - 1)
extern uint8_t TotalCmd;

extern void put_at_value(uint8_t ind, char *uack);

#endif
