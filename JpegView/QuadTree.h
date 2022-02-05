//author https://github.com/autergame
#pragma once

float weighted_average(int* hist, int& value)
{
	float error = 0.f;

	int total = 0;
	for (int i = 0; i < 256; i++)
		total += hist[i];
	if (total > 0)
	{
		for (int i = 0; i < 256; i++)
			value += i * hist[i];
		value /= total;

		for (int i = 0; i < 256; i++)
			error += hist[i] * (value - i) * (value - i);
		error = sqrtf(error / total);
	}

	return error;
}

float color_from_histogram(int* histr, int* histg, int* histb, int& r, int& g, int& b)
{
	float re = weighted_average(histr, r);
	float ge = weighted_average(histg, g);
	float be = weighted_average(histb, b);

	return re * 0.2989f + ge * 0.5870f + be * 0.1140f;
}

struct quadnode
{
	float error;
	uint8_t* image;
	int depth, boxl, boxt, boxr, boxb, r, g, b;
    struct quadnode *children_tl, *children_tr, *children_bl, *children_br;
};

quadnode* init_quad(uint8_t* image, int width, int boxl, int boxt, int boxr, int boxb, int depth)
{
	quadnode* newquad = new quadnode{};
	newquad->boxl = boxl;
	newquad->boxt = boxt;
	newquad->boxr = boxr;
	newquad->boxb = boxb;
	newquad->depth = depth;
	newquad->image = image;

	int* histr = new int[256]{};
	int* histg = new int[256]{};
	int* histb = new int[256]{};

	for (int y = boxt; y < boxb; y++)
	{
		for (int x = boxl; x < boxr; x++)
		{
			int index = (y * width + x) * 3;
			histr[image[index + 0]] += 1;
			histg[image[index + 1]] += 1;
			histb[image[index + 2]] += 1;
		}
	}

	newquad->error = color_from_histogram(histr, histg, histb, newquad->r, newquad->g, newquad->b);

	deletemod(&histr);
	deletemod(&histg);
	deletemod(&histb);

	return newquad;
}

void build_tree(quadnode* node, std::vector<quadnode*>& list, int width, int max_depth, int threshold_error, int min_size)
{
	if ((node->depth <= max_depth) && (node->error >= threshold_error) &&
		((node->boxr - node->boxl) > min_size && (node->boxb - node->boxt) > min_size))
	{
		int lr = node->boxl + (node->boxr - node->boxl) / 2;
		int tb = node->boxt + (node->boxb - node->boxt) / 2;

		node->children_tl = init_quad(node->image, width, node->boxl, node->boxt, lr, tb, node->depth + 1);
		node->children_tr = init_quad(node->image, width, lr, node->boxt, node->boxr, tb, node->depth + 1);
		node->children_bl = init_quad(node->image, width, node->boxl, tb, lr, node->boxb, node->depth + 1);
		node->children_br = init_quad(node->image, width, lr, tb, node->boxr, node->boxb, node->depth + 1);

		build_tree(node->children_tl, list, width, max_depth, threshold_error, min_size);
		build_tree(node->children_tr, list, width, max_depth, threshold_error, min_size);
		build_tree(node->children_bl, list, width, max_depth, threshold_error, min_size);
		build_tree(node->children_br, list, width, max_depth, threshold_error, min_size);
	} 
	else {
		list.push_back(node);
	}
}

void DrawLine(int x1, int y1, int x2, int y2, uint8_t* image, int width, int height)
{
	if (x2 - x1 == 0)
	{
		if (x1 <= width && y1 <= height)
		{
			if (x2 <= width && y2 <= height)
			{
				if (x1 >= width)
					x1 = width - 1;
				if (y1 >= height)
					y1 = height - 1;
				if (x2 >= width)
					x2 = width - 1;
				if (y2 >= height)
					y2 = height - 1;
				for (int y = y1; y <= y2; y++)
				{
					int index = (y * width + x1) * 3;
					image[index + 0] = 128;
					image[index + 1] = 128;
					image[index + 2] = 128;
				}
			}
		}
	}
	else if (y2 - y1 == 0)
	{
		if (x1 <= width && y1 <= height)
		{
			if (x2 <= width && y2 <= height)
			{
				if (x1 >= width)
					x1 = width - 1;
				if (y1 >= height)
					y1 = height - 1;
				if (x2 >= width)
					x2 = width - 1;
				if (y2 >= height)
					y2 = height - 1;
				for (int x = x1; x <= x2; x++)
				{
					int index = (y1 * width + x) * 3;
					image[index + 0] = 128;
					image[index + 1] = 128;
					image[index + 2] = 128;
				}
			}
		}
	}
}

void clean_node(quadnode** node)
{
	if (*node != nullptr)
	{
		clean_node(&(*node)->children_tl);
		clean_node(&(*node)->children_tr);
		clean_node(&(*node)->children_bl);
		clean_node(&(*node)->children_br);

		deletemod(node);
	}
}

void render_quadtree(JpegView* jpeg, int max_depth, int threshold_error, int min_size, bool drawline, 
	GLuint image_texturef, GLuint image_texturef_zoom)
{
	if (jpeg->final_image != nullptr)
		deletemod(&jpeg->final_image);

	quadnode* root = init_quad(jpeg->original_image, jpeg->width, 0, 0, jpeg->width, jpeg->height, 0);

	std::vector<quadnode*> list;
	build_tree(root, list, jpeg->width, max_depth, threshold_error, min_size);

	jpeg->final_image = new uint8_t[jpeg->width * jpeg->height * 3]{};
	for (size_t i = 0; i < list.size(); i++)
	{
		quadnode* quad = list[i];
		for (int y = quad->boxt; y < quad->boxb; y++)
		{
			for (int x = quad->boxl; x < quad->boxr; x++)
			{
				int index = (y * jpeg->width + x) * 3;
				jpeg->final_image[index + 0] = quad->r;
				jpeg->final_image[index + 1] = quad->g;
				jpeg->final_image[index + 2] = quad->b;
			}
		}
	}

	if (drawline)
	{
		for (size_t i = 0; i < list.size(); i++)
		{
			quadnode* quad = list[i];
			//if ((quad->boxr - quad->boxl) > min_size && (quad->boxb - quad->boxt) > min_size)
			//{
				DrawLine(quad->boxl, quad->boxt, quad->boxr, quad->boxt, jpeg->final_image, jpeg->width, jpeg->height);
				DrawLine(quad->boxl, quad->boxb, quad->boxr, quad->boxb, jpeg->final_image, jpeg->width, jpeg->height);
				DrawLine(quad->boxl, quad->boxt, quad->boxl, quad->boxb, jpeg->final_image, jpeg->width, jpeg->height);
				DrawLine(quad->boxr, quad->boxt, quad->boxr, quad->boxb, jpeg->final_image, jpeg->width, jpeg->height);
			//}
		}
	}

	clean_node(&root);

	glBindTexture(GL_TEXTURE_2D, image_texturef);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, jpeg->width, jpeg->height, GL_RGB, GL_UNSIGNED_BYTE, jpeg->final_image);

	glBindTexture(GL_TEXTURE_2D, image_texturef_zoom);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, jpeg->width, jpeg->height, GL_RGB, GL_UNSIGNED_BYTE, jpeg->final_image);
}