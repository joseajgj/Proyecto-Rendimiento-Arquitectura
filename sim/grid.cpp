#include <iostream>
#include <algorithm>
#include <cmath>
#include "grid.hpp"
Grid::Grid(const Punto& bmin, const Punto& bmax) : bmin(bmin), bmax(bmax) {}

void Grid::dividirEnBloques(double smoothingLength) {
    numberblocksx = floor(((bmax.x - bmin.x)) / smoothingLength);
    numberblocksy = floor(((bmax.y - bmin.y)) / smoothingLength);
    numberblocksz = floor(((bmax.z - bmin.z)) / smoothingLength);
    numBlocks = numberblocksx * numberblocksy * numberblocksz;
    meshx = (bmax.x - bmin.x) / numberblocksx;
    meshy = (bmax.y - bmin.y) / numberblocksy;
    meshz = (bmax.z - bmin.z) / numberblocksz;

    // Generar los bloques y almacenarlos en el vector de blocks
    blocks.clear();
    for (int i = 0; i < numberblocksz; ++i) {
        for (int j = 0; j < numberblocksy; ++j) {
            for (int k = 0; k < numberblocksx; ++k) {
                Punto blockBmin = {bmin.x + i * smoothingLength, bmin.y + j * smoothingLength, bmin.z + k * smoothingLength};
                Punto blockBmax = {blockBmin.x + smoothingLength, blockBmin.y + smoothingLength, blockBmin.z + smoothingLength};
                int blockId = i + j * numberblocksx + k * numberblocksx * numberblocksy;
                blocks.emplace_back(blockId, blockBmin, blockBmax , k ,j, i);
                // std::cout << "El bloque " << blockId << " x: "<< i << " y: " << j << " z: "<< k << '\n';
            }
        }
    }
}


void Grid::reposicionarParticulas(Fluid &fluid, std::vector<Block> &bloques) {
    // Precalculamos valores para optimizar el rendimiento
    double inv_meshx = 1.0 / meshx;
    double inv_meshy = 1.0 / meshy;
    double inv_meshz = 1.0 / meshz;
    int num_blocksxy = numberblocksx * numberblocksy;

    for (int i = 0; i < fluid.numberparticles; ++i) {
        Particle &particula = fluid.particles[i];

        int indicex = std::max(0, std::min(static_cast<int>((particula.px - bmin.x) * inv_meshx), static_cast<int>(numberblocksx) - 1));
        int indicey = std::max(0, std::min(static_cast<int>((particula.py - bmin.y) * inv_meshy), static_cast<int>(numberblocksy) - 1));
        int indicez = std::max(0, std::min(static_cast<int>((particula.pz - bmin.z) * inv_meshz), static_cast<int>(numberblocksz) - 1));

        Block &block = bloques[indicex * num_blocksxy + indicey * numberblocksz + indicez];
        particula.idBloque = block.id;
        block.addParticle(particula);
        // std::cout << "La partícula " << i << " está en el bloque " << block.id << " x: "<< block.cx << " y: "<< block.cy << " z: "<< block.cz <<'\n';
    }
}

const std::vector<Block>& Grid::getBlocks() const {
    return blocks;
}
