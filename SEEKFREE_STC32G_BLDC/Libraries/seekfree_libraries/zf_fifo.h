#ifndef __ZF_FIFO_H
#define __ZF_FIFO_H


typedef struct
{
    uint8     *buffer;                                                          // 缓存指针
    uint32    head;                                                             // 缓存头指针 总是指向空的缓存
    uint32    end;                                                              // 缓存尾指针 总是指向非空缓存（缓存全空除外）
    uint32    size;                                                             // 缓存剩余大小
    uint32    max;                                                              // 缓存总大小
}fifo_struct;

typedef enum
{
    FIFO_SUCCESS,

    FIFO_BUFFER_NULL,
    FIFO_SPACE_NO_ENOUGH,
    FIFO_DATA_NO_ENOUGH,
}fifo_state_enum;

typedef enum
{
    FIFO_READ_AND_CLEAN,
    FIFO_READ_ONLY,
}fifo_operation_enum;

fifo_state_enum fifo_init           (fifo_struct *fifo, uint8 *buffer_addr, uint32 size);
void            fifo_head_offset    (fifo_struct *fifo, uint32 offset);
void            fifo_end_offset     (fifo_struct *fifo, uint32 offset);
void            fifo_clear          (fifo_struct *fifo);
uint32          fifo_used           (fifo_struct *fifo);

fifo_state_enum fifo_read_buffer    (fifo_struct *fifo, uint8 *dat, uint32 *length, fifo_operation_enum flag);
fifo_state_enum fifo_write_buffer   (fifo_struct *fifo, uint8 *dat, uint32 length);

#endif


