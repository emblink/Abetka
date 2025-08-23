#ifdef __cplusplus
extern "C" {
#endif
#include <stdbool.h>
#define SAMPLES_PER_BUFFER 4096

void sdAudioPlayFile(char *path);
void sdAudioProcess();
bool sdAudioIsPlaying();
void sdAudioStop();

#ifdef __cplusplus
}
#endif