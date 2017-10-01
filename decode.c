#include <stdio.h>
#include <ttLibC/frame/video/bgr.h>
#include <ttLibC/util/opencvUtil.h>
#include <ttLibC/resampler/imageResampler.h>
#include <ttLibC/util/ioUtil.h>

#include <aom/aomdx.h>
#include <aom/aom_decoder.h>
#include <aom/aom_image.h>

int main() {
  puts("decodeTest");
  FILE *fp = fopen("../av1test.bin", "rb");

	ttLibC_CvWindow *dec_win  = ttLibC_CvWindow_make("target");

  aom_codec_err_t res = AOM_CODEC_OK;
  aom_codec_ctx_t decodec;
  ttLibC_Yuv420 *dyuv, *y = NULL;
  ttLibC_Bgr *dbgr, *b = NULL;

  res = aom_codec_dec_init(&decodec, aom_codec_av1_dx(), NULL, 0);

  do {
    if(fp == NULL) {
      break;
    }
    if(res != AOM_CODEC_OK) {
      puts("error on aom process.");
      break;
    }
    uint8_t buffer[1310720];
    uint32_t be_size;
    uint32_t size;
    size_t read_size;
    read_size = fread(&be_size, 1, 4, fp);
    size = be_uint32_t(be_size);
    if(size > 1310720) {
      puts("too big frame is found.");
      break;
    }
    read_size = fread(buffer, 1, size, fp);
    if(read_size != size) {
      break;
    }
    aom_codec_decode(&decodec, (const uint8_t *)buffer, size, NULL, 0);
    if(res != AOM_CODEC_OK) {
      puts("decode failed.");
      break;
    }
    aom_codec_iter_t iter = NULL;
    aom_image_t *img;
    while((img = aom_codec_get_frame(&decodec, &iter)) != NULL) {
      y = ttLibC_Yuv420_make(dyuv, Yuv420Type_planar,
      img->d_w, img->d_h, NULL, 0, 
      img->planes[0], img->stride[0],
      img->planes[1], img->stride[1],
      img->planes[2], img->stride[2],
      true, 0, 1000);
      if(y == NULL) {
        puts("failed to make yuv frame.");
        break;
      }
      dyuv = y;
      b = ttLibC_ImageResampler_makeBgrFromYuv420(dbgr, BgrType_bgr, dyuv);
      if(b == NULL) {
        puts("failed to make bgr frame from yuv.");
        break;
      }
      dbgr = b;
      ttLibC_CvWindow_showBgr(dec_win, dbgr);
      uint8_t key = ttLibC_CvWindow_waitForKeyInput(1);
      if(key == Keychar_Esc) {
        puts("escape is pushed.");
        return false;;
      }
    }
  } while(!feof(fp));
  if(fp) {
    fclose(fp);
  }
  aom_codec_destroy(&decodec);
	ttLibC_CvWindow_close(&dec_win);
	ttLibC_Bgr_close(&dbgr);
	ttLibC_Yuv420_close(&dyuv);
  return 0;
}