#define VIDEO_W 500
#define VIDEO_H 500
#define FPS 10
#define FRAME_DELAY_MS  (1000 / FPS)              // 100ms
#define FRAME_DELAY_US  (FRAME_DELAY_MS * 1000U)  // 100,000us

extern const unsigned int* video_bitmap_0allArray[20];
extern const unsigned int* video_bitmap_1allArray[20];
extern const unsigned int* video_bitmap_2allArray[20];

// extern const uint32_t * const video_bitmap_0allArray[20];
// extern const uint32_t * const video_bitmap_1allArray[20];
// extern const uint32_t * const video_bitmap_2allArray[20];