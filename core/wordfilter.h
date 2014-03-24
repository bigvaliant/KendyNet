#ifndef _WORDFILTER_H
#define _WORDFILTER_H

#include "kn_string.h"

typedef struct wordfilter *wordfilter_t;


wordfilter_t wordfilter_new(const char **forbidwords);

string_t     wordfiltrate(wordfilter_t,const char *str,char replace);

//如果输入串中不含屏蔽字返回1,否则返回0
uint8_t      isvaildword(wordfilter_t,const char *str);


#endif
