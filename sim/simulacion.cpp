#include <vector>
#include <cmath>
#include <numbers>
#include <array>
#include <limits>
#include <tuple>
#include "sim/grid.hpp"
#include "sim/constantes.hpp"
#include "sim/progargs.hpp"
#include "simulacion.hpp"

std::vector<Block> ejecutarIteraciones(Grid& malla, Argumentos& argumentos, double smoothingLength, double particleMass) {
    // Para la transferencia de densidades
    const double factorDensTransf = (315.0 / (64.0 * std::numbers::pi * std::pow(smoothingLength, 9))) * particleMass;

    // Para la transferencia de aceleraciones
    Constantes::ConstAccTransf constAccTransf{};
    constAccTransf.h = smoothingLength;
    constAccTransf.hSquared = smoothingLength * smoothingLength;
    constAccTransf.factor2 = (45 / (std::numbers::pi * std::pow(smoothingLength, 6)) * Constantes::viscosidad *
                              particleMass);
    constAccTransf.commonFactor = (15 / (std::numbers::pi * std::pow(smoothingLength, 6))) *
                                  ((3 * particleMass * Constantes::presRigidez) * Constantes::factor05);

    std::vector<Block> blocks = malla.getBlocks();
    malla.reposicionarParticulasFluid(argumentos.fluid, blocks);
    for (int iter = 0; iter < argumentos.iteraciones; ++iter) {
        initAccelerations(blocks);
        malla.reposicionarParticulasBloque(blocks);
        incrementDensities(blocks, smoothingLength, malla);
        transformDensities(blocks, smoothingLength, factorDensTransf);
        transferAcceleration(blocks, constAccTransf, malla);
        particleColissions(blocks, malla.getNumberblocksx(), malla.getNumberblocksy(), malla.getNumberblocksz());
        particlesMovement(blocks);
        limitInteractions(blocks, malla.getNumberblocksx(), malla.getNumberblocksy(), malla.getNumberblocksz());
    }

    return blocks;
}


// Función para inicializar las aceleraciones
void initAccelerations(std::vector<Block>& blocks) {
    for (auto& block : blocks) {
        for (auto& particle : block.particles) {
            // Inicializa la densidad
            particle.density = 0.0;
            particle.ax = Constantes::gravedad.x;
            particle.ay = Constantes::gravedad.y; // Configura la aceleración de gravedad
            particle.az = Constantes::gravedad.z;
        }
    }
}


void incrementDensities(std::vector<Block>& blocks, double h, Grid& malla) {
    double const hsq = h*h;
    for (auto& block1 : blocks) {
        for (auto& particle1 : block1.particles) {
            // Considera solo los bloques que son vecinos inmediatos de block1
            for (int dz = -1; dz <= 1; ++dz) {
                for (int dy = -1; dy <= 1; ++dy) {
                    for (int dx = -1; dx <= 1; ++dx) {
                        const int neighbor_cx = block1.cx + dx;
                        const int neighbor_cy = block1.cy + dy;
                        const int neighbor_cz = block1.cz + dz;
                        // Comprueba si las coordenadas del vecino estan dentro de los limites del Grid
                        if (neighbor_cx >= 0 && neighbor_cx < malla.getNumberblocksx() &&
                            neighbor_cy >= 0 && neighbor_cy < malla.getNumberblocksy() &&
                            neighbor_cz >= 0 && neighbor_cz < malla.getNumberblocksz()) {
                            // Calcula el indice del bloque vecino
                            const int neighborIndex = static_cast<int>(neighbor_cz + neighbor_cy * malla.getNumberblocksz()
                                    + neighbor_cx * malla.getNumberblocksz() * malla.getNumberblocksy());
                            comprobarParticula2Dens(blocks, particle1, hsq, neighborIndex);
                        }
                    }
                }
            }
        }
    }
}


void comprobarParticula2Dens(std::vector<Block>& blocks, Particle& particle1, double hsq, int neighborIndex) {
    Block& block2 = blocks[neighborIndex];
    for (auto& particle2 : block2.particles) {

        // Calculamos valores para todas las particle2 con un indice mayor, asi nos aseguramos de no repetir
        if (particle1.id < particle2.id) {
            double const deltaX = particle1.px - particle2.px;
            double const deltaY = particle1.py - particle2.py;
            double const deltaZ = particle1.pz - particle2.pz;
            double const distSquared = deltaX * deltaX + deltaY * deltaY + deltaZ * deltaZ;

            if (distSquared < hsq) {
                // Calcula el incremento de densidad
                double const deltaDensity= std::pow(((hsq) - distSquared), 3);

                // Incrementa la densidad de ambas particulas
                particle1.density += deltaDensity;
                particle2.density += deltaDensity;
            }
        }
    }
}


double calculateDistanceSquared(const Particle &particle1, const Particle &particle2) {
    double const deltaX = particle1.px - particle2.px;
    double const deltaY = particle1.py - particle2.py;
    double const deltaZ = particle1.pz - particle2.pz;
    return deltaX * deltaX + deltaY * deltaY + deltaZ * deltaZ;
}




void transformDensities(std::vector<Block>& blocks, double h, double factorDensTransf) {
    for (auto& block : blocks) {
        for (auto& particle : block.particles) {
            particle.density = (particle.density + std::pow(h, 6)) * factorDensTransf;
        }
    }
}

void transferAcceleration(std::vector<Block>& blocks, Constantes::ConstAccTransf& constAccTransf, Grid& malla) {
    for (auto& block1 : blocks) {
        for (auto& particle1 : block1.particles) {
            // Considera solo los bloques que son vecinos inmediatos de block1
            for (int dx = -1; dx <= 1; ++dx) {
                for (int dy = -1; dy <= 1; ++dy) {
                    for (int dz = -1; dz <= 1; ++dz) {
                        const int neighbor_cx = block1.cx + dx;
                        const int neighbor_cy = block1.cy + dy;
                        const int neighbor_cz = block1.cz + dz;

                        // Comprueba si las coordenadas del vecino estan dentro de los limites del Grid
                        if (neighbor_cx >= 0 && neighbor_cx < malla.getNumberblocksx() &&
                            neighbor_cy >= 0 && neighbor_cy < malla.getNumberblocksy() &&
                            neighbor_cz >= 0 && neighbor_cz < malla.getNumberblocksz()) {
                            // Calcula el indice del bloque vecino
                            const int neighborIndex = static_cast<int>(neighbor_cz + neighbor_cy *malla.getNumberblocksz()
                                    + neighbor_cx * malla.getNumberblocksz() * malla.getNumberblocksy());
                            comprobarParticula2Acc(blocks, particle1, constAccTransf, neighborIndex);
                        }
                    }
                }
            }
        }
    }
}


void comprobarParticula2Acc(std::vector<Block>& blocks, Particle& particle1, Constantes::ConstAccTransf& constAccTransf, int neighborIndex) {
    Block &block2 = blocks[neighborIndex];
    for (auto &particle2: block2.particles) {

        // Calculamos valores para todas las particle2 con un indice mayor, asi nos aseguramos de no repetir
        if (particle1.id < particle2.id) {
            const double distSquared = calculateDistanceSquared(particle1, particle2);

            // Solo continuamos si la distancia al cuadrado es mayor que "hSquared"
            if (distSquared >= constAccTransf.hSquared) {
                continue;
            }

            // Calculamos la diferencia para cada coordenada y se lo aplicamos a ambas particulas
            auto [deltaAijX, deltaAijY, deltaAijZ] = calcularDeltas(particle1, particle2, constAccTransf, distSquared);
            particle1.ax += deltaAijX;
            particle1.ay += deltaAijY;
            particle1.az += deltaAijZ;
            particle2.ax -= deltaAijX;
            particle2.ay -= deltaAijY;
            particle2.az -= deltaAijZ;
        }
    }
}


std::tuple<double, double, double> calcularDeltas(Particle& particle1, Particle& particle2, Constantes::ConstAccTransf& constAccTransf, double distSquared) {
    const double maxDistanceSquared = std::max(distSquared, Constantes::smallQ);
    const double dist = std::sqrt(maxDistanceSquared);
    const double distX = particle1.px - particle2.px;
    const double distY = particle1.py - particle2.py;
    const double distZ = particle1.pz - particle2.pz;

    const double distdiv = 1 / dist;
    const double hMinusDistSquared = std::pow(constAccTransf.h - dist, 2);
    const double deltaDensity = (particle1.density + particle2.density -
                                 2 * Constantes::densFluido);

    const double densitydivmul = 1 / (particle1.density * particle2.density);
    const double factorcomun = constAccTransf.commonFactor * hMinusDistSquared * distdiv * deltaDensity;

    const double deltaAijX = ((distX * (factorcomun) +
              (particle2.vx - particle1.vx) * constAccTransf.factor2)) * densitydivmul;
    const double deltaAijY = ((distY * (factorcomun) +
              (particle2.vy - particle1.vy) * constAccTransf.factor2)) * densitydivmul;
    const double deltaAijZ = ((distZ * (factorcomun) +
              (particle2.vz - particle1.vz) * constAccTransf.factor2)) * densitydivmul;

    return std::make_tuple(deltaAijX, deltaAijY, deltaAijZ);
}





void handleXCollisions(Particle& particle, int cx, double numberblocksx) {
    double const newPositionX = particle.px + particle.hvx * Constantes::pasoTiempo;
    double deltaX = NAN;

    if (cx == 0) {
        deltaX = Constantes::tamParticula - (newPositionX - Constantes::limInferior.x);
        if (deltaX > Constantes::factor1e10) {
            particle.ax += Constantes::colisRigidez * deltaX - Constantes::amortiguamiento * particle.vx;
        }
    } else if (cx == static_cast<int>(numberblocksx - 1) ){
        deltaX = Constantes::tamParticula - (Constantes::limSuperior.x - newPositionX);
        if (deltaX > Constantes::factor1e10) {
            particle.ax -= Constantes::colisRigidez * deltaX + Constantes::amortiguamiento * particle.vx;
        }
    }
}


void handleYCollisions(Particle& particle, int cy, double numberblocksy) {
    double const newPositionY = particle.py + particle.hvy * Constantes::pasoTiempo;
    double deltaY = NAN;

    if (cy == 0) {
        deltaY = Constantes::tamParticula - (newPositionY - Constantes::limInferior.y);
        if (deltaY > Constantes::factor1e10) {
            particle.ay += Constantes::colisRigidez * deltaY - Constantes::amortiguamiento * particle.vy;
        }
    } else if (cy == static_cast<int>(numberblocksy - 1)) {
        deltaY = Constantes::tamParticula - (Constantes::limSuperior.y - newPositionY);
        if (deltaY > Constantes::factor1e10) {
            particle.ay -= Constantes::colisRigidez * deltaY + Constantes::amortiguamiento * particle.vy;
        }
    }
}

void handleZCollisions(Particle& particle, int cz, double numberblocksz) {
    double const newPositionZ = particle.pz + particle.hvz * Constantes::pasoTiempo;
    double deltaZ = NAN;

    if (cz == 0) {
        deltaZ = Constantes::tamParticula - (newPositionZ - Constantes::limInferior.z);
        if (deltaZ > Constantes::factor1e10) {
            particle.az += Constantes::colisRigidez * deltaZ - Constantes::amortiguamiento * particle.vz;
        }
    } else if (cz == static_cast<int>(numberblocksz - 1)) {
        deltaZ = Constantes::tamParticula - (Constantes::limSuperior.z - newPositionZ);
        if (deltaZ > Constantes::factor1e10) {
            particle.az -= Constantes::colisRigidez * deltaZ + Constantes::amortiguamiento * particle.vz;
        }
    }
}

// Funcion para calcular las colisiones de particulas
void particleColissions(std::vector<Block>& blocks, double numberblocksx, double numberblocksy, double numberblocksz) {
    for (auto& block : blocks) {
        for (auto& particula : block.particles) {
            /* si un bloque tiene cx==0 o cx== numbrblocks-1 se actualiza el ax de todas las particulas de ese bloque, llamando
            a handleXCollisions*/
            if (block.cx == 0 || block.cx == static_cast<int>(numberblocksx) - 1) {
                handleXCollisions(particula, block.cx, numberblocksx);
            }
            /* si un bloque tiene cy==0 o cy== numbrblocks-1 se actualiza el ay de todas las particulas de ese bloque, llamando
            a handleYCollisions*/
            if (block.cy == 0 || block.cy == static_cast<int>(numberblocksy) - 1) {
                handleYCollisions(particula, block.cy, numberblocksy);
            }
            /* si un bloque tiene cz==0 o cz== numbrblocks-1 se actualiza el az de todas las particulas de ese bloque, llamando
            a handleZCollisions*/
            if (block.cz == 0 || block.cz == static_cast<int>(numberblocksz) - 1) {
                handleZCollisions(particula, block.cz, numberblocksz);
            }
        }
    }
}


// Funcion para realizar el movimiento de particulas
void particlesMovement(std::vector<Block>& blocks){
    for (auto& block : blocks) {
        for (auto &particle: block.particles) {
            particle.px = particle.px + particle.hvx * Constantes::pasoTiempo +
                          particle.ax * std::pow(Constantes::pasoTiempo, 2);
            particle.py = particle.py + particle.hvy * Constantes::pasoTiempo +
                          particle.ay * std::pow(Constantes::pasoTiempo, 2);
            particle.pz = particle.pz + particle.hvz * Constantes::pasoTiempo +
                          particle.az * std::pow(Constantes::pasoTiempo, 2);

            particle.vx = particle.hvx + (particle.ax * Constantes::pasoTiempo) * Constantes::factor05;
            particle.vy = particle.hvy + (particle.ay * Constantes::pasoTiempo) * Constantes::factor05;
            particle.vz = particle.hvz + (particle.az * Constantes::pasoTiempo) * Constantes::factor05;

            particle.hvx = particle.hvx + particle.ax * Constantes::pasoTiempo;
            particle.hvy = particle.hvy + particle.ay * Constantes::pasoTiempo;
            particle.hvz = particle.hvz + particle.az * Constantes::pasoTiempo;
        }
    }
}

// Funcion que hace la interaccion con el borde del recinto respecto a la X (si hay interaccion)
void InteractionLimitX(Particle& particle, int cx, double numberblocksx){
    if (cx == 0){
        double const deltax = particle.px - Constantes::limInferior.x;
        if (deltax < 0){
            particle.px = Constantes::limInferior.x - deltax;
            particle.vx = -particle.vx;
            particle.hvx = -particle.hvx;
        }
    }
    else if(cx == static_cast<int>(numberblocksx - 1)){
        double const deltax = Constantes::limSuperior.x - particle.px;
        if (deltax < 0){
            particle.px = Constantes::limSuperior.x + deltax;
            particle.vx = -particle.vx;
            particle.hvx = -particle.hvx;
        }
    }
}

// Funcion que hace la interaccion con el borde del recinto respecto a la Y (si hay interaccion)
void InteractionLimitY(Particle& particle, int cy, double numberblocksy){
    if (cy == 0){
        double const deltay = particle.py - Constantes::limInferior.y;
        if (deltay < 0){
            particle.py = Constantes::limInferior.y - deltay;
            particle.vy = -particle.vy;
            particle.hvy = -particle.hvy;
        }
    }
    else if(cy == static_cast<int>(numberblocksy - 1)){
        double const deltay = Constantes::limSuperior.y - particle.py;
        if (deltay < 0){
            particle.py = Constantes::limSuperior.y + deltay;
            particle.vy = -particle.vy;
            particle.hvy = -particle.hvy;
        }
    }
}

// Funcion que hace la interaccion con el borde del recinto respecto a la Z (si hay interaccion)
void InteractionLimitZ(Particle& particle, int cz, double numberblocksz){
    if (cz == 0){
        double const deltaz = particle.pz - Constantes::limInferior.z;
        if (deltaz < 0){
            particle.pz = Constantes::limInferior.z - deltaz;
            particle.vz = -particle.vz;
            particle.hvz = -particle.hvz;
        }
    }
    else if(cz == static_cast<int>(numberblocksz - 1)){
        double const deltaz = Constantes::limSuperior.z - particle.pz;
        if (deltaz < 0){
            particle.pz = Constantes::limSuperior.z + deltaz;
            particle.vz = -particle.vz;
            particle.hvz = -particle.hvz;
        }
    }
}

// Funcion para las interacciones con los límites del recinto de una particula
void limitInteractions(std::vector<Block>& blocks, double numberblocksx, double numberblocksy, double numberblocksz) {
    for (auto& block : blocks) {
        for (auto& particula : block.particles) {
            /* si un bloque tiene cx==0 o cx== numbrblocks-1 se actualiza el ax de todas las particulas de ese bloque, llamando a handleXCollisions*/
            if (block.cx == 0 || block.cx == static_cast<int>(numberblocksx) - 1) {
                InteractionLimitX(particula, block.cx, numberblocksx);
            }
            /* si un bloque tiene cy==0 o cy== numbrblocks-1 se actualiza el ay de todas las particulas de ese bloque, llamando a handleYCollisions*/
            if (block.cy == 0 || block.cy == static_cast<int>(numberblocksy) - 1) {
                InteractionLimitY(particula, block.cy, numberblocksy);
            }
            /* si un bloque tiene cz==0 o cz== numbrblocks-1 se actualiza el az de todas las particulas de ese bloque, llamando a handleZCollisions*/
            if (block.cz == 0 || block.cz == static_cast<int>(numberblocksz) - 1) {
                InteractionLimitZ(particula, block.cz, numberblocksz);
            }
        }
    }
}
