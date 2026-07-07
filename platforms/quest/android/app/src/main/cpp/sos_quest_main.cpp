#include <android/log.h>
#include <android/native_window_jni.h>
#include <EGL/egl.h>
#include <GLES3/gl3.h>
#include <jni.h>

#include <cstring>
#include <string>
#include <vector>

#include "SosCoreBridge.h"
#include "render/GlesSplatRenderer.h"

#define LOG_TAG "SOSQuest"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

namespace {

EGLDisplay g_display = EGL_NO_DISPLAY;
EGLContext g_context = EGL_NO_CONTEXT;
EGLSurface g_surface = EGL_NO_SURFACE;
sos::quest::GlesSplatRenderer* g_renderer = nullptr;
std::vector<uint8_t> g_splat_blob;
bool g_has_splat = false;

bool InitEgl(ANativeWindow* window) {
  g_display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
  eglInitialize(g_display, nullptr, nullptr);
  const EGLint config_attribs[] = {EGL_RENDERABLE_TYPE,
                                   EGL_OPENGL_ES3_BIT,
                                   EGL_SURFACE_TYPE,
                                   EGL_WINDOW_BIT,
                                   EGL_BLUE_SIZE,
                                   8,
                                   EGL_GREEN_SIZE,
                                   8,
                                   EGL_RED_SIZE,
                                   8,
                                   EGL_ALPHA_SIZE,
                                   8,
                                   EGL_NONE};
  EGLConfig config{};
  EGLint count = 0;
  eglChooseConfig(g_display, config_attribs, &config, 1, &count);
  const EGLint context_attribs[] = {EGL_CONTEXT_CLIENT_VERSION, 3, EGL_NONE};
  g_context = eglCreateContext(g_display, config, EGL_NO_CONTEXT, context_attribs);
  g_surface = eglCreateWindowSurface(g_display, config, window, nullptr);
  return eglMakeCurrent(g_display, g_surface, g_surface, g_context) == EGL_TRUE;
}

void DrawFrame() {
  glViewport(0, 0, 1920, 1080);
  glClearColor(0.f, 0.f, 0.f, 0.f);
  glClear(GL_COLOR_BUFFER_BIT);
  if (g_renderer && g_has_splat) {
    const float vp[16] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, -3, 1};
    const float cam[3] = {0.f, 0.f, 0.f};
    g_renderer->draw(vp, cam);
  }
  eglSwapBuffers(g_display, g_surface);
}

}  // namespace

extern "C" JNIEXPORT void JNICALL
Java_com_openworld_sos_quest_MainActivity_nativeInit(JNIEnv* env, jobject, jstring storeRoot, jstring splatRoomId) {
  const char* root = env->GetStringUTFChars(storeRoot, nullptr);
  sos_bridge_set_store_root(root);
  env->ReleaseStringUTFChars(storeRoot, root);

  const char* room = env->GetStringUTFChars(splatRoomId, nullptr);
  size_t size = 0;
  sos_bridge_load_splat_asset(room, nullptr, 0, &size);
  if (size > 0) {
    g_splat_blob.resize(size);
    sos_bridge_load_splat_asset(room, g_splat_blob.data(), g_splat_blob.size(), &size);
    g_renderer = new sos::quest::GlesSplatRenderer();
    g_has_splat = g_renderer->loadFromBlob(g_splat_blob.data(), g_splat_blob.size());
    LOGI("Loaded splat blob bytes=%zu ok=%d", size, g_has_splat ? 1 : 0);
  }
  env->ReleaseStringUTFChars(splatRoomId, room);
}

extern "C" JNIEXPORT void JNICALL
Java_com_openworld_sos_quest_MainActivity_nativeDrawFrame(JNIEnv*, jobject) {
  DrawFrame();
}

extern "C" JNIEXPORT void JNICALL
Java_com_openworld_sos_quest_MainActivity_nativeSetSurface(JNIEnv* env, jobject, jobject surface) {
  ANativeWindow* window = ANativeWindow_fromSurface(env, surface);
  if (window) {
    InitEgl(window);
  }
}
