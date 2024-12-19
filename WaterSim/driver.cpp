// Core OpenGL And Window Management Libraries
#include "shader.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// Standard Library Components For Simulation
#include <random>
#include <iostream>
#include <chrono>
#include <thread>
#include <mutex>


// Window And Simulation Configuration Parameters
#define WINDOW_HEIGHT 1900                              // Vertical Resolution Of Simulation Window
#define WINDOW_WIDTH 1900                               // Horizontal Resolution Of Simulation Window
#define SEGMENT_CNT 10                                  // Number Of Segments Per Particle For Circular Rendering
#define PARTICLE_COUNT 5000                             // Total Number Of Particles In Simulation
#define M_PI 3.14159265359                              // Mathematical Constant Pi
#define BIN_SIZE 0.01f                                  // Size Of Each Spatial Partitioning Cell
#define NUM_THREADS 3                                   // Number Of Parallel Processing Threads
#define GRID_SIZE_X static_cast<int>(2.0f / BIN_SIZE)   // Horizontal Grid Resolution
#define GRID_SIZE_Y static_cast<int>(2.0f / BIN_SIZE)   // Vertical Grid Resolution



// Struct For Spatial Hashing Coordinates
struct HashCoord
{
    int x, y;

    bool operator==(const HashCoord& other) const
    {
        return x == other.x && y == other.y;
    }
};


// Struct For OpenGL Vertex Data
struct Vertex
{
    glm::vec2 position;
    glm::vec3 color;
};


// Structure For Tracking Particle Collisions
struct CollisionPair
{
    int i, j;
    float time;

    bool operator<(const CollisionPair& other) const
    {
        return time < other.time;
    }
};



// Utility Function To Constrain Values Within A Range
// Preconditions:
//   1.) val Is A Valid Integer Value To Be Clamped
//   2.) min_val Is The Lower Bound Of The Valid Range
//   3.) max_val Is The Upper Bound Of The Valid Range
//   4.) min_val Must Be Less Than Or Equal To max_val
// Postconditions:
//   1.) Returns An Integer Value Constrained Between min_val And max_val
//   2.) Original Input Values Remain Unchanged
inline int clamp(int val, int min_val, int max_val)
{
    return std::min(std::max(val, min_val), max_val);
}



// Thread Synchronization Class For Coordinating Multiple Processing Threads
class ThreadSynchronizer
{

    public:
        // Constructor For Thread Synchronization
        // Preconditions:
        //   1.) thread_count Must Be A Positive Integer
        //   2.) thread_count Must Match The Actual Number Of Threads Being Synchronized
        // Postconditions:
        //   1.) Creates A New ThreadSynchronizer Instance
        //   2.) Initializes Internal Counters And Synchronization Mechanisms
        ThreadSynchronizer(int thread_count) : total_threads(thread_count), count(0)
        {
        }

        // Synchronization Point For Thread Coordination
        // Preconditions:
        //   1.) ThreadSynchronizer Instance Must Be Properly Initialized
        //   2.) Method Must Be Called By All Participating Threads
        // Postconditions:
        //   1.) All Threads Will Wait Until The Last Thread Arrives
        //   2.) All Threads Are Released Simultaneously Once All Have Arrived
        //   3.) Counter Is Reset For Next Synchronization Phase
        void arrive_and_wait()
        {
            std::unique_lock<std::mutex> lock(mtx);
            if (++count == total_threads)
            {
                count = 0;
                cv.notify_all();
            }
            else
            {
                cv.wait(lock);
            }
        }

    private:
        // Total Number Of Threads To Synchronize In The Barrier
        const int total_threads;

        // Current Count Of Threads That Have Reached The Barrier
        int count;

        // Condition Variable For Thread Waiting And Notification
        std::condition_variable cv;

        // Mutex For Thread-Safe Access To Shared Count
        std::mutex mtx;

};


// Structure To Store Particle Data In A Struct Of Arrays Format
class ParticleData
{

    public:
        // Current Velocity Vectors For Each Particle
        std::vector<glm::vec2> velocities;

        // Current Center Positions Of Each Particle
        std::vector<glm::vec2> centroids;

        // Future Predicted Positions For Collision Detection
        std::vector<glm::vec2> predictedPositions;

        // Radius Values For Each Particle's Circular Shape
        std::vector<float> radii;

        // RGB Color Values For Each Particle's Rendering
        std::vector<glm::vec3> colors;

        // Unique Identifiers For Mapping Particles To Vertex Data
        std::vector<long unsigned int> IDs;



        // Constructor For Particle Data Container
        // Preconditions:
        //   1.) count Must Be A Positive Integer
        //   2.) System Must Have Sufficient Memory For Allocation
        // Postconditions:
        //   1.) All Vectors Are Reserved With Specified Capacity
        //   2.) All Vectors Are Initialized To Specified Size
        ParticleData(unsigned int count)
        {
            velocities.reserve(count);
            centroids.reserve(count);
            predictedPositions.reserve(count);
            radii.reserve(count);
            colors.reserve(count);
            IDs.reserve(count);
            resize(count);
        }


        // Resize All Particle Data Containers
        // Preconditions:
        //   1.) count Must Be A Non-Negative Integer
        //   2.) System Must Have Sufficient Memory For Resizing
        // Postconditions:
        //   1.) All Vectors Are Resized To Specified Count
        //   2.) New Elements Are Default Initialized
        void resize(unsigned int count)
        {
            velocities.resize(count);
            centroids.resize(count);
            predictedPositions.resize(count);
            radii.resize(count);
            colors.resize(count);
            IDs.resize(count);
        }

};


// Class To Handle Spatial Partitioning And Collision Detection
class CollisionMatrix
{

    private:
        // Size Of Each Grid Cell For Spatial Partitioning
        float binSize;

        // 2D Grid Structure Storing Particle Indices In Each Cell
        std::vector<std::vector<unsigned int>> fixedGrid;

        // Array Of Mutexes For Thread-Safe Grid Cell Access
        std::vector<std::mutex> gridMutexes;



        // Calculate Time Until Wall Collision
        // Preconditions:
        //   1.) pos Must Be A Valid 2D Position Vector
        //   2.) vel Must Be A Valid 2D Velocity Vector
        //   3.) radius Must Be Positive
        // Postconditions:
        //   1.) Returns Time Until Next Wall Collision
        //   2.) Returns 1.0f If No Collision Will Occur
        float findWallCollisionTime(const glm::vec2& pos, const glm::vec2& vel, float radius)
        {
            float timeX = 1.0f, timeY = 1.0f;

            if (vel.x != 0.0f)
            {
                if (pos.x + vel.x + radius > 1.0f)
                {
                    timeX = (1.0f - radius - pos.x) / vel.x;
                }
                else if (pos.x + vel.x - radius < -1.0f)
                {
                    timeX = (-1.0f + radius - pos.x) / vel.x;
                }
            }

            if (vel.y != 0.0f)
            {
                if (pos.y + vel.y + radius > 1.0f)
                {
                    timeY = (1.0f - radius - pos.y) / vel.y;
                }
                else if (pos.y + vel.y - radius < -1.0f)
                {
                    timeY = (-1.0f + radius - pos.y) / vel.y;
                }
            }

            return std::min(timeX, timeY);
        }


        // Handle Wall Collision Resolution
        // Preconditions:
        //   1.) particles Contains Valid Particle Data
        //   2.) i Is A Valid Index Into The Particle Arrays
        //   3.) Particle Properties Are Within Valid Physical Bounds
        // Postconditions:
        //   1.) Particle Position Is Adjusted To Prevent Wall Penetration
        //   2.) Particle Velocity Is Updated With Wall Bounce Response
        //   3.) Energy Loss Is Applied Through Velocity Dampening
        void resolveWallCollision(ParticleData& particles, unsigned int i)
        {
            glm::vec2& pos = particles.centroids[i];
            glm::vec2& vel = particles.velocities[i];
            float radius = particles.radii[i];

            if (std::abs(pos.x) + radius >= 1.0f)
            {
                vel.x = -vel.x * 0.5f;
                pos.x = std::copysign(1.0f - radius, pos.x);
            }
            if (std::abs(pos.y) + radius >= 1.0f)
            {
                vel.y = -vel.y * 0.5f;
                pos.y = std::copysign(1.0f - radius, pos.y);
            }
        }


        // Convert World Position To Grid Hash Coordinates
        // Preconditions:
        //   1.) position Is A Valid 2D Vector In World Space
        //   2.) binSize Has Been Properly Initialized
        // Postconditions:
        //   1.) Returns Grid Coordinates For Spatial Partitioning
        //   2.) Returned Coordinates Are Clamped To Valid Grid Range
        HashCoord getHashCoord(const glm::vec2& position)
        {
            int x = static_cast<int>(std::floor((position.x + 1.0f) / binSize));
            int y = static_cast<int>(std::floor((position.y + 1.0f) / binSize));
            return { clamp(x, 0, GRID_SIZE_X - 1), clamp(y, 0, GRID_SIZE_Y - 1) };
        }


        // Get List Of Potential Collision Candidates
        // Preconditions:
        //   1.) centroid Is A Valid 2D Position Vector
        //   2.) currentParticleIndex Is A Valid Particle Index
        //   3.) Spatial Grid Has Been Updated With Current Particle Positions
        // Postconditions:
        //   1.) Returns Vector Of Particle Indices For Potential Collisions
        //   2.) Returned Indices Are Within Valid Range
        //   3.) Current Particle Is Not Included In Return List
        std::vector<unsigned int> getPotentialColliders(const glm::vec2& centroid, unsigned int currentParticleIndex)
        {
            static thread_local std::vector<unsigned int> potentialColliders;
            potentialColliders.clear();
            potentialColliders.reserve(PARTICLE_COUNT / 4.0f);

            HashCoord key = getHashCoord(centroid);

            const int dx[] = { 0, -1, 1, -1, 0, 1, -1, 0, 1 };
            const int dy[] = { 0, -1, -1, 0, 0, 0, 1, 1, 1 };

            for (int i = 0; i < 9; i++)
            {
                int nx = clamp(key.x + dx[i], 0, GRID_SIZE_X - 1);
                int ny = clamp(key.y + dy[i], 0, GRID_SIZE_Y - 1);
                int index = ny * GRID_SIZE_X + nx;

                for (unsigned int particleIdx : fixedGrid[index])
                {
                    if (particleIdx != currentParticleIndex && particleIdx < PARTICLE_COUNT)
                    {
                        potentialColliders.push_back(particleIdx);
                    }
                }
            }

            return potentialColliders;
        }


        // Calculate Time Until Collision Between Two Particles
        // Preconditions:
        //   1.) All Input Vectors Are Valid 2D Vectors
        //   2.) r1 And r2 Are Positive Radii
        // Postconditions:
        //   1.) Returns Time Until Collision (1.0f If No Collision)
        //   2.) Return Value Is In Range [0.0f, 1.0f]
        float findCollisionTime(const glm::vec2& p1, const glm::vec2& v1, float r1,
            const glm::vec2& p2, const glm::vec2& v2, float r2)
        {
            glm::vec2 relativePos = p1 - p2;
            glm::vec2 relativeVel = v1 - v2;
            float totalRadius = r1 + r2;

            float a = glm::dot(relativeVel, relativeVel);
            if (a < 1e-6f) return 1.0f;

            float b = 2.0f * glm::dot(relativeVel, relativePos);
            float c = glm::dot(relativePos, relativePos) - totalRadius * totalRadius;
            float discriminant = b * b - 4.0f * a * c;
            if (discriminant < 0) return 1.0f;

            float t = (-b - std::sqrt(discriminant)) / (2.0f * a);
            return (t >= 0.0f && t <= 1.0f) ? t : 1.0f;
        }


        // Resolve Collision Between Two Particles
        // Preconditions:
        //   1.) particles Contains Valid Particle Data
        //   2.) i And j Are Valid Particle Indices
        //   3.) Particles Are Actually Colliding
        // Postconditions:
        //   1.) Particle Velocities Are Updated According To Physics
        //   2.) Collision Response Includes Elasticity And Force Multiplier
        //   3.) Energy Conservation Is Maintained Within Dampening Limits
        void resolveCollision(ParticleData& particles, unsigned int i, unsigned int j)
        {
            glm::vec2& pos1 = particles.centroids[i];
            glm::vec2& pos2 = particles.centroids[j];
            glm::vec2& vel1 = particles.velocities[i];
            glm::vec2& vel2 = particles.velocities[j];

            glm::vec2 normal = glm::normalize(pos1 - pos2);
            float distance = glm::length(pos1 - pos2);
            float minDist = particles.radii[i] + particles.radii[j];

            float overlap = minDist - distance;
            float forceMultiplier = 1.0f + (overlap / minDist) * 0.12f;

            float restitution = 0.9f;
            glm::vec2 relativeVel = vel1 - vel2;
            float velAlongNormal = glm::dot(relativeVel, normal);

            if (velAlongNormal > 0) return;

            float impulse = -(1.0f + restitution) * velAlongNormal * forceMultiplier;
            impulse /= 1.0f / particles.radii[i] + 1.0f / particles.radii[j];

            vel1 += impulse * normal / particles.radii[i];
            vel2 -= impulse * normal / particles.radii[j];
        }


    public:
        // Constructor For Collision Detection System
        // Preconditions:
        //   1.) bin_size Is A Positive Float Value
        //   2.) System Has Sufficient Memory For Grid Allocation
        // Postconditions:
        //   1.) Spatial Grid Is Initialized With Proper Dimensions
        //   2.) Grid Mutexes Are Created For Thread Safety
        CollisionMatrix(float bin_size) : binSize(bin_size), gridMutexes(GRID_SIZE_X* GRID_SIZE_Y)
        {
            fixedGrid.resize(GRID_SIZE_X * GRID_SIZE_Y);
        }


        // Clear All Entries From The Spatial Grid
        // Preconditions:
        //   1.) Grid Has Been Properly Initialized
        // Postconditions:
        //   1.) All Grid Cells Are Empty
        //   2.) Memory Is Not Deallocated, Only Cleared
        void clear()
        {
            for (auto& cell : fixedGrid)
            {
                cell.clear();
            }
        }


        // Add A Particle To The Spatial Grid
        // Preconditions:
        //   1.) particleIndex Is A Valid Particle Index
        //   2.) centroid Is A Valid 2D Position Vector
        // Postconditions:
        //   1.) Particle Is Added To Appropriate Grid Cell
        //   2.) Grid Structure Maintains Spatial Organization
        void addParticle(unsigned int particleIndex, const glm::vec2& centroid)
        {
            HashCoord key = getHashCoord(centroid);
            int index = key.y * GRID_SIZE_X + key.x;
            fixedGrid[index].push_back(particleIndex);
        }


        // Detect And Resolve All Particle Collisions
        // Preconditions:
        //   1.) particles Contains Valid Particle Data
        //   2.) threadId Is Valid Thread Identifier
        //   3.) numThreads Matches Total Active Threads
        // Postconditions:
        //   1.) All Collisions In Thread's Partition Are Resolved
        //   2.) Particle Positions And Velocities Are Updated
        //   3.) Thread-Safe Updates Are Maintained
        void detectAndResolveCollisions(ParticleData& particles, int threadId, int numThreads)
        {
            const int start = (PARTICLE_COUNT * threadId) / numThreads;
            const int end = (PARTICLE_COUNT * (threadId + 1)) / numThreads;

            for (int i = start; i < end; i++)
            {
                auto nearParticles = getPotentialColliders(particles.centroids[i], i);

                for (unsigned int j : nearParticles)
                {
                    if (i == j) continue;
                    float dist = glm::length(particles.centroids[i] - particles.centroids[j]);
                    float minDist = particles.radii[i] + particles.radii[j];
                    if (dist < minDist)
                    {
                        resolveCollision(particles, i, j);
                    }
                }

                resolveWallCollision(particles, i);
                particles.velocities[i] *= 0.97f;
                particles.velocities[i].y -= 0.0004f;
            }
        }

};


// Main Class For Particle Simulation And Rendering
class ParticleWindow
{

    public:
        // Constructor For Particle Simulation Window
        // Preconditions:
        //   1.) width And height Are Positive Integers
        //   2.) OpenGL Context Can Be Created
        //   3.) System Has Sufficient GPU And CPU Resources
        // Postconditions:
        //   1.) Window And OpenGL Context Are Created
        //   2.) Particles Are Initialized
        //   3.) Rendering Buffers Are Set Up
        //   4.) Simulation Threads Are Started
        ParticleWindow(int width, int height)
            : particles(PARTICLE_COUNT), collisionMatrix(BIN_SIZE),
            updateBarrier(NUM_THREADS + 1), renderBarrier(NUM_THREADS + 1)
        {
            if (!glfwInit())
            {
                throw std::runtime_error("Failed to initialize GLFW");
            }

            window = glfwCreateWindow(width, height, "Particle Simulation", NULL, NULL);
            if (!window)
            {
                glfwTerminate();
                throw std::runtime_error("Failed to create GLFW window");
            }

            glfwMakeContextCurrent(window);
            gladLoadGL();
            glfwSetWindowUserPointer(window, this);
            glfwSetMouseButtonCallback(window, mouseButtonCallbackWrapper);
            shaderEngine = Shader("default.vert", "default.frag");
            shaderEngine.Activate();
            std::vector<GLuint> indices;
            initParticles(indices);
            glGenVertexArrays(1, &VAO);
            glBindVertexArray(VAO);
            glGenBuffers(1, &VBO);
            glGenBuffers(1, &EBO);
            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
            glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_DYNAMIC_DRAW);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);
            indiceCount = indices.size();
            glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(2 * sizeof(float)));
            glEnableVertexAttribArray(1);

            for (int i = 0; i < NUM_THREADS; ++i)
            {
                threads.emplace_back(&ParticleWindow::updateParticlesThreaded, this, i);
            }
        }
    

        // Destructor For Particle Window
        // Preconditions:
        //   1.) ParticleWindow Object Exists
        //   2.) All Resources Are Valid
        // Postconditions:
        //   1.) All Threads Are Properly Terminated
        //   2.) OpenGL Resources Are Released
        //   3.) Window Is Destroyed
        ~ParticleWindow()
        {
            terminateThreads = true;
            for (auto& thread : threads)
            {
                if (thread.joinable())
                {
                    thread.join();
                }
            }
            shaderEngine.Delete();
        }


        // Initialize Particle Properties And Rendering Data
        // Preconditions:
        //   1.) indices Vector Is Empty
        //   2.) Random Number Generator Is Available
        //   3.) Sufficient Memory For Particle Data
        // Postconditions:
        //   1.) All Particles Have Random Initial States
        //   2.) Vertex And Index Buffers Are Populated
        //   3.) Rendering Geometry Is Created For Each Particle
        void initParticles(std::vector<GLuint>& indices)
        {
            vertices.reserve(PARTICLE_COUNT * (SEGMENT_CNT + 1));
            indices.reserve(PARTICLE_COUNT * SEGMENT_CNT * 3);

            std::random_device rd;
            std::mt19937 seed(rd());
            std::uniform_real_distribution<float> radiusDist(0.005f, 0.01f);
            std::uniform_real_distribution<float> positionDist(-0.799f, 0.799f);
            std::uniform_real_distribution<float> colorDist(0.3f, 1.0f);
            std::uniform_real_distribution<float> colorDist2(0.0f, 0.2f);
            std::uniform_real_distribution<float> velocityDist(-0.00015f, 0.00015f);

            for (unsigned int i = 0, centerOffset; i < PARTICLE_COUNT; i++)
            {
                particles.centroids[i] = glm::vec2(positionDist(seed), positionDist(seed));
                particles.radii[i] = radiusDist(seed);
                particles.colors[i] = glm::vec3(colorDist2(seed), colorDist2(seed), colorDist(seed));
                particles.velocities[i] = glm::vec2(velocityDist(seed), velocityDist(seed)) / particles.radii[i];
                particles.IDs[i] = vertices.size();

                vertices.push_back({ particles.centroids[i], particles.colors[i] });
                for (int j = 0; j < SEGMENT_CNT; ++j)
                {
                    float angle = (2.0f * M_PI * j) / SEGMENT_CNT;
                    glm::vec2 pos = particles.centroids[i] + particles.radii[i] * glm::vec2(cos(angle), sin(angle));
                    vertices.push_back({ pos, particles.colors[i] });
                }

                centerOffset = vertices.size() - SEGMENT_CNT - 1;
                for (unsigned int j = 0; j < SEGMENT_CNT - 1; ++j)
                {
                    indices.push_back(centerOffset);
                    indices.push_back(centerOffset + j + 1);
                    indices.push_back(centerOffset + j + 2);
                }
                indices.push_back(centerOffset);
                indices.push_back(centerOffset + SEGMENT_CNT);
                indices.push_back(centerOffset + 1);
            }
        }


        // Mouse Button Event Callback Wrapper
        // Preconditions:
        //   1.) window Is A Valid GLFWwindow Pointer
        //   2.) button, action, And mods Are Valid GLFW Input Parameters
        // Postconditions:
        //   1.) Mouse Input Is Properly Routed To Instance Handler
        static void mouseButtonCallbackWrapper(GLFWwindow* window, int button, int action, int mods)
        {
            ParticleWindow* instance = static_cast<ParticleWindow*>(glfwGetWindowUserPointer(window));
            instance->mouseButtonCallback(button, action);
        }


        // Handle Mouse Button Events
        // Preconditions:
        //   1.) button Is A Valid GLFW Mouse Button Code
        //   2.) action Is A Valid GLFW Action Code
        // Postconditions:
        //   1.) Click Position Is Updated
        //   2.) Force Application Is Triggered If Applicable
        void mouseButtonCallback(int button, int action)
        {
            if (this->mouseButtonHeld = action == GLFW_PRESS)
            {
                double xpos, ypos;
                glfwGetCursorPos(window, &xpos, &ypos);
                clickPosition = glm::vec2(2.0f * (xpos / WINDOW_WIDTH) - 1.0f, 1.0f - 2.0f * (ypos / WINDOW_HEIGHT));
                isClick = button;
                applyRepulsiveForce();
            }
        }


        // Apply Forces Based On Mouse Input
        // Preconditions:
        //   1.) Click Position Is Valid
        //   2.) Particle Data Is Valid
        // Postconditions:
        //   1.) Particle Velocities Are Updated Based On Force
        void applyRepulsiveForce()
        {
            if (isClick)
            {
                const float forceStrength = 0.0000565f;
                const float minDistanceSquared = 0.1f;
                for (unsigned int i = 0; i < PARTICLE_COUNT; ++i)
                {
                    glm::vec2 direction = clickPosition - particles.centroids[i];
                    float distanceSquared = glm::dot(direction, direction);
                    if (distanceSquared > minDistanceSquared)
                    {
                        glm::vec2 force = direction / distanceSquared;
                        particles.velocities[i] += (force * forceStrength) / particles.radii[i];
                    }
                }
            }
            else
            {
                const float forceStrength = 0.0000095f;
                const float minDistanceSquared = 0.001f;
                for (unsigned int i = 0; i < PARTICLE_COUNT; ++i)
                {
                    glm::vec2 direction = particles.centroids[i] - clickPosition;
                    float distanceSquared = glm::dot(direction, direction);
                    if (distanceSquared > minDistanceSquared)
                    {
                        glm::vec2 force = direction / distanceSquared;
                        particles.velocities[i] += (force * forceStrength) / particles.radii[i];
                    }
                }
            }
        }


        // Apply Continuous Forces While Mouse Button Is Held
        // Preconditions:
        //   1.) Click Position Is Valid
        //   2.) Particle Data Is Valid
        // Postconditions:
        //   1.) Continuous Forces Are Applied To Particles
        void applyRepulsiveForceContinual()
        {
            if (isClick)
            {
                const float forceStrength = 0.0000065f;
                const float minDistanceSquared = 0.1f;
                for (unsigned int i = 0; i < PARTICLE_COUNT; ++i)
                {
                    glm::vec2 direction = clickPosition - particles.centroids[i];
                    float distanceSquared = glm::dot(direction, direction);
                    if (distanceSquared > minDistanceSquared)
                    {
                        glm::vec2 force = direction / distanceSquared;
                        particles.velocities[i] += (force * forceStrength) / particles.radii[i];
                    }
                }
            }
            else
            {
                const float forceStrength = 0.00000095f;
                const float minDistanceSquared = 0.1f;
                for (unsigned int i = 0; i < PARTICLE_COUNT; ++i)
                {
                    glm::vec2 direction = particles.centroids[i] - clickPosition;
                    float distanceSquared = glm::dot(direction, direction);
                    if (distanceSquared > minDistanceSquared)
                    {
                        glm::vec2 force = direction / distanceSquared;
                        particles.velocities[i] += (force * forceStrength) / particles.radii[i];
                    }
                }
            }
        }


        // Thread Function For Particle Updates
        // Preconditions:
        //   1.) thread_id Is Valid Thread Identifier
        //   2.) Particle Data Is Properly Initialized
        // Postconditions:
        //   1.) Assigned Particles Are Updated
        //   2.) Thread Synchronization Is Maintained
        void updateParticlesThreaded(int thread_id)
        {
            unsigned int chunk_size = PARTICLE_COUNT / NUM_THREADS;
            unsigned int start = thread_id * chunk_size;
            unsigned int end = (thread_id == NUM_THREADS - 1) ? PARTICLE_COUNT : start + chunk_size;

            while (!terminateThreads)
            {
                for (unsigned int i = start; i < end; i++)
                {
                    particles.centroids[i] += particles.velocities[i];
                }

                updateBarrier.arrive_and_wait();
                updateBarrier.arrive_and_wait();

                collisionMatrix.detectAndResolveCollisions(particles, thread_id, NUM_THREADS);

                for (unsigned int i = start; i < end; i++)
                {
                    vertices[particles.IDs[i]].position = particles.centroids[i];
                    for (int j = 0; j < SEGMENT_CNT; ++j)
                    {
                        float angle = (2.0f * M_PI * j) / SEGMENT_CNT;
                        glm::vec2 pos = particles.centroids[i] + particles.radii[i] * glm::vec2(cos(angle), sin(angle));
                        vertices[particles.IDs[i] + j + 1].position = pos;
                    }
                }

                renderBarrier.arrive_and_wait();
            }
        }


        // Render Current Particle States
        // Preconditions:
        //   1.) OpenGL Context Is Current
        //   2.) Vertex Data Is Updated
        // Postconditions:
        //   1.) Particles Are Rendered To Screen
        //   2.) VBO Is Updated With New Positions
        void renderParticles()
        {
            glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(Vertex), vertices.data());
            glDrawElements(GL_TRIANGLES, indiceCount, GL_UNSIGNED_INT, 0);
        }


        // Main Simulation Loop
        // Preconditions:
        //   1.) All Systems Are Initialized
        //   2.) Threads Are Running
        // Postconditions:
        //   1.) Simulation Runs Until Window Closes
        //   2.) Frame Rate Is Maintained
        void run()
        {
            const float targetFrameDuration = 1.0f / 90.0f;

            while (!glfwWindowShouldClose(window))
            {
                auto frameStart = std::chrono::high_resolution_clock::now();

                glClear(GL_COLOR_BUFFER_BIT);

                updateBarrier.arrive_and_wait();

                for (unsigned int i = 0; i < PARTICLE_COUNT; i++)
                {
                    collisionMatrix.addParticle(i, particles.centroids[i]);
                }

                if (mouseButtonHeld)
                {
                    double xpos, ypos;
                    glfwGetCursorPos(window, &xpos, &ypos);
                    clickPosition = glm::vec2(2.0f * (xpos / WINDOW_WIDTH) - 1.0f,
                        1.0f - 2.0f * (ypos / WINDOW_HEIGHT));
                    applyRepulsiveForceContinual();
                }

                updateBarrier.arrive_and_wait();
                renderBarrier.arrive_and_wait();

                renderParticles();

                glfwSwapBuffers(window);
                glfwPollEvents();

                collisionMatrix.clear();

                auto frameEnd = std::chrono::high_resolution_clock::now();
                std::chrono::duration<float> frameTime = frameEnd - frameStart;

                float sleepDuration = targetFrameDuration - frameTime.count();
                if (sleepDuration > 0)
                {
                    std::this_thread::sleep_for(std::chrono::duration<float>(sleepDuration));
                }
            }

            glfwTerminate();
        }


    private:
        // Handle To The GLFW Window Context For Rendering And Input
        GLFWwindow* window;

        // Container For All Particle Properties Using Structure Of Arrays Pattern
        ParticleData particles;

        // Collection Of Vertex Data For OpenGL Rendering Of Particles
        std::vector<Vertex> vertices;

        // Total Number Of Indices Used For Triangle Rendering
        unsigned int indiceCount;

        // OpenGL Shader Program For Particle Rendering
        Shader shaderEngine;

        // Current Mouse Position In Normalized Device Coordinates
        glm::vec2 clickPosition;

        // Flag Indicating Type Of Mouse Click (Left/Right)
        bool isClick = false;

        // OpenGL Buffer Objects:
        // VAO: Vertex Array Object For Storing Vertex Attributes
        // VBO: Vertex Buffer Object For Vertex Data
        // EBO: Element Buffer Object For Index Data
        GLuint VAO, VBO, EBO;

        // Spatial Partitioning System For Efficient Collision Detection
        CollisionMatrix collisionMatrix;

        // Collection Of Worker Threads For Parallel Particle Updates
        std::vector<std::thread> threads;

        // Flag To Signal Thread Termination
        bool terminateThreads = false;

        // Synchronization Barriers For Thread Coordination:
        // updateBarrier: Synchronizes Particle Position Updates
        // renderBarrier: Synchronizes Rendering Operations
        ThreadSynchronizer updateBarrier;
        ThreadSynchronizer renderBarrier;

        // Flag Indicating If Mouse Button Is Currently Pressed
        bool mouseButtonHeld = false;

};



int main()
{
    try
    {
        ParticleWindow app(WINDOW_WIDTH, WINDOW_HEIGHT);
        app.run();
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        return -1;
    }
    return 0;
}
