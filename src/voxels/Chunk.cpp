#include "Chunk.h"
#include "voxel.h"
#include <math.h>
#include <glm/glm.hpp>
#include <glm/gtc/noise.hpp>

Chunk::Chunk(int xpos, int ypos, int zpos) : x(xpos), y(ypos), z(zpos){
	voxels = new voxel[CHUNK_VOL];

	for (int z = 0; z < CHUNK_D; z++){
		for (int x = 0; x < CHUNK_W; x++){
			int real_x = x + this->x * CHUNK_W;
			int real_z = z + this->z * CHUNK_D;
			//float height = glm::perlin(glm::vec3(real_x*0.0026125f,real_z*0.0026125f, 0.0f));
		 	//height += glm::perlin(glm::vec3(real_x*0.006125f,real_z*0.006125f, 0.0f))*0.5f;
			for (int y = 0; y < CHUNK_H; y++){
				int real_y = y + this->y * CHUNK_H;
				int id = glm::perlin(glm::vec3(real_x*0.0125f,real_y*0.0125f, real_z*0.0125f)) > 0.1f;//real_y <= (height) * 60 + 30;
				if (real_y <= 2)
					id = 2;
				voxels[(y * CHUNK_D + z) * CHUNK_W + x].id = id;
			}
		}
	}
}

Chunk::~Chunk(){
	delete[] voxels;
}
