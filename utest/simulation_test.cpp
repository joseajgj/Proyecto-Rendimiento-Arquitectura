#include <gtest/gtest.h>
#include "sim/simulacion.hpp"

//test para comprobar que se inicializan las aceleraciones de forma correcta
TEST(SimulationTests, InitAccelerations) {
    Block block(0,0,0,0);
    const Particle particle{1, 1, 0.0, 0.0, 0.0, 1.0, 1.0, 1.0, 0.1, 0.3, 2.0, 3.0, 3.0, 3.0, 4.0};
    std::vector<Block> blocks_vector;
    block.addParticle(particle);
    blocks_vector.push_back(block);
    initAccelerations(blocks_vector);
    ASSERT_EQ(blocks_vector[0].particles[0].ax,0.0);
    ASSERT_EQ(blocks_vector[0].particles[0].ay,-9.8);
    ASSERT_EQ(blocks_vector[0].particles[0].az,0.0);
    ASSERT_EQ(blocks_vector[0].particles[0].density,0.0);
}

/*Test para comprobar el funcionamiento de particles movement en un bloque con una partícula
  Para comprobar los resultados comprobamos con una tolerancia no muy restrictiva, de 1e-5
 */
TEST(SimulationTests, ParticlesMovement) {
    Block block(0,0,0,0);
    const Particle particle{1, 1, 0.0, 1.0, 2.0, 3.0, 3.0, 2.0, 1.0, 2.0, 2.0, 3.0, 3.0, 3.0, 4.0};
    std::vector<Block> blocks_vector;
    block.addParticle(particle);
    blocks_vector.push_back(block);
    particlesMovement(blocks_vector);
    double tolerance = 1e-5;
    ASSERT_NEAR(0.003003,blocks_vector[0].particles[0].px,tolerance);
    ASSERT_NEAR(1.003,blocks_vector[0].particles[0].py,tolerance);
    ASSERT_NEAR(2.002,blocks_vector[0].particles[0].pz,tolerance);
    ASSERT_NEAR(3.0015,blocks_vector[0].particles[0].vx,tolerance);
    ASSERT_NEAR(3.0015,blocks_vector[0].particles[0].vy,tolerance);
    ASSERT_NEAR(2.0015,blocks_vector[0].particles[0].vz,tolerance);
    ASSERT_NEAR(3.003,blocks_vector[0].particles[0].hvx,tolerance);
    ASSERT_NEAR(3.003,blocks_vector[0].particles[0].hvy,tolerance);
    ASSERT_NEAR(2.003,blocks_vector[0].particles[0].hvz,tolerance);
}
TEST(SimulationTests, ParticlesMovement2) {
    Block block(0,0,0,0);
    Block block2(1,0,0,0);
    const Particle particle{0, 1, 0.0, 1.0, 2.0, 3.0, 3.0, 2.0, 1.0, 2.0, 2.0, 3.0, 3.0, 3.0, 4.0};
    const Particle particle2{1, 1, 0.0, 1.0, 2.0, 3.0, 3.0, 2.0, 1.0, 2.0, 2.0, 3.0, 3.0, 3.0, 4.0};
    std::vector<Block> blocks_vector;
    block.addParticle(particle);
    block2.addParticle(particle2);
    blocks_vector.push_back(block);
    blocks_vector.push_back(block2);
    particlesMovement(blocks_vector);
    double tolerance = 1e-5;
    //comprobación para la partícula del primer bloque
    ASSERT_EQ(0,blocks_vector[0].particles[0].id);
    ASSERT_NEAR(0.003003,blocks_vector[0].particles[0].px,tolerance);
    ASSERT_NEAR(1.003,blocks_vector[0].particles[0].py,tolerance);
    ASSERT_NEAR(2.002,blocks_vector[0].particles[0].pz,tolerance);
    ASSERT_NEAR(3.0015,blocks_vector[0].particles[0].vx,tolerance);
    ASSERT_NEAR(3.0015,blocks_vector[0].particles[0].vy,tolerance);
    ASSERT_NEAR(2.0015,blocks_vector[0].particles[0].vz,tolerance);
    ASSERT_NEAR(3.003,blocks_vector[0].particles[0].hvx,tolerance);
    ASSERT_NEAR(3.003,blocks_vector[0].particles[0].hvy,tolerance);
    ASSERT_NEAR(2.003,blocks_vector[0].particles[0].hvz,tolerance);
    //comprobación para la partícula del segundo bloque
    ASSERT_EQ(1,blocks_vector[1].particles[0].id);
    ASSERT_NEAR(0.003003,blocks_vector[1].particles[0].px,tolerance);
    ASSERT_NEAR(1.003,blocks_vector[1].particles[0].py,tolerance);
    ASSERT_NEAR(2.002,blocks_vector[1].particles[0].pz,tolerance);
    ASSERT_NEAR(3.0015,blocks_vector[1].particles[0].vx,tolerance);
    ASSERT_NEAR(3.0015,blocks_vector[1].particles[0].vy,tolerance);
    ASSERT_NEAR(2.0015,blocks_vector[1].particles[0].vz,tolerance);
    ASSERT_NEAR(3.003,blocks_vector[1].particles[0].hvx,tolerance);
    ASSERT_NEAR(3.003,blocks_vector[1].particles[0].hvy,tolerance);
    ASSERT_NEAR(2.003,blocks_vector[1].particles[0].hvz,tolerance);
}

//Test para combrobar un bloque con cx=0, pero con deltax < 1e-10
TEST(SimulationTests, ParticlesCollisions) {
    Block block(0,0,2,2);
    const Particle particle{0, 1, 1.0, 1.0, 1.0, 3.0, 3.0, 2.0, 1.0, 2.0, 2.0, 1.0, 1.0, 1.0, 4.0};
    std::vector<Block> blocks_vector;
    block.addParticle(particle);
    blocks_vector.push_back(block);
    particleColissions(blocks_vector,4.0,4.0,4.0);
    ASSERT_EQ(1.0,blocks_vector[0].particles[0].ax);
}

//Test para combrobar un bloque con cx=0, con deltax > 1e-10
TEST(SimulationTests, ParticlesCollisionsx) {
    Block block(0,0,2,2);
    const Particle particle{0, 1, -0.0652, 1.0, 1.0, -0.1, 3.0, 2.0, 1.0, 2.0, 2.0, 1.0, 1.0, 1.0, 4.0};
    std::vector<Block> blocks_vector;
    block.addParticle(particle);
    blocks_vector.push_back(block);
    particleColissions(blocks_vector,4.0,4.0,4.0);
    ASSERT_EQ(1,blocks_vector[0].particles[0].az);
    ASSERT_EQ(1,blocks_vector[0].particles[0].ay);
    ASSERT_NEAR(-112.0,blocks_vector[0].particles[0].ax,1e-5);
}

//Test para combrobar un bloque con cy=0, con deltay > 1e-10
TEST(SimulationTests, ParticlesCollisionsy) {
    Block block(0,2,0,2);
    const Particle particle{0, 1, 1.0, -0.08, 1.0, 1.0, -0.2, 2.0, 1.0, 2.0, 2.0, 1.0, 1.0, 1.0, 4.0};
    std::vector<Block> blocks_vector;
    block.addParticle(particle);
    blocks_vector.push_back(block);
    particleColissions(blocks_vector,4.0,4.0,4.0);
    ASSERT_EQ(1,blocks_vector[0].particles[0].az);
    ASSERT_EQ(1,blocks_vector[0].particles[0].ax);
    ASSERT_NEAR(-242.99999,blocks_vector[0].particles[0].ay,1e-5);
}

//Test para combrobar un bloque con cz=0, con deltaz > 1e-10
TEST(SimulationTests, ParticlesCollisionsz) {
    Block block(0,2,2,0);
    const Particle particle{0, 1, 1.0, 1.0, -0.0652, 1.0, 1.0, -0.1, 1.0, 2.0, 2.0, 1.0, 1.0, 1.0, 4.0};
    std::vector<Block> blocks_vector;
    block.addParticle(particle);
    blocks_vector.push_back(block);
    particleColissions(blocks_vector,4.0,4.0,4.0);
    ASSERT_EQ(1,blocks_vector[0].particles[0].ay);
    ASSERT_EQ(1,blocks_vector[0].particles[0].ax);
    ASSERT_NEAR(-240,blocks_vector[0].particles[0].az,1e-5);
}

//Test para combrobar un bloque con cx=0,cy=0 y cz=0 con deltax,deltay,deltaz > 1e-10
TEST(SimulationTests, ParticlesCollisionsxyzliminf) {
    Block block(0,0,0,0);
    const Particle particle{0, 1, -0.0652, -0.08, -0.0652, -0.1, -0.2, -0.1, 1.0, 2.0, 2.0, 1.0, 1.0, 1.0, 4.0};
    std::vector<Block> blocks_vector;
    block.addParticle(particle);
    blocks_vector.push_back(block);
    particleColissions(blocks_vector,4.0,4.0,4.0);
    ASSERT_NEAR(-112.0,blocks_vector[0].particles[0].ax,1e-5);
    ASSERT_NEAR(-242.99999,blocks_vector[0].particles[0].ay,1e-5);
    ASSERT_NEAR(-240,blocks_vector[0].particles[0].az,1e-5);
}

//test para comprobar un bloque con cx,cy,cz=numblocs-1 y deltaz,deltax,deltay >1e-10
TEST(SimulationTests, ParticlesCollisionsxyzlimsup) {
    Block block(0,3,3,3);
    const Particle particle{0, 1, 0.065, 0.12, 0.065, -0.1, -0.2, -0.1, 1.0, 2.0, 2.0, 1.0, 1.0, 1.0, 4.0};
    std::vector<Block> blocks_vector;
    block.addParticle(particle);
    blocks_vector.push_back(block);
    particleColissions(blocks_vector,4.0,4.0,4.0);
    std::cout<<blocks_vector[0].particles[0].ax<<"\n";
    std::cout<<blocks_vector[0].particles[0].ay<<"\n";
    std::cout<<blocks_vector[0].particles[0].az<<"\n";
    ASSERT_NEAR(-130,blocks_vector[0].particles[0].ax,1e-5);
    ASSERT_NEAR(-855,blocks_vector[0].particles[0].ay,1e-5);
    ASSERT_NEAR(-258,blocks_vector[0].particles[0].az,1e-5);
}

//test para comprobar que particlecollisions funciona pasando un vector de bloques de tamaño mayor que uno
TEST(SimulationTests, ParticlesCollisionsxyzDifBlocks) {
    Block block1(0,0,2,2);
    Block block2(0,2,0,2);
    Block block3(0,2,2,0);
    const Particle particle1{0, 1, -0.0652, 1.0, 1.0, -0.1, 3.0, 2.0, 1.0, 2.0, 2.0, 1.0, 1.0, 1.0, 4.0};
    const Particle particle2{0, 1, 1.0, -0.08, 1.0, 1.0, -0.2, 2.0, 1.0, 2.0, 2.0, 1.0, 1.0, 1.0, 4.0};
    const Particle particle3{0, 1, 1.0, 1.0, -0.0652, 1.0, 1.0, -0.1, 1.0, 2.0, 2.0, 1.0, 1.0, 1.0, 4.0};
    std::vector<Block> blocks_vector;
    block1.addParticle(particle1);
    block2.addParticle(particle2);
    block3.addParticle(particle3);
    blocks_vector.push_back(block1);
    blocks_vector.push_back(block2);
    blocks_vector.push_back(block3);
    particleColissions(blocks_vector,4.0,4.0,4.0);
    //comprobamos la particula del primer bloque
    ASSERT_EQ(1,blocks_vector[0].particles[0].ay);
    ASSERT_EQ(1,blocks_vector[0].particles[0].az);
    ASSERT_NEAR(-112.0,blocks_vector[0].particles[0].ax,1e-5);
    //comprobamos la particula del segundo bloque
    ASSERT_EQ(1,blocks_vector[1].particles[0].az);
    ASSERT_EQ(1,blocks_vector[1].particles[0].ax);
    ASSERT_NEAR(-242.99999,blocks_vector[1].particles[0].ay,1e-5);
    //comprobamos la particula del tercer bloque
    ASSERT_EQ(1,blocks_vector[2].particles[0].ay);
    ASSERT_EQ(1,blocks_vector[2].particles[0].ax);
    ASSERT_NEAR(-240,blocks_vector[2].particles[0].az,1e-5);
}

TEST(SimulationTests, LimitInteractionsx) {
    Block block(0,0,2,2);
    const Particle particle{0, 1, -0.066, 1.0, 1.0, -0.1, 3.0, 2.0, 1.0, 2.0, 2.0, 1.0, 1.0, 1.0, 4.0};
    std::vector<Block> blocks_vector;
    block.addParticle(particle);
    blocks_vector.push_back(block);
    limitInteractions(blocks_vector,4.0,4.0,4.0);
    ASSERT_EQ(-0.064,blocks_vector[0].particles[0].px);
    ASSERT_EQ(+0.1,blocks_vector[0].particles[0].hvx);
    ASSERT_NEAR(-1.0,blocks_vector[0].particles[0].vx,1e-5);
}

TEST(SimulationTests, LimitInteractionsy) {
    Block block(0,2,0,2);
    const Particle particle{0, 1, 1.0, -0.09, 1.0, 1.0, -0.1, 2.0, 1.0, 1.0, 2.0, 1.0, 1.0, 1.0, 4.0};
    std::vector<Block> blocks_vector;
    block.addParticle(particle);
    blocks_vector.push_back(block);
    limitInteractions(blocks_vector,4.0,4.0,4.0);
    ASSERT_EQ(-0.07,blocks_vector[0].particles[0].py);
    ASSERT_EQ(+0.1,blocks_vector[0].particles[0].hvy);
    ASSERT_NEAR(-1.0,blocks_vector[0].particles[0].vy,1e-5);
}

TEST(SimulationTests, LimitInteractionsz) {
    Block block(0,2,2,0);
    const Particle particle{0, 1, 1.0, 1.0, -0.066, 1.0, 3.0, -0.1, 1.0, 2.0, 1.0, 1.0, 1.0, 1.0, 4.0};
    std::vector<Block> blocks_vector;
    block.addParticle(particle);
    blocks_vector.push_back(block);
    limitInteractions(blocks_vector,4.0,4.0,4.0);
    ASSERT_EQ(-0.064,blocks_vector[0].particles[0].pz);
    ASSERT_EQ(+0.1,blocks_vector[0].particles[0].hvz);
    ASSERT_NEAR(-1.0,blocks_vector[0].particles[0].vz,1e-5);
}

TEST(SimulationTests, LimitInteractionsxyzliminf) {
    Block block(0,0,0,0);
    const Particle particle{0, 1, -0.066, -0.09, -0.066, -0.1, -0.1, -0.1, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 4.0};
    std::vector<Block> blocks_vector;
    block.addParticle(particle);
    blocks_vector.push_back(block);
    limitInteractions(blocks_vector,4.0,4.0,4.0);
    ASSERT_EQ(-0.064,blocks_vector[0].particles[0].px);
    ASSERT_EQ(+0.1,blocks_vector[0].particles[0].hvx);
    ASSERT_NEAR(-1.0,blocks_vector[0].particles[0].vx,1e-5);
    ASSERT_EQ(-0.07,blocks_vector[0].particles[0].py);
    ASSERT_EQ(+0.1,blocks_vector[0].particles[0].hvy);
    ASSERT_NEAR(-1.0,blocks_vector[0].particles[0].vy,1e-5);
    ASSERT_EQ(-0.064,blocks_vector[0].particles[0].pz);
    ASSERT_EQ(+0.1,blocks_vector[0].particles[0].hvz);
    ASSERT_NEAR(-1.0,blocks_vector[0].particles[0].vz,1e-5);
}

TEST(SimulationTests, LimitInteractionsxyzlimsup) {
    Block block(0,3,3,3);
    const Particle particle{0, 1, +0.066, +0.11, +0.066, +0.1, +0.1, +0.1, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 4.0};
    std::vector<Block> blocks_vector;
    block.addParticle(particle);
    blocks_vector.push_back(block);
    limitInteractions(blocks_vector,4.0,4.0,4.0);
    ASSERT_EQ(+0.064,blocks_vector[0].particles[0].px);
    ASSERT_EQ(-0.1,blocks_vector[0].particles[0].hvx);
    ASSERT_NEAR(-1.0,blocks_vector[0].particles[0].vx,1e-5);
    ASSERT_NEAR(0.09,blocks_vector[0].particles[0].py,1e-5);
    ASSERT_EQ(-0.1,blocks_vector[0].particles[0].hvy);
    ASSERT_NEAR(-1.0,blocks_vector[0].particles[0].vy,1e-5);
    ASSERT_EQ(+0.064,blocks_vector[0].particles[0].pz);
    ASSERT_EQ(-0.1,blocks_vector[0].particles[0].hvz);
    ASSERT_NEAR(-1.0,blocks_vector[0].particles[0].vz,1e-5);
}

TEST(SimulationTests, LimitInteractionsxyzdifbloklimsup) {
    Block block1(0,3,0,0);
    Block block2(0,0,3,0);
    Block block3(0,0,0,3);
    const Particle particle1{0, 1, +0.066, +1.0, +1.0, +0.1, +0.2, +0.2, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 4.0};
    const Particle particle2{0, 1, 1.0, +0.11, +1.0, +0.2, +0.1, +0.2, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 4.0};
    const Particle particle3{0, 1, 1.0, 1.0, +0.066, +0.2, +0.2, +0.1, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 4.0};
    std::vector<Block> blocks_vector;
    block1.addParticle(particle1);
    block2.addParticle(particle2);
    block3.addParticle(particle3);
    blocks_vector.push_back(block1);
    blocks_vector.push_back(block2);
    blocks_vector.push_back(block3);
    limitInteractions(blocks_vector,4.0,4.0,4.0);
    ASSERT_EQ(+0.064,blocks_vector[0].particles[0].px);
    ASSERT_EQ(-0.1,blocks_vector[0].particles[0].hvx);
    ASSERT_NEAR(-1.0,blocks_vector[0].particles[0].vx,1e-5);
    ASSERT_NEAR(0.09,blocks_vector[1].particles[0].py,1e-5);
    ASSERT_EQ(-0.1,blocks_vector[1].particles[0].hvy);
    ASSERT_NEAR(-1.0,blocks_vector[1].particles[0].vy,1e-5);
    ASSERT_EQ(+0.064,blocks_vector[2].particles[0].pz);
    ASSERT_EQ(-0.1,blocks_vector[2].particles[0].hvz);
    ASSERT_NEAR(-1.0,blocks_vector[2].particles[0].vz,1e-5);
}

//por razones de simplicidad para probar transformdesnsities utilizamos un h y un factordenstransf sencillos
TEST(SimulationTests, TransformDensities) {
    Block block1(0,3,0,0);
    const Particle particle1{0, 1, +0.066, +1.0, +1.0, +0.1, +0.2, +0.2, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0};
    std::vector<Block> blocks_vector;
    block1.addParticle(particle1);
    blocks_vector.push_back(block1);
    transformDensities(blocks_vector,2.0,0.1);
    ASSERT_NEAR(blocks_vector[0].particles[0].density,6.5,1e-5);
}

//test para comprobr el funcionamiento de transformdensities con varios bloques
TEST(SimulationTests, TransformDensities2) {
    Block block1(0,3,0,0);
    Block block2(1,3,1,0);
    Block block3(2,3,0,1);
    const Particle particle1{0, 1, +0.066, +1.0, +1.0, +0.1, +0.2, +0.2, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0};
    const Particle particle2{0, 1, +0.066, +1.0, +1.0, +0.1, +0.2, +0.2, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 2.0};
    const Particle particle3{0, 1, +0.066, +1.0, +1.0, +0.1, +0.2, +0.2, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 3.0};
    std::vector<Block> blocks_vector;
    block1.addParticle(particle1);
    block2.addParticle(particle2);
    block3.addParticle(particle3);
    blocks_vector.push_back(block1);
    blocks_vector.push_back(block2);
    blocks_vector.push_back(block3);
    transformDensities(blocks_vector,2.0,0.1);
    ASSERT_NEAR(blocks_vector[0].particles[0].density,6.5,1e-5);
    ASSERT_NEAR(blocks_vector[1].particles[0].density,6.6,1e-5);
    ASSERT_NEAR(blocks_vector[2].particles[0].density,6.7,1e-5);
}