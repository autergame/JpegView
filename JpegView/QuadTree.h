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

	return (0.299f * re) + (0.587f * ge) + (0.114f * be);
}

struct quadnode
{
	float error;
	uint8_t* image;
	int depth, boxl, boxt, boxr, boxb, width, height, r, g, b;
    struct quadnode *children_tl, *children_tr, *children_bl, *children_br;
};

quadnode* init_quad(uint8_t* image, int width, int height, int boxl, int boxt, int boxr, int boxb, int depth)
{
	if (boxl < width && boxt < height)
	{
		quadnode* newquad = new quadnode{};
		newquad->boxl = boxl;
		newquad->boxt = boxt;
		newquad->boxr = boxr;
		newquad->boxb = boxb;
		newquad->depth = depth;
		newquad->image = image;
		newquad->width = boxr - boxl;
		newquad->height = boxb - boxt;

		int histr[256] = { 0 };
		int histg[256] = { 0 };
		int histb[256] = { 0 };

		for (int y = boxt; y < boxb; y++)
		{
			for (int x = boxl; x < boxr; x++)
			{
				if (x < width && y < height)
				{
					int index = (y * width + x) * 3;
					histr[image[index + 0]] += 1;
					histg[image[index + 1]] += 1;
					histb[image[index + 2]] += 1;
				}
			}
		}

		newquad->error = color_from_histogram(histr, histg, histb, newquad->r, newquad->g, newquad->b);

		return newquad;
	}
	else {
		return nullptr;
	}
}

void build_tree(quadnode* node, std::vector<quadnode*>& list,
	int width, int height, int max_depth, int threshold_error, int min_size, int max_size)
{
	if (node != nullptr)
	{
		if (
			(node->width > max_size && node->height > max_size) ||
			(
				(node->depth <= max_depth) &&
				(node->error >= threshold_error) &&
				(node->width > min_size && node->height > min_size)
				)
			)
		{
			int lr = node->boxl + (node->width / 2);
			int tb = node->boxt + (node->height / 2);

			node->children_tl = init_quad(node->image, width, height, node->boxl, node->boxt, lr, tb, node->depth + 1);
			node->children_tr = init_quad(node->image, width, height, lr, node->boxt, node->boxr, tb, node->depth + 1);
			node->children_bl = init_quad(node->image, width, height, node->boxl, tb, lr, node->boxb, node->depth + 1);
			node->children_br = init_quad(node->image, width, height, lr, tb, node->boxr, node->boxb, node->depth + 1);

			build_tree(node->children_tl, list, width, height, max_depth, threshold_error, min_size, max_size);
			build_tree(node->children_tr, list, width, height, max_depth, threshold_error, min_size, max_size);
			build_tree(node->children_bl, list, width, height, max_depth, threshold_error, min_size, max_size);
			build_tree(node->children_br, list, width, height, max_depth, threshold_error, min_size, max_size);
		}
		else {
			list.push_back(node);
		}
	}
}

void DrawLine(int x1, int y1, int x2, int y2, uint8_t* image, int width, int height)
{
	if (x2 - x1 == 0)
	{
		for (int y = y1; y <= y2; y++)
		{
			int index = (y * width + x1) * 3;
			image[index + 0] = 128;
			image[index + 1] = 128;
			image[index + 2] = 128;
		}
	}
	else if (y2 - y1 == 0)
	{
		for (int x = x1; x <= x2; x++)
		{
			int index = (y1 * width + x) * 3;
			image[index + 0] = 128;
			image[index + 1] = 128;
			image[index + 2] = 128;
		}
	}
}

void DrawRect(int x1, int y1, int x2, int y2, uint8_t* image, int width, int height)
{
	DrawLine(x1, y1, x2, y1, image, width, height);
	DrawLine(x1, y2, x2, y2, image, width, height);
	DrawLine(x1, y1, x1, y2, image, width, height);
	DrawLine(x2, y1, x2, y2, image, width, height);
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

int next_power_of_2(int n)
{
	int p = 1;
	if (n && !(n & (n - 1)))
		return n;

	while (p < n)
		p <<= 1;

	return p;
}

void render_quadtree(JpegView* jpeg, int max_depth, int threshold_error,
	int min_size, int max_size, bool drawline, bool quadtreepo2)
{
	deletemod(&jpeg->final_image);

	quadnode* root = nullptr;
	if (quadtreepo2)
	{
		int squaresize = next_power_of_2(jpeg->width > jpeg->height ? jpeg->width : jpeg->height);
		root = init_quad(jpeg->original_image, jpeg->width, jpeg->height, 0, 0, squaresize, squaresize, 0);
	} 
	else {
		root = init_quad(jpeg->original_image, jpeg->width, jpeg->height, 0, 0, jpeg->width, jpeg->height, 0);
	}

	std::vector<quadnode*> list;
	build_tree(root, list, jpeg->width, jpeg->height, max_depth, threshold_error, min_size, max_size);

	jpeg->final_image = new uint8_t[jpeg->width * jpeg->height * 3]{};
	for (size_t i = 0; i < list.size(); i++)
	{
		quadnode* quad = list[i];
		for (int y = quad->boxt; y < quad->boxb; y++)
		{
			for (int x = quad->boxl; x < quad->boxr; x++)
			{
				if (x < jpeg->width && y < jpeg->height)
				{
					int index = (y * jpeg->width + x) * 3;
					jpeg->final_image[index + 0] = quad->r;
					jpeg->final_image[index + 1] = quad->g;
					jpeg->final_image[index + 2] = quad->b;
				}
			}
		}
	}

	if (drawline)
	{
		for (size_t i = 0; i < list.size(); i++)
		{
			quadnode* quad = list[i];
			quad->boxr = quad->boxr >= jpeg->width ? jpeg->width - 1 : quad->boxr;
			quad->boxb = quad->boxb >= jpeg->height ? jpeg->height - 1 : quad->boxb;
			DrawRect(quad->boxl, quad->boxt, quad->boxr, quad->boxb, jpeg->final_image, jpeg->width, jpeg->height);
		}
	}

	clean_node(&root);
}

void render_quadtree_jpeg(JpegView* jpeg, int max_depth, int threshold_error,
	int min_size, int max_size, bool drawline, int quality)
{
	deletemod(&jpeg->final_image);

	int squaresize = next_power_of_2(jpeg->width > jpeg->height ? jpeg->width : jpeg->height);
	quadnode* root = init_quad(jpeg->original_image, jpeg->width, jpeg->height, 0, 0, squaresize, squaresize, 0);

	std::vector<quadnode*> list;
	build_tree(root, list, jpeg->width, jpeg->height, max_depth, threshold_error, min_size, max_size);

	int mwidth = jpeg->width;
	int mheight = jpeg->height;

	for (size_t i = 0; i < list.size(); i++)
	{
		quadnode* quad = list[i];
		if (quad->boxr > mwidth)
			mwidth = quad->boxr;
		if (quad->boxb > mheight)
			mheight = quad->boxb;
	}

	while (mwidth % max_depth != 0)
		mwidth++;
	while (mheight % max_depth != 0)
		mheight++;

	uint8_t** YCbCr = image_to_matrix(jpeg->original_image, jpeg->width, jpeg->height, mwidth, mheight);

	int tablesize = max_size / 2;
	float** DCTTable = new float*[tablesize] {};
	for (int i = 0; i < tablesize; i++)
		DCTTable[i] = generate_DCTtable(2 * (i + 1));

	float* alpha = generate_Alphatable(max_size);

	float factor = (float)quality;
	if (factor >= 50.f)
		factor = 200.f - factor * 2.f;
	else
		factor = 5000.f / factor;

	float* qMatrix = generate_QMatrix(max_size, factor);

	float* DCTMatrix = new float[max_size * max_size]{};

	uint8_t** result = new uint8_t*[3]{};
	for (int i = 0; i < 3; i++)
		result[i] = new uint8_t[mheight * mwidth]{};

	for (size_t i = 0; i < list.size(); i++)
	{
		quadnode* quad = list[i];

		int quadblocksize = quad->width > quad->height ? quad->width : quad->height;

		float block = 2.f / (float)quadblocksize;
		int tableindex = (quadblocksize / 2) - 1;

		for (int j = 0; j < 3; j++)
		{
			memset(DCTMatrix, 0, quadblocksize * quadblocksize);

			for (int k = 0; k < quadblocksize * quadblocksize; k++)
			{
				int u = (k % quadblocksize);
				int v = (k / quadblocksize);

				float sum = 0.0f;
				for (int l = 0; l < quadblocksize * quadblocksize; l++)
				{
					int x = (l % quadblocksize);
					int y = (l / quadblocksize);
					int index = (quad->boxt + y) * mwidth + (quad->boxl + x);

					float yv = DCTTable[tableindex][y * quadblocksize + v];
					float xu = DCTTable[tableindex][x * quadblocksize + u];

					sum += (YCbCr[j][index] - 128.f) * yv * xu;
				}

				DCTMatrix[v * max_size + u] = alpha[v * max_size + u] * sum * block;
			}

			for (int k = 0; k < quadblocksize * quadblocksize; k++)
			{
				int x = (k % quadblocksize);
				int y = (k / quadblocksize);
				int index = y * max_size + x;

				float qMatrix_value = minmaxq(qMatrix[index]);
				DCTMatrix[index] = roundf(DCTMatrix[index] / qMatrix_value) * qMatrix_value;
			}

			for (int k = 0; k < quadblocksize * quadblocksize; k++)
			{
				int x = (k % quadblocksize);
				int y = (k / quadblocksize);

				float sum = 0.f;
				for (int l = 0; l < quadblocksize * quadblocksize; l++)
				{
					int u = (l % quadblocksize);
					int v = (l / quadblocksize);

					float yv = DCTTable[tableindex][y * quadblocksize + v];
					float xu = DCTTable[tableindex][x * quadblocksize + u];
		
					sum += alpha[v * max_size + u] * DCTMatrix[v * max_size + u] * yv * xu;
				}

				int index = (quad->boxt + y) * mwidth + (quad->boxl + x);
				result[j][index] = minmaxcolor((sum * block) + 128.f);
			}
		}
	}

	jpeg->final_image = matrix_to_image(result, jpeg->width, jpeg->height, mwidth);

	if (drawline)
	{
		for (size_t i = 0; i < list.size(); i++)
		{
			quadnode* quad = list[i];
			quad->boxr = quad->boxr >= jpeg->width ? jpeg->width - 1 : quad->boxr;
			quad->boxb = quad->boxb >= jpeg->height ? jpeg->height - 1 : quad->boxb;
			DrawRect(quad->boxl, quad->boxt, quad->boxr, quad->boxb, jpeg->final_image, jpeg->width, jpeg->height);
		}
	}

	clean_node(&root);

	deletemod(&alpha);
	deletemod(&qMatrix);
	deletemod(&DCTMatrix);

	for (int i = 0; i < tablesize; i++)
		deletemod(&DCTTable[i]);
	deletemod(&DCTTable);

	for (int i = 0; i < 3; i++)
	{
		deletemod(&YCbCr[i]);
		deletemod(&result[i]);
	}

	deletemod(&YCbCr);
	deletemod(&result);	
}