#include "particleemitter.h"

#include <iostream>

using namespace std;

ParticleEmitter::ParticleEmitter(GLuint textureId, float3 color, float3 velocity,
                                 float3 force, float scale, float fuzziness, float speed,
                                 unsigned maxParticles) :
        m_textureid(textureId), m_color(color), m_velocity(velocity), m_force(force),
        m_scale(scale), m_fuzziness(fuzziness), m_speed(speed), m_maxparticles(maxParticles) {
    m_particles = new Particle[maxParticles];
    resetParticles();
    for(unsigned i = 0; i < m_maxparticles; ++i) m_particles[i].active = false;
    m_speed /= (10000.0f); //rescale the speed
}

ParticleEmitter::~ParticleEmitter() {
    delete[] m_particles;
}


/**
  This method should reset particle i to its intial state
  by setting its position, life, decay, color, direction and force.

  @TODO: Finish filling this in!
 **/
void ParticleEmitter::resetParticle(unsigned i) {
    m_particles[i].pos.zero();
    m_particles[i].life = 1;
    m_particles[i].decay = urand(.0025, .15);
    m_particles[i].color = m_color;
    m_particles[i].force.x = urand(-m_fuzziness * .01, m_fuzziness * .01);
    m_particles[i].force.y = urand(-m_fuzziness * .01, m_fuzziness * .01);
    m_particles[i].force.z = urand(-m_fuzziness * .01, m_fuzziness * .01);
    m_particles[i].force += m_force;
    m_particles[i].dir.x = urand(-m_fuzziness, m_fuzziness);
    m_particles[i].dir.y = urand(-m_fuzziness, m_fuzziness);
    m_particles[i].dir.z = urand(-m_fuzziness, m_fuzziness);
    m_particles[i].dir += m_velocity;


}
/**
  This method will loop through each particle and reset it.
  **/
void ParticleEmitter::resetParticles() {
    for (unsigned i = 0; i < m_maxparticles; i++)
        resetParticle(i);
}

/**
  This method should update the particles properties.  This should perform
  the main physics calculations of the particles.

  @TODO: Finish filling this in!
  **/
void ParticleEmitter::updateParticles() {
    for(unsigned i = 0; i < m_maxparticles; ++i) {
        if(!m_particles[i].active){
            m_particles[i].active = true;
            resetParticle(i);
        }
        m_particles[i].pos += m_particles[i].dir * m_speed;
        m_particles[i].dir += m_particles[i].force;
        m_particles[i].life -= m_particles[i].decay;
        if(m_particles[i].life < 0){
            m_particles[i].active = false;
        }
    }
}

/**
  This method should draw each particle as a small, texture mapped square of size
  m_scale with a zdepth given by the particles z position.

  @TODO: Finish filling this in!
  **/
void ParticleEmitter::drawParticles() {

    glEnable(GL_BLEND);
    glBindTexture(GL_TEXTURE_2D, m_textureid);
    glDepthMask(false);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glBegin(GL_QUADS);
    for(int i=0; i < m_maxparticles; i++){
        if(m_particles[i].active){
            glColor4f(m_color.r, m_color.g, m_color.b, m_particles[i].life);
            glTexCoord2f(0, 0);
            glVertex3f(m_particles[i].pos.x - m_scale, m_particles[i].pos.y - m_scale, m_particles[i].pos.z);
            glTexCoord2f(0, 1);

            glVertex3f(m_particles[i].pos.x - m_scale, m_particles[i].pos.y + m_scale, m_particles[i].pos.z);
            glTexCoord2f(1, 1);

            glVertex3f(m_particles[i].pos.x + m_scale, m_particles[i].pos.y + m_scale, m_particles[i].pos.z);
            glTexCoord2f(1, 0);

            glVertex3f(m_particles[i].pos.x + m_scale, m_particles[i].pos.y - m_scale, m_particles[i].pos.z);
        }
    }
    glDepthMask(true);

    glEnd();

    glAccum(GL_MULT, .8);
    glAccum(GL_ACCUM, 1);
    glAccum(GL_RETURN, 1);
}
