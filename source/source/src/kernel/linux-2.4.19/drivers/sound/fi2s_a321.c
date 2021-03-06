//apb dma test program for A321 mono test
/* 
fi2s.c programmed by Ivan Wang
I2S program for audio 2004/8/23 06:53pm
*/
#include <linux/config.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/stddef.h>
#include <linux/pm.h>
#include <linux/delay.h>
#include <asm/system.h>
#include <linux/interrupt.h>
#include <asm/arch/hardware.h>
#include <asm/dma.h>
#include <asm/arch/cpe/cpe.h>
#include <asm/arch/cpe_int.h>
#include <asm/arch/apb_dma.h>
#include <asm/arch/ssp.h>
#include "sound_config.h"


/***************************************************************************
    I2S Driver
 **************************************************************************/
static int fi2s_open(int dev, int mode);
static void fi2s_close(int dev);
static void fi2s_output_block(int dev, unsigned long buf, int count, int intrflag);
static void fi2s_start_input(int dev, unsigned long buf, int count, int intrflag);
//static int fi2s_audio_ioctl(int dev, unsigned int cmd, caddr_t arg);
static int fi2s_prepare_for_input(int dev, int bsize, int bcount);
static int fi2s_prepare_for_output(int dev, int bsize, int bcount);
static void fi2s_halt(int dev);
static void fi2s_halt_input(int dev);
static void fi2s_halt_output(int dev);
static void fi2s_trigger(int dev_id, int state);
static int fi2s_set_speed(int dev, int speed);
static unsigned int fi2s_set_bits(int dev, unsigned int arg);
static short fi2s_set_channels(int dev, short arg);
static void fi2s_init_hw(int);
void fi2s_a321_interrupt_handler(int irq, void *dev_id, struct pt_regs *dummy);


static struct address_info fi2s_cfg;

static apb_dma_parm_t out_parm;

typedef struct
{
    unsigned int    io_base;
    unsigned int    io_base_phys;
    unsigned char   name[32];
    unsigned int    irq;
    unsigned int    recBuffer_Phy;
    unsigned int    playBuffer_Phy;
    unsigned int    RemainRecCount;	
    unsigned int    RemainPlayCount;
    unsigned int    first_time;
    unsigned int    audio_mode;		
    unsigned int    open_mode;		
    unsigned int    dev_no;
	unsigned int    sclk_div;
	unsigned int    fpclk;
	apb_dma_data_t  *priv;
} fi2s_info;


typedef struct fi2s_port_info
{
    int             open_mode;
    int             speed;
    unsigned char   speed_bits;
    int             channels;
    int             audio_format;
    unsigned char   format_bits;
} fi2s_port_info;


static struct audio_driver fi2s_a321_audio_driver =
{
    owner:          THIS_MODULE,
    open:           fi2s_open,
    close:          fi2s_close,
    output_block:   fi2s_output_block,
    start_input:    fi2s_start_input,
//        ioctl:          fi2s_audio_ioctl,
    prepare_for_input:      fi2s_prepare_for_input,
    prepare_for_output:     fi2s_prepare_for_output,
    halt_io:        fi2s_halt,
    halt_input:     fi2s_halt_input,
    halt_output:    fi2s_halt_output,
    trigger:        fi2s_trigger,
    set_speed:      fi2s_set_speed,
    set_bits:       fi2s_set_bits,
    set_channels:   fi2s_set_channels
};

//Playing: null,null,null,data0,data1,data2
//Recording: null,null,null,data0,data1,data2
#define             FI2S_BUFFER_NUM    6   //null,null,null,data0,data1,data2
#define             FI2S_BUFFER_START  3   //null,null,null,data0,data1,data2
//#define             FI2S_BUFFER_SIZE   0x6000
//#define             FI2S_NULL_SIZE     0x800
#define             FI2S_BUFFER_SIZE   0x4000
#define             FI2S_NULL_SIZE     0x400

static fi2s_info        fi2s_dev_info[MAX_AUDIO_DEV];
static spinlock_t       fi2s_lock;
static int              nr_fi2s_devs=0;
static unsigned int     fi2s_buf_idx;
static unsigned int     fi2s_copy_idx;
static unsigned char    *fi2s_buf[FI2S_BUFFER_NUM];   
static unsigned int     fi2s_buf_phys[FI2S_BUFFER_NUM];
static unsigned int     va_start,pa_start;


/*
Copy action from => to with byte size (bsize)
*/
static inline void fi2s_do_copy_action(fi2s_port_info *portc,unsigned int to,unsigned int bsize)
{
    unsigned int    from=(unsigned int)fi2s_buf[0];
    
    if((from==0)||(to==0)||(bsize==0))
        return;

    /* handle for mono record */
    if(portc->channels==1)
    {
        unsigned int i;
        //printk("copy from 0x%x to 0x%x with 0x%x\n",from,to,bsize);
        for(i=0;i<bsize/2;i=i+2) //16 bit count
            *(unsigned short *)(to+i)=*(unsigned short *)(from+i*2);
    }
    else
        memcpy((char *)to,(char *)from,bsize);
#if 0        
    if(fi2s_copy_idx==(FI2S_BUFFER_NUM-1))
        fi2s_copy_idx=FI2S_BUFFER_START;
    else
        fi2s_copy_idx++;
#endif
}


static void dma_interrupt_handler(int irq, void *dev_id, struct pt_regs *dummy)
{
	fi2s_info *dev_info= (fi2s_info *)audio_devs[(int)dev_id]->devc;
	fi2s_port_info      *portc = (fi2s_port_info *) audio_devs[(int)dev_id]->portc;
	
    apbdma_clear_int(dev_info->priv);
    //printk("<%d,0x%x,0x%x>",irq,dev_info->audio_mode,*(volatile unsigned int *)0xf9040148);

    if(dev_info->audio_mode&PCM_ENABLE_INPUT)
    {
        fi2s_do_copy_action(portc,(unsigned int)__va(dev_info->recBuffer_Phy),(unsigned int)dev_info->RemainRecCount);
        DMAbuf_inputintr((int)dev_id);
    }
	else if(dev_info->audio_mode&PCM_ENABLE_OUTPUT)
        DMAbuf_outputintr((int)dev_id, 1);
}

void fi2s_a321_interrupt_handler(int irq, void *dev_id, struct pt_regs *dummy)
{
    fi2s_info      *dev_info=(fi2s_info *) audio_devs[(int)dev_id]->devc;  

    printk("<<%d>>",irq);
    *(volatile unsigned int *)(dev_info->io_base+SSP_CONTROL2)=0;
    apb_dma_reset(dev_info->priv);
    printk("***SSP underrun error status:0x%x from IRQ %d\n",
        *(unsigned int *)(dev_info->io_base+SSP_INT_STATUS),irq);
    //printk("==>0x%x\n",*(unsigned int *)0xfb080014);
    dev_info->first_time=1;
}

static inline void fill_ssp_buffer(unsigned int base)
{
    int i;
    *(volatile unsigned int *)(base+SSP_CONTROL2)=(SSP_TXFCLR|SSP_RXFCLR);
    udelay(100);
    for(i=0;i<16;i++)    
        *(volatile unsigned int *)(base+SSP_DATA)=0;
}

/**********************************************
    PLAY
 *********************************************/
static inline void fi2s_trigger_play(fi2s_info *dev_info)
{
//    apb_dma_parm_t parm;
    u32 base;
//	u32 temp;

	base = dev_info->io_base;
/*
    //printk("fi2s_trigger_play:0x%x\n",jiffies);
    while(*(volatile unsigned int*)(base+SSP_INT_STATUS)&0x3)
        ;
    fill_ssp_buffer(base);
    dev_info->first_time=0;
*/
    out_parm.src=(unsigned int)dev_info->playBuffer_Phy;
    out_parm.size=(dev_info->RemainPlayCount)/2;

/*
    parm.src=(unsigned int)dev_info->playBuffer_Phy;
    parm.dest=dev_info->io_base_phys+SSP_DATA;
    parm.width=APBDMA_WIDTH_16BIT;
    parm.req_num=0; //needn't request/grant number
    parm.width=APBDMA_WIDTH_16BIT;
    parm.sctl=APBDMA_CTL_INC2;
    parm.dctl=APBDMA_CTL_FIX;
    parm.stype=APBDMA_TYPE_AHB;
    parm.dtype=APBDMA_TYPE_APB;
    parm.burst=0;
    parm.req_num=6;
    parm.size=(dev_info->RemainPlayCount)/2;
    parm.irq=APBDMA_TRIGGER_IRQ;
*/

    //printk("apb add\n");
    apb_dma_add(dev_info->priv,&out_parm);

    //printk("fi2s_trigger_play:0x%x\n",jiffies);
//	temp = *(volatile unsigned int*)(base+SSP_INT_STATUS);
//    while(temp &0x3)
//    {
//	   temp = *(volatile unsigned int*)(base+SSP_INT_STATUS);
//	}

//    fill_ssp_buffer(base);
    dev_info->first_time=0;

    apb_dma_start(dev_info->priv);
    *(volatile unsigned int *)(base+SSP_INT_CONTROL)=(SSP_FIFO_THOD|SSP_TXDMAEN|SSP_TFURIEN);
    *(volatile unsigned int *)(base+SSP_CONTROL2)=(SSP_SSPEN|SSP_TXDOE);
}
/**********************************************
    RECORD
 *********************************************/
static inline void fi2s_trigger_record(fi2s_info *dev_info)
{
    apb_dma_parm_t parm;
    /* clear SSP irq status */
    //while(*(volatile unsigned int*)(dev_info->io_base+SSP_INT_STATUS)&0x3)
      //  ;
    if(dev_info->first_time)
        *(volatile unsigned int *)(dev_info->io_base+SSP_CONTROL2)=(SSP_TXFCLR|SSP_RXFCLR);

    dev_info->first_time=0;
    fi2s_buf_idx=FI2S_BUFFER_START;
    
    /* real data 0*/
    parm.src=dev_info->io_base_phys+SSP_DATA;
    parm.dest=(unsigned int)fi2s_buf_phys[0];
    parm.width=APBDMA_WIDTH_16BIT;
    parm.sctl=APBDMA_CTL_FIX;
    parm.dctl=APBDMA_CTL_INC2;
    parm.stype=APBDMA_TYPE_APB;
    parm.dtype=APBDMA_TYPE_AHB;
    parm.burst=0;
    parm.req_num=6;
    parm.size=(dev_info->RemainRecCount)/2;
    parm.irq=APBDMA_TRIGGER_IRQ;
    apb_dma_add(dev_info->priv,&parm);

    /* start fi2s */
    apb_dma_start(dev_info->priv);
    *(volatile unsigned int *)(dev_info->io_base+SSP_INT_CONTROL)=(SSP_FIFO_THOD|SSP_RXDMAEN|SSP_RFURIEN);
    *(volatile unsigned int *)(dev_info->io_base+SSP_CONTROL2)=(SSP_SSPEN|SSP_TXDOE);
}

static void fi2s_trigger(int dev_id, int state)
{
    fi2s_port_info         *portc = (fi2s_port_info *) audio_devs[dev_id]->portc;
    fi2s_info              *dev_info=(fi2s_info *) audio_devs[(int)dev_id]->devc;
    unsigned long           flags;

//printk("<t>");
    state&=dev_info->audio_mode;
    spin_lock_irqsave(&fi2s_lock,flags);

    if((portc->open_mode&OPEN_WRITE)&&(state&PCM_ENABLE_OUTPUT))
        fi2s_trigger_play(dev_info);
    else if((portc->open_mode&OPEN_READ)&&(state&PCM_ENABLE_INPUT))
        fi2s_trigger_record(dev_info);
    else
        printk("No trigger action:0x%x,0x%x\n",portc->open_mode,state);
    spin_unlock_irqrestore(&fi2s_lock,flags);
}

static int pmu_set_mclk(fi2s_info *dev_info,unsigned int speed)
{
#if 0
    unsigned int pmu_val;

    /* set clock */
    pmu_val=*(volatile unsigned int *)(CPE_PMU_VA_BASE+0x34);
	pmu_val&=0xfff0ffff;

    switch (speed)
    {
        case 8000:
            dev_info->fpclk=2048000;
            pmu_val|=(0x0<<16);        
            break;
        case 16000:
            dev_info->fpclk=4096000;
            pmu_val|=(0x2<<16);        
            break;
        case 32000:
            dev_info->fpclk=8192000;
            pmu_val|=(0x4<<16);        
            break;
        case 44100:
            dev_info->fpclk=11289600;
            pmu_val|=(0x5<<16);        
            break;
        case 48000:
            dev_info->fpclk=12288000;
            pmu_val|=(0x6<<16);        
            break;
        default: //44.1KHz
            dev_info->fpclk=11289600;
            pmu_val|=(0x5<<16);        
    }
	*(unsigned int *)(CPE_PMU_VA_BASE+0x34)=pmu_val;
	return 1;
#endif
}

//affect fragment size
//can only play 96k sample data (or its common factor)
static int fi2s_set_speed(int dev, int speed)
{
	fi2s_port_info  *portc=(fi2s_port_info *)audio_devs[dev]->portc;
	fi2s_info       *dev_info=(fi2s_info *) audio_devs[dev]->devc;

    //printk("fi2s_set_speed=0x%x 0x%x clk=%d\n",portc->channels,speed,dev_info->fpclk);

    if(speed==0)
        speed=portc->speed;

    pmu_set_mclk(dev_info,speed);
#if 0
    dev_info->sclk_div = (dev_info->fpclk)/(2*portc->channels*16*speed)-1;
    //printk("dev_info->sclk_div=%d dev_info->fpclk=%d\n",dev_info->sclk_div,dev_info->fpclk);
    portc->speed=(dev_info->fpclk)/((2 *(portc->channels)*16)*(dev_info->sclk_div+1));
#else
    //dev_info->sclk_div = (dev_info->fpclk)/(2*portc->channels*16*speed)-1;
    dev_info->sclk_div = (dev_info->fpclk)/(2*2*16*speed)-1;//fix to set stereo
    //printk("dev_info->sclk_div=%d dev_info->fpclk=%d\n",dev_info->sclk_div,dev_info->fpclk);
    //portc->speed=(dev_info->fpclk)/((2 *(portc->channels)*16)*(dev_info->sclk_div+1));
    portc->speed=(dev_info->fpclk)/((2*2*16)*(dev_info->sclk_div+1)); //fix to set stereo
#endif
	*(volatile unsigned int *)(dev_info->io_base+SSP_CONTROL1)=(dev_info->sclk_div)|0xf0000;

    //printk("set speed to %d\n",portc->speed);

	return portc->speed;
}

//affect fragment size
static unsigned int fi2s_set_bits(int dev, unsigned int arg)
{
    fi2s_port_info *portc=(fi2s_port_info *) audio_devs[dev]->portc;
    portc->audio_format=AFMT_S16_LE;
    return portc->audio_format;
}

//affect fragment size
static short fi2s_set_channels(int dev, short arg)
{
    fi2s_port_info *portc = (fi2s_port_info *) audio_devs[dev]->portc;
    fi2s_info      *dev_info=(fi2s_info *)audio_devs[dev]->devc;
    
    //printk("***     set channels=%d\n",arg);


    if(arg==2)
        portc->channels=2;
    else if(arg==1) //set to mono
    {
        portc->channels=1;
        *(volatile unsigned int *)(dev_info->io_base+SSP_CONTROL0)=
            //(*(volatile unsigned int *)(dev_info->io_base+SSP_CONTROL0)&0xfffffff3)|0x8;
            (*(volatile unsigned int *)(dev_info->io_base+SSP_CONTROL0)&0xfffffff3)|0xc; //simulate to stereo
    }
    //else
    //   portc->channels=1;
   
    //printk("set channel to %d\n",portc->channels);
    return portc->channels;
}


static void fi2s_init_codec_rec(unsigned int base)
{
/* nothing to do */
    return;
}


static void fi2s_init_codec_play(unsigned int base)
{
/* nothing to do */
    return;
}

static int fi2s_open(int dev, int mode)
{
    fi2s_info          *dev_info = (fi2s_info *) audio_devs[dev]->devc;
    fi2s_port_info     *portc = (fi2s_port_info *) audio_devs[dev]->portc;
    unsigned long       flags;
//printk("open\n");

    if (dev < 0 || dev >= num_audiodevs)
    	return -ENXIO;    
    //init buffer
    memset(fi2s_buf[0],0,FI2S_BUFFER_SIZE);

    spin_lock_irqsave(&fi2s_lock,flags);
    if(portc->open_mode||(dev_info->open_mode&mode))
    {
		spin_unlock_irqrestore(&fi2s_lock,flags);
        return -EBUSY;
    }
        
    dev_info->audio_mode = 0;
    dev_info->open_mode |= mode;    
    dev_info->recBuffer_Phy = 0;
    dev_info->playBuffer_Phy = 0;
    dev_info->RemainRecCount = 0;
    dev_info->RemainPlayCount = 0;    
    dev_info->first_time=1;
    portc->open_mode = mode;
    
    fi2s_buf_idx=FI2S_BUFFER_START;
    fi2s_copy_idx=FI2S_BUFFER_START;
    
    /* mclk is 12.288Khz for A321 */
    portc->speed=8000; //set default to 8KHz
    //portc->speed=48000; //set default to 48KHz
    dev_info->fpclk=12288000;
    //dev_info->fpclk=2048000;
    //dev_info->fpclk=24576000;    //set to 96K for codec

    /* init apb dma */    
    dev_info->priv=apb_dma_alloc();
    //printk("dev_info->priv=0x%x\n",dev_info->priv);
    dev_info->priv->base=CPE_A321_APBDMA_VA_BASE;
    dev_info->priv->channel=0;    
    apb_dma_init(dev_info->priv);

    spin_unlock_irqrestore(&fi2s_lock,flags);
    return 0;
}

static void fi2s_halt_input(int dev)
{
    unsigned long   flags;
    fi2s_info      *dev_info = (fi2s_info *) audio_devs[dev]->devc;

    spin_lock_irqsave(&fi2s_lock,flags);	
	dev_info->audio_mode &= ~PCM_ENABLE_OUTPUT;
	spin_unlock_irqrestore(&fi2s_lock,flags);
}


static void fi2s_halt_output(int dev)
{
	fi2s_info *dev_info=(fi2s_info *)audio_devs[dev]->devc;
	unsigned long flags;

	spin_lock_irqsave(&fi2s_lock,flags);
	*(volatile unsigned int *)(dev_info->io_base+SSP_INT_CONTROL)&=0xff00;
	dev_info->audio_mode &= ~PCM_ENABLE_OUTPUT;
	spin_unlock_irqrestore(&fi2s_lock,flags);
}	

static void fi2s_halt(int dev)
{
    fi2s_info      *dev_info=(fi2s_info *)audio_devs[dev]->devc;
    fi2s_port_info *portc=(fi2s_port_info *) audio_devs[dev]->portc;
    
    *(volatile unsigned int *)(dev_info->io_base+SSP_CONTROL2)=0;
    if (portc->open_mode & OPEN_WRITE)
		fi2s_halt_output(dev);
    if (portc->open_mode & OPEN_READ)
		fi2s_halt_input(dev);
    dev_info->audio_mode = 0;
}

static void fi2s_close(int dev)
{
    unsigned long   flags;
    fi2s_info        *dev_info = (fi2s_info *) audio_devs[dev]->devc;
    fi2s_port_info   *portc = (fi2s_port_info *) audio_devs[dev]->portc;
	//printk("fi2s_close\n");
    spin_lock_irqsave(&fi2s_lock,flags);
    if (dev_info->priv)
       apb_dma_free(dev_info->priv);  
    //printk("ahb_dma_free\n");	
    fi2s_halt(dev);
    dev_info->audio_mode = 0;
    dev_info->open_mode &= ~portc->open_mode;
    portc->open_mode = 0;
    /* SSP state machine reset */
    *(volatile unsigned int *)(dev_info->io_base+SSP_CONTROL2)=0x40;
    spin_unlock_irqrestore(&fi2s_lock,flags);       
}


static void fi2s_output_block(int dev, unsigned long buf, int count, int intrflag)
{
    fi2s_port_info  *portc = (fi2s_port_info *) audio_devs[dev]->portc;
    unsigned long   flags;
    fi2s_info       *dev_info = (fi2s_info *) audio_devs[dev]->devc;
    //unsigned int    *play_buf;
    //unsigned int    virt_addr;

    //if(count>8192)
    //{
    //    printk("fail count %d\n",count);
    //    return;
    //}   
    
    //printk("out_blk:0x%x,cnt=0x%x\n",(int)buf,(int)count);    
    
    spin_lock_irqsave(&fi2s_lock,flags);

 	dev_info->audio_mode |= PCM_ENABLE_OUTPUT;
    //play_buf=(unsigned int *)fi2s_buf[fi2s_buf_idx];
    //virt_addr=(unsigned int)__va(buf);

    if(portc->channels==1)
    {
        int i;
        unsigned short *dbuf=(unsigned short *)fi2s_buf[fi2s_buf_idx];
        unsigned short *sbuf=(unsigned short *)__va(buf);
        //memcpy(play_buf,(unsigned int *)virt_addr,count);
        for(i=0;i<count/2;i++) //16 bit count
        {
            //printf("\r0x%x",dbuf+2*i);
            *(dbuf+2*i)=*(sbuf+i);
            *(dbuf+2*i+1)=*(sbuf+i);
        }
        count=count*2;
    }
    else {
       //memcpy(play_buf,(unsigned int*)virt_addr,count);
	   u32 *tmp = (u32 *) fi2s_buf[fi2s_buf_idx];
	   u32 *s = (u32 *) __va(buf);
       u32 temp;

	   temp = count >> 2;
	   while (temp--)
	      *tmp++ = *s++;	   
    }

    //printk("va=0x%x pa=0x%x idx=%d\n",(unsigned int)play_buf,(unsigned int)fi2s_buf_phys[fi2s_buf_idx],fi2s_buf_idx);

    dev_info->playBuffer_Phy=(unsigned int)fi2s_buf_phys[fi2s_buf_idx];

    if (fi2s_buf_idx==FI2S_BUFFER_NUM-1)
       fi2s_buf_idx=FI2S_BUFFER_START;
    else
       fi2s_buf_idx++;
    dev_info->RemainPlayCount=count;

	spin_unlock_irqrestore(&fi2s_lock,flags);
}


static void fi2s_start_input(int dev, unsigned long buf, int count, int intrflag)
{
    unsigned long       flags;
    fi2s_info           *dev_info = (fi2s_info *) audio_devs[dev]->devc;
    fi2s_port_info  *portc = (fi2s_port_info *) audio_devs[dev]->portc;
    
    spin_lock_irqsave(&fi2s_lock,flags);
 	dev_info->audio_mode |= PCM_ENABLE_INPUT;
 	//printk("start_input:0x%x/0x%x sz:0x%x\n",(int)buf,(unsigned int)__va(buf),(int)count);

	dev_info->recBuffer_Phy = buf;
	//dev_info->RemainRecCount = count; 
	//mono => stereo for ivan
	if(portc->channels==1)
	    dev_info->RemainRecCount = count*2; //mono => record by stereo
	else
	    dev_info->RemainRecCount = count;
    //printk("dev_info->RemainRecCount =0x%x count=0x%x channel=%x\n",dev_info->RemainRecCount,count,portc->channels);
	spin_unlock_irqrestore(&fi2s_lock,flags);
}


static int fi2s_prepare_for_input(int dev, int bsize, int bcount)
{
    fi2s_info      *dev_info = (fi2s_info *) audio_devs[dev]->devc;
    unsigned long   flags;
    
    //printk("fi2s_prepare_for_input,bsize\n");
    spin_lock_irqsave(&fi2s_lock,flags);
    fi2s_init_codec_rec(dev_info->io_base);
    //flib_set_codec(dev_info->io_base);
	audio_devs[dev]->dmap_in->flags |= DMA_NODMA;
    fi2s_halt_input(dev);    
    spin_unlock_irqrestore(&fi2s_lock,flags);
	return 0;
}


static int fi2s_prepare_for_output(int dev, int bsize, int bcount)
{
    fi2s_info      *dev_info = (fi2s_info *) audio_devs[dev]->devc;
    unsigned long   flags;
    //printk("fi2s_prepare_for_output\n");
    spin_lock_irqsave(&fi2s_lock,flags);   
    fi2s_init_codec_play(dev_info->io_base);
    audio_devs[dev]->dmap_out->flags |= DMA_NODMA;        
	fi2s_halt_output(dev);
	spin_unlock_irqrestore(&fi2s_lock,flags);
	return 0;
}


static void fi2s_init_hw(int dev)
{
    fi2s_info          *dev_info = (fi2s_info *) audio_devs[dev]->devc;
    

	/* set control 0 */
	*(volatile unsigned int *)(dev_info->io_base+SSP_CONTROL0)=0x311c;
	/* clear control 1 */
	*(volatile unsigned int *)(dev_info->io_base+SSP_CONTROL1)=0;
	/* int control */
	*(volatile unsigned int *)(dev_info->io_base+SSP_INT_CONTROL)=SSP_FIFO_THOD;
    
    /* SSP state machine reset */
    *(volatile unsigned int *)(dev_info->io_base+SSP_CONTROL2)=0x40;
        
}

static int __init fi2s_init(void)
{
	fi2s_info      *dev_info = &fi2s_dev_info[nr_fi2s_devs];
	fi2s_port_info *portc = NULL;
    char            dev_name[100];
    int             my_dev,i;
    
    printk(KERN_INFO "Faraday A321 I2S OSS driver(2004/12/2)\n");
    spin_lock_init(&fi2s_lock);
    
    /* address info */
    fi2s_cfg.io_base=CPE_A321_SSP_VA_BASE;
    fi2s_cfg.irq= IRQ_A321_I2S;
	fi2s_cfg.dma=-1;			// playback
	fi2s_cfg.dma2=-1;			// record	
	
	/* device info */
    dev_info->irq = fi2s_cfg.irq;
    dev_info->io_base= fi2s_cfg.io_base;
    dev_info->io_base_phys=CPE_A321_SSP_BASE;
    dev_info->open_mode = 0;

    sprintf(dev_name,"A321 fi2s Sound System(I2S module)");
    conf_printf2(dev_name, dev_info->io_base, dev_info->irq, -1, -1);
    
    portc = (fi2s_port_info *) kmalloc(sizeof(fi2s_port_info), GFP_KERNEL);
    if(portc==NULL)
		return -1;

    if((my_dev = sound_install_audiodrv(AUDIO_DRIVER_VERSION,dev_name,&fi2s_a321_audio_driver,
                                        sizeof(struct audio_driver),DMA_NODMA,AFMT_S16_LE|AFMT_S8,
                                        dev_info,-1,-1)) < 0)
    {
            kfree(portc);
            portc=NULL;
            return -1;
    }
    printk(KERN_INFO "dsp%d: Faraday A321 I2S OSS driver(2004/12/2)\n",my_dev);
    audio_devs[my_dev]->portc = portc;
    audio_devs[my_dev]->mixer_dev = -1;
    audio_devs[my_dev]->d->owner = THIS_MODULE;

    memset((char *) portc, 0, sizeof(*portc));
    nr_fi2s_devs++;
    
    fi2s_init_hw(my_dev);
    dev_info->dev_no = my_dev;

    strcpy(dev_info->name,"FI2S A321 APB DMA");
    cpe_int_set_irq(IRQ_A321_APB_DMA, LEVEL, H_ACTIVE);
    if(request_irq(IRQ_A321_APB_DMA,dma_interrupt_handler,SA_INTERRUPT,dev_info->name,(void *)my_dev)<0)
    {
		printk(KERN_WARNING "Faraday I2S: Unable to allocate IRQ %d\n",IRQ_CPE_APB_DMA);
		kfree(portc);
		return -1;
	}

//////////////////////////////////////////////////////
/*  to avoid unnecessary interrupt for speeding up 
	strcpy(dev_info->name,"FI2S(A321)");
	cpe_int_set_irq(dev_info->irq,LEVEL,H_ACTIVE);
    if(request_irq(dev_info->irq,fi2s_a321_interrupt_handler,SA_INTERRUPT,dev_info->name,(void *)my_dev)<0)
    {
		printk(KERN_WARNING "Faraday I2S: Unable to allocate IRQ %d\n",dev_info->irq);
		kfree(portc);
		return -1;
	}	
*/
////////////////////////////////////////////////////////

	// setup out parameter
    out_parm.dest=dev_info->io_base_phys+SSP_DATA;
    out_parm.width=APBDMA_WIDTH_16BIT;
    out_parm.req_num=0; //needn't request/grant number
    out_parm.width=APBDMA_WIDTH_16BIT;
    out_parm.sctl=APBDMA_CTL_INC2;
    out_parm.dctl=APBDMA_CTL_FIX;
    out_parm.stype=APBDMA_TYPE_AHB;
    out_parm.dtype=APBDMA_TYPE_APB;
    out_parm.burst=0;
    out_parm.req_num=6;
    out_parm.irq=APBDMA_TRIGGER_IRQ;

	//allocate buffer 
	va_start=(unsigned int)consistent_alloc(GFP_DMA|GFP_KERNEL,FI2S_BUFFER_SIZE*FI2S_BUFFER_NUM,(dma_addr_t *)&pa_start);
	
	for(i=0;i<FI2S_BUFFER_NUM;i++)
	{
		fi2s_buf[i]=(unsigned char *)(va_start+(i*FI2S_BUFFER_SIZE));
		fi2s_buf_phys[i]=pa_start+(i*FI2S_BUFFER_SIZE);
		//printk("fi2s_buf[%d]=0x%x phys=0x%x\n",i,(unsigned int)fi2s_buf[i],(unsigned int)fi2s_buf_phys[i]);
	}
	memset(fi2s_buf[0],0,FI2S_BUFFER_SIZE);
	fi2s_cfg.slots[0]=my_dev;
	return 0;
}


static void fi2s_unload(struct address_info *hw_config)
{
    int             devid=hw_config->slots[0];
    fi2s_port_info *portc;
    
    portc=(fi2s_port_info *)audio_devs[devid]->portc;
    kfree(portc);
    
	consistent_free((void *)va_start,FI2S_BUFFER_SIZE,(dma_addr_t)pa_start);

    sound_unload_audiodev(hw_config->slots[0]);
    release_region(hw_config->io_base, 4);
}

static void __exit cleanup_fi2s(void)
{
    struct address_info *hw_config=&fi2s_cfg;
    fi2s_unload(hw_config);
    sound_unload_audiodev(hw_config->slots[0]);    
    release_region(hw_config->io_base, 4);
}

module_init(fi2s_init);
module_exit(cleanup_fi2s);

MODULE_LICENSE("GPL");

