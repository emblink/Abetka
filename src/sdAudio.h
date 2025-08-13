#ifdef __cplusplus
extern "C" {
#endif

#define SAMPLES_PER_BUFFER 4096

void sdAudioPlayFile(char *path);
void sdAudioProcess();

#ifdef __cplusplus
}
#endif