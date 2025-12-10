void _start() {
    char* video = (char*)0xB8000;
    video[0] = 'K';
    video[1] = 0x0F;
    while (1);
}
