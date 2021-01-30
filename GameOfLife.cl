struct Pixel
{
	unsigned char r, g, b;
};

bool isOscilator(__global struct Pixel* a, int row, int col, const int n)
{
	return a[(row)*n + (col - 1)].r == 0 &&
		a[(row)*n + (col + 1)].r == 0 &&
		a[(row + 1) * n + (col)].r == 255 &&
		a[(row - 1) * n + (col)].r == 255 &&
		a[(row - 1) * n + (col - 1)].r == 255 &&
		a[(row + 1) * n + (col + 1)].r == 255;
}

__kernel void gameOfLife(__global struct Pixel* mat,
						 const int n)
{
	int row = get_global_id(0);
	int col = get_global_id(1);
	if (row == 0 || row == n - 1 || col == 0 || col == n - 1)
	{
		return;//
	}
	int liveNeighbours=0;
	int isAlive=0;
	if (row < n && col < n)
	{
		isAlive = !(mat[(row)*n + (col)].g);
		liveNeighbours = 0;
		liveNeighbours += !(mat[(row - 1) * n + (col)].g);
		liveNeighbours += !(mat[(row)*n + (col - 1)].g);
		liveNeighbours += !(mat[(row)*n + (col + 1)].g);
		liveNeighbours += !(mat[(row + 1) * n + (col)].g);
		liveNeighbours += !(mat[(row - 1) * n + (col - 1)].g);
		liveNeighbours += !(mat[(row - 1) * n + (col + 1)].g);
		liveNeighbours += !(mat[(row + 1) * n + (col - 1)].g);
		liveNeighbours += !(mat[(row + 1) * n + (col + 1)].g);
		
		if ((isAlive && liveNeighbours == 2) || (liveNeighbours == 3))
		{
			mat[(row)*n + (col)].r = 0;
			mat[(row)*n + (col)].g = 0;
			mat[(row)*n + (col)].b = 0;
		}
		else
		{
			mat[(row)*n + (col)].r = 255;
			mat[(row)*n + (col)].g = 255;
			mat[(row)*n + (col)].b = 255;
		}
	}
	barrier(CLK_LOCAL_MEM_FENCE);
	if(isAlive && (liveNeighbours==2) && isOscilator(mat,row,col,n))//(mat[(row)*n + (col-1)].r==0) && (mat[(row)*n + (col+1)].r==0) &&(mat[(row+1)*n + (col)].r==255)&& (mat[(row - 1) * n + (col)].r == 255)
	{
		mat[(row)*n + (col)].r = 240;
		mat[(row)*n + (col)].g = 0;
		mat[(row)*n + (col)].b = 0;
		
		mat[(row)*n + (col-1)].r = 240;
		mat[(row)*n + (col-1)].g = 0;
		mat[(row)*n + (col-1)].b = 0;
		
		mat[(row)*n + (col+1)].r = 240;
		mat[(row)*n + (col+1)].g = 0;
		mat[(row)*n + (col+1)].b = 0;
	}
}