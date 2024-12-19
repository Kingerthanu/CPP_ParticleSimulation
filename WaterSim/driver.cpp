#include "shader.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <random>
#include <iostream>
#include <chrono>
#include <thread>
#include <mutex>


#define WINDOW_HEIGHT 1300
#define WINDOW_WIDTH 1300
#define SEGMENT_CNT 9
#define PARTICLE_COUNT 500
#define M_PI 3.14159265359
#define BIN_SIZE 0.025f
#define NUM_THREADS 2
#define GRID_SIZE_X static_cast<int>(2.0f / BIN_SIZE)
#define GRID_SIZE_Y static_cast<int>(2.0f / BIN_SIZE)


#define RADIUS_MIN 0.0045f
#define RADIUS_MAX 0.025f
#define POSITION_MIN -0.799f
#define POSITION_MAX 0.799f
#define VELOCITY_MIN - 0.00015f
#define VELOCITY_MAX 0.00015f

#define COLOR_R_MIN 0.1f
#define COLOR_R_MAX 0.6f

#define COLOR_G_MIN 0.1f
#define COLOR_G_MAX 0.5f

#define COLOR_B_MIN 0.1f
#define COLOR_B_MAX 1.0f



// Struct To Represent Hash Coordinates In The Spatial Grid
struct HashCoord
{
    int x, y;
    bool operator==(const HashCoord& other) const
    {
        return x == other.x && y == other.y;
    }
};


// Struct To Represent A Vertex In OpenGL Rendering
struct Vertex
{
    glm::vec2 position;
    glm::vec3 color;
};



// Function To Clamp The Value Within The Specified Range
// Preconditions:
//   1.) val Is The Input Value To Be Clamped
//   2.) min_val And max_val Define The Minimum And Maximum Bounds
// Postconditions:
//   1.) Returns The Clamped Value
inline int clamp(int val, int min_val, int max_val)
{
    return std::min(std::max(val, min_val), max_val);
}



// Class To Synchronize Threads At Specific Points In Execution
class ThreadSynchronizer
{

    public:
        // Preconditions:
        //   1.) thread_count Specifies The Number Of Threads To Synchronize
        // Postconditions:
        //   1.) ThreadSynchronizer Object Is Created With Initialized Variables
        ThreadSynchronizer(int thread_count) : total_threads(thread_count), count(0) {}

        // Function To Ensure Threads Wait For Each Other To Reach This Point
        // Preconditions:
        //   1.) All Threads Are Running In A Loop And Arrive Here To Synchronize
        // Postconditions:
        //   1.) All Threads Will Wait Here Until The Last Thread Arrives, Then Continue
        void arrive_and_wait()
        {
            std::unique_lock<std::mutex> lock(mtx);
            if (++count == total_threads)
            {
                count = 0;             // Reset The Count For The Next Phase
                cv.notify_all();       // All Threads Are Done, Wake Up All Waiting Threads
            }
            else
            {
                cv.wait(lock);
            }
        }

    private:
        const int total_threads;
        int count;
        std::condition_variable cv;
        std::mutex mtx;

};


// Structure To Store Particle Data In A Struct Of Arrays Style
class ParticleData
{

    public:
        std::vector<glm::vec2> velocities;
        std::vector<glm::vec2> centroids;
        std::vector<float> radii;
        std::vector<glm::vec3> colors;
        std::vector<unsigned short int> IDs;

        // Preconditions:
        //   1.) count Specifies The Number Of Particles
        // Postconditions:
        //   1.) ParticleData Object Is Created With Empty Vectors Of Given Size
        ParticleData(size_t count)
        {
            velocities.resize(count);
            centroids.resize(count);
            radii.resize(count);
            colors.resize(count);
            IDs.resize(count);
        }

};


// Class To Store Particle Data Within A Spatial Grid To Optimize Collision Detection
class CollisionMatrix
{

    private:
        float binSize;
        std::vector<std::vector<size_t>> fixedGrid;

    public:
        // Preconditions:
        //   1.) bin_size Defines The Size Of Each Grid Cell
        // Postconditions:
        //   1.) CollisionMatrix Is Created With A Fixed Grid Ready For Particle Assignment
        CollisionMatrix(float bin_size) : binSize(bin_size)
        {
            fixedGrid.resize(GRID_SIZE_X * GRID_SIZE_Y);
        }

        // Function To Clear All Particles From The Grid
        // Preconditions:
        //   1.) Called After Particle Updates To Reset The Grid
        // Postconditions:
        //   1.) All Particles Are Removed From The Grid
        void clear()
        {
            for (size_t i = 0; i < fixedGrid.size(); ++i)
            {
                fixedGrid[i].clear();
            }
        }

        // Function To Get The Hash Coordinate Based On A Particle's Position
        // Preconditions:
        //   1.) position Specifies The Particle's Coordinates In The Simulation
        // Postconditions:
        //   1.) Returns The Hash Coordinate Of The Specified Position
        HashCoord getHashCoord(const glm::vec2& position)
        {
            int x = static_cast<int>(std::floor((position.x + 1.0f) / BIN_SIZE));
            int y = static_cast<int>(std::floor((position.y + 1.0f) / BIN_SIZE));
            x = clamp(x, 0, GRID_SIZE_X - 1);
            y = clamp(y, 0, GRID_SIZE_Y - 1);
            return { x, y };
        }

        // Function To Add A Particle To The Grid
        // Preconditions:
        //   1.) particleIndex Specifies The Index Of The Particle To Be Added
        //   2.) centroid Specifies The Particle's Current Position
        // Postconditions:
        //   1.) Particle Is Added To Its Corresponding Grid Cell
        void addParticle(size_t particleIndex, const glm::vec2& centroid)
        {
            HashCoord key = getHashCoord(centroid);
            int index = key.y * GRID_SIZE_X + key.x;
            fixedGrid[index].push_back(particleIndex);
        }

        // Function To Get Potential Colliders Within Neighboring Grid Cells
        // Preconditions:
        //   1.) centroid Specifies The Particle's Position
        // Postconditions:
        //   1.) Returns A List Of Indices For Particles That Are Potential Colliders
        std::vector<size_t> getPotentialColliders(const glm::vec2& centroid)
        {
            std::vector<size_t> potentialColliders;
            HashCoord key = getHashCoord(centroid);
            for (int dx = -1; dx <= 1; ++dx)
            {
                for (int dy = -1; dy <= 1; ++dy)
                {
                    int nx = clamp(key.x + dx, 0, GRID_SIZE_X - 1);
                    int ny = clamp(key.y + dy, 0, GRID_SIZE_Y - 1);
                    int index = ny * GRID_SIZE_X + nx;
                    potentialColliders.insert(potentialColliders.end(), fixedGrid[index].begin(), fixedGrid[index].end());
                }
            }
            return potentialColliders;
        }
};


// Main Class For Particle Simulation And Rendering
class ParticleWindow
{
    public:
        ParticleWindow(int width, int height);
        ~ParticleWindow();
        void run();

    private:
        GLFWwindow* window;
        ParticleData particles;
        std::vector<Vertex> vertices;
        unsigned int indiceCount;
        Shader shaderEngine;
        glm::vec2 clickPosition;
        bool isClick = false;

        GLuint VAO, VBO, EBO;
        CollisionMatrix collisionMatrix;
        std::vector<std::thread> threads;
        bool terminateThreads = false;
        ThreadSynchronizer updateBarrier;
        ThreadSynchronizer renderBarrier;

        void initParticles(std::vector<GLuint>& indices);
        void mouseButtonCallback(int button, int action);
        static void mouseButtonCallbackWrapper(GLFWwindow* window, int button, int action, int mods);
        void applyRepulsiveForce();
        void updateParticles();
        void renderParticles();
        void updateParticlesThreaded(int thread_id);
        void resolveRadialCollision(size_t i, const std::vector<size_t>& potentialColliders);
};



// Constructor To Set Up The Particle Simulation And Render Context
// Preconditions:
//   1.) width And height Define The Dimensions Of The Simulation Window
// Postconditions:
//   1.) Creates A Window, Sets Up OpenGL, Initializes Particles And Rendering Buffers
ParticleWindow::ParticleWindow(int width, int height)
    : particles(PARTICLE_COUNT), collisionMatrix(BIN_SIZE), updateBarrier(NUM_THREADS + 1), renderBarrier(NUM_THREADS + 1)
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


// Destructor To Clean Up Resources
// Preconditions:
//   1.) ParticleWindow Object Must Be Active
// Postconditions:
//   1.) Joins All Threads, Deletes Shader, Releases GLFW Resources
ParticleWindow::~ParticleWindow()
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


// Function To Initialize Particles And Generate Rendering Buffers
// Preconditions:
//   1.) indices Is An Empty Vector To Store Triangle Indices For Rendering
// Postconditions:
//   1.) Initializes All Particle Properties And Adds Their Vertices To Rendering Buffers
void ParticleWindow::initParticles(std::vector<GLuint>& indices)
{
    std::random_device rd;
    std::mt19937 seed(rd());
    std::uniform_real_distribution<float> radiusDist(RADIUS_MIN, RADIUS_MAX);
    std::uniform_real_distribution<float> positionDist(POSITION_MIN, POSITION_MAX);
    std::uniform_real_distribution<float> colorDistR(COLOR_R_MIN, COLOR_R_MAX);
    std::uniform_real_distribution<float> colorDistG(COLOR_G_MIN, COLOR_G_MAX);
    std::uniform_real_distribution<float> colorDistB(COLOR_B_MIN, COLOR_B_MAX);
    std::uniform_real_distribution<float> velocityDist(VELOCITY_MIN, VELOCITY_MAX);

    for (unsigned int i = 0, centerOffset; i < PARTICLE_COUNT; i++)
    {
        particles.centroids[i] = glm::vec2(positionDist(seed), positionDist(seed));
        particles.radii[i] = radiusDist(seed);
        particles.colors[i] = glm::vec3(colorDistR(seed), colorDistG(seed), colorDistB(seed));
        particles.velocities[i] = glm::vec2(velocityDist(seed), velocityDist(seed)) / particles.radii[i];
        particles.IDs[i] = vertices.size();

        // Generate Initial Vertices
        vertices.push_back({ particles.centroids[i], particles.colors[i] });
        for (int j = 0; j < SEGMENT_CNT; ++j)
        {
            float angle = (2.0f * M_PI * j) / SEGMENT_CNT;
            glm::vec2 pos = particles.centroids[i] + particles.radii[i] * glm::vec2(cos(angle), sin(angle));
            vertices.push_back({ pos, particles.colors[i] });
        }

        // Generate Indices For GL_TRIANGLES
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


// Wrapper Function For Mouse Click Event Callback
// Preconditions:
//   1.) window Must Be A Valid GLFWwindow Pointer
//   2.) button, action, And mods Specify The Click Event Details
// Postconditions:
//   1.) Calls The Instance's mouseButtonCallback Function
void ParticleWindow::mouseButtonCallbackWrapper(GLFWwindow* window, int button, int action, int mods)
{
    ParticleWindow* instance = static_cast<ParticleWindow*>(glfwGetWindowUserPointer(window));
    instance->mouseButtonCallback(button, action);
}


// Function To Handle Mouse Click Events For Applying Forces
// Preconditions:
//   1.) button And action Define The Mouse Click Event
// Postconditions:
//   1.) Sets Up Click Position And Applies The Appropriate Force To Particles
void ParticleWindow::mouseButtonCallback(int button, int action)
{
    if (action == GLFW_PRESS)
    {
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        clickPosition = glm::vec2(2.0f * (xpos / WINDOW_WIDTH) - 1.0f, 1.0f - 2.0f * (ypos / WINDOW_HEIGHT));
        isClick = button;
        applyRepulsiveForce();
    }
}


// Function To Apply Repulsive Or Attractive Force Based On Mouse Click Type
// Preconditions:
//   1.) isClick Or isRightClick Must Be Set To True Based On Mouse Click
// Postconditions:
//   1.) Updates Particle Velocities To Repel (Left Click) Or Attract (Right Click) Based On Mouse Position
void ParticleWindow::applyRepulsiveForce()
{
    if (isClick)
    {
        const float forceStrength = 0.000095f;
        const float minDistanceSquared = 0.000000001f;
        for (size_t i = 0; i < PARTICLE_COUNT; ++i)
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
        const float forceStrength = 0.000095f;
        const float minDistanceSquared = 0.00001f;
        for (size_t i = 0; i < PARTICLE_COUNT; ++i) {
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


// Function To Resolve Collisions Between A Particle And Its Potential Colliders
// Preconditions:
//   1.) i Is A Valid Particle Index
//   2.) potentialColliders Contains Valid Particle Indices
//   3.) Particle Data Arrays (centroids, velocities, radii) Are Properly Initialized
//   4.) Collision Matrix Has Been Updated For Current Frame
// Postconditions:
//   1.) Velocities Are Updated For Colliding Particles
//   2.) Binding Impulse Forces Are Applied Between Colliding Particles
//   3.) Particle Momentum Is Conserved In Collisions
void ParticleWindow::resolveRadialCollision(size_t i, const std::vector<size_t>& potentialColliders) 
{
    for (auto otherIndex : potentialColliders) {
        if (i != otherIndex) {
            glm::vec2 diff = particles.centroids[otherIndex] - particles.centroids[i];
            float distSquared = glm::dot(diff, diff);
            float minDist = particles.radii[i] + particles.radii[otherIndex];

            if (distSquared <= minDist * minDist) {
                glm::vec2 normal = diff / sqrtf(distSquared);
                glm::vec2 relativeVelocity = particles.velocities[i] - particles.velocities[otherIndex];
                float velocityAlongNormal = glm::dot(relativeVelocity, normal);

                if (velocityAlongNormal > 0) continue;

                float impulseScalar = -(0.021f * velocityAlongNormal);
                glm::vec2 impulse = impulseScalar * normal;
                particles.velocities[i] += impulse;
                particles.velocities[otherIndex] -= impulse;
            }
        }
    }
}


// Function For Threaded Particle Updates
// Preconditions:
//   1.) thread_id Specifies Which Thread This Function Belongs To
// Postconditions:
//   1.) Updates The Positions Of Particles Assigned To This Thread
void ParticleWindow::updateParticlesThreaded(int thread_id)
{
    while (!terminateThreads)
    {
        updateBarrier.arrive_and_wait();

        for (size_t i = thread_id; i < PARTICLE_COUNT; i += NUM_THREADS)
        {
            particles.centroids[i].x += particles.velocities[i].x;
            particles.centroids[i].y += particles.velocities[i].y;

            if (particles.centroids[i].x - particles.radii[i] < -1.0f || particles.centroids[i].x + particles.radii[i] > 1.0f)
            {
                particles.velocities[i].x = -particles.velocities[i].x;
            }
            if (particles.centroids[i].y - particles.radii[i] < -1.0f || particles.centroids[i].y + particles.radii[i] > 1.0f)
            {
                particles.velocities[i].y = -particles.velocities[i].y;
            }

            auto colliders = collisionMatrix.getPotentialColliders(particles.centroids[i]);
            resolveRadialCollision(i, colliders);

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


// Function To Render Particles In The Simulation
// Preconditions:
//   1.) VBO And EBO Must Be Bound
// Postconditions:
//   1.) Renders The Updated Particles To The Window
void ParticleWindow::renderParticles()
{
    glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(Vertex), vertices.data());
    glDrawElements(GL_TRIANGLES, indiceCount, GL_UNSIGNED_INT, 0);
}


// Main Function To Run The Simulation
// Preconditions:
//   1.) window Must Be A Valid GLFWwindow Pointer
// Postconditions:
//   1.) Runs The Simulation Loop, Handling Updates And Rendering Until Window Close
void ParticleWindow::run()
{
    const float targetFrameDuration = 1.0f / 60.0f;

    while (!glfwWindowShouldClose(window))
    {
        auto frameStart = std::chrono::high_resolution_clock::now();

        glClear(GL_COLOR_BUFFER_BIT);

        for (size_t i = 0; i < PARTICLE_COUNT; i++)
        {
            collisionMatrix.addParticle(i, particles.centroids[i]);
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
