#ifndef __INTEL_8259A_H__
#define __INTEL_8259A_H__


#define PIT_IRQ		0
void pic_setup(int offset);
void pic_mask(int irq);
void pic_unmask(int irq);
void pic_ack(int irq);

#endif /*__INTEL_8259A_H__*/
