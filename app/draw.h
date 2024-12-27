#include <vita2d.h>

#define MENUOVERLAY_WIDTH (494)
#define MENUOVERLAY_HEIGHT (506) 

#define MENUOVERLAY_PAD (19)
#define MENUOVERLAY_POS_X ((SCREEN_WIDTH - MENUOVERLAY_WIDTH) - MENUOVERLAY_PAD)
#define MENUOVERLAY_POS_Y MENUOVERLAY_PAD

#define COLOR_WHITE ( RGBA8(251,251,251,255) )
#define COLOR_BLACK ( RGBA8(0,0,0,255) )

#define SCREEN_WIDTH 960
#define SCREEN_HEIGHT 544


void init_vita2d();
void term_vita2d();
void draw_background();
void start_draw();
void end_draw();
void draw_texture(vita2d_texture* texture, int x, int y);
void draw_texture_center(int y, vita2d_texture* texture);
void draw_text(int x, int y, char* msg);
void draw_text_center(int y, char* msg);
void draw_option(int y, char* opt, int selected);
void draw_title(char* msg);
void free_texture(vita2d_texture* texture);
void draw_progress_bar(int y, uint64_t done, uint64_t total);

vita2d_texture* load_texture(char* path);