#include "..\ht.h"

HINSTANCE ghInst;

int __stdcall WinMain(HINSTANCE hInst, HINSTANCE hInstPrev, char *pszCmds, int nCmdShow)
{
	WSADATA wsad;
	WSAStartup(MAKEWORD(1, 1), &wsad);

	ghInst = hInst;
	ReporterInit();
	GameMain(pszCmds);
	ReporterExit();

	WSACleanup();
	return 1;
}


// Ring buffer of old allocs to check for corruption

#ifdef CHECK_OLD_ALLOCS

void *gapvRing[500];
int gipv;

void my_check(void *pv)
{
	dword *pcb = (dword *)((dword *)pv - 1);
	byte *pb = (byte *)pv;
	for (dword n = 0; n < *pcb; n++) {
		if (pb[n] != 0xdd)
			_asm { int 3 };
	}
}

void my_free(void *pv)
{
	my_check(pv);
	void *pvT = (void *)((dword *)pv - 1);
	free(pvT);
}

void my_freeall()
{
	for (int i = 0; i < ARRAYSIZE(gapvRing); i++) {
		if (gapvRing[i] != NULL)
			my_free(gapvRing[i]);
	}
}

void my_fill(void *pv)
{
	dword *pcb = (dword *)((dword *)pv - 1);
	memset(pv, 0xdd, *pcb); 
}

void my_delete(void *pv)
{
	if (pv == NULL)
		return;
	if (gapvRing[gipv] != NULL)
		my_free((dword *)gapvRing[gipv]);
	gapvRing[gipv] = pv;
	gipv++;
	if (gipv >= ARRAYSIZE(gapvRing))
		gipv = 0;
	my_fill(pv);
}

void *my_new(int cb)
{
	int cbT = cb + 4;
	void *pv = malloc(cbT);
	*((dword *)pv) = cb;
	return (void *)((dword *)pv + 1);
}

void my_alloccheck()
{
	for (int i = 0; i < ARRAYSIZE(gapvRing); i++) {
		if (gapvRing[i] != NULL)
			my_check(gapvRing[i]);
	}
}
#endif