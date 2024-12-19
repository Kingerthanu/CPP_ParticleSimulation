# CPP_ParticleSimulation
This Project Works With OpenGL To Create A Visual Real-Time Particle Simulation With Their Own Collision Detection To Help See How We Can Optimize Means Of Collision Detection Going From A Brute-Force Runtime Complexity For The Collision Algorithm Of O(n!) To A O(n) Complexity By Utilizing Radial Sweeps/Checks On Neighboring Particles Which Are Apportioned Into HashMap Cells Using A Custom Hash Mapping Algorithm To Allow For O(1) Insertions/Removals.

While Currently Not At Its Final Stable Build, It Is Still Very Functional And Has Been Showing Smooth 60fps Even With 100s Of Particles Concurrently Being Simulated And Collided. Currently Working On This Project As Means Of Learning About Realistic Implementations For Algorithmic Complexity Analysis And How Values Of n Can Heavily Bias Runtime Efficiency. While I've Been Comfortable And Knowledgable About Varying Means Of Solving Problems Optimally, This Project And My Courses Have Helped Me Understand How To Think More Theoretically Instead Of More Pragmatically About Problems; Allowing Me To Reason Through Means Of Using HashMaps And Other Standardized Data Structures For Optimal And Practical Applications Mainly In The Sector Of Simulations. 

Interesting Lessons Have Been Learned In This Project With Multithreading Contention And Means Of Cleanly Partitioning Regions To Allow For Non-Conflicting--And In Turn Non-Synchronizing Regions--To Be Employed For Intuitive And Scalable Solutions.

While The Collision Detection Currently Is Quite Primitive (Mostly With The Screen Edges As Theres Cases Where Particles Clip Out Of The Visual Region) It Still Has Worked Great And Will Be Iterated On In Future Patches For More Dynamic And Stable Detection Algorithms. If Two Particles Currently Collide, They Don't Have Rigid Bodies And Have A Binding Impulse Force Which Will Try To Stick Them Together If They Collide As Would Particles With Chemical Bonding (Albeit Very Primitive Currently). Also The Codebase Mainly Resides In The Driver.cpp Which Is Kind Of Party-Foul But Will Be Cleaned Up In Future Patches.

You Can Also Click To Create A Force-Pulse Which Is Basically A Circular Region From Your Cursor Which Will Push Away All Particles In The Circles Radius If You Left Click, And Pull In All Particles In The Circles Radius If You Right Click.

----------------------------------------------

<img src="https://github.com/user-attachments/assets/4d236ec7-91a8-4128-851b-db5c476a7086" alt="Cornstarch <3" width="55" height="49"> <img src="https://github.com/user-attachments/assets/4d236ec7-91a8-4128-851b-db5c476a7086" alt="Cornstarch <3" width="55" height="49"> <img src="https://github.com/user-attachments/assets/4d236ec7-91a8-4128-851b-db5c476a7086" alt="Cornstarch <3" width="55" height="49"> <img src="https://github.com/user-attachments/assets/4d236ec7-91a8-4128-851b-db5c476a7086" alt="Cornstarch <3" width="55" height="49"> 

<h2>The Breakdown:</h2>

The Program Starts By Initializing A Window Based Upon The Custom `ParticleWindow` Class. This Class Works As Means Of Encapsulating All Data Pertaining To The Simulation Including A GLFW Window, The Shader For This Provided Window, As Well As Data Pertaining To Rendering And The Current Particles Being Simulated.

<h4>Initialization</h4>

When The Process Initializes The Class, It Starts By Contextualizing The Window For OpenGL Buffer Contexting As Well As Generating Our Particles Through `initParticles(...)`.

```C++
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
```
    
<h4>Particle Generation</h4>

In `initParticles(...)` We Utilize Random Generation To Get Varying Particles From Their Color, Initial Position, Initial Velocity, As Well As Their Radial Size. For This Current Project, Instead Of Utilizing An Array-Of-Structs (AoS [Array Of Particle Instances]) We Utilize A Struct-Of-Arrays (SoA [Instance Of Particle Traits Arrays]) To Decrease Cache Thrashing And Increase Locality Of Shared Data. This Ensures Data Like Positions Are All Accessed And Checked Together In The Cache, Which Is Better Than Having Multiple Particles And All Their Data Being In The Cache One-At-A-Time. This Is A Trade-Off As While More Efficient, It Is A Little Less Readable And Intuitive As Individual Particles Are Represented Through Indexes Instead Of Class Instances. After Each Particle Has Its Backend Members Initialized (Centroid, Radii, Color, Velocity, ID), We Start Building The Circles' Visual Elements Through A Circle Algorithm Which Generates All Vertices For The VBO (Vertex Array Buffer) And Indices For The EBO (Element Array Buffer). These Two Entries Allow OpenGL To Properly Render Our Particles.

```C++
void ParticleWindow::initParticles(std::vector<GLuint>& indices)
{
    std::random_device rd;
    std::mt19937 seed(rd());
    std::uniform_real_distribution<float> radiusDist(0.005f, 0.025f);
    std::uniform_real_distribution<float> positionDist(-0.799f, 0.799f);
    std::uniform_real_distribution<float> colorDist(0.1f, 1.0f);
    std::uniform_real_distribution<float> velocityDist(-0.00015f, 0.00015f);

    for (unsigned int i = 0; i < PARTICLE_COUNT; i++)
    {
        particles.centroids[i] = glm::vec2(positionDist(seed), positionDist(seed));
        particles.radii[i] = radiusDist(seed);
        particles.colors[i] = glm::vec3(colorDist(seed), colorDist(seed), colorDist(seed));
        particles.velocities[i] = glm::vec2(velocityDist(seed), velocityDist(seed)) / particles.radii[i];
        particles.IDs[i] = vertices.size();

        // Generate initial vertices
        vertices.push_back({ particles.centroids[i], particles.colors[i] });
        for (int j = 0; j < SEGMENT_CNT; ++j)
        {
            float angle = (2.0f * M_PI * j) / SEGMENT_CNT;
            glm::vec2 pos = particles.centroids[i] + particles.radii[i] * glm::vec2(cos(angle), sin(angle));
            vertices.push_back({ pos, particles.colors[i] });
        }
    }
}
```
<h4>Threading</h4>

After Generation Is Done In `initParticles(...)` We Then Create Threads Which Allows Us To Partition The Workload For Collision Detection To Multiple "Workers" To Allow Us To Gain Runtime Benefits As Instead Of One Thread Sequentially Updating All Particles Itself It Divides The Workload To Multiple Threads (Two Threads Currently As From Testing It Seemed Two Allowed Runtime Benefits Without Diminishing Returns From Context Swapping). These Threads Are Synchronized With The Main Window Thread Through The `updateBarrier` & `renderBarrier` Barriers To Ensure Workers Render For Each Frame And Don't Run Ahead Frames Which Could Cause Artifacts In Our Simulation.

```C++
for (int i = 0; i < NUM_THREADS; ++i)
{
    threads.emplace_back(&ParticleWindow::updateParticlesThreaded, this, i);
}
```
Each Thread Does The Motion & Collision Detection For Particles It Is Delegated. This Delegation Is Done By Providing Each Thread With A Start Position In The SoA Starting At Index 0 And Going Up Based On Its Provided `thread_id` (Simply What Thread Was Initialized First, Second, Etc. [i.e., first thread initialized is 0, second is 1]) Then It Increments By The `NUM_THREADS` We Have To Allow It To Jump Across To The Next Thread It Is Delegated, Ensuring Multiple Threads Don't Access The Same Particle (Which Could Lead To Double-Updates Which Could Lead To Crashes Or Unintended Behavior). Each Particle Will Reflect Off Window Borders By Inverting Its Velocity, And If Colliding With A Particle Will Cause The Binding Impulsive Force On These Two Particles; This Force Is Light To Allow Bonds To Be Broken And To Ensure Particles Don't Snap Together And Is More A Gradual Attraction Towards Each Other.

```C++
void ParticleWindow::updateParticlesThreaded(int thread_id)
{
    while (!terminateThreads)
    {
      updateBarrier.arrive_and_wait();

      // Process Particles Assigned To This Thread
      for (size_t i = thread_id; i < PARTICLE_COUNT; i += NUM_THREADS)
      {
          particles.centroids[i].x += particles.velocities[i].x;
          particles.centroids[i].y += particles.velocities[i].y;

          // Reflect Velocity At Boundaries
          if (particles.centroids[i].x - particles.radii[i] < -1.0f || particles.centroids[i].x + particles.radii[i] > 1.0f)
          {
              particles.velocities[i].x = -particles.velocities[i].x;
          }

          if (particles.centroids[i].y - particles.radii[i] < -1.0f || particles.centroids[i].y + particles.radii[i] > 1.0f)
          {
              particles.velocities[i].y = -particles.velocities[i].y;
          }

          // Update VBO
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
```

<h4>Main Loop</h4>

We Then Start On The `ParticleWindow`'s `run(...)` Function In The Main Thread Which Is The Main Rendering Loop. This Is A Simple Rendering Loop With A Target Frame Rate To Reach And Simple Buffer Clearing And Swapping. Other Than This, We Have The `addParticle(...)` Function Which For Each Frame And Each Particle Will Emplace Them In The `collisionMatrix` (Our HashMap). We Will Then Arrive At Our `updateBarrier` Which Will Tell Our Worker Threads To Do Their Collision And Motion Logic As We Wait At The `renderBarrier` Which Will Push Us Forward After Each Worker Has Finalized Their Collision And Motion Logic For Us To Then Render The Results In `renderParticles(...)`. We Then Clear Our HashMap For The Next Frame As We Will Have To Again Load Our Particles Into The HashMap Using Their New Positions After This Frame.

```C++
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
```

<h4>Mouse Interaction</h4>

This Is The Main Loop Of Our Simulation; We Also Include A `mouseButtonCallbackWrapper(...)` Which Will Call Our `mouseButtonCallback(...)` Which Will Check What Type Of Click We Have Done On The Screen And Apply The Provided Repulsive Force To All Particles In That Radius In `applyRepulsiveForce(...)`.

```C++
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
```

<h4>Configurations</h4>

At The Start Of The Code There Is A Section With A Lot Of Definitions For Precompilation, Some Of These Relate To The Amount Of Particles To Generate And Their Traits (Like Segment Count; `SEGMENT_CNT`, `PARTICLE_COUNT`), While Some Relate To The Means Of Hashing Our Given Screenspace (`BIN_SIZE`, `GRID_SIZE_X`, `GRID_SIZE_Y`), Others Also Being `WINDOW_HEIGHT` & `WINDOW_WIDTH` For The GLSL Window, And `M_PI` Being A Value For Pi For Trigonometry Computations And `NUM_THREADS` Being For Amount Of Workers To Use For Simulation Logic.

```C++
#define WINDOW_HEIGHT 1300
#define WINDOW_WIDTH 1300
#define SEGMENT_CNT 7
#define PARTICLE_COUNT 500
#define M_PI 3.14159265359
#define BIN_SIZE 0.025f
#define NUM_THREADS 2
#define GRID_SIZE_X static_cast<int>(2.0f / BIN_SIZE)
#define GRID_SIZE_Y static_cast<int>(2.0f / BIN_SIZE)
```


<img src="https://github.com/user-attachments/assets/0c481edf-693f-4b4b-bca2-f6017a3e15d4" alt="Cornstarch <3" width="55" height="49"> <img src="https://github.com/user-attachments/assets/0c481edf-693f-4b4b-bca2-f6017a3e15d4" alt="Cornstarch <3" width="55" height="49"> <img src="https://github.com/user-attachments/assets/0c481edf-693f-4b4b-bca2-f6017a3e15d4" alt="Cornstarch <3" width="55" height="49"> <img src="https://github.com/user-attachments/assets/0c481edf-693f-4b4b-bca2-f6017a3e15d4" alt="Cornstarch <3" width="55" height="49"> 

----------------------------------------------

<img src="https://github.com/user-attachments/assets/69234949-e52b-4056-9b5b-382fa1f28745" alt="Cornstarch <3" width="55" height="49"> <img src="https://github.com/user-attachments/assets/69234949-e52b-4056-9b5b-382fa1f28745" alt="Cornstarch <3" width="55" height="49"> <img src="https://github.com/user-attachments/assets/69234949-e52b-4056-9b5b-382fa1f28745" alt="Cornstarch <3" width="55" height="49"> <img src="https://github.com/user-attachments/assets/69234949-e52b-4056-9b5b-382fa1f28745" alt="Cornstarch <3" width="55" height="49"> 

<h3>Features:</h3>

![Untitledvideo2-ezgif com-optimize (1)](https://github.com/user-attachments/assets/6786c8a5-fc40-4971-a572-abb5c573d51d)

![Untitledvideo-ezgif com-optimize (1)](https://github.com/user-attachments/assets/c1f3bd6d-4ade-4ce7-9b35-26cb4ad8fd83)

![Untitledvideo3-ezgif com-optimize](https://github.com/user-attachments/assets/6a9c0e27-df04-44cf-baa6-012b9d089b2b)


<img src="https://github.com/user-attachments/assets/0363619c-11fb-472f-b338-39b361304dd5" alt="Cornstarch <3" width="55" height="69"> <img src="https://github.com/user-attachments/assets/0363619c-11fb-472f-b338-39b361304dd5" alt="Cornstarch <3" width="55" height="69"> <img src="https://github.com/user-attachments/assets/0363619c-11fb-472f-b338-39b361304dd5" alt="Cornstarch <3" width="55" height="69"> <img src="https://github.com/user-attachments/assets/0363619c-11fb-472f-b338-39b361304dd5" alt="Cornstarch <3" width="55" height="69">

