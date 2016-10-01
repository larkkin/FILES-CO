#include <idt.h>
#include <desc.h>
#include <ints.h>
#include <memory.h>

#define IDT_SIZE 256

/* struct PtrToIdt   //указатель на таблицу дескрипторов прерываний
{
    uint16_t size;
    uint32_t address; //адрес идт
} __attribute__ ((packed)) ; //as we don't need to align data
*/
struct InterruptDescriptor
{
    uint16_t address_1; //first quarter
    uint16_t selector; 
    uint16_t type_container;
    uint16_t address_2; //second quarter of the address
    uint32_t address_3; //second half of the address
    uint32_t alwayszero;
   
} __attribute__((packed));

struct InterruptDescriptor idt[IDT_SIZE]; //no need  it's idt itself; static as we don't want it to be seen from everywhere

void add_to_idt(uint8_t interrupt_index, uint64_t address_of_interrupt_handler, uint16_t selector)
{
    struct InterruptDescriptor idinstance;

    idinstance.alwayszero = 0x00000000;
    idinstance.address_3 = address_of_interrupt_handler >> 32; //second half 
    idinstance.address_2 = (address_of_interrupt_handler & 0x00000000FFFF0000) >> 16;
    idinstance.type_container = 0x8E00; 
    idinstance.selector = selector;
    idinstance.address_1 = address_of_interrupt_handler & 0x000000000000FFFF;
    idt[interrupt_index] = idinstance; //kladem tolko chto sozdanniy
}

void init_idt()
{
    extern uint64_t entry_table[];
    
    static struct desc_table_ptr ptr_idt; //we create the structure instance
    ptr_idt.size = IDT_SIZE;//size in bytes -1
    ptr_idt.addr = (uint64_t) idt; //cast type as addr is 64
    write_idtr(&ptr_idt); //so we procede to taking the address 

    for (uint8_t i = 0; i < 49; i++)
    {
        add_to_idt(i, entry_table[i], KERNEL_CS);
    }
    
    
}
void clear_idt()
{
    int8_t* g =(int8_t*) idt;//we create the pointer and use the ponter arithmetics and clean every byte
    for (uint32_t i=0; i< IDT_SIZE * sizeof(struct InterruptDescriptor); i++) 
    {
        *g =0;
        g++;
    }
}