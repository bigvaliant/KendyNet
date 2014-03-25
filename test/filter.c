#include "core/wordfilter.h"
#include <stdio.h>
#include <string.h>
#include "core/systime.h"

int main(){
	
	int i,j;
	const char *words[100001];
	for(i = 0; i < 100000; ++i){
		char *word = calloc(1,21);
		for(j = 0; j < 20; ++j){
			word[j] = rand()%26;
			word[j] += 'a';
		}
		words[i] = word;
	}
	words[100000] = NULL;
	wordfilter_t f = wordfilter_new(words);
	
	uint32_t begin = GetSystemMs();
	
	for(j = 0; j < 10;++j){	
		for(i = 0; i < 100000; ++i){
			isvaildword(f,"aaaaaaaaaaaaaaa");
			//isvaildword(f,words[i]);
		}
	}
	
	printf("%d\n",GetSystemMs()-begin);
		
	return 0;
}
