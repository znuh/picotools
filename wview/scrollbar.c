#include "scrollbar.h"

scrollbar_t *scrollbar_create(SDL_Surface *sf, int sf_xofs, int sf_yofs, int sf_w, int sf_h, unsigned long max_len) {
	scrollbar_t *res = NULL;
	
	if(!(res = malloc(sizeof(scrollbar_t))))
		return NULL;
	
	res->dst = sf;
	res->sf_xofs = sf_xofs;
	res->sf_yofs = sf_yofs;
	res->sf_w = sf_w;
	res->sf_h = sf_h;
	res->max_len = max_len;
	
	return res;
}

void scrollbar_destroy(scrollbar_t *sb) {
	free(sb);
}

void scrollbar_draw(scrollbar_t *sb) {
	boxColor(sb->dst, sb->sf_xofs, sb->sf_yofs, sb->sf_xofs + sb->sf_w, sb->sf_yofs + sb->sf_h, 0x404040ff);
}

void scrollbar_event(scrollbar_t *sb, SDL_Event *evt) {
	
}
