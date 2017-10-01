#include <stdio.h>
#include <ttLibC/container/mp4.h>
#include <ttLibC/decoder/avcodecDecoder.h>
#include <ttLibC/frame/video/bgr.h>
#include <ttLibC/frame/video/h264.h>
#include <ttLibC/resampler/imageResampler.h>
#include <ttLibC/util/ioUtil.h>

#include <aom/aomcx.h>
#include <aom/aom_encoder.h>
#include <aom/aom_image.h>

typedef struct {
  ttLibC_AvcodecDecoder *decoder;
  ttLibC_Bgr *dbgr;

  bool isReady;
  aom_codec_ctx_t codec;
  aom_codec_enc_cfg_t cfg;
  int frame_count;
  aom_image_t raw;

  FILE *fp;
} data_t;

bool decodeCallback(void *ptr, ttLibC_Frame *frame) {
  data_t *data = (data_t *)ptr;
  if(frame->type != frameType_yuv420) {
    puts("only yuv420 works.");
    return false;
  }
  ttLibC_Yuv420 *yuv = (ttLibC_Yuv420 *)frame;
  aom_codec_err_t res = AOM_CODEC_OK;
  if(!data->isReady) {
    puts("initialize aom.");
    res = aom_codec_enc_config_default(aom_codec_av1_cx(), &data->cfg, 0);
    if(res != AOM_CODEC_OK) {
      puts("failed to initialize aom.");
      return false;
    }
    data->cfg.g_w = yuv->inherit_super.width;
    data->cfg.g_h = yuv->inherit_super.height;
		data->cfg.rc_max_quantizer = 31;
		data->cfg.rc_min_quantizer = 11;
		data->cfg.g_timebase.num = 1;
		data->cfg.g_timebase.den = 60;
		data->cfg.rc_target_bitrate = 500; // 500kbps
		data->cfg.g_pass = AOM_RC_ONE_PASS; // onepassを強制でいれてもだめだった・・どうすりゃいいの？
		data->cfg.g_threads = 4;
		res = aom_codec_enc_init(&data->codec, aom_codec_av1_cx(), &data->cfg, 0);
		if(res != AOM_CODEC_OK) {
			puts("filed to initialize aom. part2.");
      return false;
		}
		aom_codec_control(&data->codec, AOME_SET_CPUUSED, 8);
  	aom_img_alloc(&data->raw, AOM_IMG_FMT_I420, yuv->inherit_super.width, yuv->inherit_super.height, 1);
    data->isReady = true;
  }
  aom_codec_iter_t iter = NULL;
  const aom_codec_cx_pkt_t *pkt = NULL;
  data->raw.planes[0] = yuv->y_data;
  data->raw.planes[1] = yuv->u_data;
  data->raw.planes[2] = yuv->v_data;
  data->raw.stride[0] = yuv->y_stride;
  data->raw.stride[1] = yuv->u_stride;
  data->raw.stride[2] = yuv->v_stride;
  res = aom_codec_encode(
    &data->codec, &data->raw, data->frame_count ++, 1, 0, AOM_DL_GOOD_QUALITY
  );
  if(res != AOM_CODEC_OK) {
    puts("failed to encode.");
    return false;
  }
  while((pkt = aom_codec_get_cx_data(&data->codec, &iter)) != NULL) {
    printf("encoded flag:%x sz:%zu pts:%llu\n", pkt->data.frame.flags, pkt->data.frame.sz, pkt->data.frame.pts);
    // size:4byte (big endian)
    // frame:any byte
    if(data->fp) {
      uint32_t size = (uint32_t)pkt->data.frame.sz;
      uint32_t b_size = be_uint32_t(size);
      fwrite(&b_size, 1, 4, data->fp);
      fwrite(pkt->data.frame.buf, 1, size, data->fp);
    }
  }
  return true;
}

bool frameCallback(void *ptr, ttLibC_Frame *frame) {
  if(frame->type == frameType_h264) {
    data_t *data = (data_t *)ptr;
    ttLibC_H264 *h264 = (ttLibC_H264 *)frame;
    if(h264->type == H264Type_unknown) {
      return true;
    }
    if(data->decoder == NULL) {
      data->decoder = ttLibC_AvcodecVideoDecoder_make(frameType_h264, h264->inherit_super.width, h264->inherit_super.height);
    }
    return ttLibC_AvcodecDecoder_decode(data->decoder, frame, decodeCallback, ptr);
  }
  return true;
}

bool mp4ReadCallback(void *ptr, ttLibC_Mp4 *mp4) {
  return ttLibC_Mp4_getFrame(mp4, frameCallback, ptr);
}

int main() {
  puts("encodeTest");
  FILE *fp = fopen("../test.h264.aac.mp4", "rb");
  ttLibC_Mp4Reader *reader = ttLibC_Mp4Reader_make();

  data_t data;
  data.decoder = NULL;
  data.dbgr = NULL;

  data.isReady = false;

  data.fp = fopen("../av1test.bin", "wb");

  do {
    uint8_t buffer[65536];
    if(fp == NULL) {
      puts("fp is null");
      break;
    }
    size_t read_size = fread(buffer, 1, 65536, fp);
    if(!ttLibC_Mp4Reader_read(reader, buffer, read_size, mp4ReadCallback, &data)) {
      puts("error on mp4Reader.");
      break;
    }
  } while(!feof(fp));
  if(fp) {
    fclose(fp);
  }
  if(data.fp) {
    fclose(data.fp);
  }
  if(data.isReady) {
    aom_img_free(&data.raw);
    aom_codec_destroy(&data.codec);
  }
  ttLibC_Bgr_close(&data.dbgr);
  ttLibC_AvcodecDecoder_close(&data.decoder);
  ttLibC_Mp4Reader_close(&reader);
  return 0;
}