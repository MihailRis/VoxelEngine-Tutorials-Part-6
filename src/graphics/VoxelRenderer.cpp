#include "VoxelRenderer.h"
#include "Mesh.h"
#include "../voxels/Chunk.h"
#include "../voxels/voxel.h"

#define VERTEX_SIZE (3 + 2 + 1)

#define CDIV(X,A) (((X) < 0) ? ((X) / (A) - 1) : ((X) / (A)))
#define LOCAL_NEG(X, SIZE) (((X) < 0) ? ((SIZE)+(X)) : (X))
#define LOCAL(X, SIZE) ((X) >= (SIZE) ? ((X) - (SIZE)) : LOCAL_NEG(X, SIZE))
#define IS_CHUNK(X,Y,Z) (GET_CHUNK(X,Y,Z) != nullptr)
#define GET_CHUNK(X,Y,Z) (chunks[((CDIV(Y, CHUNK_H)+1) * 3 + CDIV(Z, CHUNK_D) + 1) * 3 + CDIV(X, CHUNK_W) + 1])

#define VOXEL(X,Y,Z) (GET_CHUNK(X,Y,Z)->voxels[(LOCAL(Y, CHUNK_H) * CHUNK_D + LOCAL(Z, CHUNK_D)) * CHUNK_W + LOCAL(X, CHUNK_W)])
#define IS_BLOCKED(X,Y,Z) ((!IS_CHUNK(X, Y, Z)) || VOXEL(X, Y, Z).id)

#define VERTEX(INDEX, X,Y,Z, U,V, L) buffer[INDEX+0] = (X);\
								  buffer[INDEX+1] = (Y);\
								  buffer[INDEX+2] = (Z);\
								  buffer[INDEX+3] = (U);\
								  buffer[INDEX+4] = (V);\
								  buffer[INDEX+5] = (L);\
								  INDEX += VERTEX_SIZE;

int chunk_attrs[] = {3,2,1, 0};

VoxelRenderer::VoxelRenderer(size_t capacity) : capacity(capacity) {
	buffer = new float[capacity * VERTEX_SIZE * 6];
}

VoxelRenderer::~VoxelRenderer(){
	delete[] buffer;
}

Mesh* VoxelRenderer::render(Chunk* chunk, const Chunk** chunks, bool ambientOcclusion){
	float aoFactor = 0.15f;
	size_t index = 0;
	for (int y = 0; y < CHUNK_H; y++){
		for (int z = 0; z < CHUNK_D; z++){
			for (int x = 0; x < CHUNK_W; x++){
				voxel vox = chunk->voxels[(y * CHUNK_D + z) * CHUNK_W + x];
				unsigned int id = vox.id;

				if (!id){
					continue;
				}

				float l;
				float uvsize = 1.0f/16.0f;
				float u1 = (id % 16) * uvsize;
				float v1 = 1-((1 + id / 16) * uvsize);
				float u2 = u1 + uvsize;
				float v2 = v1 + uvsize;

				// AO values
				float a,b,c,d,e,f,g,h;
				a = b = c = d = e = f = g = h = 0.0f;

				if (!IS_BLOCKED(x,y+1,z)){
					l = 1.0f;

					if (ambientOcclusion){
						a = IS_BLOCKED(x+1,y+1,z)*aoFactor;
						b = IS_BLOCKED(x,y+1,z+1)*aoFactor;
						c = IS_BLOCKED(x-1,y+1,z)*aoFactor;
						d = IS_BLOCKED(x,y+1,z-1)*aoFactor;

						e = IS_BLOCKED(x-1,y+1,z-1)*aoFactor;
						f = IS_BLOCKED(x-1,y+1,z+1)*aoFactor;
						g = IS_BLOCKED(x+1,y+1,z+1)*aoFactor;
						h = IS_BLOCKED(x+1,y+1,z-1)*aoFactor;
					}

					VERTEX(index, x - 0.5f, y + 0.5f, z - 0.5f, u2,v1, l*(1.0f-c-d-e));
					VERTEX(index, x - 0.5f, y + 0.5f, z + 0.5f, u2,v2, l*(1.0f-c-b-f));
					VERTEX(index, x + 0.5f, y + 0.5f, z + 0.5f, u1,v2, l*(1.0f-a-b-g));

					VERTEX(index, x - 0.5f, y + 0.5f, z - 0.5f, u2,v1, l*(1.0f-c-d-e));
					VERTEX(index, x + 0.5f, y + 0.5f, z + 0.5f, u1,v2, l*(1.0f-a-b-g));
					VERTEX(index, x + 0.5f, y + 0.5f, z - 0.5f, u1,v1, l*(1.0f-a-d-h));
				}
				if (!IS_BLOCKED(x,y-1,z)){
					l = 0.75f;

					if (ambientOcclusion){
						a = IS_BLOCKED(x+1,y-1,z)*aoFactor;
						b = IS_BLOCKED(x,y-1,z+1)*aoFactor;
						c = IS_BLOCKED(x-1,y-1,z)*aoFactor;
						d = IS_BLOCKED(x,y-1,z-1)*aoFactor;

						e = IS_BLOCKED(x-1,y-1,z-1)*aoFactor;
						f = IS_BLOCKED(x-1,y-1,z+1)*aoFactor;
						g = IS_BLOCKED(x+1,y-1,z+1)*aoFactor;
						h = IS_BLOCKED(x+1,y-1,z-1)*aoFactor;
					}

					VERTEX(index, x - 0.5f, y - 0.5f, z - 0.5f, u1,v1, l*(1.0f-c-d-e));
					VERTEX(index, x + 0.5f, y - 0.5f, z + 0.5f, u2,v2, l*(1.0f-a-b-g));
					VERTEX(index, x - 0.5f, y - 0.5f, z + 0.5f, u1,v2, l*(1.0f-c-b-f));

					VERTEX(index, x - 0.5f, y - 0.5f, z - 0.5f, u1,v1, l*(1.0f-c-d-e));
					VERTEX(index, x + 0.5f, y - 0.5f, z - 0.5f, u2,v1, l*(1.0f-a-d-h));
					VERTEX(index, x + 0.5f, y - 0.5f, z + 0.5f, u2,v2, l*(1.0f-a-b-g));
				}

				if (!IS_BLOCKED(x+1,y,z)){
					l = 0.95f;

					if (ambientOcclusion){
						a = IS_BLOCKED(x+1,y+1,z)*aoFactor;
						b = IS_BLOCKED(x+1,y,  z+1)*aoFactor;
						c = IS_BLOCKED(x+1,y-1,z)*aoFactor;
						d = IS_BLOCKED(x+1,y,  z-1)*aoFactor;

						e = IS_BLOCKED(x+1,y-1,z-1)*aoFactor;
						f = IS_BLOCKED(x+1,y-1,z+1)*aoFactor;
						g = IS_BLOCKED(x+1,y+1,z+1)*aoFactor;
						h = IS_BLOCKED(x+1,y+1,z-1)*aoFactor;
					}

					VERTEX(index, x + 0.5f, y - 0.5f, z - 0.5f, u2,v1, l*(1.0f-c-d-e));
					VERTEX(index, x + 0.5f, y + 0.5f, z - 0.5f, u2,v2, l*(1.0f-d-a-h));
					VERTEX(index, x + 0.5f, y + 0.5f, z + 0.5f, u1,v2, l*(1.0f-a-b-g));

					VERTEX(index, x + 0.5f, y - 0.5f, z - 0.5f, u2,v1, l*(1.0f-c-d-e));
					VERTEX(index, x + 0.5f, y + 0.5f, z + 0.5f, u1,v2, l*(1.0f-a-b-g));
					VERTEX(index, x + 0.5f, y - 0.5f, z + 0.5f, u1,v1, l*(1.0f-b-c-f));
				}
				if (!IS_BLOCKED(x-1,y,z)){
					l = 0.85f;

					if (ambientOcclusion){
						a = IS_BLOCKED(x-1,y+1,z)*aoFactor;
						b = IS_BLOCKED(x-1,y,  z+1)*aoFactor;
						c = IS_BLOCKED(x-1,y-1,z)*aoFactor;
						d = IS_BLOCKED(x-1,y,  z-1)*aoFactor;

						e = IS_BLOCKED(x-1,y-1,z-1)*aoFactor;
						f = IS_BLOCKED(x-1,y-1,z+1)*aoFactor;
						g = IS_BLOCKED(x-1,y+1,z+1)*aoFactor;
						h = IS_BLOCKED(x-1,y+1,z-1)*aoFactor;
					}

					VERTEX(index, x - 0.5f, y - 0.5f, z - 0.5f, u1,v1, l*(1.0f-c-d-e));
					VERTEX(index, x - 0.5f, y + 0.5f, z + 0.5f, u2,v2, l*(1.0f-a-b-g));
					VERTEX(index, x - 0.5f, y + 0.5f, z - 0.5f, u1,v2, l*(1.0f-d-a-h));

					VERTEX(index, x - 0.5f, y - 0.5f, z - 0.5f, u1,v1, l*(1.0f-c-d-e));
					VERTEX(index, x - 0.5f, y - 0.5f, z + 0.5f, u2,v1, l*(1.0f-b-c-f));
					VERTEX(index, x - 0.5f, y + 0.5f, z + 0.5f, u2,v2, l*(1.0f-a-b-g));
				}

				if (!IS_BLOCKED(x,y,z+1)){
					l = 0.9f;

					if (ambientOcclusion){
						a = IS_BLOCKED(x,  y+1,z+1)*aoFactor;
						b = IS_BLOCKED(x+1,y,  z+1)*aoFactor;
						c = IS_BLOCKED(x,  y-1,z+1)*aoFactor;
						d = IS_BLOCKED(x-1,y,  z+1)*aoFactor;

						e = IS_BLOCKED(x-1,y-1,z+1)*aoFactor;
						f = IS_BLOCKED(x+1,y-1,z+1)*aoFactor;
						g = IS_BLOCKED(x+1,y+1,z+1)*aoFactor;
						h = IS_BLOCKED(x-1,y+1,z+1)*aoFactor;
					}

					VERTEX(index, x - 0.5f, y - 0.5f, z + 0.5f, u1,v1, l*(1.0f-c-d-e));
					VERTEX(index, x + 0.5f, y + 0.5f, z + 0.5f, u2,v2, l*(1.0f-a-b-g));
					VERTEX(index, x - 0.5f, y + 0.5f, z + 0.5f, u1,v2, l*(1.0f-a-d-h));

					VERTEX(index, x - 0.5f, y - 0.5f, z + 0.5f, u1,v1, l*(1.0f-c-d-e));
					VERTEX(index, x + 0.5f, y - 0.5f, z + 0.5f, u2,v1, l*(1.0f-b-c-f));
					VERTEX(index, x + 0.5f, y + 0.5f, z + 0.5f, u2,v2, l*(1.0f-a-b-g));
				}
				if (!IS_BLOCKED(x,y,z-1)){
					l = 0.8f;

					if (ambientOcclusion){
						a = IS_BLOCKED(x,  y+1,z-1)*aoFactor;
						b = IS_BLOCKED(x+1,y,  z-1)*aoFactor;
						c = IS_BLOCKED(x,  y-1,z-1)*aoFactor;
						d = IS_BLOCKED(x-1,y,  z-1)*aoFactor;

						e = IS_BLOCKED(x-1,y-1,z-1)*aoFactor;
						f = IS_BLOCKED(x+1,y-1,z-1)*aoFactor;
						g = IS_BLOCKED(x+1,y+1,z-1)*aoFactor;
						h = IS_BLOCKED(x-1,y+1,z-1)*aoFactor;
					}

					VERTEX(index, x - 0.5f, y - 0.5f, z - 0.5f, u2,v1, l*(1.0f-c-d-e));
					VERTEX(index, x - 0.5f, y + 0.5f, z - 0.5f, u2,v2, l*(1.0f-a-d-h));
					VERTEX(index, x + 0.5f, y + 0.5f, z - 0.5f, u1,v2, l*(1.0f-a-b-g));

					VERTEX(index, x - 0.5f, y - 0.5f, z - 0.5f, u2,v1, l*(1.0f-c-d-e));
					VERTEX(index, x + 0.5f, y + 0.5f, z - 0.5f, u1,v2, l*(1.0f-a-b-g));
					VERTEX(index, x + 0.5f, y - 0.5f, z - 0.5f, u1,v1, l*(1.0f-b-c-f));
				}
			}
		}
	}
	return new Mesh(buffer, index / VERTEX_SIZE, chunk_attrs);
}
