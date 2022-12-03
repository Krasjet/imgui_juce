#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-conversion"
#endif

#include <imgui.h>
#include <imgui_internal.h>
#include <juce_opengl/juce_opengl.h>

#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

#include "imgui_impl_juce.h"

typedef juce::Array<ImGuiInputEvent> ImGui_ImplJuce_EventQueue;

struct ImGui_ImplJuce_Data
{
  double Time;
  juce::Component *Component;
  juce::OpenGLContext *Context;
  juce::MouseListener *MouseListener;
  juce::KeyListener *KeyListener;
  juce::MouseCursor MouseCursors[ImGuiMouseCursor_COUNT];
  juce::MouseCursor MouseCursorNone;

  ImGui_ImplJuce_EventQueue Events;
  juce::CriticalSection Mutex;

  ImGui_ImplJuce_Data()
  {
    Events.ensureStorageAllocated(2048);
  }
};

static ImGui_ImplJuce_Data *ImGui_ImplJuce_GetBackendData()
{
  return ImGui::GetCurrentContext() ?
    (ImGui_ImplJuce_Data*)ImGui::GetIO().BackendPlatformUserData
    : nullptr;
}

// Because imgui doesn't support multi-threading natively but JUCE do
// use an extra thread to handle mouse and key press callbacks, we
// need to queue all input events and only dispatch before
// ImGui::NewFrame, otherwise it will crash imgui
static void queueKeyEvent(ImGui_ImplJuce_Data *bd, ImGuiKey key, bool down)
{
  ImGuiInputEvent e;
  e.Type = ImGuiInputEventType_Key;
  e.Key.Key = key;
  e.Key.Down = down;
  {
    const juce::ScopedLock lk(bd->Mutex);
    bd->Events.add(e);
  }
}
static void queueMousePosEvent(ImGui_ImplJuce_Data *bd, float x, float y)
{
  ImGuiInputEvent e;
  e.Type = ImGuiInputEventType_MousePos;
  e.MousePos.PosX = x;
  e.MousePos.PosY = y;
  {
    const juce::ScopedLock lk(bd->Mutex);
    bd->Events.add(e);
  }
}
static void queueMouseButtonEvent(ImGui_ImplJuce_Data *bd, int button, bool down)
{
  ImGuiInputEvent e;
  e.Type = ImGuiInputEventType_MouseButton;
  e.MouseButton.Button = button;
  e.MouseButton.Down = down;
  {
    const juce::ScopedLock lk(bd->Mutex);
    bd->Events.add(e);
  }
}
static void queueMouseWheelEvent(ImGui_ImplJuce_Data *bd, float wh_x, float wh_y)
{
  ImGuiInputEvent e;
  e.Type = ImGuiInputEventType_MouseWheel;
  e.MouseWheel.WheelX = wh_x;
  e.MouseWheel.WheelY = wh_y;
  {
    const juce::ScopedLock lk(bd->Mutex);
    bd->Events.add(e);
  }
}
static void queueInputCharacter(ImGui_ImplJuce_Data *bd, unsigned int c)
{
  ImGuiInputEvent e;
  e.Type = ImGuiInputEventType_Text;
  e.Text.Char = c;
  {
    const juce::ScopedLock lk(bd->Mutex);
    bd->Events.add(e);
  }
}

static void dispatchEventQueue(ImGui_ImplJuce_Data *bd)
{
  ImGuiIO& io = ImGui::GetIO();
  {
    const juce::ScopedLock lk(bd->Mutex);

    for (auto &e : bd->Events) {
      if (e.Type == ImGuiInputEventType_Key)
        io.AddKeyEvent(e.Key.Key, e.Key.Down);
      else if (e.Type == ImGuiInputEventType_MousePos)
        io.AddMousePosEvent(e.MousePos.PosX, e.MousePos.PosY);
      else if (e.Type == ImGuiInputEventType_MouseButton)
        io.AddMouseButtonEvent(e.MouseButton.Button, e.MouseButton.Down);
      else if (e.Type == ImGuiInputEventType_MouseWheel)
        io.AddMouseWheelEvent(e.MouseWheel.WheelX, e.MouseWheel.WheelY);
      else if (e.Type == ImGuiInputEventType_Text)
        io.AddInputCharacter(e.Text.Char);
    }

    // flush queue
    bd->Events.clearQuick();
  }
}

// hack for juce's horrible key press api
static void updateKeyModifiers(ImGui_ImplJuce_Data *bd, const juce::ModifierKeys *mods = nullptr)
{
  if (!mods)
    mods = &juce::ModifierKeys::currentModifiers;
  queueKeyEvent(bd, ImGuiKey_ModCtrl, mods->isCtrlDown());
  queueKeyEvent(bd, ImGuiKey_ModShift, mods->isShiftDown());
  queueKeyEvent(bd, ImGuiKey_ModAlt, mods->isAltDown());
}

struct ImGui_ImplJuce_MouseListener : public juce::MouseListener
{
  // listeners does not happen on opengl thread and it doesn't have
  // access to the thread local imgui context. we need to store this
  // to get access to backend data
  ImGuiContext *ctx;

  ImGui_ImplJuce_MouseListener(ImGuiContext *c) : ctx(c) {}

  void mouseMove(const juce::MouseEvent &e) override
  {
    ImGui::SetCurrentContext(ctx);

    ImGui_ImplJuce_Data* bd = ImGui_ImplJuce_GetBackendData();
    IM_ASSERT(bd != NULL && "Did you call ImGui_ImplJuce_InitForXXX()?");

    // stale key modifier can sometimes prevent scrolling
    updateKeyModifiers(bd);
    queueMousePosEvent(bd, e.position.x, e.position.y);
  }

  void mouseEnter(const juce::MouseEvent &) override {}
  void mouseExit(const juce::MouseEvent &) override {}

  void mouseDown(const juce::MouseEvent &e) override
  {
    ImGui::SetCurrentContext(ctx);

    ImGui_ImplJuce_Data* bd = ImGui_ImplJuce_GetBackendData();
    IM_ASSERT(bd != NULL && "Did you call ImGui_ImplJuce_InitForXXX()?");

    updateKeyModifiers(bd, &e.mods);
    if (e.mods.isLeftButtonDown())
      queueMouseButtonEvent(bd, ImGuiMouseButton_Left, true);
    if (e.mods.isRightButtonDown())
      queueMouseButtonEvent(bd, ImGuiMouseButton_Right, true);
    if (e.mods.isMiddleButtonDown())
      queueMouseButtonEvent(bd, ImGuiMouseButton_Middle, true);
  }
  void mouseDrag(const juce::MouseEvent &e) override
  {
    ImGui::SetCurrentContext(ctx);

    ImGui_ImplJuce_Data* bd = ImGui_ImplJuce_GetBackendData();
    IM_ASSERT(bd != NULL && "Did you call ImGui_ImplJuce_InitForXXX()?");

    queueMousePosEvent(bd, e.position.x, e.position.y);
  }
  void mouseUp(const juce::MouseEvent &e) override
  {
    ImGui::SetCurrentContext(ctx);

    ImGui_ImplJuce_Data* bd = ImGui_ImplJuce_GetBackendData();
    IM_ASSERT(bd != NULL && "Did you call ImGui_ImplJuce_InitForXXX()?");

    updateKeyModifiers(bd, &e.mods);
    if (e.mods.isLeftButtonDown())
      queueMouseButtonEvent(bd, ImGuiMouseButton_Left, false);
    if (e.mods.isRightButtonDown())
      queueMouseButtonEvent(bd, ImGuiMouseButton_Right, false);
    if (e.mods.isMiddleButtonDown())
      queueMouseButtonEvent(bd, ImGuiMouseButton_Middle, false);
  }
  void mouseDoubleClick(const juce::MouseEvent &) override {}
  void mouseWheelMove(const juce::MouseEvent &, const juce::MouseWheelDetails &wheel) override
  {
    ImGui::SetCurrentContext(ctx);

    ImGui_ImplJuce_Data* bd = ImGui_ImplJuce_GetBackendData();
    IM_ASSERT(bd != NULL && "Did you call ImGui_ImplJuce_InitForXXX()?");

    queueMouseWheelEvent(bd, wheel.deltaX, wheel.deltaY);
  }
  void mouseMagnify(const juce::MouseEvent &, float) override {}
};

struct ImGui_ImplJuce_KeyListener : public juce::KeyListener
{
  // listeners does not happen on opengl thread and it doesn't have
  // access to the thread local imgui context. we need to store this
  // to get access to backend data
  ImGuiContext *ctx;

  ImGui_ImplJuce_KeyListener(ImGuiContext *c) : ctx(c) {}

  bool keyPressed(const juce::KeyPress &key, juce::Component *) override
  {
    ImGui::SetCurrentContext(ctx);

    ImGuiIO &io = ImGui::GetIO();
    ImGui_ImplJuce_Data* bd = ImGui_ImplJuce_GetBackendData();
    IM_ASSERT(bd != NULL && "Did you call ImGui_ImplJuce_InitForXXX()?");

    queueInputCharacter(bd, (unsigned int)key.getTextCharacter());

    return io.WantCaptureKeyboard;
  }
  bool keyStateChanged(bool, juce::Component *) override
  {
    ImGui::SetCurrentContext(ctx);

    ImGuiIO &io = ImGui::GetIO();
    ImGui_ImplJuce_Data* bd = ImGui_ImplJuce_GetBackendData();
    IM_ASSERT(bd != NULL && "Did you call ImGui_ImplJuce_InitForXXX()?");

    updateKeyModifiers(bd);
#define MAP_KEY(KEY_JUCE, KEY_IMGUI) do { queueKeyEvent(bd, KEY_IMGUI, juce::KeyPress::isKeyCurrentlyDown(KEY_JUCE)); } while (0)
    MAP_KEY(juce::KeyPress::tabKey, ImGuiKey_Tab);
    MAP_KEY(juce::KeyPress::leftKey, ImGuiKey_LeftArrow);
    MAP_KEY(juce::KeyPress::rightKey, ImGuiKey_RightArrow);
    MAP_KEY(juce::KeyPress::upKey, ImGuiKey_UpArrow);
    MAP_KEY(juce::KeyPress::downKey, ImGuiKey_DownArrow);
    MAP_KEY(juce::KeyPress::pageUpKey, ImGuiKey_PageUp);
    MAP_KEY(juce::KeyPress::pageDownKey, ImGuiKey_PageDown);
    MAP_KEY(juce::KeyPress::homeKey, ImGuiKey_Home);
    MAP_KEY(juce::KeyPress::endKey, ImGuiKey_End);
    MAP_KEY(juce::KeyPress::insertKey, ImGuiKey_Insert);
    MAP_KEY(juce::KeyPress::deleteKey, ImGuiKey_Delete);
    MAP_KEY(juce::KeyPress::backspaceKey, ImGuiKey_Backspace);
    MAP_KEY(juce::KeyPress::spaceKey, ImGuiKey_Space);
    MAP_KEY(juce::KeyPress::returnKey, ImGuiKey_Enter);
    MAP_KEY(juce::KeyPress::escapeKey, ImGuiKey_Escape);
    MAP_KEY('\'', ImGuiKey_Apostrophe);
    MAP_KEY(',', ImGuiKey_Comma);
    MAP_KEY('-', ImGuiKey_Minus);
    MAP_KEY('.', ImGuiKey_Period);
    MAP_KEY('/', ImGuiKey_Slash);
    MAP_KEY(';', ImGuiKey_Semicolon);
    MAP_KEY('=', ImGuiKey_Equal);
    MAP_KEY('[', ImGuiKey_LeftBracket);
    MAP_KEY('\\', ImGuiKey_Backslash);
    MAP_KEY(']', ImGuiKey_RightBracket);
    MAP_KEY('`', ImGuiKey_GraveAccent);
    MAP_KEY(juce::KeyPress::numberPad0, ImGuiKey_Keypad0);
    MAP_KEY(juce::KeyPress::numberPad1, ImGuiKey_Keypad1);
    MAP_KEY(juce::KeyPress::numberPad2, ImGuiKey_Keypad2);
    MAP_KEY(juce::KeyPress::numberPad3, ImGuiKey_Keypad3);
    MAP_KEY(juce::KeyPress::numberPad4, ImGuiKey_Keypad4);
    MAP_KEY(juce::KeyPress::numberPad5, ImGuiKey_Keypad5);
    MAP_KEY(juce::KeyPress::numberPad6, ImGuiKey_Keypad6);
    MAP_KEY(juce::KeyPress::numberPad7, ImGuiKey_Keypad7);
    MAP_KEY(juce::KeyPress::numberPad8, ImGuiKey_Keypad8);
    MAP_KEY(juce::KeyPress::numberPad9, ImGuiKey_Keypad9);
    MAP_KEY(juce::KeyPress::numberPadDecimalPoint, ImGuiKey_KeypadDecimal);
    MAP_KEY(juce::KeyPress::numberPadDivide, ImGuiKey_KeypadDivide);
    MAP_KEY(juce::KeyPress::numberPadMultiply, ImGuiKey_KeypadMultiply);
    MAP_KEY(juce::KeyPress::numberPadSubtract, ImGuiKey_KeypadSubtract);
    MAP_KEY(juce::KeyPress::numberPadAdd, ImGuiKey_KeypadAdd);
    MAP_KEY(juce::KeyPress::numberPadEquals, ImGuiKey_KeypadEqual);
    MAP_KEY('0', ImGuiKey_0);
    MAP_KEY('1', ImGuiKey_1);
    MAP_KEY('2', ImGuiKey_2);
    MAP_KEY('3', ImGuiKey_3);
    MAP_KEY('4', ImGuiKey_4);
    MAP_KEY('5', ImGuiKey_5);
    MAP_KEY('6', ImGuiKey_6);
    MAP_KEY('7', ImGuiKey_7);
    MAP_KEY('8', ImGuiKey_8);
    MAP_KEY('9', ImGuiKey_9);
    MAP_KEY('a', ImGuiKey_A);
    MAP_KEY('b', ImGuiKey_B);
    MAP_KEY('c', ImGuiKey_C);
    MAP_KEY('d', ImGuiKey_D);
    MAP_KEY('e', ImGuiKey_E);
    MAP_KEY('f', ImGuiKey_F);
    MAP_KEY('g', ImGuiKey_G);
    MAP_KEY('h', ImGuiKey_H);
    MAP_KEY('i', ImGuiKey_I);
    MAP_KEY('j', ImGuiKey_J);
    MAP_KEY('k', ImGuiKey_K);
    MAP_KEY('l', ImGuiKey_L);
    MAP_KEY('m', ImGuiKey_M);
    MAP_KEY('n', ImGuiKey_N);
    MAP_KEY('o', ImGuiKey_O);
    MAP_KEY('p', ImGuiKey_P);
    MAP_KEY('q', ImGuiKey_Q);
    MAP_KEY('r', ImGuiKey_R);
    MAP_KEY('s', ImGuiKey_S);
    MAP_KEY('t', ImGuiKey_T);
    MAP_KEY('u', ImGuiKey_U);
    MAP_KEY('v', ImGuiKey_V);
    MAP_KEY('w', ImGuiKey_W);
    MAP_KEY('x', ImGuiKey_X);
    MAP_KEY('y', ImGuiKey_Y);
    MAP_KEY('z', ImGuiKey_Z);
    MAP_KEY(juce::KeyPress::F1Key, ImGuiKey_F1);
    MAP_KEY(juce::KeyPress::F2Key, ImGuiKey_F2);
    MAP_KEY(juce::KeyPress::F3Key, ImGuiKey_F3);
    MAP_KEY(juce::KeyPress::F4Key, ImGuiKey_F4);
    MAP_KEY(juce::KeyPress::F5Key, ImGuiKey_F5);
    MAP_KEY(juce::KeyPress::F6Key, ImGuiKey_F6);
    MAP_KEY(juce::KeyPress::F7Key, ImGuiKey_F7);
    MAP_KEY(juce::KeyPress::F8Key, ImGuiKey_F8);
    MAP_KEY(juce::KeyPress::F9Key, ImGuiKey_F9);
    MAP_KEY(juce::KeyPress::F10Key, ImGuiKey_F10);
    MAP_KEY(juce::KeyPress::F11Key, ImGuiKey_F11);
    MAP_KEY(juce::KeyPress::F12Key, ImGuiKey_F12);
#undef MAP_KEY
    return io.WantCaptureKeyboard;
  }
};

static const char* ImGui_ImplJuce_GetClipboardText(void *)
{
  return juce::SystemClipboard::getTextFromClipboard().toRawUTF8();
}

static void ImGui_ImplJuce_SetClipboardText(void *, const char *text)
{
  juce::SystemClipboard::copyTextToClipboard(juce::String(text));
}

void ImGui_ImplJuce_Init(juce::Component &component, juce::OpenGLContext &context)
{
  ImGuiIO &io = ImGui::GetIO();
  IM_ASSERT(io.BackendPlatformUserData == NULL && "Already initialized a platform backend!");

  // Setup backend capabilities flags
  ImGui_ImplJuce_Data *bd = IM_NEW(ImGui_ImplJuce_Data)();
  io.BackendPlatformUserData = (void*)bd;
  io.BackendPlatformName = "imgui_impl_juce";
  io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;

  bd->Time = 0.0;
  bd->Component = &component;
  bd->Context = &context;

  io.SetClipboardTextFn = ImGui_ImplJuce_SetClipboardText;
  io.GetClipboardTextFn = ImGui_ImplJuce_GetClipboardText;

  // Create mouse cursors
  bd->MouseCursors[ImGuiMouseCursor_Arrow] = juce::MouseCursor(juce::MouseCursor::StandardCursorType::NormalCursor);
  bd->MouseCursors[ImGuiMouseCursor_TextInput] = juce::MouseCursor(juce::MouseCursor::StandardCursorType::IBeamCursor);
  bd->MouseCursors[ImGuiMouseCursor_ResizeNS] = juce::MouseCursor(juce::MouseCursor::StandardCursorType::UpDownResizeCursor);
  bd->MouseCursors[ImGuiMouseCursor_ResizeEW] = juce::MouseCursor(juce::MouseCursor::StandardCursorType::LeftRightResizeCursor);
  bd->MouseCursors[ImGuiMouseCursor_Hand] = juce::MouseCursor(juce::MouseCursor::StandardCursorType::PointingHandCursor);
  bd->MouseCursors[ImGuiMouseCursor_ResizeAll] = juce::MouseCursor(juce::MouseCursor::StandardCursorType::UpDownLeftRightResizeCursor);
  bd->MouseCursors[ImGuiMouseCursor_ResizeNESW] = juce::MouseCursor(juce::MouseCursor::StandardCursorType::BottomLeftCornerResizeCursor);
  bd->MouseCursors[ImGuiMouseCursor_ResizeNWSE] = juce::MouseCursor(juce::MouseCursor::StandardCursorType::BottomRightCornerResizeCursor);
  bd->MouseCursors[ImGuiMouseCursor_NotAllowed] = juce::MouseCursor(juce::MouseCursor::StandardCursorType::NormalCursor); // unavailable
  bd->MouseCursorNone = juce::MouseCursor(juce::MouseCursor::StandardCursorType::NoCursor);

  // Install callbacks
  bd->MouseListener = IM_NEW(ImGui_ImplJuce_MouseListener)(ImGui::GetCurrentContext());
  bd->KeyListener = IM_NEW(ImGui_ImplJuce_KeyListener)(ImGui::GetCurrentContext());

  {
    // we can't manipulate component on message thread directly on
    // opengl thread. obtain lock for message thread first.
    const juce::MessageManagerLock mml(juce::ThreadPoolJob::getCurrentThreadPoolJob());
    //                                 ^ this part needed to prevent deadlock on exit
    if (mml.lockWasGained()) {
      component.addMouseListener(bd->MouseListener, false);
      component.addKeyListener(bd->KeyListener);
    }
  }
}

void ImGui_ImplJuce_Shutdown()
{
  ImGui_ImplJuce_Data* bd = ImGui_ImplJuce_GetBackendData();
  IM_ASSERT(bd != NULL && "No platform backend to shutdown, or already shutdown?");

  {
    // we can't manipulate component on message thread directly on
    // opengl thread. obtain lock for message thread first.
    // NOTE: the lock is no longer necessary after JUCE 7.0.3,
    //       but we will keep it for compatibility
    const juce::MessageManagerLock mml(juce::ThreadPoolJob::getCurrentThreadPoolJob());
    //                                 ^ this part needed to prevent deadlock on exit
    if (mml.lockWasGained()) {
      bd->Component->removeMouseListener(bd->MouseListener);
      bd->Component->removeKeyListener(bd->KeyListener);
    }
  }

  ImGuiIO& io = ImGui::GetIO();
  io.BackendPlatformName = nullptr;
  io.BackendPlatformUserData = nullptr;

  IM_DELETE(bd->MouseListener);
  IM_DELETE(bd->KeyListener);
  IM_DELETE(bd);
}

static void ImGui_ImplJuce_UpdateMouseCursor()
{
  ImGuiIO& io = ImGui::GetIO();
  ImGui_ImplJuce_Data* bd = ImGui_ImplJuce_GetBackendData();
  IM_ASSERT(bd != NULL && "Did you call ImGui_ImplJuce_InitForXXX()?");

  if ((io.ConfigFlags & ImGuiConfigFlags_NoMouseCursorChange))
    return;

  ImGuiMouseCursor imgui_cursor = ImGui::GetMouseCursor();
  if (imgui_cursor == ImGuiMouseCursor_None || io.MouseDrawCursor) {
    // hide OS mouse cursor if imgui is drawing it or if it wants no cursor
    bd->Component->setMouseCursor(bd->MouseCursorNone);
  } else {
    bd->Component->setMouseCursor(bd->MouseCursors[imgui_cursor]);
  }
}

void ImGui_ImplJuce_NewFrame()
{
  ImGuiIO& io = ImGui::GetIO();
  ImGui_ImplJuce_Data* bd = ImGui_ImplJuce_GetBackendData();
  IM_ASSERT(bd != NULL && "Did you call ImGui_ImplJuce_InitForXXX()?");

  io.DisplaySize = ImVec2((float)bd->Component->getWidth(), (float)bd->Component->getHeight());
  float scale = (float)bd->Context->getRenderingScale();
  io.DisplayFramebufferScale = ImVec2(scale, scale);

  double current_time = juce::Time::getMillisecondCounterHiRes();
  io.DeltaTime = bd->Time > 0.0 ? (float)((current_time - bd->Time)/1000) : 1.0f / 60.0f;
  bd->Time = current_time;

  dispatchEventQueue(bd);
  ImGui_ImplJuce_UpdateMouseCursor();
}
