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
		error /= total;
		error = powf(error, 0.5f);
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
	bool leaf = false;
	int depth, boxl, boxt, boxr, boxb, r, g, b;
    struct quadnode *childrentl, *childrentr, *childrenbl, *childrenbr;
};

quadnode* initquad(uint8_t* image, int width, int boxl, int boxt, int boxr, int boxb, int depth)
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

	delete histr; 
	delete histg;
	delete histb;

	return newquad;
}

void build_tree(quadnode* node, int width, int max_depth, int errormax, int& max_depthe)
{
	if ((node->depth >= max_depth) || (node->error <= errormax))
	{
		if (node->depth > max_depthe)
			max_depthe = node->depth;
		node->leaf = true;
		return;
	}

	int lr = node->boxl + (node->boxr - node->boxl) / 2;
	int tb = node->boxt + (node->boxb - node->boxt) / 2;

	node->childrentl = initquad(node->image, width, node->boxl, node->boxt, lr, tb, node->depth + 1);
	node->childrentr = initquad(node->image, width, lr, node->boxt, node->boxr, tb, node->depth + 1);
	node->childrenbl = initquad(node->image, width, node->boxl, tb, lr, node->boxb, node->depth + 1);
	node->childrenbr = initquad(node->image, width, lr, tb, node->boxr, node->boxb, node->depth + 1);

	build_tree(node->childrentl, width, max_depth, errormax, max_depthe);
	build_tree(node->childrentr, width, max_depth, errormax, max_depthe);
	build_tree(node->childrenbl, width, max_depth, errormax, max_depthe);
	build_tree(node->childrenbr, width, max_depth, errormax, max_depthe);
}

void get_leaf_nodes_recusion(quadnode* node, std::vector<quadnode*>& list)
{
	if (node)
	{
		if (node->leaf == true)
			list.push_back(node);
		else
		{
			get_leaf_nodes_recusion(node->childrentl, list);
			get_leaf_nodes_recusion(node->childrentr, list);
			get_leaf_nodes_recusion(node->childrenbl, list);
			get_leaf_nodes_recusion(node->childrenbr, list);
		}
	}
}

void DrawLine(int x1, int y1, int x2, int y2, uint8_t* image, int width, int height)
{
	if (x2 - x1 == 0)
	{
		if (y2 < y1)
			std::swap(y1, y2);
		if (y1 <  height && y2 < height)
		{
			for (int y = y1; y <= y2; y++)
			{
				int index = (y * width + x1) * 3;
				image[index + 0] = 128;
				image[index + 1] = 128;
				image[index + 2] = 128;
			}
		}
		return;
	}
	if (y2 - y1 == 0)
	{
		if (x2 < x1)
			std::swap(x1, x2);
		if (x1 < width && x2 < width)
		{
			for (int x = x1; x <= x2; x++)
			{
				int index = (y1 * width + x) * 3;
				image[index + 0] = 128;
				image[index + 1] = 128;
				image[index + 2] = 128;
			}
		}
		return;
	}
}

void cleanroot(quadnode* root)
{
	if (root)
	{
		if (root->childrenbl)
		{
			cleanroot(root->childrenbl);
			delete root->childrenbl;
		}
		if (root->childrenbr)
		{
			cleanroot(root->childrenbr);
			delete root->childrenbr;
		}
		if (root->childrentl)
		{
			cleanroot(root->childrentl);
			delete root->childrentl;
		}
		if (root->childrentr)
		{
			cleanroot(root->childrentr);
			delete root->childrentr;
		}
	}
}

void renderquad(JpegView* jpeg, quadnode* root, int depth, int error, int width, int height, bool drawline, GLuint image_texturef)
{
	if (jpeg->finalimage)
		delete jpeg->finalimage;

	int max_depthe = 0;
	build_tree(root, width, depth, error, max_depthe);

	std::vector<quadnode*> list;
	get_leaf_nodes_recusion(root, list);

	jpeg->finalimage = new uint8_t[width * height * 3]{};
	for (size_t i = 0; i < list.size(); i++)
	{
		quadnode* quad = list[i];
		for (int y = quad->boxt; y < quad->boxb; y++)
		{
			for (int x = quad->boxl; x < quad->boxr; x++)
			{
				int index = (y * width + x) * 3;
				jpeg->finalimage[index + 0] = quad->r;
				jpeg->finalimage[index + 1] = quad->g;
				jpeg->finalimage[index + 2] = quad->b;
			}
		}
	}

	if (drawline)
	{
		for (size_t i = 0; i < list.size(); i++)
		{
			quadnode* quad = list[i];
			if ((quad->boxr - quad->boxl) >= 3 && (quad->boxb - quad->boxt) >= 3)
			{
				DrawLine(quad->boxl, quad->boxt, quad->boxr, quad->boxt, jpeg->finalimage, width, height);
				DrawLine(quad->boxl, quad->boxb, quad->boxr, quad->boxb, jpeg->finalimage, width, height);
				DrawLine(quad->boxr, quad->boxt, quad->boxr, quad->boxb, jpeg->finalimage, width, height);
				DrawLine(quad->boxl, quad->boxt, quad->boxl, quad->boxb, jpeg->finalimage, width, height);
			}
		}
	}

	glBindTexture(GL_TEXTURE_2D, image_texturef);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, jpeg->finalimage);

	cleanroot(root);
}