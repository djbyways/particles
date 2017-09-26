/*
 * Copyright 1993-2012 NVIDIA Corporation.  All rights reserved.
 *
 * Please refer to the NVIDIA end user license agreement (EULA) associated
 * with this source code for terms and conditions that govern your use of
 * this software. Any use, reproduction, disclosure, or distribution of
 * this software and related documentation outside the terms of the EULA
 * is strictly prohibited.
 *
 */
  /** \file particleSystem.h
  * \brief Definicja klasy ParticleSystem i części jej prostych metod.
  *
  */

#ifndef __PARTICLESYSTEM_H__
#define __PARTICLESYSTEM_H__

#define DEBUG_GRID 0
#define DO_TIMING 0

#include <helper_functions.h>
#include "particles_kernel.cuh"
#include "vector_functions.h"

// Particle system class
/** \brief Klasa odpowiadająca za parametry i ustawienie symulacji.
 */
class ParticleSystem
{
    public:
        ParticleSystem(uint numParticles, uint3 gridSize, bool bUseOpenGL);
        ~ParticleSystem();

        enum ParticleConfig
        {
            CONFIG_RANDOM,
            CONFIG_GRID,
            _NUM_CONFIGS
        };

        enum ParticleArray
        {
            POSITION,
            VELOCITY,
        };

        void update(double deltaTime);
        void reset(ParticleConfig config);

		double *getArray(ParticleArray array);
        void   setArray(ParticleArray array, const double *data, int start, int count);

        int    getNumParticles() const
        {
            return m_numParticles;
        }

        unsigned int getCurrentReadBuffer() const
        {
            return m_posVbo;
        }
        unsigned int getColorBuffer()       const
        {
            return m_colorVBO;
        }

        void *getCudaPosVBO()              const
        {
            return (void *)m_cudaPosVBO;
        }
        void *getCudaColorVBO()            const
        {
            return (void *)m_cudaColorVBO;
        }

        void dumpGrid();
        void dumpParticles(uint start, uint count);

        void setIterations(int i)
        {
            m_solverIterations = i;
        }

        void setDamping(double x)
        {
            m_params.globalDamping = x;
        }
        void setGravity(double x)
        {
            m_params.gravity = make_float3(0.0f, x, 0.0f);
        }

		void setBoundaryDamping(double x)// ustawienie wsp napiêcia powierzchniowego
        {
            m_params.boundaryDamping = x;
        }
		/** \brief ustawia masę cząstki
		 * \todo jakoś poprawić bo rożne typy cząstek mają różne masy
		 */
		void setParticleMass(double x)
		{
			m_params.particleMass[0]=x;
		}
		void setEpsi(double x)
		{
			m_params.epsi=x;
		}

        void setCollideSpring(double x)
        {
            m_params.spring = x;
        }
        void setCollideDamping(double x)
        {
            m_params.damping = x;
        }
        void setCollideShear(double x)
        {
            m_params.shear = x;
        }
        void setCollideAttraction(double x)
        {
            m_params.attraction = x;
        }

		void setBigRadius(double x)//ustawienie promienia duzej kuli
		{
			m_params.bigradius=x;
		}
		void setBigRadius0(double x)//ustawienie promienia duzej kuli
		{
			m_params.bigradius0=x;
		}
		void setBrown(double x)
		{
			m_params.brown=x;
		}
		void setBrownQuality(unsigned long long int x)
		{
			m_params.brownQuality=x;
		}

		void setBoundaries(bool x)//ograniczenia zewnêtrzne
		{
			m_params.boundaries=x;
		}

		void setParticleTypesNum(int x)//ograniczenia zewnêtrzne
		{
			m_params.particleTypesNum=x;
		}

        void setColliderPos(float3 x)
        {
            m_params.colliderPos = x;
        }

        /** \brief ustawienie zmiennej logicznej czy chcemy
         * policzyć i zwrócić ciśnienie powierzchniowe kropli
         * \param x bool
         * \return void
         *
         */
        void setCalcSurfacePreasure(bool x)
        {
            m_params.calcSurfacePreasure=x;
        }

        /** \brief ustawia chwilową prędkość zmniejszania się promienia kropli
         *
         * \param x float
         * \return void
         *
         */
        void setSurfaceVel(double x)
        {
            m_params.surfaceVel=x;
        }

		/** \brief pobiera promień cząstki
		 * \todo poprawić z uwzględnieniem różnych typów cząstek
		 */
		double getParticleRadius()
        {
            return m_params.particleRadius[0];
        }

        /** \brief Zwraca promień największej cząstki.
         *
         * \return float
         *
         */
		double getMaxParticleRadius()
        {
			double tmpMax=0.0f;
            for(int i=0; i<m_params.particleTypesNum;i++)
            {
                if(m_params.particleRadius[i]>tmpMax)
                {
                    tmpMax=m_params.particleRadius[i];
                }
            }
            return tmpMax;
        }

        /** \brief ustawia czy zmienny krok czasu, czy stały
         *
         * \param x bool
         * \return void
         *
         */
        void setAutoDt(bool x)
        {
            m_params.autoDt=x;
        }

        float3 getColliderPos()
        {
            return m_params.colliderPos;
        }
		double getColliderRadius()
        {
            return m_params.colliderRadius;
        }
        uint3 getGridSize()
        {
            return m_params.gridSize;
        }
        float3 getWorldOrigin()
        {
            return m_params.worldOrigin;
        }
        float3 getCellSize()
        {
            return m_params.cellSize;
        }

        void addSphere(int index, double *pos, double *vel, int r, double spacing);

		//class particleType;

    protected: // methods
        ParticleSystem() {}
        uint createVBO(uint size);

        void _initialize(int numParticles);
        void _finalize();

        void initGrid(uint *size, double spacing, double jitter, uint numParticles);

    protected: // data
        bool m_bInitialized, m_bUseOpenGL;
        uint m_numParticles;

        // CPU data
		double *m_hPos;              // particle positions
		double *m_hVel;              // particle velocities

        uint  *m_hParticleHash;
        uint  *m_hCellStart;
        uint  *m_hCellEnd;

        // GPU data
		double *m_dPos;
		double *m_dVel;

		double *m_dForce;/**<  */

		double *m_dSortedPos;
		double *m_dSortedVel;

        // grid data for sorting method
        uint  *m_dGridParticleHash; // grid hash value for each particle
        uint  *m_dGridParticleIndex;// particle index for each particle
        uint  *m_dCellStart;        // index of start of each cell in sorted list
        uint  *m_dCellEnd;          // index of end of cell

        uint   m_gridSortBits;

        uint   m_posVbo;            // vertex buffer object for particle positions
        uint   m_colorVBO;          // vertex buffer object for colors

        double *m_cudaPosVBO;        // these are the CUDA deviceMem Pos
		double *m_cudaColorVBO;      // these are the CUDA deviceMem Color


        struct cudaGraphicsResource *m_cuda_posvbo_resource; // handles OpenGL-CUDA exchange
        struct cudaGraphicsResource *m_cuda_colorvbo_resource; // handles OpenGL-CUDA exchange

        // params
        SimParams m_params;
        uint3 m_gridSize;
        uint m_numGridCells;

        StopWatchInterface *m_timer;

        uint m_solverIterations;
};

#endif // __PARTICLESYSTEM_H__
