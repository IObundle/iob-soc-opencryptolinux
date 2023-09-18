#include <stdint.h>

#include "iob-plic.h"
#include "riscv-csr.h"

void plic_init(int base_address){
    base = base_address;

    int i = 0;
    //clear all EL
    for (i=0; i < EDGE_LEVEL_REGS; i++)
        plic_write((EL_BASE_ADDRESS+i)*DATA_W/8, 0);
    //set priority for all sources to '1'; '0' means 'never interrupt'
    int write_pr = write_pr_regs();
    for (i=0; i < PRIORITY_REGS; i++)
        plic_write((PR_BASE_ADDRESS+i)*DATA_W/8, write_pr);
    //clear all IE
    for (i=0; i < IE_REGS; i++)
        plic_write((IE_BASE_ADDRESS+i)*DATA_W/8, 0);
    //set all threshold to '0'
    for (i=0; i < PTHRESHOLD_REGS; i++)
        plic_write((TH_BASE_ADDRESS+i)*DATA_W/8, 0);
}
void plic_write(int address, int data){
    (*(volatile uint32_t *) (base+address))     = (uint32_t)(data);
}
int plic_read(int address){
    return (uint64_t)(*(volatile uint32_t *) (base+address));
}

int plic_enable_interrupt(int source){
    int target;
    target = csr_read_mhartid();
    plic_write((IE_BASE_ADDRESS+(target*EDGE_LEVEL_REGS)+(source/DATA_W))*DATA_W/8, 1 << (source % DATA_W));
    return target;
}
int plic_disable_interrupt(int source){
    int target;
    target = csr_read_mhartid();
    plic_write((IE_BASE_ADDRESS+(target*EDGE_LEVEL_REGS)+(source/DATA_W))*DATA_W/8, 0);
    return target;
}
/* Returns interrupt ID */
int plic_claim_interrupt(){
    int target;
    target = csr_read_mhartid();
    return plic_read((ID_BASE_ADDRESS + target) * DATA_W/8);
}
/* Sends complete signal to PLIC */
void plic_complete_interrupt(int source_id){
    int target;
    target = csr_read_mhartid();
    plic_write((ID_BASE_ADDRESS + target) * DATA_W/8, 1 << (source_id % DATA_W));
}

int write_pr_regs(){
    int res = 0;
    int i = 0;
    for (i = 0; i < PRIORITY_FIELDS_PER_REG; i++) {
        res = (res << (DATA_W/8)*PRIORITY_NIBBLES) | 0x01;
    }
    return res;
}
