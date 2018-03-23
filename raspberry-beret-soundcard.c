/*
 * ASoC Driver for RaspberryBeretSoundcard.
 *
 * Author:	Colin Zyskowski <colin.zyskowski@gmail.com>
 *		Copyright 2017
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 */

#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/types.h>
#include <linux/gpio/consumer.h>
#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/soc.h>
#include <linux/clk.h>
#include <sound/control.h>

static unsigned int raspberry_beret_rate;

static const unsigned int raspberry_beret_rates[] = {
	176400, 96000, 88200, 48000, 44100, 24000, 22050 
};

static struct snd_pcm_hw_constraint_list raspberry_beret_constraints = {
	.list = raspberry_beret_rates,
	.count = ARRAY_SIZE(raspberry_beret_rates),
};

static int raspberry_beret_init(struct snd_soc_pcm_runtime *rtd)
{
	return snd_soc_dai_set_tdm_slot(rtd->cpu_dai, 0x03, 0x0c, 8, 32);
	//return snd_soc_dai_set_bclk_ratio(rtd->cpu_dai, 256);
}

static int raspberry_beret_startup(struct snd_pcm_substream *substream)
{
	snd_pcm_hw_constraint_list(substream->runtime, 0,
				SNDRV_PCM_HW_PARAM_RATE,
				&raspberry_beret_constraints);
	return 0;
}

static void raspberry_beret_shutdown(struct snd_pcm_substream *substream)
{
	
}

static int raspberry_beret_hw_params(struct snd_pcm_substream *substream,
				       struct snd_pcm_hw_params *params)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;

	raspberry_beret_rate = params_rate(params);

	// Set the correct sysclock for the codec
	switch (raspberry_beret_rate) {
	
	case 96000:
	case 48000:
		return snd_soc_dai_set_sysclk(rtd->codec_dai, 0, 12800000,
									0);
		break;
	case 24000:
		return snd_soc_dai_set_sysclk(rtd->codec_dai, 0, 12800000/2,
									0);
		break;
	case 176400:
	case 88200:
	case 44100:
		return snd_soc_dai_set_sysclk(rtd->codec_dai, 0, 22579200,
									0);
		break;
	case 22050:
		return snd_soc_dai_set_sysclk(rtd->codec_dai, 0, 22579200/2,
									0);
		break;
	default:
		return -EINVAL;
	}


}

/* machine stream operations */
static struct snd_soc_ops raspberry_beret_ops = {
	.startup	= raspberry_beret_startup,
	.shutdown	= raspberry_beret_shutdown,
	.hw_params 	= raspberry_beret_hw_params,
};

static struct snd_soc_dai_link raspberry_beret_dai[] = {
{
	.name		= "raspberry beret",
	.stream_name	= "RaspberryBeret-HiFi",
	//.cpu_dai_name	= "bcm2708-i2s.0",
	.codec_dai_name	= "cs42448",
	//.platform_name	= "bcm2708-i2s.0",
	.dai_fmt	= SND_SOC_DAIFMT_DSP_A | SND_SOC_DAIFMT_NB_NF |
				SND_SOC_DAIFMT_CBS_CFS,
	.ops		= &raspberry_beret_ops,
	.init		= raspberry_beret_init,
},
};

/* audio machine driver */
static struct snd_soc_card snd_soc_raspberry_beret = {
	.name         = "raspberry-beret-soundcard",
	.owner        = THIS_MODULE,
	.dai_link     = raspberry_beret_dai,
	.num_links    = ARRAY_SIZE(raspberry_beret_dai),
};

static int raspberry_beret_probe(struct platform_device *pdev)
{
	struct snd_soc_card *card = &snd_soc_raspberry_beret;
	int ret = 0;

	card->dev = &pdev->dev;
	
	if (pdev->dev.of_node) {
		
		struct snd_soc_dai_link *dai = &raspberry_beret_dai[0];
		struct device_node *i2s_node =
					of_parse_phandle(pdev->dev.of_node,
							"i2s-controller", 0);
		struct device_node *codec_node =
					of_parse_phandle(pdev->dev.of_node,
								"codec", 0);


		if (i2s_node) {
			dai->cpu_dai_name = NULL;
			dai->cpu_of_node = i2s_node;
			dai->platform_name = NULL;
			dai->platform_of_node = i2s_node;
			dai->codec_name = NULL;
			dai->codec_of_node = codec_node;

		}
	}
	
	ret = snd_soc_register_card(card);
	if (ret && ret != -EPROBE_DEFER)
		dev_err(&pdev->dev, "snd_soc_register_card() failed: %d\n", ret);

	return ret;
}

static int raspberry_beret_remove(struct platform_device *pdev)
{
	struct snd_soc_card *card = platform_get_drvdata(pdev);
	return snd_soc_unregister_card(card);

}

static const struct of_device_id raspberry_beret_of_match[] = {
	{ .compatible = "raspberry-beret,raspberry-beret-soundcard", },
	{},
};
MODULE_DEVICE_TABLE(of, raspberry_beret_of_match);

static struct platform_driver raspberry_beret_driver = {
        .driver = {
                .name   = "raspberry-beret",
                .owner  = THIS_MODULE,
                .of_match_table = raspberry_beret_of_match,
        },
        .probe          = raspberry_beret_probe,
        .remove         = raspberry_beret_remove,
};

module_platform_driver(raspberry_beret_driver);

MODULE_AUTHOR("Colin Zyskowski <colin.zyskowski@gmail.com>");
MODULE_DESCRIPTION("ASoC Driver for Raspberry-Beret");
MODULE_LICENSE("GPL v2");
