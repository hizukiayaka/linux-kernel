/*
 * sound/soc/samsung/idma.c
 *
 * Copyright (c) 2011 Samsung Electronics Co., Ltd.
 *		http://www.samsung.com
 *
 * I2S0's Internal DMA driver
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 */
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/dma-mapping.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/soc.h>

#include "i2s.h"
#include "idma.h"
#include "dma.h"
#include "i2s-regs.h"

#define ST_RUNNING		(1<<0)
#define ST_OPENED		(1<<1)
//#define pr_debug(fmt...) printk(fmt)

static const struct snd_pcm_hardware idma_hardware = {
	.info = SNDRV_PCM_INFO_INTERLEAVED |
		    SNDRV_PCM_INFO_BLOCK_TRANSFER |
		    SNDRV_PCM_INFO_MMAP |
		    SNDRV_PCM_INFO_MMAP_VALID |
		    SNDRV_PCM_INFO_PAUSE |
		    SNDRV_PCM_INFO_RESUME,
	.formats = SNDRV_PCM_FMTBIT_S16_LE |
		    SNDRV_PCM_FMTBIT_U16_LE |
		    SNDRV_PCM_FMTBIT_S24_LE |
		    SNDRV_PCM_FMTBIT_U24_LE |
		    SNDRV_PCM_FMTBIT_U8 |
		    SNDRV_PCM_FMTBIT_S8,
	.channels_min = 1,
	.channels_max = 2,
	.buffer_bytes_max = 128 * 1024,
	.period_bytes_min = PAGE_SIZE,
	.period_bytes_max = PAGE_SIZE * 2,
	.periods_min = 2,
	.periods_max = 128,
	.fifo_size = 32,
};

struct idma_ctrl {
	spinlock_t	lock;
	int		state;
	dma_addr_t	start;
	dma_addr_t	pos;
	dma_addr_t	end;
	dma_addr_t	period;
	dma_addr_t	periodsz;
	void		*token;
	void		(*cb)(void *dt, int bytes_xfer);
};

static struct idma_info {
	struct idma_ctrl *prtd;
	spinlock_t	lock;
	void		 __iomem  *regs;
	int		trigger_stat;
//	dma_addr_t	lp_tx_addr;
} idma;

static void idma_getpos(dma_addr_t *res, struct snd_pcm_substream *substream)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct idma_ctrl *prtd = runtime->private_data;
	u32 maxcnt = runtime->buffer_size;
	u32 trncnt = readl(idma.regs + I2STRNCNT) & 0xffffff;

	/*
	 * When bus is busy, I2STRNCNT could be increased without dma transfer
	 * in rare cases.
	 */
	if (prtd->state == ST_RUNNING)
		trncnt = trncnt == 0 ? maxcnt - 1 : trncnt - 1;

	*res = frames_to_bytes(runtime, trncnt);
}

static int idma_enqueue(struct snd_pcm_substream *substream)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct idma_ctrl *prtd = substream->runtime->private_data;
	u32 val = prtd->start;

	spin_lock(&prtd->lock);
	prtd->token = (void *) substream;
	spin_unlock(&prtd->lock);
	
	/* Start address0 of I2S internal DMA operation. */
	writel(val, idma.regs + I2SSTR0);

	/* Internal DMA Level0 Interrupt Address */
	val = prtd->start + prtd->periodsz;
	writel(val, idma.regs + I2SLVL0ADDR);

	/*
	 * Transfer block size for I2S internal DMA.
	 * Should decide transfer size before start dma operation
	 */
	val = readl(idma.regs + I2SSIZE);
	val &= ~(I2SSIZE_TRNMSK << I2SSIZE_SHIFT);
	val |= (((runtime->dma_bytes >> 2) &
			I2SSIZE_TRNMSK) << I2SSIZE_SHIFT);
	writel(val, idma.regs + I2SSIZE);
	return 0;
}

static void idma_setcallbk(struct snd_pcm_substream *substream,
				void (*cb)(void *, int))
{
	struct idma_ctrl *prtd = substream->runtime->private_data;

	spin_lock(&prtd->lock);
	prtd->cb = cb;
	spin_unlock(&prtd->lock);

	pr_debug("%s:%d dma_period=%x\n", __func__, __LINE__, prtd->periodsz);
}

static void idma_ctrl(int op)
{
	u32 val = readl(idma.regs + I2SAHB);

	spin_lock(&idma.lock);

	switch (op) {
	case LPAM_DMA_START:
		val |= (AHB_INTENLVL0 | AHB_DMAEN);
		break;
	case LPAM_DMA_STOP:
		val &= ~(AHB_INTENLVL0 | AHB_DMAEN);
		break;
	default:
		spin_unlock(&idma.lock);
		return;
	}

	writel(val, idma.regs + I2SAHB);
	spin_unlock(&idma.lock);
}

static void idma_done(void *id, int bytes_xfer)
{
	struct snd_pcm_substream *substream = id;
	struct idma_ctrl *prtd = substream->runtime->private_data;

	if (prtd && (prtd->state & ST_RUNNING))
		snd_pcm_period_elapsed(substream);
}

static int idma_hw_params(struct snd_pcm_substream *substream,
				struct snd_pcm_hw_params *params)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct idma_ctrl *prtd = substream->runtime->private_data;
	u32 mod = readl(idma.regs + I2SMOD);
	u32 ahb = readl(idma.regs + I2SAHB);

	ahb |= (AHB_DMARLD | AHB_INTMASK);
	
	writel(ahb, idma.regs + I2SAHB);

	snd_pcm_set_runtime_buffer(substream, &substream->dma_buffer);
	runtime->dma_bytes = params_buffer_bytes(params);
	memset(runtime->dma_area, 0, runtime->dma_bytes);

	prtd->start = prtd->pos = runtime->dma_addr;
	prtd->period = params_periods(params);
	prtd->periodsz = params_period_bytes(params);
	prtd->end = prtd->start + runtime->dma_bytes;

	idma_setcallbk(substream, idma_done);

	pr_debug("DmaAddr=@%x Total=%dbytes PrdSz=%d #Prds=%d dma_area=0x%x\n",
			prtd->start, runtime->dma_bytes, prtd->periodsz,
			prtd->period, (unsigned int)runtime->dma_area);

	return 0;
}

static int idma_hw_free(struct snd_pcm_substream *substream)
{
	snd_pcm_set_runtime_buffer(substream, NULL);

	return 0;
}

static int idma_prepare(struct snd_pcm_substream *substream)
{
	struct idma_ctrl *prtd = substream->runtime->private_data;

	pr_debug("Entered %s\n", __func__);

	prtd->pos = prtd->start;

	/* flush the DMA channel */
	idma_ctrl(LPAM_DMA_STOP);
	idma_enqueue(substream);

	return 0;
}

static int idma_trigger(struct snd_pcm_substream *substream, int cmd)
{
	struct idma_ctrl *prtd = substream->runtime->private_data;
	int ret = 0;

	pr_debug("Entered %s\n", __func__);

	spin_lock(&prtd->lock);

	switch (cmd) {
	case SNDRV_PCM_TRIGGER_RESUME:
	case SNDRV_PCM_TRIGGER_START:
	case SNDRV_PCM_TRIGGER_PAUSE_RELEASE:
		prtd->state |= ST_RUNNING;
		idma.trigger_stat = LPAM_DMA_START;
		idma_ctrl(LPAM_DMA_START);
		break;

	case SNDRV_PCM_TRIGGER_SUSPEND:
	case SNDRV_PCM_TRIGGER_STOP:
	case SNDRV_PCM_TRIGGER_PAUSE_PUSH:
		prtd->state &= ~ST_RUNNING;
		idma.trigger_stat = LPAM_DMA_STOP;
		idma_ctrl(LPAM_DMA_STOP);
		break;

	default:
		ret = -EINVAL;
		break;
	}

	spin_unlock(&prtd->lock);

	return ret;
}

static snd_pcm_uframes_t
	idma_pointer(struct snd_pcm_substream *substream)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct idma_ctrl *prtd = runtime->private_data;
	unsigned long flags;
	dma_addr_t res;
	spin_lock_irqsave(&prtd->lock, flags);

	idma_getpos(&res, substream);

	spin_unlock_irqrestore(&prtd->lock, flags);

	if (res >= snd_pcm_lib_buffer_bytes(substream)) {
		if (res == snd_pcm_lib_buffer_bytes(substream))
			res = 0;
	}

	return bytes_to_frames(substream->runtime, res);
}

static int idma_mmap(struct snd_pcm_substream *substream,
	struct vm_area_struct *vma)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	unsigned long size, offset;
	int ret;

	pr_debug("Entered %s\n", __func__);

	/* From snd_pcm_lib_mmap_iomem */
	vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);
	vma->vm_flags |= VM_IO;
	size = vma->vm_end - vma->vm_start;
	offset = vma->vm_pgoff << PAGE_SHIFT;
	ret = io_remap_pfn_range(vma, vma->vm_start,
			(runtime->dma_addr + offset) >> PAGE_SHIFT,
			size, vma->vm_page_prot);

	return ret;
}

static irqreturn_t iis_irq(int irqno, void *dev_id)
{
	struct idma_ctrl *prtd = idma.prtd;
	u32 iiscon = readl(idma.regs + I2SCON);
	u32 iisahb = readl(idma.regs + I2SAHB);
	u32 addr = 0;
	u32 val = 0;

	/* Check RX Overflow INT */
	if (iiscon & CON_FRXOFSTATUS) {
		pr_err("RX overflow occurs!! I2SCON[0x%08x])\n", iiscon);
		iiscon |= CON_FRXOFSTATUS;
		writel(iiscon, idma.regs + I2SCON);
	}

	/* Check TX_P Underrun INT */
	if (iiscon & CON_FTXURSTATUS) {
		pr_err("Tx_P underrun occurs!! I2SCON[0x%08x])\n", iiscon);
		iiscon |= CON_FTXURSTATUS;
		writel(iiscon, idma.regs + I2SCON);
	}

	/* Check TX_S Underrun INT */
	if (iiscon & CON_FTXSURSTAT) {
		pr_err("Tx_S underrun occurs!! I2SCON[0x%08x])\n", iiscon);
		iiscon |= CON_FTXSURSTAT;
		writel(iiscon, idma.regs + I2SCON);
	}

	/* Check I2SAHB Level[0~3] INT */
	if (iisahb & AHB_LVL0INT)
		val = AHB_CLRLVL0INT;
	else if (iisahb & AHB_LVL1INT)
		val = AHB_CLRLVL1INT;
	else if (iisahb & AHB_LVL2INT)
		val = AHB_CLRLVL2INT;
	else if (iisahb & AHB_LVL3INT)
		val = AHB_CLRLVL3INT;

	pr_debug("--iis interrupt val=0x%x-\n",val);
	if (val) {
		iisahb |= val;
		writel(iisahb, idma.regs + I2SAHB);

		addr = readl(idma.regs + I2SLVL0ADDR);
		addr += prtd->periodsz;

		if (addr >= prtd->end)
			addr = prtd->start;

		writel(addr, idma.regs + I2SLVL0ADDR);

		/* Finished dma transfer ? */
		if (iisahb & AHB_LVLINTMASK) {
			if (prtd->cb)
				prtd->cb(prtd->token, prtd->periodsz);
		}
	}

	return IRQ_HANDLED;
}

static int idma_open(struct snd_pcm_substream *substream)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct idma_ctrl *prtd;

	snd_soc_set_runtime_hwparams(substream, &idma_hardware);
	/* Clear AHB register */
	writel(0, idma.regs + I2SAHB);

	prtd = kzalloc(sizeof(struct idma_ctrl), GFP_KERNEL);
	if (prtd == NULL)
		return -ENOMEM;
	idma.prtd = prtd;
	
	spin_lock_init(&prtd->lock);

	runtime->private_data = prtd;

	return 0;
}

static int idma_close(struct snd_pcm_substream *substream)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct idma_ctrl *prtd = runtime->private_data;


	if (!prtd)
		pr_err("idma_close called with prtd == NULL\n");

	kfree(prtd);

	return 0;
}

static struct snd_pcm_ops idma_ops = {
	.open		= idma_open,
	.close		= idma_close,
	.ioctl		= snd_pcm_lib_ioctl,
	.trigger	= idma_trigger,
	.pointer	= idma_pointer,
	.mmap		= idma_mmap,
	.hw_params	= idma_hw_params,
	.hw_free	= idma_hw_free,
	.prepare	= idma_prepare,
};

static void idma_free(struct snd_pcm *pcm)
{
	struct snd_pcm_substream *substream;
	struct snd_dma_buffer *buf;

	pr_debug("Entered %s\n", __func__);

	substream = pcm->streams[SNDRV_PCM_STREAM_PLAYBACK].substream;
	if (!substream)
		return;

	buf = &substream->dma_buffer;
	if (!buf->area)
		return;

	iounmap(buf->area);

	buf->area = NULL;
	buf->addr = 0;
}

static int preallocate_idma_buffer(struct snd_pcm *pcm, int stream)
{
	struct snd_pcm_substream *substream = pcm->streams[stream].substream;
	struct snd_dma_buffer *buf = &substream->dma_buffer;

	pr_debug("Entered %s\n", __func__);
	buf->dev.dev = pcm->card->dev;
	buf->private_data = NULL;

	/* Assign PCM buffer pointers */
	buf->dev.type = SNDRV_DMA_TYPE_CONTINUOUS;
	buf->addr = LP_TXBUFF_ADDR;
	buf->bytes = idma_hardware.buffer_bytes_max;
	buf->area = (unsigned char *)ioremap(buf->addr, buf->bytes);
	pr_info("%s:  VA-%p  PA-%X  %ubytes\n",
			__func__, buf->area, buf->addr, buf->bytes);

	return 0;
}

static u64 idma_mask = DMA_BIT_MASK(32);

static int idma_new(struct snd_soc_pcm_runtime *rtd)
{
	struct snd_card *card = rtd->card->snd_card;
	struct snd_pcm *pcm = rtd->pcm;
	int ret = 0;

	if (!card->dev->dma_mask)
		card->dev->dma_mask = &idma_mask;
	if (!card->dev->coherent_dma_mask)
		card->dev->coherent_dma_mask = DMA_BIT_MASK(32);

	if (pcm->streams[SNDRV_PCM_STREAM_PLAYBACK].substream) {
		ret = preallocate_idma_buffer(pcm,
				SNDRV_PCM_STREAM_PLAYBACK);
	}

	return ret;
}

void idma_init(void *regs)
{
	spin_lock_init(&idma.lock);
	idma.regs = regs;
}

#ifdef CONFIG_SND_SAMSUNG_RP
int idma_irq_callback(void)
{
	u32 iisahb;
	int ret = 0;

	iisahb = readl(idma.regs + I2SAHB);

	if (iisahb & AHB_LVL0INT) {
		iisahb |= AHB_CLRLVL0INT;
		ret = 1;
	}

	if (iisahb & AHB_LVL1INT) {
		iisahb |= AHB_CLRLVL1INT;
		ret = 1;
	}

	if (ret)
		writel(iisahb, idma.regs + I2SAHB);

	return ret;
}
EXPORT_SYMBOL(idma_irq_callback);

void idma_stop(void)
{
	u32 val;

	val  = readl(idma.regs + I2SAHB);
	val &= ~AHB_DMARLD;
	val |= AHB_DMA_STRADDRRST;
	val &= ~AHB_DMAEN;
	val &= ~(AHB_LVL0INT | AHB_LVL1INT);
	val |= AHB_CLRLVL0INT | AHB_CLRLVL1INT;
	writel(val, idma.regs + I2SAHB);

	writel(0, idma.regs + I2SAHB);
	writel(0x00000000, idma.regs + I2SLVL0ADDR);
	writel(0x00000000, idma.regs + I2SLVL1ADDR);
}
EXPORT_SYMBOL(idma_stop);
#endif

static struct snd_soc_platform_driver asoc_idma_platform = {
	.ops = &idma_ops,
	.pcm_new = idma_new,
	.pcm_free = idma_free,
};

static int __devinit asoc_idma_platform_probe(struct platform_device *pdev)
{
	int ret;
	
	ret = request_irq(IRQ_I2S0, iis_irq, 0, "i2s", NULL);
	if (ret < 0) {
		pr_err("fail to request i2s irq , ret = %d\n", ret);
		return ret;
	}

	return snd_soc_register_platform(&pdev->dev, &asoc_idma_platform);
}

static int __devexit asoc_idma_platform_remove(struct platform_device *pdev)
{
	free_irq(IRQ_I2S0, idma.prtd);
	snd_soc_unregister_platform(&pdev->dev);
	return 0;
}

static struct platform_driver asoc_idma_driver = {
	.driver = {
		.name = "samsung-idma",
		.owner = THIS_MODULE,
	},

	.probe = asoc_idma_platform_probe,
	.remove = __devexit_p(asoc_idma_platform_remove),
};

module_platform_driver(asoc_idma_driver);

MODULE_AUTHOR("Jaswinder Singh, <jassisinghbrar@gmail.com>");
MODULE_DESCRIPTION("Samsung ASoC IDMA Driver");
MODULE_LICENSE("GPL");
