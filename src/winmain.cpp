#ifdef _WIN32

#include <windows.h>
#include <cstdlib>

// raylib defines its own entry flow; we just forward to main
extern int main(int argc, char* argv[]);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine, int nShowCmd) {
  int argc = __argc;
  char** argv = __argv;
  int result = main(argc, argv);


  return result;
}

#endif