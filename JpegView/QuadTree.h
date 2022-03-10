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
	int16_t* DCTMatrix_zigzag[3];
	int depth, boxl, boxt, boxr, boxb, width_block_size, height_block_size, r, g, b;
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
		newquad->width_block_size = boxr - boxl;
		newquad->height_block_size = boxb - boxt;

		int histr[256] = { 0 };
		int histg[256] = { 0 };
		int histb[256] = { 0 };

		int boxrlimited = boxr >= width ? width - 1 : boxr;
		int boxblimited = boxb >= height ? height - 1 : boxb;

		for (int y = boxt; y < boxblimited; y++)
		{
			for (int x = boxl; x < boxrlimited; x++)
			{
				int index = (y * width + x) * 3;
				histr[image[index + 0]] += 1;
				histg[image[index + 1]] += 1;
				histb[image[index + 2]] += 1;
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
			(node->width_block_size > max_size && node->height_block_size > max_size) ||
			(
				(node->depth <= max_depth) &&
				(node->error >= threshold_error) &&
				(node->width_block_size > min_size && node->height_block_size > min_size)
				)
			)
		{
			int lr = node->boxl + (node->width_block_size / 2);
			int tb = node->boxt + (node->height_block_size / 2);

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

		for (int i = 0; i < 3; i++)
			deletemod(&(*node)->DCTMatrix_zigzag[i]);
		deletemod(node);
	}
}

int next_power_of_2(int n)
{
	if (n && !(n & (n - 1)))
		return n;

	int p = 1;
	while (p < n)
		p <<= 1;
	return p;
}

#pragma pack(push,1)
struct quadnodesimple
{
	uint16_t x;
	uint16_t y;
	uint8_t width_block_size;
	uint8_t height_block_size;
	uint8_t r;
	uint8_t g;
	uint8_t b;
};
#pragma pack(pop)

#pragma pack(push,1)
struct quadnodejpeg
{
	uint16_t x;
	uint16_t y;
	uint8_t block_size;
};
#pragma pack(pop)

int savequad(char* filename, std::vector<quadnode*>& listquad,
	int width, int height, int quality, bool qtablege, bool useycbcr)
{
	FILE* filequad;
	errno_t err = fopen_s(&filequad, filename, "wb");
	if (err)
	{
		char errMsg[255] = { '\0' };
		strerror_s(errMsg, 255, err);
		char msgTitle[512] = { '\0' };
		sprintf_s(msgTitle, 512, "Cannot write file %s %s", filename, errMsg);
		MessageBoxA(nullptr, msgTitle, "ERROR", MB_OK | MB_ICONERROR | MB_TOPMOST);
		return 0;
	}

	const char* quadsignature = "QUADMIND";
	fwrite(quadsignature, 8, 1, filequad);

	int16_t swidth = (int16_t)width;
	int16_t sheight = (int16_t)height;
	fwrite(&swidth, 2, 1, filequad);
	fwrite(&sheight, 2, 1, filequad);

	uint32_t quadlistsize = (uint32_t)listquad.size();
	fwrite(&quadlistsize, 4, 1, filequad);

	uint8_t version = listquad[0]->DCTMatrix_zigzag[0] == nullptr ? 0 : 1;
	fwrite(&version, 1, 1, filequad);

	if (version == 0)
	{
		quadnodesimple* qnsarray = new quadnodesimple[quadlistsize]{};
		for (uint32_t i = 0; i < quadlistsize; i++)
		{
			quadnode* quad = listquad[i];
			qnsarray[i].x = (int16_t)quad->boxl;
			qnsarray[i].y = (int16_t)quad->boxt;
			qnsarray[i].width_block_size = (uint8_t)quad->width_block_size;
			qnsarray[i].height_block_size = (uint8_t)quad->height_block_size;
			qnsarray[i].r = quad->r;
			qnsarray[i].g = quad->g;
			qnsarray[i].b = quad->b;
		}	

		uint8_t sha512qnj[64] = { 0 };
		sha512((uint8_t*)qnsarray, sizeof(quadnodesimple) * quadlistsize, sha512qnj);
		fwrite(sha512qnj, 64, 1, filequad);

		const char* qnsstart = "SQNS";
		fwrite(qnsstart, 4, 1, filequad);

		fwrite(qnsarray, sizeof(quadnodesimple), quadlistsize, filequad);

		const char* qnsend = "EQNS";
		fwrite(qnsend, 4, 1, filequad);

		deletemod(&qnsarray);
	}
	else if (version == 1)
	{
		uint8_t suseycbcr = (uint8_t)useycbcr;
		fwrite(&suseycbcr, 1, 1, filequad);

		uint8_t sqtablege = (uint8_t)qtablege;
		fwrite(&sqtablege, 1, 1, filequad);

		uint8_t squality = (uint8_t)quality;
		fwrite(&squality, 1, 1, filequad);

		uint32_t quadblocksizemax = 0;
		quadnodejpeg* qnjarray = new quadnodejpeg[quadlistsize]{};
		for (uint32_t i = 0; i < quadlistsize; i++)
		{
			quadnode* quad = listquad[i];
			qnjarray[i].x = (int16_t)quad->boxl;
			qnjarray[i].y = (int16_t)quad->boxt;
			qnjarray[i].block_size = (uint8_t)log2i(quad->width_block_size);
			quadblocksizemax += quad->width_block_size * quad->height_block_size * 3;
		}

		uint8_t sha512qnj[64] = { 0 };
		sha512((uint8_t*)qnjarray, sizeof(quadnodejpeg) * quadlistsize, sha512qnj);
		fwrite(sha512qnj, 64, 1, filequad);

		const char* qnjstart = "SQNJ";
		fwrite(qnjstart, 4, 1, filequad);

		fwrite(qnjarray, sizeof(quadnodejpeg), quadlistsize, filequad);

		const char* qnjend = "EQNJ";
		fwrite(qnjend, 4, 1, filequad);

		deletemod(&qnjarray);

		fwrite(&quadblocksizemax, 4, 1, filequad);

		int index = 0;
		int16_t* dctmzzarray = new int16_t[quadblocksizemax]{};
		for (uint32_t i = 0; i < quadlistsize; i++)
		{
			quadnode* quad = listquad[i];

			for (int j = 0; j < 3; j++)
				for (int k = 0; k < quad->width_block_size * quad->height_block_size; k++)
					dctmzzarray[index++] = quad->DCTMatrix_zigzag[j][k];
		}

		uint8_t sha512dctzz[64] = { 0 };
		sha512((uint8_t*)dctmzzarray, quadblocksizemax * 2, sha512dctzz);
		fwrite(sha512dctzz, 64, 1, filequad);

		const char* dctzzstart = "SDCT";
		fwrite(dctzzstart, 4, 1, filequad);

		fwrite(dctmzzarray, 2, quadblocksizemax, filequad);

		const char* dctzzend = "EDCT";
		fwrite(dctzzend, 4, 1, filequad);

		deletemod(&dctmzzarray);
	}

	fclose(filequad);

	return 1;
}

uint8_t* loadquad(char* filename, int* width, int* height)
{
	FILE* filequad;
	errno_t err = fopen_s(&filequad, filename, "rb");
	if (err)
	{
		char errMsg[255] = { '\0' };
		strerror_s(errMsg, 255, err);
		char msgTitle[512] = { '\0' };
		sprintf_s(msgTitle, 512, "Cannot read file %s %s", filename, errMsg);
		MessageBoxA(nullptr, msgTitle, "ERROR", MB_OK | MB_ICONERROR | MB_TOPMOST);
		return nullptr;
	}

	char quadsignature[9] = { '\0' };
	fread(quadsignature, 8, 1, filequad);

	uint8_t* resultimage = nullptr;
	if (strcmp(quadsignature, "QUADMIND") == 0)
	{
		int16_t swidth = 0, sheight = 0;
		fread(&swidth, 2, 1, filequad);
		fread(&sheight, 2, 1, filequad);
		int iwidth = *width = swidth;
		int iheight = *height = sheight;

		uint32_t quadlistsize = 0;
		fread(&quadlistsize, 4, 1, filequad);

		uint8_t version = 0;
		fread(&version, 1, 1, filequad);

		if (version == 0)
		{
			uint8_t sha512qns[64] = { 0 };
			fread(&sha512qns, 64, 1, filequad);

			char qnsstart[5] = { '\0' };
			fread(qnsstart, 4, 1, filequad);

			if (strcmp(qnsstart, "SQNS") != 0)
				return nullptr;

			quadnodesimple* qnsarray = new quadnodesimple[quadlistsize]{};
			fread(qnsarray, sizeof(quadnodesimple), quadlistsize, filequad);

			char qnsend[5] = { '\0' };
			fread(qnsend, 4, 1, filequad);

			if (strcmp(qnsend, "EQNS") != 0)
				return nullptr;

			uint8_t sha512qnstest[64] = { 0 };
			sha512((uint8_t*)qnsarray, sizeof(quadnodesimple) * quadlistsize, sha512qnstest);

			if (memcmp(sha512qnstest, sha512qns, 64) != 0)
				return nullptr;

			resultimage = new uint8_t[iwidth * iheight * 3]{};
			for (uint32_t i = 0; i < quadlistsize; i++)
			{
				quadnodesimple* quad = &qnsarray[i];

				int quadboxl = quad->x;
				int quadboxt = quad->y;
				int quadboxr = quadboxl + (int)quad->width_block_size;
				int quadboxb = quadboxt + (int)quad->height_block_size;

				for (int y = quadboxt; y < quadboxb; y++)
				{
					for (int x = quadboxl; x < quadboxr; x++)
					{
						int index = (y * iwidth + x) * 3;
						resultimage[index + 0] = quad->r;
						resultimage[index + 1] = quad->g;
						resultimage[index + 2] = quad->b;
					}
				}
			}

			deletemod(&qnsarray);
		}
		else if (version == 1)
		{
			uint8_t useycbcr = 0;
			fread(&useycbcr, 1, 1, filequad);

			uint8_t qtablege = 0;
			fread(&qtablege, 1, 1, filequad);

			uint8_t squality = 0;
			fread(&squality, 1, 1, filequad);

			uint8_t sha512qnj[64] = { 0 };
			fread(&sha512qnj, 64, 1, filequad);

			char qnjstart[5] = { '\0' };
			fread(qnjstart, 4, 1, filequad);

			if (strcmp(qnjstart, "SQNJ") != 0)
				return nullptr;

			quadnodejpeg* qnjarray = new quadnodejpeg[quadlistsize]{};
			fread(qnjarray, sizeof(quadnodejpeg), quadlistsize, filequad);

			char qnjend[5] = { '\0' };
			fread(qnjend, 4, 1, filequad);

			if (strcmp(qnjend, "EQNJ") != 0)
				return nullptr;

			uint8_t sha512qnstest[64] = { 0 };
			sha512((uint8_t*)qnjarray, sizeof(quadnodejpeg) * quadlistsize, sha512qnstest);

			if (memcmp(sha512qnstest, sha512qnj, 64) != 0)
				return nullptr;

			uint32_t quadblocksizemax = 0;
			fread(&quadblocksizemax, 4, 1, filequad);

			uint8_t sha512dctzz[64] = { 0 };
			fread(&sha512dctzz, 64, 1, filequad);

			char dctmzzstart[5] = { '\0' };
			fread(dctmzzstart, 4, 1, filequad);

			if (strcmp(dctmzzstart, "SDCT") != 0)
				return nullptr;

			int16_t* dctmzzarray = new int16_t[quadblocksizemax] {};
			fread(dctmzzarray, 2, quadblocksizemax, filequad);

			char dctmzzend[5] = { '\0' };
			fread(dctmzzend, 4, 1, filequad);

			if (strcmp(dctmzzend, "EDCT") != 0)
				return nullptr;

			uint8_t sha512dctmzztest[64] = { 0 };
			sha512((uint8_t*)dctmzzarray, quadblocksizemax * 2, sha512dctmzztest);

			if (memcmp(sha512dctmzztest, sha512dctzz, 64) != 0)
				return nullptr;

			int max_size = 0;
			int mwidth = iwidth;
			int mheight = iheight;
			for (uint32_t i = 0; i < quadlistsize; i++)
			{
				quadnodejpeg* quad = &qnjarray[i];

				int block_size = 1 << (int)quad->block_size;

				int quadboxl = quad->x;
				int quadboxt = quad->y;
				int quadboxr = quadboxl + block_size;
				int quadboxb = quadboxt + block_size;

				if (quadboxr > mwidth)
					mwidth = quadboxr;
				if (quadboxb > mheight)
					mheight = quadboxb;

				if (block_size > max_size)
					max_size = block_size;
			}

			mwidth = round_up_block_size(mwidth, max_size);
			mheight = round_up_block_size(mheight, max_size);

			int tablesize = log2i(max_size);

			//float** DCTTable = new float*[tablesize]{};
			//for (int i = 0; i < tablesize; i++)
			//	DCTTable[i] = generate_DCT_table(1 << (i + 1));

			//float** alphaTable = new float*[tablesize]{};
			//for (int i = 0; i < tablesize; i++)
			//	alphaTable[i] = generate_Alpha_table(1 << (i + 1));

			int** DeZigZagtable = new int* [tablesize] {};
			for (int i = 0; i < tablesize; i++)
				DeZigZagtable[i] = generate_ZigZag_table(1 << (i + 1));

			float** image_block = new float* [tablesize] {};
			for (int i = 0; i < tablesize; i++)
				image_block[i] = new float[(1 << (i + 1)) * (1 << (i + 1))]{};

			float** DCTMatrix = new float* [tablesize] {};
			for (int i = 0; i < tablesize; i++)
				DCTMatrix[i] = new float[(1 << (i + 1)) * (1 << (i + 1))]{};

			float factor = (float)squality;
			if (factor >= 50.f && factor <= 99.f)
				factor = 200.f - factor * 2.f;
			else if (factor < 50.f)
				factor = 5000.f / factor;
			else if (factor == 100.f)
				factor = 1.f;

			float* qMatrix_luma = generate_QMatrix(qMatrix_luma_const, max_size, factor, qtablege);
			float* qMatrix_chroma = generate_QMatrix(qMatrix_chroma_const, max_size, factor, qtablege);

			uint8_t** result = new uint8_t*[3]{};
			result[0] = new uint8_t[mheight * mwidth]{};
			result[1] = new uint8_t[mheight * mwidth]{};
			result[2] = new uint8_t[mheight * mwidth]{};

			jpeg_steps_struct* jss = new jpeg_steps_struct{};
			jss->mwidth = mwidth;

			int index = 0;
			for (uint32_t i = 0; i < quadlistsize; i++)
			{
				quadnodejpeg* quad = &qnjarray[i];

				int block_size = 1 << (int)quad->block_size;
				int quadblocksizefull = (block_size * block_size);
				int tableindex = log2i(block_size) - 1;

				jss->image_block = image_block[tableindex];
				jss->DCTMatrix = DCTMatrix[tableindex];
				jss->ZigZagtable = DeZigZagtable[tableindex];
				//jss->DCTTable = DCTTable[tableindex];
				//jss->alphaTable = alphaTable[tableindex];
				//jss->block = 2.f / (float)quadblocksize;
				jss->block_size = block_size;
				jss->block_size_index = tableindex;
				jss->start_x = quad->x;
				jss->start_y = quad->y;

				JPEG_steps_decompress_load(jss, result[0], qMatrix_luma, dctmzzarray + index);
				index += quadblocksizefull;

				JPEG_steps_decompress_load(jss, result[1], qMatrix_chroma, dctmzzarray + index);
				index += quadblocksizefull;

				JPEG_steps_decompress_load(jss, result[2], qMatrix_chroma, dctmzzarray + index);
				index += quadblocksizefull;
			}

			if (useycbcr)
				resultimage = YCbCr_matrix_to_image(result, iwidth, iheight, mwidth);
			else
				resultimage = RGB_matrix_to_image(result, iwidth, iheight, mwidth);

			deletemod(&jss);
			deletemod(&qMatrix_luma);
			deletemod(&qMatrix_chroma);

			for (int i = 0; i < tablesize; i++)
			{
				deletemod(&DCTMatrix[i]);
				deletemod(&image_block[i]);
				deletemod(&DeZigZagtable[i]);
				//deletemod(&alphaTable[i]);
				//deletemod(&DCTTable[i]);
			}
			deletemod(&DCTMatrix);
			deletemod(&image_block);
			deletemod(&DeZigZagtable);
			//deletemod(&alphaTable);
			//deletemod(&DCTTable);

			for (int i = 0; i < 3; i++)
				deletemod(&result[i]);

			deletemod(&result);
			deletemod(&qnjarray);
			deletemod(&dctmzzarray);
		}
	}

	fclose(filequad);

	return resultimage;
}

std::vector<quadnode*> render_quadtree(quadnode** rootquad, JpegView* jpeg, int max_depth, int threshold_error,
	int min_size, int max_size, bool drawline, bool quadtreepow2, int subsampling_index)
{
	if (quadtreepow2)
	{
		int squaresize = next_power_of_2(jpeg->width > jpeg->height ? jpeg->width : jpeg->height);
		*rootquad = init_quad(jpeg->original_image, jpeg->width, jpeg->height, 0, 0, squaresize, squaresize, 0);
	} 
	else {
		*rootquad = init_quad(jpeg->original_image, jpeg->width, jpeg->height, 0, 0, jpeg->width, jpeg->height, 0);
	}

	std::vector<quadnode*> listquad;
	build_tree(*rootquad, listquad, jpeg->width, jpeg->height, max_depth, threshold_error, min_size, max_size);

	deletemod(&jpeg->final_image);
	jpeg->final_image = new uint8_t[jpeg->width * jpeg->height * 3]{};
	for (size_t i = 0; i < listquad.size(); i++)
	{
		quadnode* quad = listquad[i];
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
		for (size_t i = 0; i < listquad.size(); i++)
		{
			quadnode* quad = listquad[i];
			DrawRect(quad->boxl, quad->boxt, quad->boxr, quad->boxb, jpeg->final_image, jpeg->width, jpeg->height);
		}
	}

	return listquad;
}

std::vector<quadnode*> render_quadtree_jpeg(quadnode** rootquad, JpegView* jpeg, int max_depth, int threshold_error,
	int min_size, int max_size, bool drawline, int quality, bool qtablege, int subsampling_index, bool useycbcr)
{
	int squaresize = next_power_of_2(jpeg->width > jpeg->height ? jpeg->width : jpeg->height);
	*rootquad = init_quad(jpeg->original_image, jpeg->width, jpeg->height, 0, 0, squaresize, squaresize, 0);

	std::vector<quadnode*> listquad;
	build_tree(*rootquad, listquad, jpeg->width, jpeg->height, max_depth, threshold_error, min_size, max_size);

	int mwidth = jpeg->width;
	int mheight = jpeg->height;

	for (size_t i = 0; i < listquad.size(); i++)
	{
		quadnode* quad = listquad[i];
		if (quad->boxr > mwidth)
			mwidth = quad->boxr;
		if (quad->boxb > mheight)
			mheight = quad->boxb;
	}

	mwidth = round_up_block_size(mwidth, max_size);
	mheight = round_up_block_size(mheight, max_size);

	uint8_t** image_converted;
	if (useycbcr)
		image_converted = image_to_matrix_YCbCr(jpeg->original_image,
			jpeg->width, jpeg->height, mwidth, mheight, subsampling_index);
	else
		image_converted = image_to_matrix_RGB(jpeg->original_image,
			jpeg->width, jpeg->height, mwidth, mheight, subsampling_index);

	int tablesize = log2i(max_size);

	//float** DCTTable = new float*[tablesize]{};
	//for (int i = 0; i < tablesize; i++)
	//	DCTTable[i] = generate_DCT_table(1 << (i + 1));

	//float** alphaTable = new float*[tablesize]{};
	//for (int i = 0; i < tablesize; i++)
	//	alphaTable[i] = generate_Alpha_table(1 << (i + 1));

	int** ZigZagtable = new int*[tablesize]{};
	for (int i = 0; i < tablesize; i++)
		ZigZagtable[i] = generate_ZigZag_table(1 << (i + 1));

	float** image_block = new float*[tablesize]{};
	for (int i = 0; i < tablesize; i++)
		image_block[i] = new float[(1 << (i + 1)) * (1 << (i + 1))]{};

	float** DCTMatrix = new float*[tablesize] {};
	for (int i = 0; i < tablesize; i++)
		DCTMatrix[i] = new float[(1 << (i + 1)) * (1 << (i + 1))]{};

	float* qMatrix_luma, *qMatrix_chroma;
	if (!jpeg->compression_rate)
	{
		float factor = (float)quality;
		if (factor >= 50.f && factor <= 99.f)
			factor = 200.f - factor * 2.f;
		else if (factor < 50.f)
			factor = 5000.f / factor;
		else if (factor == 100.f)
			factor = 1.f;
		qMatrix_luma = generate_QMatrix(qMatrix_luma_const, max_size, factor, qtablege);
		qMatrix_chroma = generate_QMatrix(qMatrix_chroma_const, max_size, factor, qtablege);
	}
	else {
		qMatrix_luma = generate_QMatrix_nofactor(qMatrix_luma_const, max_size, qtablege);
		qMatrix_chroma = generate_QMatrix_nofactor(qMatrix_chroma_const, max_size, qtablege);
	}

	uint8_t** result = new uint8_t*[3]{};
	result[0] = new uint8_t[mheight * mwidth]{};
	result[1] = new uint8_t[mheight * mwidth]{};
	result[2] = new uint8_t[mheight * mwidth]{};

	jpeg_steps_struct* jss = new jpeg_steps_struct{};
	jss->mwidth = mwidth;
	jss->compression_rate = jpeg->compression_rate;
	jss->quality_start = jpeg->quality_start;
	jss->Q_control = 100.f - jpeg->quality_start;

	for (size_t i = 0; i < listquad.size(); i++)
	{
		quadnode* quad = listquad[i];

		int quadblocksize = quad->width_block_size > quad->height_block_size 
			? quad->width_block_size : quad->height_block_size;
		int tableindex = log2i(quadblocksize) - 1;

		jss->image_block = image_block[tableindex];
		jss->DCTMatrix = DCTMatrix[tableindex];
		jss->ZigZagtable = ZigZagtable[tableindex];
		//jss->DCTTable = DCTTable[tableindex];
		//jss->alphaTable = alphaTable[tableindex];
		//jss->block = 2.f / (float)quadblocksize;
		jss->block_size = quadblocksize;
		jss->block_size_index = tableindex;
		jss->start_x = quad->boxl;
		jss->start_y = quad->boxt;

		quad->DCTMatrix_zigzag[0] = Quad_JPEG_steps(jss, result[0], image_converted[0], qMatrix_luma);
		quad->DCTMatrix_zigzag[1] = Quad_JPEG_steps(jss, result[1], image_converted[1], qMatrix_chroma);
		quad->DCTMatrix_zigzag[2] = Quad_JPEG_steps(jss, result[2], image_converted[2], qMatrix_chroma);
	}

	deletemod(&jpeg->final_image);
	if (useycbcr)
		jpeg->final_image = YCbCr_matrix_to_image(result, jpeg->width, jpeg->height, mwidth);
	else
		jpeg->final_image = RGB_matrix_to_image(result, jpeg->width, jpeg->height, mwidth);

	if (drawline)
	{
		for (size_t i = 0; i < listquad.size(); i++)
		{
			quadnode* quad = listquad[i];
			quad->boxr = quad->boxr >= jpeg->width ? jpeg->width - 1 : quad->boxr;
			quad->boxb = quad->boxb >= jpeg->height ? jpeg->height - 1 : quad->boxb;
			DrawRect(quad->boxl, quad->boxt, quad->boxr, quad->boxb, jpeg->final_image, jpeg->width, jpeg->height);
		}
	}

	deletemod(&jss);
	deletemod(&qMatrix_luma);
	deletemod(&qMatrix_chroma);

	for (int i = 0; i < tablesize; i++)
	{
		deletemod(&DCTMatrix[i]);
		deletemod(&image_block[i]);
		deletemod(&ZigZagtable[i]);
		//deletemod(&alphaTable[i]);
		//deletemod(&DCTTable[i]);
	}
	deletemod(&DCTMatrix);
	deletemod(&image_block);
	deletemod(&ZigZagtable);
	//deletemod(&alphaTable);
	//deletemod(&DCTTable);

	for (int i = 0; i < 3; i++)
	{
		deletemod(&image_converted[i]);
		deletemod(&result[i]);
	}

	deletemod(&image_converted);
	deletemod(&result);

	return listquad;
}