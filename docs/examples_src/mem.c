/*
 * This part should be done in ll initialization function only once on startup
 * Check GSM_LL part of library for more info
 */

#if !GSM_CFG_MEM_CUSTOM

/* We can simply create a big array variable which will be linked to internal memory by linker */
uint8_t mem_int[0x1000];

/*
 * Define memory regions for allocating algorithm,
 * make sure regions are in correct order for memory location
 */
lwgsm_mem_region_t mem_regions[] = {
    { mem_int, sizeof(mem_int) },               /* Set first memory region to internal memory of length 0x1000 bytes */ 
    { (void *)0xC0000000, 0x8000 },             /* External heap memory is located on 0xC0000000 and has 0x8000 bytes of memory */ 
};

/* On startup, user must call function to assign memory regions */
lwgsm_mem_assignmemory(mem_regions, GSM_ARRAYSIZE(mem_regions));

#endif /* !GSM_CFG_MEM_CUSTOM */