#include "draw.h"
#include <vita2d.h>
#include <vitasdk.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>

static vita2d_pgf *pgf;
static vita2d_pvf *pvf;

static vita2d_texture *background;
static vita2d_texture *menuoverlay;


void init_vita2d() {
	vita2d_init();
	vita2d_set_clear_color(RGBA8(0xFF, 0xFF, 0xFF, 0xFF));
	
	pgf = vita2d_load_default_pgf();
	pvf = vita2d_load_default_pvf();

	background = load_texture("app0:/res/background.png");
	menuoverlay = load_texture("app0:/res/menuoverlay.png");
}

void term_vita2d() {
	vita2d_fini();

	vita2d_free_pgf(pgf);
	vita2d_free_pvf(pvf);

	vita2d_free_texture(background);
	vita2d_free_texture(menuoverlay);
}

void draw_text(int x, int y, char* msg) {
	float size = 1.0f;
	vita2d_pgf_draw_text(pgf, x, y, COLOR_WHITE, size, msg);
}

void draw_progress_bar(int y, uint64_t done, uint64_t total) {
	int end = (MENUOVERLAY_WIDTH - ((MENUOVERLAY_PAD) * 2));
	int start = MENUOVERLAY_POS_X + MENUOVERLAY_PAD;
	int height = 32;
	
	int bar_px = (int)(((float)done / (float)total) * (float)end);
	
	vita2d_draw_rectangle(start, y, end, height, RGBA8(128,128,128,128));
	vita2d_draw_rectangle(start, y, bar_px, height, RGBA8(0,255,0,128));

}

void draw_text_center(int y, char* msg) {
	float size = 1.0f;
	int text_width = 0;
	int text_height = 0;
	vita2d_pgf_text_dimensions(pgf, size, msg, &text_width, &text_height);
	int text_x = MENUOVERLAY_POS_X + (((MENUOVERLAY_WIDTH)/2) - (text_width/2));
	draw_text(text_x, y, msg);
}

void draw_option(int y, char* opt, int selected) {
	char option_text[512];
	if(!selected)
		snprintf(option_text, sizeof(option_text), "  %s  ", opt);
	else
		snprintf(option_text, sizeof(option_text), "> %s <", opt);
	
	draw_text_center(y, option_text);
}

void draw_title(char* title) {
	draw_text_center(60, title);	
}



void draw_texture_center(int y, vita2d_texture* texture) {
	int x = MENUOVERLAY_WIDTH / 2;	
	int w = vita2d_texture_get_width(texture)/2;
	
	draw_texture(texture, MENUOVERLAY_POS_X + (x - w), y);
}

void draw_background() {
	draw_texture(background, 0, 0);
	draw_texture(menuoverlay, MENUOVERLAY_POS_X, MENUOVERLAY_POS_Y);
}

void start_draw() {
	vita2d_start_drawing();
	vita2d_clear_screen();
}

void end_draw() {
	vita2d_end_drawing();
	vita2d_swap_buffers();	
}

void draw_texture(vita2d_texture* texture, int x, int y) {
	vita2d_draw_texture(texture, x, y);
}

void free_texture(vita2d_texture* texture) {
	vita2d_free_texture(texture);
}

vita2d_texture* load_texture(char* path) {
	return vita2d_load_PNG_file(path);
}