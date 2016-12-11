#include "_FIFO.h"
#include "board.h"


#define FIFO_POS     (s->in - s->out)

void FIFO_flush ( FIFO_t *s)
{
    s->in = s->out;
}


bool_t FIFO_init ( FIFO_t *s, uint8_t *buf, uint32_t FIFO_size)
{
    if ((NULL == buf) || 
        (FIFO_size < 2) || // FIFO size is too small.  It must be larger than 1
        ((FIFO_size & (FIFO_size - 1)) != 0)) //must be a power of 2
    {
        return FALSE;
    }
    else
    {
        __disable_irq ();
        s->buf = buf;
        s->size = FIFO_size;
        __enable_irq ();
        FIFO_flush (s);
        return TRUE;
    }
}


uint32_t FIFO_available ( FIFO_t *s) //TODO data size
{
    int32_t i;

    __disable_irq ();
    i = FIFO_POS;
    __enable_irq ();
    if (i < 0)
      i = -i;
    return (uint32_t)i;
}


bool_t FIFO_get ( FIFO_t *s, uint8_t *c) //TODO data size
{
    if (FIFO_available (s) > 0)
    {
        __disable_irq ();
        *c = (s->buf [(s->out) & (s->size - 1)]);
        s->out++;
        __enable_irq ();
        return  TRUE;
    }
    return FALSE;
}


bool_t FIFO_put ( FIFO_t *s, uint8_t c) 
{
    __disable_irq ();
    if (FIFO_POS < s->size) // If the buffer is full, return an error value
    {
        s->buf [s->in & (s->size - 1)] = c; 
        s->in++;
        __enable_irq ();
        return TRUE;
    }
    __enable_irq ();
    return FALSE;
}


bool_t FIFO_gets ( FIFO_t *s, uint8_t *c, uint32_t data_size) //TODO data size
{
    uint32_t i;

    if (NULL == c)
        return FALSE;
    if (data_size > s->size)
        return FALSE;
    if (FIFO_available (s) >= data_size)
    {
        __disable_irq ();
        for (i = 0; i < data_size; i++)
        {
            c [i] = (s->buf [(s->out) & (s->size - 1)]);
            s->out++;
        }
        __enable_irq ();
        return TRUE;
    }
    return FALSE;
}


bool_t FIFO_puts ( FIFO_t *s, uint8_t *c, uint32_t data_size) 
{
    uint32_t i;
    
    if (NULL == c)
        return FALSE;
    if (data_size > s->size)
        return FALSE;
    if (s->size - FIFO_available (s) >= data_size) // If the buffer is full, return an error value
    {
        __disable_irq ();
        for (i = 0; i < data_size; i++)
        {
            s->buf [s->in & (s->size - 1)] = c [i]; 
            s->in++;
        }
        __enable_irq ();
        return TRUE;
    }
    return FALSE;
}
