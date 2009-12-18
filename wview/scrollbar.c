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
	
	res->mouse_mode = SB_OUT;
	
	res->pos = 0;
	
	return res;
}

void scrollbar_destroy(scrollbar_t *sb) {
	free(sb);
}

#define	GRIP_SIZE	3

void scrollbar_draw(scrollbar_t *sb) {
	int x_start = 5;
	int x_end = 30;
	
	// background
	boxColor(sb->dst, sb->sf_xofs, sb->sf_yofs, sb->sf_xofs + sb->sf_w, sb->sf_yofs + sb->sf_h, 0x404040ff);
	
	// start pos
	boxColor(sb->dst, sb->sf_xofs + sb->pos, sb->sf_yofs, sb->sf_xofs + sb->pos + GRIP_SIZE, sb->sf_yofs + sb->sf_h, 0x808080ff);
	
	// filler
	//boxColor(sb->dst, sb->sf_xofs + x_start + 2 * GRIP_SIZE, sb->sf_yofs, sb->sf_xofs + x_end - 2 * GRIP_SIZE, sb->sf_yofs + sb->sf_h, 0x808080ff);
	
	// end pos
	//boxColor(sb->dst, sb->sf_xofs + x_end - GRIP_SIZE, sb->sf_yofs, sb->sf_xofs + x_end , sb->sf_yofs + sb->sf_h, 0x808080ff);
}

int mouse_in_rect(int mouse_x, int mouse_y, int x, int y, int w, int h, int *ofs) {
	if((mouse_x >= x) && (mouse_x <= (x+w)) && (mouse_y >= y) && (mouse_y <= (y+h))) {
		if(ofs)
			*ofs = x-mouse_x;
		return 1;
	}
	return 0;
}

void scrollbar_event(scrollbar_t *sb, SDL_Event *evt) {
	int x, y;
	int xofs = sb->sf_xofs;
	int yofs = sb->sf_yofs;
	int w = sb->sf_w;
	int h = sb->sf_h;
	int start = xofs + sb->pos;
	int end = xofs; // TODO x_end
	int mid_start = xofs; // TODO
	int mid_len = 10; // TODO
	
	switch(evt->type) {
		
		case SDL_MOUSEBUTTONDOWN:
			x = evt->button.x;
			y = evt->button.y;
		
			if(mouse_in_rect(x,y,start,yofs,GRIP_SIZE,h,&(sb->mouse_ofs)))
				sb->mouse_mode = SB_START;
			
			else if(mouse_in_rect(x,y,mid_start,yofs,mid_len,h,&(sb->mouse_ofs)))
				sb->mouse_mode = SB_POS;
			
			else if(mouse_in_rect(x,y,end,yofs,GRIP_SIZE,h,&(sb->mouse_ofs)))
				sb->mouse_mode = SB_END;
			
			else
				sb->mouse_mode = SB_OUT;
			break;
		
		case SDL_MOUSEBUTTONUP:
			sb->mouse_mode = SB_OUT;
			break;
		
		case SDL_MOUSEMOTION:
			x = evt->motion.x;
			y = evt->motion.y;
		
			switch(sb->mouse_mode) {
				case SB_START:
					sb->pos = x - xofs + sb->mouse_ofs;
					//printf("%d\n",sb->pos);
					if(sb->pos < 0)
						sb->pos = 0;
					break;
				case SB_POS:
					break;
				case SB_END:
					break;
				default:
					break;
			}
			break;
		
	}
}
