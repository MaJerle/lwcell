uint8_t initialized = 0;

/*
 * \brief           Core callback function which must be implemented by user
 * \param[in]       ll: Low-Level structure
 * \return          gsmOK on success, member of \ref gsmr_t otherwise
 */
espr_t
gsm_ll_init(gsm_ll_t* ll) {
#if !GSM_CFG_MEM_CUSTOM
    /* Make sure that external memory is addressable in the memory area */

    /* Read documentation of your device where external memory is available in address space */
    uint8_t* memory = (void *)0x12345678;
    gsm_mem_region_t mem_regions[] = {
        { memory /* Pointer to memory */, 0x1234 /* Size of memory in bytes */ }
    };
    if (!initialized) {
        gsm_mem_assignmemory(mem_regions, GSM_ARRAYSIZE(mem_regions)); 
    }
#endif /* !GSM_CFG_MEM_CUSTOM */

    /* Do other steps */
}