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

bool compare_depth(const quadnode* quad1, const quadnode* quad2)
{
	return quad1->depth < quad2->depth;
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
		quad->boxr = quad->boxr >= jpeg->width ? jpeg->width - 1 : quad->boxr;
		quad->boxb = quad->boxb >= jpeg->height ? jpeg->height - 1 : quad->boxb;
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

	int tablesize = max_size / 2;
	float** DCTTable = new float*[tablesize]{};
	for (int i = 0; i < tablesize; i++)
	{
		int blocksize = 2 * (i + 1);
		DCTTable[i] = generate_DCTtable(blocksize);
	}

	float* alpha = generate_Alphatable(max_size);

	float factor = (float)quality;
	if (factor >= 50.f)
		factor = 200.f - factor * 2.f;
	else
		factor = 5000.f / factor;

	float* qMatrix = generate_QMatrix(max_size, factor);

	float** DCT = new float*[3]{};
	for (int i = 0; i < 3; i++)
		DCT[i] = new float[jpeg->height * jpeg->width]{};

	jpeg->final_image = new uint8_t[jpeg->width * jpeg->height * 3]{};
	for (size_t i = 0; i < list.size(); i++)
	{
		quadnode* quad = list[i];
		if (quad->boxr >= jpeg->width) {
			quad->boxr = jpeg->width - 1;
			quad->width = quad->boxr - quad->boxl;
		}
		if (quad->boxb >= jpeg->height) {
			quad->boxb = jpeg->height - 1;
			quad->height = quad->boxb - quad->boxt;
		}

		float block = 2.f / (float)quad->width;
		int tableindex = (next_power_of_2(quad->width) / 2) - 1;

		for (int by = quad->boxt; by < quad->boxb; by += quad->height)
		{
			for (int bx = quad->boxl; bx < quad->boxr; bx += quad->width)
			{
				for (int u = 0; u < quad->width; u++)
				{
					for (int v = 0; v < quad->height; v++)
					{
						float sum1 = 0.0f;
						float sum2 = 0.0f;
						float sum3 = 0.0f;
						for (int x = 0; x < quad->width; x++)
						{
							for (int y = 0; y < quad->height; y++)
							{
								int index = ((by + y) * jpeg->width + (bx + x)) * 3;

								float yv = DCTTable[tableindex][y * quad->width + v];
								float xu = DCTTable[tableindex][x * quad->width + u];

								sum1 += (jpeg->original_image[index + 0] - 128.f) * yv * xu;
								sum2 += (jpeg->original_image[index + 1] - 128.f) * yv * xu;
								sum3 += (jpeg->original_image[index + 2] - 128.f) * yv * xu;
							}
						}

						int index = (by + v) * jpeg->width + (bx + u);

						DCT[0][index] = alpha[u * quad->width + v] * sum1 * block;
						DCT[1][index] = alpha[u * quad->width + v] * sum2 * block;
						DCT[2][index] = alpha[u * quad->width + v] * sum3 * block;
					}
				}

				for (int y = 0; y < quad->height; y++)
				{
					for (int x = 0; x < quad->width; x++)
					{
						int index = (by + y) * jpeg->width + (bx + x);
						int qindex = ((by + y) % quad->height) * quad->width + ((bx + x) % quad->width);

						float qMatrix_value = minmaxq(qMatrix[qindex]);
						DCT[0][index] = roundf(DCT[0][index] / qMatrix_value) * qMatrix_value;
						DCT[1][index] = roundf(DCT[1][index] / qMatrix_value) * qMatrix_value;
						DCT[2][index] = roundf(DCT[2][index] / qMatrix_value) * qMatrix_value;
					}
				}

				for (int x = 0; x < quad->width; x++)
				{
					for (int y = 0; y < quad->height; y++)
					{
						float isum1 = 0.f;
						float isum2 = 0.f;
						float isum3 = 0.f;
						for (int u = 0; u < quad->width; u++)
						{
							for (int v = 0; v < quad->height; v++)
							{
								int index = (by + v) * jpeg->width + (bx + u);

								float yv = DCTTable[tableindex][y * quad->width + v];
								float xu = DCTTable[tableindex][x * quad->width + u];

								isum1 += alpha[u * quad->width + v] * DCT[0][index] * yv * xu;
								isum2 += alpha[u * quad->width + v] * DCT[1][index] * yv * xu;
								isum3 += alpha[u * quad->width + v] * DCT[2][index] * yv * xu;
							}
						}

						int index = ((by + y) * jpeg->width + (bx + x)) * 3;

						jpeg->final_image[index + 0] = minmaxcolor((isum1 * block) + 128.f);
						jpeg->final_image[index + 1] = minmaxcolor((isum2 * block) + 128.f);
						jpeg->final_image[index + 2] = minmaxcolor((isum3 * block) + 128.f);
					}
				}
			}
		}
	}

	if (drawline)
	{
		for (size_t i = 0; i < list.size(); i++)
		{
			quadnode* quad = list[i];
			DrawRect(quad->boxl, quad->boxt, quad->boxr, quad->boxb, jpeg->final_image, jpeg->width, jpeg->height);
		}
	}

	clean_node(&root);
}