#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include <jni.h>

#include "libretro.h"

static uint32_t *frame_buf;
static struct retro_log_callback logging;
static retro_log_printf_t log_cb;

static retro_video_refresh_t video_cb;
static retro_audio_sample_t audio_cb;
static retro_audio_sample_batch_t audio_batch_cb;
static retro_environment_t environ_cb;
static retro_input_poll_t input_poll_cb;
static retro_input_state_t input_state_cb;

static void fallback_log(enum retro_log_level level, const char *fmt, ...) {
    (void) level;
    va_list va;
    va_start(va, fmt);
    vfprintf(stderr, fmt, va);
    va_end(va);
}
JavaVM *vm;
JNIEnv *env;
JavaVMInitArgs vm_args;
jint res;
jclass cls;
bool java_init(const char* path) {

    jmethodID mid;
    jstring jstr;
    jobjectArray main_args;

    JavaVMOption options[1];
    //strcpy(options[0], "blah");

    vm_args.version = JNI_VERSION_1_8;
    vm_args.nOptions = 1;

   char str1[] = "-Djava.class.path=";
   // char *optionString = strcat(dest,path);

    size_t len1 = strlen(str1), len2 = strlen(path);
    char *optionString = (char*) malloc(len1 + len2 + 1);

    memcpy(optionString, str1, len1);
    memcpy(optionString+len1, path, len2+1);


    options[0].optionString = optionString;

    vm_args.options = options;
    res = JNI_CreateJavaVM(&vm, (void **) &env, &vm_args);
    if (res != JNI_OK) {
        printf("Failed to create Java VM\n");
        return false;
    }

    cls = (*env)->FindClass(env, "LibRetro");
    if (cls == NULL) {
        printf("Failed to find Main class\n");
        return false;
    }
    printf("create Java VM\n");
    mid = (*env)->GetStaticMethodID(env, cls, "main", "([Ljava/lang/String;)V");
    if (mid == NULL) {
        printf("Failed to find main function\n");
        return false;
    }

    jstr = (*env)->NewStringUTF(env, "");
    main_args = (*env)->NewObjectArray(env, 1, (*env)->FindClass(env, "java/lang/String"), jstr);
    (*env)->CallStaticVoidMethod(env, cls, mid, main_args);

    return true;
}

int java_render(uint32_t *buf ) {
    jmethodID mid;
    jstring jstr;
    jobjectArray main_args;

    mid = (*env)->GetStaticMethodID(env, cls, "render", "(Ljava/nio/ByteBuffer;)V");
    if (mid == NULL) {
        printf("Failed to find render function\n");
        return 1;
    }

    jstr = (*env)->NewStringUTF(env, "");
    jobject direct_buffer = (*env)->NewDirectByteBuffer(env, buf, sizeof(uint32_t)*320*240);
    (*env)->CallStaticVoidMethod(env, cls, mid, direct_buffer);

    return 0;
}

void retro_init(void) {
    frame_buf = calloc(320 * 240, sizeof(uint32_t));


}

void retro_deinit(void) {
    free(frame_buf);
    frame_buf = NULL;
}

unsigned retro_api_version(void) {
    return RETRO_API_VERSION;
}

void retro_set_controller_port_device(unsigned port, unsigned device) {
    log_cb(RETRO_LOG_INFO, "Plugging device %u into port %u.\n", device, port);
}

void retro_get_system_info(struct retro_system_info *info) {
    memset(info, 0, sizeof(*info));
    info->library_name = "LibRetroJava";
    info->library_version = "v1";
    info->need_fullpath = true;
    info->valid_extensions = NULL; // Anything is fine, we don't care.
}



void retro_get_system_av_info(struct retro_system_av_info *info) {
    float aspect = 4.0f / 3.0f;

    info->timing = (struct retro_system_timing) {
            .fps = 60.0,
            .sample_rate = 0.0,
    };

    info->geometry = (struct retro_game_geometry) {
            .base_width   = 320,
            .base_height  = 240,
            .max_width    = 320,
            .max_height   = 240,
            .aspect_ratio = aspect,
    };

}

void retro_set_environment(retro_environment_t cb) {
    environ_cb = cb;

    bool no_content = false;

    cb(RETRO_ENVIRONMENT_SET_SUPPORT_NO_GAME, &no_content);

    if (cb(RETRO_ENVIRONMENT_GET_LOG_INTERFACE, &logging))
        log_cb = logging.log;
    else
        log_cb = fallback_log;
}

void retro_set_audio_sample(retro_audio_sample_t cb) {
    audio_cb = cb;
}

void retro_set_audio_sample_batch(retro_audio_sample_batch_t cb) {
    audio_batch_cb = cb;
}

void retro_set_input_poll(retro_input_poll_t cb) {
    input_poll_cb = cb;
}

void retro_set_input_state(retro_input_state_t cb) {
    input_state_cb = cb;
}

void retro_set_video_refresh(retro_video_refresh_t cb) {
    video_cb = cb;
}

static unsigned x_coord;
static unsigned y_coord;
static int mouse_rel_x;
static int mouse_rel_y;

void retro_reset(void) {
    x_coord = 0;
    y_coord = 0;
}

static void update_input(void) {
    input_poll_cb();

    if (input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_UP)) {
        /* Stub */
    }
}

static void render_checkered(void) {
    /* Try rendering straight into VRAM if we can. */
    uint32_t *buf = NULL;
    unsigned stride = 0;
    struct retro_framebuffer fb = {0};
    fb.width = 320;
    fb.height = 240;
    fb.access_flags = RETRO_MEMORY_ACCESS_WRITE;
    if (environ_cb(RETRO_ENVIRONMENT_GET_CURRENT_SOFTWARE_FRAMEBUFFER, &fb) &&
        fb.format == RETRO_PIXEL_FORMAT_XRGB8888) {
        buf = fb.data;
        stride = fb.pitch / 4;
    } else {
        buf = frame_buf;
        stride = 320;
    }


//    uint32_t color_r = 0xff << 16;
//    uint32_t color_g = 0xff << 8;
//
//    uint32_t *line = buf;
//    for (unsigned y = 0; y < 240; y++, line += stride) {
//        unsigned index_y = ((y - y_coord) >> 4) & 1;
//        for (unsigned x = 0; x < 320; x++) {
//            unsigned index_x = ((x - x_coord) >> 4) & 1;
//            line[x] = (index_y ^ index_x) ? color_r : color_g;
//        }
//    }
//
//    for (unsigned y = mouse_rel_y - 5; y <= mouse_rel_y + 5; y++)
//        for (unsigned x = mouse_rel_x - 5; x <= mouse_rel_x + 5; x++)
//            buf[y * stride + x] = 0xff;

    buf[3000] = 0xff;

    java_render(buf);

    video_cb(buf, 320, 240, stride * 4);
}

static void check_variables(void) {
}

static void audio_callback(void) {
    audio_cb(0, 0);
}

void retro_run(void) {
    update_input();
    render_checkered();
    audio_callback();

    bool updated = false;
    if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE_UPDATE, &updated) && updated)
        check_variables();
}


bool retro_load_game(const struct retro_game_info *info) {
    log_cb(RETRO_LOG_INFO, "game path: %s\n", info->path);
    enum retro_pixel_format fmt = RETRO_PIXEL_FORMAT_XRGB8888;
    if (!environ_cb(RETRO_ENVIRONMENT_SET_PIXEL_FORMAT, &fmt)) {
        log_cb(RETRO_LOG_INFO, "XRGB8888 is not supported.\n");
        return false;
    }

    check_variables();
    (void) info;

    return java_init(info->path);
}

void retro_unload_game(void) {
}

unsigned retro_get_region(void) {
    return RETRO_REGION_NTSC;
}

bool retro_load_game_special(unsigned type, const struct retro_game_info *info, size_t num) {
    if (type != 0x200)
        return false;
    if (num != 2)
        return false;
    return retro_load_game(NULL);
}

size_t retro_serialize_size(void) {
    return 2;
}

bool retro_serialize(void *data_, size_t size) {
    if (size < 2)
        return false;

    uint8_t *data = data_;
    data[0] = x_coord;
    data[1] = y_coord;
    return true;
}

bool retro_unserialize(const void *data_, size_t size) {
    if (size < 2)
        return false;

    const uint8_t *data = data_;
    x_coord = data[0] & 31;
    y_coord = data[1] & 31;
    return true;
}

void *retro_get_memory_data(unsigned id) {
    (void) id;
    return NULL;
}

size_t retro_get_memory_size(unsigned id) {
    (void) id;
    return 0;
}

void retro_cheat_reset(void) {}

void retro_cheat_set(unsigned index, bool enabled, const char *code) {
    (void) index;
    (void) enabled;
    (void) code;
}

