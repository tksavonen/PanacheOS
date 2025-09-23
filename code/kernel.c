// kernel.c

void _start()
{
	const char* msg = "panacheOS kernel loaded";
	char* video = (char*)0xB8000;
	for (int i = 0; msg[i] != '\0'; ++i) 
	{
		video[i*2]=msg[i];
		video[i*2+1]=0x0F; // white on black
	}
	while(1); // hang
}
