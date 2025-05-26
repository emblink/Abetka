#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/dma.h"
#include "hardware/pio.h"
#include "sd_card.h"
#include "ff.h"
#include "i2s.pio.h"
#include "hardware/clocks.h"


uint8_t f_buf0[4010], f_buf1[4010]; //double buffer 
uint8_t *file_ptr = f_buf0;
uint8_t *fifo_ptr = f_buf1;
uint8_t *temp_ptr=NULL;
bool read_flag=true;

uint byte_trans_count=0;
uint MAX_BYTE_TRANS=4000;

struct i2s_pio_dma_param_t {
    //for pio
    pio_program_t program;
    uint8_t bitsPerSample;
    uint8_t offset;
    uint8_t entry_point;
    uint8_t bitInsCycles; // 4 pio clock cycles for mono, 2 pio clocks for stereo
    //for dma
    int chan_num;
    uint8_t dma_size;
} i2s_pio_dma_param;

struct wave_file_header_t {
    char blockID[4];
    uint32_t totalSize;
    char typeHeader[4];
    char fmt[4];
    uint32_t headerLen;
    uint16_t typeOfFormat;
    uint16_t numbeOfChannel;
    uint32_t sampleRate;
    uint32_t byteRate;
    uint16_t blockAlign;
    uint16_t bitsPerSample;
    char dataHeader[4];
    uint32_t dataSize;
} wave_file_header;


void trans_wav();


void i2s_pio_init(PIO pio, uint sm, uint sideset_base, uint out_base, uint numberOfChannels, uint bitsPerSample, uint freq) {
    uint offset = 0;
    pio_sm_config c;
    if (numberOfChannels == 1) {
        offset = pio_add_program(pio, &i2s_pio_dma_mono_program);
        c = i2s_pio_dma_mono_program_get_default_config(offset);
        i2s_pio_dma_param.program = i2s_pio_dma_mono_program;
        i2s_pio_dma_param.entry_point = i2s_pio_dma_mono_offset_entry_point_m;
        sm_config_set_out_shift(&c, false, false, bitsPerSample);
    } else {
        offset = pio_add_program(pio, &i2s_pio_dma_stereo_program);
        c = i2s_pio_dma_stereo_program_get_default_config(offset);
        i2s_pio_dma_param.program = i2s_pio_dma_stereo_program;
        i2s_pio_dma_param.entry_point = i2s_pio_dma_stereo_offset_entry_point_s;
        sm_config_set_out_shift(&c, false, true, bitsPerSample);
    }
    i2s_pio_dma_param.offset = offset;
    i2s_pio_dma_param.bitsPerSample = bitsPerSample;
    pio_gpio_init(pio, out_base);
    pio_gpio_init(pio, sideset_base);
    pio_gpio_init(pio, sideset_base+1);
    pio_sm_set_consecutive_pindirs(pio, sm, sideset_base, 2, true);
    pio_sm_set_consecutive_pindirs(pio, sm, out_base, 1, true);
    sm_config_set_fifo_join(&c, PIO_FIFO_JOIN_TX);
    sm_config_set_out_pins(&c, out_base, 1);
    sm_config_set_sideset_pins(&c, sideset_base);
    float div = clock_get_hz(clk_sys)/freq;
    sm_config_set_clkdiv(&c, div);
    pio_sm_init(pio, sm, offset, &c);
}

void i2s_pio_enable(PIO pio, uint sm) {
    dma_channel_set_irq1_enabled(i2s_pio_dma_param.chan_num, true);
    pio_sm_set_enabled(pio, sm, true);
    pio_sm_exec(pio, sm, pio_encode_set(pio_y, i2s_pio_dma_param.bitsPerSample-2));
    pio_sm_exec(pio, sm, pio_encode_jmp(i2s_pio_dma_param.offset+i2s_pio_dma_param.entry_point));
}

void i2s_reset(PIO pio, uint sm) {
    pio_sm_drain_tx_fifo(pio, sm);
    pio_sm_set_enabled(pio, sm, false);

    pio_remove_program(pio, &i2s_pio_dma_param.program, i2s_pio_dma_param.offset);
    pio_clear_instruction_memory(pio);

    dma_channel_unclaim(i2s_pio_dma_param.chan_num);
    irq_set_enabled(DMA_IRQ_1,false);
    irq_remove_handler(DMA_IRQ_1, trans_wav);
    dma_channel_set_irq1_enabled(i2s_pio_dma_param.chan_num, false);
}

void i2s_pio_dma_init(PIO pio, uint sm) {
    i2s_pio_dma_param.chan_num = dma_claim_unused_channel(true);
    dma_channel_config c = dma_channel_get_default_config(i2s_pio_dma_param.chan_num);
    channel_config_set_write_increment(&c, false);
    channel_config_set_read_increment(&c, true);
    channel_config_set_dreq(&c, pio_get_dreq(pio, sm, true));
    channel_config_set_transfer_data_size(&c, i2s_pio_dma_param.dma_size); //DMA_SIZE_8,16,32
    dma_channel_acknowledge_irq1(i2s_pio_dma_param.chan_num);  // clear channel interrupt(ints1)
    dma_channel_set_irq1_enabled(i2s_pio_dma_param.chan_num, true); // enable irq1 (inte1)
    irq_add_shared_handler(DMA_IRQ_1, trans_wav, PICO_SHARED_IRQ_HANDLER_DEFAULT_ORDER_PRIORITY);
    irq_set_enabled(DMA_IRQ_1, true);
    dma_channel_configure(i2s_pio_dma_param.chan_num, &c, (void*) (PIO0_BASE+PIO_TXF0_OFFSET), 
             fifo_ptr, MAX_BYTE_TRANS>> i2s_pio_dma_param.dma_size, false); //DMA_SIZE_8 or 16 or 32
}

void trans_wav() {
    if (dma_channel_get_irq1_status(i2s_pio_dma_param.chan_num)) {
        dma_channel_acknowledge_irq1(i2s_pio_dma_param.chan_num);
        temp_ptr = file_ptr;
        file_ptr=fifo_ptr;
        fifo_ptr=temp_ptr;
        
        dma_channel_set_trans_count(i2s_pio_dma_param.chan_num, byte_trans_count >> i2s_pio_dma_param.dma_size, false);
        dma_channel_set_read_addr(i2s_pio_dma_param.chan_num, fifo_ptr,true);
        
        read_flag=true;
        if (byte_trans_count < MAX_BYTE_TRANS) {
            dma_channel_set_irq1_enabled(i2s_pio_dma_param.chan_num, false);
        }
    }
}

void i2s_init() {
    i2s_pio_dma_param.bitInsCycles=2;
    if (wave_file_header.numbeOfChannel == 1) i2s_pio_dma_param.bitInsCycles=4;
    uint freq = wave_file_header.sampleRate*i2s_pio_dma_param.bitInsCycles*wave_file_header.bitsPerSample*2;
    i2s_pio_dma_param.dma_size = wave_file_header.bitsPerSample >> 4; // DMA_SIZE_8, 16, 32

    i2s_pio_dma_init(pio0, 0);

    i2s_pio_init(pio0, 0, 6, 8, wave_file_header.numbeOfChannel,wave_file_header.bitsPerSample, freq);
    i2s_pio_enable(pio0, 0);
}

void playWave(char* fn) {
    printf("%s\n", fn);
    FRESULT fr;
    uint br;
    FIL pfile;
    fr = f_open(&pfile, fn, FA_READ);
    if (fr != FR_OK) {
        printf("open file error\n");
        return;
    }
    f_read(&pfile, &wave_file_header, 44, &br);
    if (fr != FR_OK) {
        printf("open file error\n");
        return;
    }

    i2s_init();

    f_read(&pfile, fifo_ptr, MAX_BYTE_TRANS, &byte_trans_count);
    read_flag=true;

    dma_channel_set_trans_count(i2s_pio_dma_param.chan_num, byte_trans_count >> i2s_pio_dma_param.dma_size, false);
    dma_channel_set_read_addr(i2s_pio_dma_param.chan_num, fifo_ptr,true);

    while (1) {
        if (read_flag) {
            read_flag=false;
            f_read(&pfile, file_ptr, MAX_BYTE_TRANS, &byte_trans_count);
            if (byte_trans_count < MAX_BYTE_TRANS) {
                f_close(&pfile);
                i2s_reset(pio0,0);
                break;
            }
        }
    }
}

void sdAudioTest()
{
    playWave("misc/Sword.wav");
}
