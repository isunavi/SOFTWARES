#ifndef _ATOMIC_H_
#define _ATOMIC_H_

// http://we.easyelectronics.ru/STM32/atomic-makrosy-dlya-arm.html

#ifdef __cplusplus
extern "C" {
#endif

//=============================================================================
static __asm __inline uint32_t get_interrupt_state(void)
{
    mrs r0, primask
    bx lr
}

//=============================================================================
static __asm __inline void set_interrupt_state(uint32_t status)
{
    msr primask, r0
    bx lr
}

#define ENTER_CRITICAL_SECTION()  do {uint32_t sreg_temp = get_interrupt_state(); __disable_irq()
#define LEAVE_CRITICAL_SECTION()  set_interrupt_state(sreg_temp);} while (0)

#ifdef __cplusplus
}
#endif

#endif
