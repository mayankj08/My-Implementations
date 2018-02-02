#include "memory_manager.h"
#include "interposition.h"
#include <stdio.h>
#include <assert.h>
#include <limits.h>

int main(){

	mem_mngr_init();

	void* p = malloc(5);

    void *q = (void*)((char*)p+1000);

    free(q);


    //mem_mngr_print_snapshot();
	//mem_mngr_leave();	
}
