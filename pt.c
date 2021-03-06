#include "os.h"
#define LEVELS 5
#include <stdio.h>

/* Recieve pt without the 12 bits of offset */
uint64_t page_table_query(uint64_t pt, uint64_t vpn){
    uint64_t pointer_to_table, *actual_pointer_to_table;
    int i, index_in_table;  
    pointer_to_table = (pt<<12)+1;
    for(i = LEVELS-1; i >= 0; i--){
        index_in_table = find_index(vpn, i);
        actual_pointer_to_table = phys_to_virt(pointer_to_table-1);
        pointer_to_table = actual_pointer_to_table[index_in_table];
        if ((pointer_to_table&1) == 0){
	    
            return NO_MAPPING;
        }
        else if(i == 0){ 
            return actual_pointer_to_table[index_in_table]>>12;
        } 
    }
     
    
    return NO_MAPPING;
}


int find_index(uint64_t vpn, int i){
    return (vpn >> i*9) & 0x1ff;
}

/* Recieve pt without the 12 bits of offset */
void page_table_update(uint64_t pt, uint64_t vpn, uint64_t ppn){
    uint64_t array[5];
    uint64_t pointer_to_table, *actual_pointer_to_table, *actual_pointer_to_abba;
    int i, j, k, index_in_table;
    
    pointer_to_table = (pt<<12)+1;
    for(i = LEVELS-1; i >= 0; i--){
        array[i] = pointer_to_table;
        index_in_table = find_index(vpn, i);
        actual_pointer_to_table = phys_to_virt(pointer_to_table-1);
        pointer_to_table = actual_pointer_to_table[index_in_table];
        if(i == 0){
            actual_pointer_to_table[index_in_table] = (ppn<<12)+1;
            if(ppn == NO_MAPPING){
                actual_pointer_to_table[index_in_table]=0;
                for(j = 0; j < 4; j++){
                    actual_pointer_to_table = phys_to_virt(array[j]-1);
                    for(k = 0; k < 512; k++){
                        if((actual_pointer_to_table[k]&1) != 0){
                            return;
                        }
                    }
                    free_page_frame(array[j]>>12);
                    actual_pointer_to_abba = phys_to_virt(array[j+1]-1);
                    actual_pointer_to_abba[find_index(vpn, j+1)] = 0;
                }
            }
        }
        else if ((pointer_to_table&1) == 0){
            if(ppn == NO_MAPPING){
                return;
            }
            else{
                actual_pointer_to_table[index_in_table] = alloc_page_frame();
		actual_pointer_to_table[index_in_table] = (actual_pointer_to_table[index_in_table]<<12)+1;
                pointer_to_table = actual_pointer_to_table[index_in_table];
            }
        }
    }
}

/**********************************************************
 * PSEUDO CODE
 * 
 * page_table_update(pt, vpn, ppn):
 *      uint64_t array[5];
 *      pointer_to_table = pt
 *      for(i = 4 to 0):
 *            array[i] = pointer_to_table
 *            index_in_table = find_index(vpn, i) 
 *            actual_pointer_to_table = phys_to_virt(pointer_to_table)
 *            pointer_to_table = actual_pointer_to_table[index_in_table]
 *            if (i == 0):
 *                actual_pointer_to_table[index_in_table] = ppn
 *                    if (ppn == NO_MAPPING):
 *                        for(j = 0 to 3):
 *                            actual_pointer_to_table = phys_to_virt(array[j])
 *                            for(k = 0 to 512):
 *                                if (actual_pointer_to_table[k] != NO_MAPPING):
 *                                    return
 *                            free_page_frame(array[j])
 *                            actual_pointer_to_abba = phys_to_virt(array[j+1])
 *                            actual_pointer_to_abba[find_index(vpn, j+1)] = NO_MAPPING
 * 
 *            elif (pointer_to_table == NO_MAPPING):
 *                if (ppn == NO_MAPPING):
 *                     return
 *                else:             
 *                    actual_pointer_to_table[index_in_table] = alloc_page_frame()
 *                    pointer_to_table = actual_pointer_to_table[index_in_table]
 * 
 * 
 * ********************************************************/
