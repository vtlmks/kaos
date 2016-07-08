
/*
** Some warnings:
**  Structure line alignment is changed from default to a 1 byte alignment; (normally it is 8 or 16 byte)
**
**  All around rusty assembly, but it'll be fine...  mostly...
*/



char *loaderWelcome = "Second stage loader started";
char *loaderInitializing = " [  ] Initializing memory, setting up structures";
char *loaderMemoryprobe = " [  ] Trying to find out how much memory this machine have available";
char *loaderSwitchingvideomode = " [  ] Switching to something better than 80x25";
char *loaderMessage = " [  ] Loading kernel";
char *loaderOK = " [OK]";

__declspec(naked) void loader() {

	char *a = loaderWelcome;

	_asm {
		cli
		mov	ax, 0x10
		mov	ds, ax
		mov	es, ax
		mov	fs, ax
		mov	gs, ax
		mov	ss, ax
		mov	esp, 0x80000
		mov	ebp, esp
		push	ebp
		sti
	}

	volatile int b = 0;

	for(unsigned int i = 0; i < 10; ++i) {
		a[i] == 0 ? b = 1 : b = 0;
	}

	if(b) a[2] = 0;
}
