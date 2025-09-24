// kernel.c

void _start() {
    char* video = (char*)0xB8000;
    for (int i = 0; i < 80 * 25; ++i) {
        video[i * 2] = 'X';
        video[i * 2 + 1] = 0x4F; // red on white
    }
    while (1); // just looping
}