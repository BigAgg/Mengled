#include "core/m_main.h"

int main(int argc, char* argv[]) {
	m_main(argc, argv);
}

#ifdef _MSC_VER
#ifdef NDEBUG
#include <Windows.h>
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	return main(__argc, __argv);
}
#endif
#endif