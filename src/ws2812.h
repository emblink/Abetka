#ifdef __cplusplus
extern "C" {
#endif

#define RED 0x00000A
#define GREEN 0x000A00
#define BLUE 0x0A0000
#define BLACK 0
#define WHITE 0x0A0A0A

void ws2812Init();
void ws2812Test();
void ws2812SetColor(uint32_t col);

#ifdef __cplusplus
}
#endif