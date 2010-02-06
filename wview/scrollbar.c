#include "scrollbar.h"
#include <assert.h>

scrollbar_t *scrollbar_create(SDL_Surface * sf, int sf_xofs, int sf_yofs,
			      int sf_w, int sf_h, unsigned long min_len,
			      unsigned long max_len)
{
	scrollbar_t *res = NULL;

	assert(min_len > 0);
	assert(max_len > min_len);

	if (!(res = malloc(sizeof(scrollbar_t))))
		return NULL;

	res->dst = sf;

	// widget pos
	res->sf_xofs = sf_xofs;
	res->sf_yofs = sf_yofs;

	// widget size
	res->sf_w = sf_w;
	res->sf_h = sf_h;

	// params
	res->max_len = max_len;
	res->min_len = min_len;

	// initial values
	res->pos = 0;
	res->len = max_len;

	res->grip_start = 0;
	res->grip_end = sf_w - GRIP_SIZE;

	res->mouse_mode = SB_OUT;

	return res;
}

void scrollbar_destroy(scrollbar_t * sb)
{
	free(sb);
}

void scrollbar_draw(scrollbar_t * sb)
{
	int ofs = sb->sf_xofs + sb->grip_start;

	// background
	boxColor(sb->dst, sb->sf_xofs, sb->sf_yofs, sb->sf_xofs + sb->sf_w,
		 sb->sf_yofs + sb->sf_h, 0x404040ff);

	// start pos
	boxColor(sb->dst, ofs, sb->sf_yofs, ofs + GRIP_SIZE - 1,
		 sb->sf_yofs + sb->sf_h, 0x808080ff);

	// filler
	ofs += GRIP_SIZE + GRIP_SPACE;
	boxColor(sb->dst, ofs, sb->sf_yofs,
		 sb->sf_xofs + sb->grip_end - GRIP_SPACE - 1,
		 sb->sf_yofs + sb->sf_h, 0x808080ff);

	// end pos
	ofs = sb->sf_xofs + sb->grip_end;
	boxColor(sb->dst, ofs, sb->sf_yofs, ofs + GRIP_SIZE - 1,
		 sb->sf_yofs + sb->sf_h, 0x808080ff);
}

int scrollbar_adjust(scrollbar_t * sb)
{
	float diff =
	    sb->grip_end - sb->grip_start - (GRIP_SIZE * 2) - (GRIP_SPACE * 2);
	float max_diff = sb->sf_w - (GRIP_SIZE * 3) - (GRIP_SPACE * 2);
	float max_pos = max_diff;
	float len = (float)(sb->max_len - sb->min_len) * diff;
	float pos = (float)(sb->max_len - sb->min_len) * sb->grip_start;
	float last_len = len;
	float last_pos = pos;

	len /= max_diff;
	len += sb->min_len;
	sb->len = len;

	//printf("len %f\n",len);

	pos /= max_pos;
	sb->pos = pos;

	//printf("pos %f\n",pos);

	if ((sb->pos != last_pos) || (sb->len != last_len))
		return SB_VALS_CHANGED;

	return 0;
}

int scrollbar_move(scrollbar_t * sb, int x)
{
	int changed = 0;
	int last_start = sb->grip_start;
	int last_end = sb->grip_end;
	int diff = sb->grip_end - sb->grip_start;

	switch (sb->mouse_mode) {

	case SB_START:
		LOWER_LIMIT(x, 0);
		UPPER_LIMIT(x,
			    sb->grip_end - (2 * GRIP_SPACE) - (2 * GRIP_SIZE));
		sb->grip_start = x;
		break;

	case SB_POS:
		LOWER_LIMIT(x, GRIP_SIZE + GRIP_SPACE);
		UPPER_LIMIT(x, sb->sf_w - diff + GRIP_SPACE);
		sb->grip_start = x - GRIP_SIZE - GRIP_SPACE;
		sb->grip_end = sb->grip_start + diff;
		break;

	case SB_END:
		LOWER_LIMIT(x,
			    sb->grip_start + (2 * GRIP_SPACE) +
			    (2 * GRIP_SIZE));
		UPPER_LIMIT(x, sb->sf_w - GRIP_SIZE);
		sb->grip_end = x;
		break;

	default:
		break;
	}

	if ((last_start != sb->grip_start) || (last_end != sb->grip_end))
		changed = SB_CHANGED;

	changed |= scrollbar_adjust(sb);

	return changed;
}

int mouse_in_rect(int mouse_x, int mouse_y, int x, int y, int w, int h,
		  int *ofs)
{
	if ((mouse_x >= x) && (mouse_x <= (x + w)) && (mouse_y >= y)
	    && (mouse_y <= (y + h))) {
		if (ofs)
			*ofs = x - mouse_x;
		return 1;
	}
	return 0;
}

int scrollbar_event(scrollbar_t * sb, SDL_Event * evt)
{
	int changed = 0;
	int x, y;
	int h = sb->sf_h;
	int grip_pos = sb->grip_start + GRIP_SIZE + GRIP_SPACE;
	int grip_pos_len = sb->grip_end - GRIP_SPACE - grip_pos;

	switch (evt->type) {

	case SDL_MOUSEBUTTONDOWN:
		x = evt->button.x - sb->sf_xofs;
		y = evt->button.y - sb->sf_yofs;

		if (mouse_in_rect
		    (x, y, sb->grip_start, 0, GRIP_SIZE, h, &(sb->mouse_ofs)))
			sb->mouse_mode = SB_START;

		else if (mouse_in_rect
			 (x, y, grip_pos, 0, grip_pos_len, h, &(sb->mouse_ofs)))
			sb->mouse_mode = SB_POS;

		else if (mouse_in_rect
			 (x, y, sb->grip_end, 0, GRIP_SIZE, h,
			  &(sb->mouse_ofs)))
			sb->mouse_mode = SB_END;

		else
			sb->mouse_mode = SB_OUT;
		break;

	case SDL_MOUSEBUTTONUP:
		sb->mouse_mode = SB_OUT;
		break;

	case SDL_MOUSEMOTION:
		x = evt->motion.x - sb->sf_xofs;
		y = evt->motion.y - sb->sf_yofs;

		if (sb->mouse_mode != SB_OUT)
			changed = scrollbar_move(sb, x + sb->mouse_ofs);
		break;

	}

	return changed;
}
