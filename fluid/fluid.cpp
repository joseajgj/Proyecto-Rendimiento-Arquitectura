#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cmath>
#include <numbers>
#include <span>
#include <array>
#include <ctime>
#include "sim/grid.hpp"
#include "sim/constantes.hpp"
#include <iomanip>
#include <limits>


std::pair<double, double> mesh_simulation(const Fluid &fluid, Grid &malla);

void incrementDensities(Fluid &fluid, double smoothingLength);
void transformDensities(Fluid &fluid, double h, double particleMass);
void transferAcceleration(Fluid &fluid, double h, double particleMass);
void initAccelerations(Fluid &fluid);
void particleColissions(Fluid &fluid, std::vector<Block>& blocks, double numberblocksx, double numberblocksy, double numberblocksz);
void particlesMovement(Fluid &fluid);
void limitInteractions(Fluid &fluid, std::vector<Block>& blocks, double numberblocksx, double numberblocksy, double numberblocksz);

//NOLINTBEGIN(cppcoreguidelines-pro-type-reinterpret-cast)
void readFluid(std::ifstream& in, Fluid& fluid) {
    in.read(reinterpret_cast<char*>(&fluid.particlespermeter), sizeof(float));
    in.read(reinterpret_cast<char*>(&fluid.numberparticles), sizeof(int));
    fluid.particles.resize(fluid.numberparticles);
    for (int i = 0; i < fluid.numberparticles; ++i) {
        fluid.particles[i].id = i;
        for (double* attr : {&fluid.particles[i].px, &fluid.particles[i].py, &fluid.particles[i].pz,
                             &fluid.particles[i].hvx, &fluid.particles[i].hvy, &fluid.particles[i].hvz,
                             &fluid.particles[i].vx, &fluid.particles[i].vy, &fluid.particles[i].vz}) {
            float temp = 0;
            in.read(reinterpret_cast<char*>(&temp), sizeof(float));
            *attr = static_cast<double>(temp);
        }
    }
}
// NOLINTEND(cppcoreguidelines-pro-type-reinterpret-cast)

//NOLINTBEGIN(cppcoreguidelines-pro-type-reinterpret-cast)
void writeFluid(std::ofstream& out, const Fluid& fluid) {
    out.write(reinterpret_cast<const char*>(&fluid.particlespermeter), sizeof(float));
    out.write(reinterpret_cast<const char*>(&fluid.numberparticles), sizeof(int));
    for (const auto& particle : fluid.particles) {
        for (const double* attr : {&particle.px, &particle.py, &particle.pz,
                                   &particle.hvx, &particle.hvy, &particle.hvz,
                                   &particle.vx, &particle.vy, &particle.vz}) {
            auto temp = static_cast<float>(*attr);
            out.write(reinterpret_cast<const char*>(&temp), sizeof(float));
        }
    }
}
// NOLINTEND(cppcoreguidelines-pro-type-reinterpret-cast)

// Definir constantes para los códigos de error
const int ERROR_INVALID_PARTICLE_COUNT = -5;

const double factor05 = 0.5;

const double factor1e10 = 1e-10;

int main(int argc, char *argv[]) {
    unsigned const tiempo0=clock();
    std::span const args_view{argv, static_cast<std::size_t>(argc)};
    std::vector<std::string> const arguments{args_view.begin() + 1, args_view.end()};
    if (argc != 4) {
        std::cerr << "Error: Invalid number of arguments. Usage: " << arguments.size()
                  << " <nts> <inputfile> <outputfile>\n";
        return -1;
    }
    const int iteraciones = std::stoi(arguments[0]);
    try {
        if (iteraciones < 0) {
            std::cerr << "Error: Invalid number of time steps.\n";
            return -2;
        }
    } catch (const std::invalid_argument &e) {
        std::cerr << "Error: time steps must be numeric.\n";
        return -1;
    }

    const std::string &archivoEntrada = arguments[1];
    const std::string &archivoSalida = arguments[2];

    // Leer el archivo de entrada
    std::ifstream input(archivoEntrada, std::ios::binary);
    if (!input) {
        std::cerr << "Error: Cannot open " << archivoEntrada << " for reading\n";
        return -3;
    }

    Fluid fluid;
    readFluid(input, fluid);

    // Verificar si el número de partículas leídas coincide con numberparticles OPCIONAL
    if (fluid.particles.size() != static_cast<std::vector<Particle>::size_type>(fluid.numberparticles)) {
        std::cerr << "Error: El número de partículas leídas (" << fluid.particles.size()
                  << ") no coincide con numberparticles (" << fluid.numberparticles << ").\n";
        return ERROR_INVALID_PARTICLE_COUNT;
    }

    // Calcula los valores utilizando la función calculateValues
    Grid malla(Constantes::limInferior, Constantes::limSuperior);
    auto result = mesh_simulation(fluid, malla);
    double const smoothingLength = result.first;
    double const particleMass = result.second;

    std::vector<Block> blocks = malla.getBlocks();
    //const double factor1 = 15.0 / (M_PI * std::pow(smoothingLength, 6));
    //const double factor2 = 45.0 / (M_PI * std::pow(smoothingLength, 6) * Constantes::viscosidad * particleMass);
    for (int iter = 0; iter < iteraciones; ++iter) {
        std::cout << "Iteración " << iter + 1 << "\n";

        initAccelerations(fluid);
        malla.reposicionarParticulas(fluid, blocks);
        incrementDensities(fluid,  smoothingLength);
        transformDensities(fluid, smoothingLength, particleMass);
        transferAcceleration(fluid, smoothingLength, particleMass);
        particleColissions(fluid, blocks, malla.numberblocksx, malla.numberblocksy, malla.numberblocksz);
        particlesMovement(fluid);
        limitInteractions(fluid, blocks, malla.numberblocksx, malla.numberblocksy, malla.numberblocksz);

        std::ofstream const outFile("salida.txt");
        if (iter == iteraciones-1) {
            for (auto & particle : fluid.particles) {
                std::cout<<std::setprecision(15) << "La partícula " << particle.id << " " <<particle.density << " está en el bloque "
                         << particle.idBloque << " x: " << particle.px << " y: " << particle.py
                         << " z: " << particle.pz << " "
                         << "     Aceleración: (" << particle.ax << ", " << particle.ay << ", "
                         << particle.az << ")" << '\n';
            }
        }
    }

    // Escribir el estado final del fluido en el archivo de salida
    std::ofstream output(archivoSalida, std::ios::binary);
    if (!output) {
        std::cerr << "Error: Cannot open " << archivoSalida << " for writing\n";
        return -4;
    }
    writeFluid(output, fluid);

    output.close();
    std::cout << "Simulación completada. Estado final del fluido guardado en: " << archivoSalida << "\n";
    unsigned const tiempo1= clock();
    double const time = (double(tiempo1-tiempo0)/CLOCKS_PER_SEC);
    std::cout << "Execution Time: " << time;
    return 0;
}


// Funcion que simula la malla
std::pair<double, double> mesh_simulation(const Fluid &fluid, Grid &malla) {
    const double smoothingLength = Constantes::multRadio / fluid.particlespermeter;
    const double particleMass = std::pow(10.0, 3.0) / std::pow(fluid.particlespermeter, 3.0);


    malla.dividirEnBloques(smoothingLength);

    // Mostrar los valores por pantalla en el formato requerido
    std::cout << "Number of particles: " << fluid.numberparticles << "\n";
    std::cout << "Particles per meter: " << fluid.particlespermeter << "\n";
    std::cout << "Smoothing length: " << smoothingLength << "\n";
    std::cout << "Particle mass: " << particleMass << "\n";
    std::cout << "Grid size: " << malla.numberblocksx << " x " << malla.numberblocksy << " x " << malla.numberblocksz << "\n";
    std::cout << "Number of blocks: " << malla.numBlocks << "\n";
    std::cout << "Block size: " << malla.meshx << " x " << malla.meshy << " x " << malla.meshz << "\n";

    return std::make_pair(smoothingLength, particleMass);
}

// Función para inicializar las aceleraciones
void initAccelerations(Fluid &fluid) {
    for (int i = 0; i < fluid.numberparticles; ++i) {
        // Inicializa la densidad
        fluid.particles[i].density = 0.0;
        fluid.particles[i].ax = Constantes::gravedad.x;
        fluid.particles[i].ay = Constantes::gravedad.y; // Configura la aceleración de gravedad
        fluid.particles[i].az = Constantes::gravedad.z;
    }
}

double calculateDistanceSquared(const Particle &particle1, const Particle &particle2) {
    double const deltaX = particle1.px - particle2.px;
    double const deltaY = particle1.py - particle2.py;
    double const deltaZ = particle1.pz - particle2.pz;
    return deltaX * deltaX + deltaY * deltaY + deltaZ * deltaZ;
}


double calculateDeltaDensity(double h, double distSquared) {
    if (distSquared < h * h) {
        return std::pow(((h * h) - distSquared), 3);
    }
    return 0.0;
}

void incrementDensities(Fluid &fluid, double h) {
    for (int i = 0; i < fluid.numberparticles; ++i) {
        for (int j = i + 1; j < fluid.numberparticles; ++j) {
            double const distSquared = calculateDistanceSquared(fluid.particles[i], fluid.particles[j]);

            // Calcula el incremento de densidad ∆ρij
            double const deltaDensity = calculateDeltaDensity(h, distSquared);

            // Incrementa la densidad de ambas partículas
            fluid.particles[i].density += deltaDensity;
            fluid.particles[j].density += deltaDensity;
        }
    }
}


void transformDensities(Fluid &fluid, double h, double particleMass) {
    const double factor = (315.0 / (64.0 * std::numbers::pi * std::pow(h, 9))) * particleMass;

    for (int i = 0; i < fluid.numberparticles; ++i) {
        fluid.particles[i].density = (fluid.particles[i].density + std::pow(h, 6)) * factor;
    }
}

void transferAcceleration(Fluid &fluid, double h, double particleMass) {
    const double smoothingLengthSquared = h * h;
    const double smallQ = 10e-12;
    const double factor1 = 15 / (std::numbers::pi * std::pow(h, 6));
    const double factor2 = (45 / (std::numbers::pi * std::pow(h, 6)) * Constantes::viscosidad * particleMass);
    const double commonFactor = factor1 * ((3 * particleMass * Constantes::presRigidez) * factor05);
    for (int i = 0; i < fluid.numberparticles; ++i) {
        for (int j = i + 1; j < fluid.numberparticles; ++j) {
            const double distSquared = calculateDistanceSquared(fluid.particles[i], fluid.particles[j]);
            if (distSquared >= smoothingLengthSquared) {
                continue;
            }
            const double maxDistanceSquared = std::max(distSquared, smallQ);
            const double dist = std::sqrt(maxDistanceSquared);
            const double distX = fluid.particles[i].px - fluid.particles[j].px;
            const double distY = fluid.particles[i].py - fluid.particles[j].py;
            const double distZ = fluid.particles[i].pz - fluid.particles[j].pz;
            const double distdiv = 1 / dist;
            const double hMinusDistSquared = std::pow(h - dist, 2);
            const double deltaDensity = (fluid.particles[i].density + fluid.particles[j].density - 2 * Constantes::densFluido);
            const double deltaAijX = std::fma(distX * commonFactor * hMinusDistSquared * distdiv, deltaDensity,
                                              (fluid.particles[j].vx - fluid.particles[i].vx) * factor2) / (fluid.particles[i].density * fluid.particles[j].density);
            const double deltaAijY = std::fma(distY * commonFactor * hMinusDistSquared * distdiv, deltaDensity,
                                              (fluid.particles[j].vy - fluid.particles[i].vy) * factor2) / (fluid.particles[i].density * fluid.particles[j].density);
            const double deltaAijZ = std::fma(distZ * commonFactor * hMinusDistSquared * distdiv, deltaDensity,
                                              (fluid.particles[j].vz - fluid.particles[i].vz) * factor2) / (fluid.particles[i].density * fluid.particles[j].density);
            fluid.particles[i].ax += deltaAijX;
            fluid.particles[i].ay += deltaAijY;
            fluid.particles[i].az += deltaAijZ;
            fluid.particles[j].ax -= deltaAijX;
            fluid.particles[j].ay -= deltaAijY;
            fluid.particles[j].az -= deltaAijZ;
        }
    }
}





void handleXCollisions(Particle& particle, int cx, double numberblocksx) {
    double const newPositionX = particle.px + particle.hvx * Constantes::pasoTiempo;
    double deltaX = NAN;

    if (cx == 0) {
        deltaX = Constantes::tamParticula - (newPositionX - Constantes::limInferior.x);
        if (deltaX > factor1e10) {
            particle.ax += Constantes::colisRigidez * deltaX - Constantes::amortiguamiento * particle.vx;
        }
    } else if (cx == static_cast<int>(numberblocksx - 1) ){
        deltaX = Constantes::tamParticula - (Constantes::limSuperior.x - newPositionX);
        if (deltaX > factor1e10) {
            particle.ax -= Constantes::colisRigidez * deltaX + Constantes::amortiguamiento * particle.vx;
        }
    }
}


void handleYCollisions(Particle& particle, int cy, double numberblocksy) {
    double const newPositionY = particle.py + particle.hvy * Constantes::pasoTiempo;
    double deltaY = NAN;

    if (cy == 0) {
        deltaY = Constantes::tamParticula - (newPositionY - Constantes::limInferior.y);
        if (deltaY > factor1e10) {
            particle.ay += std::fma(Constantes::colisRigidez, deltaY, -Constantes::amortiguamiento * particle.vy);
        }
    } else if (cy == static_cast<int>(numberblocksy - 1)) {
        deltaY = Constantes::tamParticula - (Constantes::limSuperior.y - newPositionY);
        if (deltaY > factor1e10) {
            particle.ay -= std::fma(Constantes::colisRigidez, deltaY, Constantes::amortiguamiento * particle.vy);
        }
    }
}

void handleZCollisions(Particle& particle, int cz, double numberblocksz) {
    double const newPositionZ = particle.pz + particle.hvz * Constantes::pasoTiempo;
    double deltaZ = NAN;

    if (cz == 0) {
        deltaZ = Constantes::tamParticula - (newPositionZ - Constantes::limInferior.z);
        if (deltaZ > factor1e10) {
            particle.az += std::fma(Constantes::colisRigidez, deltaZ, -Constantes::amortiguamiento * particle.vz);
        }
    } else if (cz == static_cast<int>(numberblocksz - 1)) {
        deltaZ = Constantes::tamParticula - (Constantes::limSuperior.z - newPositionZ);
        if (deltaZ > factor1e10) {
            particle.az -= std::fma(Constantes::colisRigidez, deltaZ, Constantes::amortiguamiento * particle.vz);
        }
    }
}

// Funcion para calcular las colisiones de particulas
void particleColissions(Fluid &fluid, std::vector<Block>& blocks, double numberblocksx, double numberblocksy, double numberblocksz) {
    for (int blockIndex = 0; blockIndex < numberblocksx * numberblocksy * numberblocksz; ++blockIndex) {
        Block  const&block = blocks[blockIndex];
        for (auto& particula : fluid.particles) {
            if (particula.idBloque == blockIndex) {
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
}


// Funcion para realizar el movimiento de particulas
void particlesMovement(Fluid &fluid){
    for (Particle& particle : fluid.particles) {
        particle.px = particle.px + particle.hvx * Constantes::pasoTiempo + particle.ax * std::pow(Constantes::pasoTiempo,2);
        particle.py = particle.py + particle.hvy * Constantes::pasoTiempo + particle.ay * std::pow(Constantes::pasoTiempo,2);
        particle.pz = particle.pz + particle.hvz * Constantes::pasoTiempo + particle.az * std::pow(Constantes::pasoTiempo,2);

        particle.vx = particle.hvx + (particle.ax * Constantes::pasoTiempo) * factor05;
        particle.vy = particle.hvy + (particle.ay * Constantes::pasoTiempo) * factor05;
        particle.vz = particle.hvz + (particle.az * Constantes::pasoTiempo) * factor05;

        particle.hvx = particle.hvx + particle.ax * Constantes::pasoTiempo;
        particle.hvy = particle.hvy + particle.ay * Constantes::pasoTiempo;
        particle.hvz = particle.hvz + particle.az * Constantes::pasoTiempo;
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
void limitInteractions(Fluid& fluid,std::vector<Block>& blocks, double numberblocksx, double numberblocksy, double numberblocksz) {
   for (int blockIndex = 0; blockIndex < numberblocksx * numberblocksy * numberblocksz; ++blockIndex) {
          Block  const& block = blocks[blockIndex];
          for (auto & particula : fluid.particles) {
            if (particula.idBloque == blockIndex) {
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
}