#pragma once
#include "common.h"
#include <QtOpenGL>

class ParticleEmitter {

    /** Basic particle structure, you should not need to modify this.
        active - Determines whether or not this particle is active.  If
                 it is not active, it should not be drawn or modified by
                 updateParticles().
        life   - The amount of life remaining.  1.0 should be full life (when
                 a particle is first born, and should decrease down to zero by
                 decay (each step, life -= decay).  When it reaches zero, the
                 particle should be reset.
        color  - The particle's color.  Constant for all particles at the moment
                 although it would be possible to have randomly generated colors
                 or colors based on time.  Used when drawing the particle.
        pos    - The particles current position in 3 space.  Updated every step based
                 on its direction (dir) or velocity.
        dir    - The velocity of the particle, the current direction it is travelling in.
                 At each step, the position is modified such that pos += dir * mSpeed.
        force  - The force acting on the particle (ex. gravity) at each update, dir += force.
    **/
    struct __attribute__ ((aligned (16))) Particle {
        bool active;
        float life, decay;
        float3 color, pos, dir, force;
    };


public:
    ParticleEmitter(GLuint textureId = 0,
                    float3 color = float3(1.0f, 0.6f, 0.2f),
                    float3 velocity = float3(0.0f, 0.0001f, 0.0f),
                    float3 force = float3(0.0f, 0.0001f, 0.0f),
                    float scale = .5f,
                    float fuzziness = 50.0f,
                    float speed = 50.0f,
                    unsigned maxParticles = 10000);

    ~ParticleEmitter();


    void drawParticles();
    void resetParticle(unsigned i);
    void resetParticles();
    void updateParticles();


    //Don't modify these please. kthx.
    inline float3& force()       { return m_force; }
    inline float3& velocity()    { return m_velocity; }
    inline float3& color()       { return m_color; }
    inline float& speed()        { return m_speed; }
    inline GLuint texture() { return m_textureid; }

protected:
    Particle                 *m_particles;			// Particle Array (holding each particle)
    unsigned                  m_maxparticles;                   // Maximum number of particles to allow
    GLuint                     m_textureid;                      // The texture id to use / bind to when drawing particles
    float                     m_speed, m_fuzziness, m_scale;    // The speed of the simulation, the randomness of particle direction, and the size of particles
    float3                    m_color, m_velocity, m_force;     // The color of the particles, initial velocity to reset to and initial force to reset to
};


